/*
========================================================
  07-maze-direita-celula-estavel.ino
  Maze básico – sempre direita
  Estrutura por célula (robusto para entre-eixos largo)
========================================================
*/

#include <Arduino.h>

// ===================== MOTORES =========================

#define AIN1 4
#define AIN2 5
#define PWMA 12
#define BIN1 13
#define BIN2 14
#define PWMB 15

#define PWM_FREQ 20000
#define PWM_RES 8
#define PWM_LIMIT 70

// ===================== ENCODERS ========================

#define ENC_DIR_A 16
#define ENC_DIR_B 17
#define ENC_ESQ_A 21
#define ENC_ESQ_B 42

volatile long contadorDir = 0;
volatile long contadorEsq = 0;

// ===================== QTR =============================

const int qtrPins[8] = {1,2,3,6,7,8,9,10};
#define CENTER_POSITION 3500

float Kp_line = 0.8;
float Kd_line = 6.0;
float lastLineError = 0;

// ===================== VELOCIDADE ======================

float Kp_vel = 0.4;
float Ki_vel = 0.002;
float Kd_vel = 0.03;

float integralDir = 0, integralEsq = 0;
float lastErroDir = 0, lastErroEsq = 0;

long lastDir = 0, lastEsq = 0;

const int SPEED_BASE = 28;
const int SPEED_ROT = 25;
const int accelStep = 1;

// ===================== GEOMETRIA =======================

#define PULSOS_90 310
#define AVANCO_CENTRO 150   // ajustar entre 120–200

// ===================== CONTROLO ========================

int baseSpeedTarget = 0;
int baseSpeedCurrent = 0;
int correction = 0;

// ===================== ESTADOS =========================

enum EstadoMaze {
  SEGUIR_LINHA,
  AVANCAR_CENTRO,
  TRAVAR,
  RODAR_DIREITA
};

EstadoMaze estadoAtual = SEGUIR_LINHA;

// ===================== ISR =============================

void IRAM_ATTR encoderDir_ISR() {
  if (digitalRead(ENC_DIR_B)) contadorDir++;
  else contadorDir--;
}

void IRAM_ATTR encoderEsq_ISR() {
  if (digitalRead(ENC_ESQ_B)) contadorEsq--;
  else contadorEsq++;
}

// ===================== UTIL ============================

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

void definirRotacaoDireita() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
}

// ===================== QTR =============================

float readLinePosition() {

  long weightedSum = 0;
  long total = 0;

  for(int i=0;i<8;i++) {
    int raw = analogRead(qtrPins[i]);

    // Branco ~600
    // Preto ~3200–4095
    int value = map(raw, 600, 4095, 0, 1000);
    value = constrain(value, 0, 1000);

    weightedSum += (long)value * (i * 1000);
    total += value;
  }

  if(total < 100)
    return CENTER_POSITION;

  return (float)weightedSum / total;
}

bool detectarCruzamento() {
  int ativos = 0;

  for(int i=0;i<8;i++) {
    if(analogRead(qtrPins[i]) > 3000)
      ativos++;
  }

  return (ativos >= 7);  // mais restritivo
}

// ===================== PID VELOCIDADE ==================

int calcularPID(float target, float vel,
                float &integral, float &lastErro) {

  float erro = target - vel;
  integral += erro;
  integral = constrain(integral, -100, 100);

  float deriv = erro - lastErro;
  lastErro = erro;

  return Kp_vel*erro + Ki_vel*integral + Kd_vel*deriv;
}

void calcularVelocidade(float &velDir, float &velEsq) {
  long deltaDir = contadorDir - lastDir;
  long deltaEsq = contadorEsq - lastEsq;
  lastDir = contadorDir;
  lastEsq = contadorEsq;
  velDir = deltaDir * 100.0;
  velEsq = deltaEsq * 100.0;
}

// ===================== MOTORES =========================

void updateRamp() {
  if (baseSpeedCurrent < baseSpeedTarget)
    baseSpeedCurrent += accelStep;
  else if (baseSpeedCurrent > baseSpeedTarget)
    baseSpeedCurrent -= accelStep;
}

void applyMotorOutput() {

  float velDir, velEsq;
  calcularVelocidade(velDir, velEsq);

  int ajusteDir = calcularPID(baseSpeedCurrent, velDir,
                              integralDir, lastErroDir);

  int ajusteEsq = calcularPID(baseSpeedCurrent, velEsq,
                              integralEsq, lastErroEsq);

  int pwmDir = constrain(baseSpeedCurrent - correction + ajusteDir, 0, PWM_LIMIT);
  int pwmEsq = constrain(baseSpeedCurrent + correction + ajusteEsq, 0, PWM_LIMIT);

  ledcWrite(PWMA, pwmDir);
  ledcWrite(PWMB, pwmEsq);
}

// ===================== CONTROL LOOP ====================

void controlUpdate() {

  switch (estadoAtual) {

    case SEGUIR_LINHA: {

      definirFrente();

      float pos = readLinePosition();
      float error = pos - CENTER_POSITION;

      float deriv = error - lastLineError;
      lastLineError = error;

      correction = -(Kp_line * error + Kd_line * deriv);
      correction = constrain(correction, -60, 60);

      baseSpeedTarget = SPEED_BASE;

      if (detectarCruzamento()) {
        resetEncoders();
        estadoAtual = AVANCAR_CENTRO;
      }

      break;
    }

    case AVANCAR_CENTRO:

      definirFrente();
      correction = 0;
      baseSpeedTarget = SPEED_BASE;

      if (abs(contadorDir) >= AVANCO_CENTRO) {
        estadoAtual = TRAVAR;
      }

      break;

    case TRAVAR:

      baseSpeedTarget = 0;

      if (abs(baseSpeedCurrent) < 2) {
        resetEncoders();
        estadoAtual = RODAR_DIREITA;
      }

      break;

    case RODAR_DIREITA:

      definirRotacaoDireita();
      correction = 0;
      baseSpeedTarget = SPEED_ROT;

      if (abs(contadorDir) >= PULSOS_90) {
        resetEncoders();
        estadoAtual = SEGUIR_LINHA;
      }

      break;
  }

  updateRamp();
  applyMotorOutput();
}

// ===================== SETUP ===========================

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
}

// ===================== LOOP ============================

void loop() {
  if (millis() - lastUpdate >= CONTROL_INTERVAL) {
    lastUpdate = millis();
    controlUpdate();
  }
}