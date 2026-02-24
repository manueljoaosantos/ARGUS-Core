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
// ===================== QTR SENSOR =====================
// ======================================================

const int qtrPins[8] = {1,2,3,6,7,8,9,10};

int qtrMin[8];
int qtrMax[8];

const int CENTER_POSITION = 3515;

// 🎯 Ganhos estáveis para maze
float Kp_line = 30.0;
float Kd_line = 10.0;

float previousError = 0;
float lastError = 0;

bool lineDetected = true;

// ======================================================
// ===================== CORE CONTROLO ==================
// ======================================================

int baseSpeedTarget = 0;
int baseSpeedCurrent = 0;
int correction = 0;

const int passoAceleracao = 4;

unsigned long lastControlUpdate = 0;
const unsigned long CONTROL_INTERVAL = 10;  // 100Hz

// ======================================================
// ===================== MOTOR FUNCS ====================
// ======================================================

void definirFrente(){
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
}

void updateBaseSpeedRamp(){

  if(baseSpeedCurrent < baseSpeedTarget)
    baseSpeedCurrent += passoAceleracao;
  else if(baseSpeedCurrent > baseSpeedTarget)
    baseSpeedCurrent -= passoAceleracao;

  if(abs(baseSpeedCurrent - baseSpeedTarget) < passoAceleracao)
    baseSpeedCurrent = baseSpeedTarget;
}

void applyMotorOutput(){

  int velDir = baseSpeedCurrent - correction;
  int velEsq = baseSpeedCurrent + correction;

  velDir = constrain(velDir,0,255);
  velEsq = constrain(velEsq,0,255);

  ledcWrite(PWMA, velDir);
  ledcWrite(PWMB, velEsq);
}

// ======================================================
// ===================== CALIBRAÇÃO =====================
// ======================================================

void calibrateQTR(){

  Serial.println("Calibrando QTR (5s)...");

  for(int i=0;i<8;i++){
    qtrMin[i]=4095;
    qtrMax[i]=0;
  }

  unsigned long start=millis();

  while(millis()-start<5000){
    for(int i=0;i<8;i++){
      int value=analogRead(qtrPins[i]);
      if(value<qtrMin[i]) qtrMin[i]=value;
      if(value>qtrMax[i]) qtrMax[i]=value;
    }
  }

  Serial.println("Calibracao concluida.");
}

// ======================================================
// ===================== LEITURA LINHA ==================
// ======================================================

float readLinePosition(){

  long weightedSum=0;
  long total=0;

  int minVal=10000;
  int maxVal=0;

  for(int i=0;i<8;i++){

    int raw=analogRead(qtrPins[i]);

    int value=map(raw,qtrMin[i],qtrMax[i],0,1000);
    value=constrain(value,0,1000);

    value=1000-value;  // linha preta

    weightedSum+=(long)value*(i*1000);
    total+=value;

    minVal=min(minVal,value);
    maxVal=max(maxVal,value);
  }

  if((maxVal-minVal)<100)
    lineDetected=false;
  else
    lineDetected=true;

  if(total<20)
    return CENTER_POSITION;

  return (float)weightedSum/total;
}

// ======================================================
// ===================== CORREÇÃO PD ====================
// ======================================================

int computeLineCorrection(){

  float position=readLinePosition();
  float error=position-CENTER_POSITION;

  if(lineDetected){

    lastError=error;

    float derivative=error-previousError;
    previousError=error;

    int corr=(Kp_line*error +
              Kd_line*derivative)/1000.0;

    corr=constrain(corr,-180,180);

    return corr;
  }

  // 🔴 Linha perdida → modo ultra seguro
  baseSpeedTarget=30;

  if(lastError>0)
    return 110;
  else
    return -110;
}

// ======================================================
// ===================== CONTROL UPDATE =================
// ======================================================

void controlUpdate(){

  definirFrente();

  correction=computeLineCorrection();

  float errorAbs=abs(lastError);

  // 🎯 Perfil ultra estável para maze
  if(errorAbs>700)
      baseSpeedTarget=30;
  else if(errorAbs>450)
      baseSpeedTarget=45;
  else if(errorAbs>250)
      baseSpeedTarget=60;
  else if(errorAbs>120)
      baseSpeedTarget=70;
  else
      baseSpeedTarget=85;

  updateBaseSpeedRamp();
  applyMotorOutput();
}

// ======================================================
// ===================== SETUP ==========================
// ======================================================

void setup(){

  Serial.begin(115200);
  delay(1000);

  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);
  pinMode(BIN1,OUTPUT);
  pinMode(BIN2,OUTPUT);

  ledcAttach(PWMA,PWM_FREQ,PWM_RES);
  ledcAttach(PWMB,PWM_FREQ,PWM_RES);

  definirFrente();

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  for(int i=0;i<8;i++)
    pinMode(qtrPins[i],INPUT);

  calibrateQTR();
}

// ======================================================
// ===================== LOOP ===========================
// ======================================================

void loop(){

  unsigned long now=millis();

  if(now-lastControlUpdate>=CONTROL_INTERVAL){
    lastControlUpdate=now;
    controlUpdate();
  }
}