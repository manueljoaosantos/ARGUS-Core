/*
========================================================
  07-maze-hibrido-ajustado-aos-teus-sensores.ino
  Híbrido Encoder + QTR
  Ajustado aos valores reais do teu QTR
========================================================
*/

#include <Arduino.h>

// ======================================================
// ===================== MOTORES =========================
// ======================================================

#define AIN1 4
#define AIN2 5
#define PWMA 12

#define BIN1 13
#define BIN2 14
#define PWMB 15

#define PWM_FREQ 20000
#define PWM_RES 8
#define PWM_LIMIT 55   // limite físico (anti-salto)

// ======================================================
// ===================== ENCODERS ========================
// ======================================================

#define ENC_DIR_A 16
#define ENC_DIR_B 17
#define ENC_ESQ_A 21
#define ENC_ESQ_B 42

volatile long contadorDir = 0;
volatile long contadorEsq = 0;

// ======================================================
// ===================== QTR =============================
// ======================================================

const int qtrPins[8] = {1,2,3,6,7,8,9,10};
int sensorValues[8];

#define CENTER_POSITION 3500

float Kp_line = 0.5;
float Kd_line = 4.0;
float lastLineError = 0;

// ======================================================
// ===================== GEOMETRIA =======================
// ======================================================

#define PULSOS_CELULA 507

// ======================================================
// ===================== PID VELOCIDADE ==================
// ======================================================

float Kp_vel = 0.5;
float Ki_vel = 0.003;
float Kd_vel = 0.04;

float integralDir = 0, integralEsq = 0;
float lastErroDir = 0, lastErroEsq = 0;

long lastDir = 0, lastEsq = 0;

// ======================================================
// ===================== CONTROLO ========================
// ======================================================

const int SPEED_MAX = 28;
const int SPEED_MIN = 20;

const int brakeZone = 140;
const int accelStep = 1;

int baseSpeedTarget = 0;
int baseSpeedCurrent = 0;
int correction = 0;

unsigned long tempoArranque = 0;
bool arranqueSuave = true;

// ======================================================
// ===================== ESTADOS =========================
// ======================================================

enum EstadoMaze {
  AVANCAR,
  TRAVAR
};

EstadoMaze estadoAtual = AVANCAR;
long alvoPulsos = PULSOS_CELULA;

// ======================================================
// ===================== ENCODER ISR =====================
// ======================================================

void IRAM_ATTR encoderDir_ISR() {
  if (digitalRead(ENC_DIR_B)) contadorDir++;
  else contadorDir--;
}

void IRAM_ATTR encoderEsq_ISR() {
  if (digitalRead(ENC_ESQ_B)) contadorEsq--;
  else contadorEsq++;
}

// ======================================================
// ===================== UTIL ============================
// ======================================================

void resetEncoders() {
  contadorDir = 0;
  contadorEsq = 0;
  lastDir = 0;
  lastEsq = 0;
  integralDir = 0;
  integralEsq = 0;
}

void definirFrente() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
}

// ======================================================
// ===================== LEITURA QTR (AJUSTADA) =========
// ======================================================

float readLinePosition() {

  long weightedSum = 0;
  long total = 0;

  for(int i=0;i<8;i++){

    int raw = analogRead(qtrPins[i]);

    // Branco ~600
    // Preto  ~3200-4095
    int value = map(raw, 600, 4095, 0, 1000);
    value = constrain(value, 0, 1000);

    weightedSum += (long)value * (i * 1000);
    total += value;
  }

  if(total < 100)
    return CENTER_POSITION;

  return (float)weightedSum / total;
}

// ======================================================
// ===================== PID VELOCIDADE ==================
// ======================================================

int calcularPID(float target, float vel,
                float &integral, float &lastErro) {

  float erro = target - vel;

  integral += erro;
  integral = constrain(integral, -120, 120);

  float deriv = erro - lastErro;
  lastErro = erro;

  return Kp_vel*erro + Ki_vel*integral + Kd_vel*deriv;
}

