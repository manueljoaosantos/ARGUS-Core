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

#define PWM_FREQ 1000
#define PWM_RES 8

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
// ===================== CONTROLO ========================
// ======================================================

const long DISTANCIA = 1500;
const long ZONA_TRAVAGEM = 400;

int velMax = 150;
int velMin = 60;
int velRotacao = 130;

// 🔥 NOVA ESTRUTURA (CORE REFATORADO)

int baseSpeedTarget = 0;     // definido pela máquina de estados
int baseSpeedCurrent = 0;    // sofre rampa

int correction = 0;          // aplicado APÓS a rampa

const int passoAceleracao = 3;

// ======================================================
// ===================== ISR =============================
// ======================================================

void IRAM_ATTR encoderDir_ISR() {
  if (digitalRead(ENC_DIR_B) == HIGH)
    contadorDir++;
  else
    contadorDir--;
}

void IRAM_ATTR encoderEsq_ISR() {
  if (digitalRead(ENC_ESQ_B) == HIGH)
    contadorEsq--;
  else
    contadorEsq++;
}

// ======================================================
// ===================== ESTADOS =========================
// ======================================================

enum Estado {
  ANDAR,
  RODAR
};

Estado estadoAtual = ANDAR;
int ladosFeitos = 0;

// ======================================================
// ===================== UTIL ============================
// ======================================================

void resetEncoders() {
  contadorDir = 0;
  contadorEsq = 0;
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

// ======================================================
// ============ Rampa aplicada só à baseSpeed ===========
// ======================================================

void updateBaseSpeedRamp() {

  if (baseSpeedCurrent < baseSpeedTarget)
    baseSpeedCurrent += passoAceleracao;
  else if (baseSpeedCurrent > baseSpeedTarget)
    baseSpeedCurrent -= passoAceleracao;

  if (abs(baseSpeedCurrent - baseSpeedTarget) < passoAceleracao)
    baseSpeedCurrent = baseSpeedTarget;
}

// ======================================================
// ============= Aplicação Final Diferencial ============
// ======================================================

void applyMotorOutput() {

  // Fórmula estrutural do ARGUS
  int velDir = baseSpeedCurrent - correction;
  int velEsq = baseSpeedCurrent + correction;

  velDir = constrain(velDir, 0, 255);
  velEsq = constrain(velEsq, 0, 255);

  ledcWrite(PWMA, velDir);
  ledcWrite(PWMB, velEsq);
}

// ======================================================
// ===================== SETUP ===========================
// ======================================================

void setup() {

  Serial.begin(115200);

  // Motores
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);

  // Encoders
  pinMode(ENC_DIR_A, INPUT_PULLUP);
  pinMode(ENC_DIR_B, INPUT_PULLUP);
  pinMode(ENC_ESQ_A, INPUT_PULLUP);
  pinMode(ENC_ESQ_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_DIR_A), encoderDir_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_ESQ_A), encoderEsq_ISR, RISING);

  resetEncoders();
}

// ======================================================
// ===================== LOOP ============================
// ======================================================

void loop() {

  switch (estadoAtual) {

    // ==================================================
    case ANDAR: {

      definirFrente();

      long progresso = abs(contadorDir);
      long restante = DISTANCIA - progresso;

      int velDesejada = velMax;

      // Zona de desaceleração progressiva
      if (restante < ZONA_TRAVAGEM) {
        float fator = (float)restante / ZONA_TRAVAGEM;
        velDesejada = velMin + (velMax - velMin) * fator;
      }

      // 🔥 Apenas altera baseSpeedTarget
      baseSpeedTarget = velDesejada;

      // Correção proporcional simples (pré-PID)
      long erro = contadorDir - contadorEsq;
      float Kp = 0.5;
      correction = Kp * erro;

      if (progresso >= DISTANCIA) {
        baseSpeedTarget = 0;
        correction = 0;
        resetEncoders();
        estadoAtual = RODAR;
      }

      break;
    }

    // ==================================================
    case RODAR:

      definirRotacaoDireita();

      baseSpeedTarget = velRotacao;
      correction = 0;

      if (abs(contadorDir) >= 500) {

        baseSpeedTarget = 0;
        resetEncoders();

        ladosFeitos++;
        if (ladosFeitos >= 4)
          ladosFeitos = 0;

        estadoAtual = ANDAR;
      }

      break;
  }

  // 🔥 Ordem correta do Core
  updateBaseSpeedRamp();
  applyMotorOutput();
}