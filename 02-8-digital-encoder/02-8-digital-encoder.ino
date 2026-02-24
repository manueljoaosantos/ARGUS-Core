#include <Arduino.h>

// ===================== MOTORES =====================
#define AIN1 4
#define AIN2 5
#define PWMA 12

#define BIN1 13
#define BIN2 14
#define PWMB 15

#define PWM_FREQ 1000
#define PWM_RES 8

int velReta = 150;
int velCurvaRapido = 160;
int velCurvaLento  = 80;

// ===================== ENCODERS =====================
#define ENC_DIR_A 16
#define ENC_DIR_B 17

#define ENC_ESQ_A 21
#define ENC_ESQ_B 42

volatile long contadorDir = 0;
volatile long contadorEsq = 0;

// -------- ISR DIREITO --------
void IRAM_ATTR encoderDir_ISR() {
  if (digitalRead(ENC_DIR_B) == HIGH)
    contadorDir++;
  else
    contadorDir--;
}

// -------- ISR ESQUERDO --------
void IRAM_ATTR encoderEsq_ISR() {
  if (digitalRead(ENC_ESQ_B) == HIGH)
    contadorEsq--;
  else
    contadorEsq++;
}

// ===================== UTIL =====================
void resetEncoders() {
  contadorDir = 0;
  contadorEsq = 0;
}

void motores_stop() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);

  ledcWrite(PWMA, 0);
  ledcWrite(PWMB, 0);
}

// ===================== MOVIMENTOS =====================

// -------- ANDAR RETO --------
void andarReto(long alvo) {

  resetEncoders();

  // FRENTE (validado)
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  ledcWrite(PWMA, velReta);
  ledcWrite(PWMB, velReta);

  while (abs(contadorDir) < alvo) {}

  motores_stop();
  delay(300);
}

// -------- CURVA DIREITA --------
void curvaDireita(long alvo) {

  resetEncoders();

  // FRENTE
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  // Direita mais lenta → curva para direita
  ledcWrite(PWMA, velCurvaRapido);
  ledcWrite(PWMB, velCurvaLento);

  while (abs(contadorDir) < alvo) {}

  motores_stop();
  delay(300);
}

// -------- CURVA ESQUERDA --------
void curvaEsquerda(long alvo) {

  resetEncoders();

  // FRENTE
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  // Esquerda mais lenta → curva para esquerda
  ledcWrite(PWMA, velCurvaLento);
  ledcWrite(PWMB, velCurvaRapido);

  while (abs(contadorEsq) < alvo) {}

  motores_stop();
  delay(300);
}

// ===================== SETUP =====================
void setup() {

  Serial.begin(115200);

  // Motores
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);

  motores_stop();

  // Encoders
  pinMode(ENC_DIR_A, INPUT_PULLUP);
  pinMode(ENC_DIR_B, INPUT_PULLUP);
  pinMode(ENC_ESQ_A, INPUT_PULLUP);
  pinMode(ENC_ESQ_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_DIR_A), encoderDir_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_ESQ_A), encoderEsq_ISR, RISING);
}

// ===================== LOOP =====================
void loop() {

  // Lado direito do 8
  curvaDireita(1200);
  andarReto(400);

  // Lado esquerdo do 8
  curvaEsquerda(1200);
  andarReto(400);
}