// ======================================================
// ===================== VELOCIDADE REAL =================
// ======================================================

void calcularVelocidade(float &velDir, float &velEsq) {

  long deltaDir = contadorDir - lastDir;
  long deltaEsq = contadorEsq - lastEsq;

  lastDir = contadorDir;
  lastEsq = contadorEsq;

  velDir = deltaDir * 100.0;
  velEsq = deltaEsq * 100.0;
}

// ======================================================
// ===================== OUTPUT MOTORES ==================
// ======================================================

void updateRamp() {

  if (baseSpeedCurrent < baseSpeedTarget)
    baseSpeedCurrent += accelStep;
  else if (baseSpeedCurrent > baseSpeedTarget)
    baseSpeedCurrent -= accelStep;
}

void applyMotorOutput() {

  float velDir, velEsq;
  calcularVelocidade(velDir, velEsq);

  int ajusteDir = calcularPID(baseSpeedCurrent - correction,
                              velDir,
                              integralDir,
                              lastErroDir);

  int ajusteEsq = calcularPID(baseSpeedCurrent + correction,
                              velEsq,
                              integralEsq,
                              lastErroEsq);

  int pwmDir = constrain(baseSpeedCurrent + ajusteDir, 0, PWM_LIMIT);
  int pwmEsq = constrain(baseSpeedCurrent + ajusteEsq, 0, PWM_LIMIT);

  ledcWrite(PWMA, pwmDir);
  ledcWrite(PWMB, pwmEsq);
}

// ======================================================
// ===================== CONTROL LOOP ====================
// ======================================================

void controlUpdate() {

  definirFrente();

  float pos = readLinePosition();
  float lineError = pos - CENTER_POSITION;

  float deriv = lineError - lastLineError;
  lastLineError = lineError;

  correction = Kp_line * lineError + Kd_line * deriv;
  correction = constrain(correction, -25, 25);

  long progresso = abs(contadorDir);
  long restante = alvoPulsos - progresso;

  if (arranqueSuave) {

    baseSpeedTarget = 15;

    if (millis() - tempoArranque > 1000)
      arranqueSuave = false;
  }
  else {

    if (restante < brakeZone) {
      float fator = (float)restante / brakeZone;
      baseSpeedTarget = SPEED_MIN +
                        (SPEED_MAX - SPEED_MIN) * fator;
    } else {
      baseSpeedTarget = SPEED_MAX;
    }
  }

  if (progresso >= alvoPulsos)
    estadoAtual = TRAVAR;

  if (estadoAtual == TRAVAR) {

    baseSpeedTarget = 0;

    if (abs(baseSpeedCurrent) < 2) {
      resetEncoders();
      estadoAtual = AVANCAR;
      arranqueSuave = true;
      tempoArranque = millis();
    }
  }

  updateRamp();
  applyMotorOutput();
}

// ======================================================
// ===================== SETUP ===========================
// ======================================================

unsigned long lastUpdate = 0;
const unsigned long CONTROL_INTERVAL = 10;

void setup() {

  Serial.begin(115200);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);

  analogReadResolution(12);

  for(int i=0;i<8;i++)
    pinMode(qtrPins[i], INPUT);

  pinMode(ENC_DIR_A, INPUT_PULLUP);
  pinMode(ENC_DIR_B, INPUT_PULLUP);
  pinMode(ENC_ESQ_A, INPUT_PULLUP);
  pinMode(ENC_ESQ_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_DIR_A), encoderDir_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_ESQ_A), encoderEsq_ISR, RISING);

  resetEncoders();
  tempoArranque = millis();
}

// ======================================================
// ===================== LOOP ============================
// ======================================================

void loop() {

  if (millis() - lastUpdate >= CONTROL_INTERVAL) {
    lastUpdate = millis();
    controlUpdate();
  }
}