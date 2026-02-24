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

int velocidade = 150;

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

// -------- ISR ESQUERDO (invertido para alinhar sinais) --------
void IRAM_ATTR encoderEsq_ISR() {
  if (digitalRead(ENC_ESQ_B) == HIGH)
    contadorEsq--;
  else
    contadorEsq++;
}

// ===================== FUNÇÕES MOTOR =====================
void motores_stop() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);

  ledcWrite(PWMA, 0);
  ledcWrite(PWMB, 0);
}

void motores_frente() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  ledcWrite(PWMA, velocidade);
  ledcWrite(PWMB, velocidade);
}

void motores_tras() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  ledcWrite(PWMA, velocidade);
  ledcWrite(PWMB, velocidade);
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

  // -------- FRENTE --------
  contadorDir = 0;
  contadorEsq = 0;

  Serial.println("Frente 3s");
  motores_frente();
  delay(3000);

  motores_stop();
  delay(500);

  Serial.print("Direito: ");
  Serial.print(contadorDir);
  Serial.print(" | Esquerdo: ");
  Serial.println(contadorEsq);

  delay(2000);

  // -------- TRÁS --------
  contadorDir = 0;
  contadorEsq = 0;

  Serial.println("Tras 3s");
  motores_tras();
  delay(3000);

  motores_stop();
  delay(500);

  Serial.print("Direito: ");
  Serial.print(contadorDir);
  Serial.print(" | Esquerdo: ");
  Serial.println(contadorEsq);

  delay(3000);
}