#pragma once
#include <Arduino.h>

namespace ArgusConfig
{

    // TB6612FNG — motores
    constexpr uint8_t AIN1 = 4;
    constexpr uint8_t AIN2 = 5;
    constexpr uint8_t PWMA = 12;
    constexpr uint8_t BIN1 = 13;
    constexpr uint8_t BIN2 = 14;
    constexpr uint8_t PWMB = 15;

    constexpr uint32_t PWM_FREQ = 20000;
    constexpr uint8_t PWM_RES = 8;
    constexpr uint8_t PWM_LIMIT = 70;

    // Encoders
    constexpr uint8_t ENC_DIR_A = 16;
    constexpr uint8_t ENC_DIR_B = 17;
    constexpr uint8_t ENC_ESQ_A = 21;
    constexpr uint8_t ENC_ESQ_B = 42;

    // Sensores QTR
    constexpr uint8_t QTR_PINS[8] = {1, 2, 3, 6, 7, 8, 9, 10};
    constexpr int CENTER_POSITION = 3500;

    // Parâmetros iniciais
    constexpr int SPEED_BASE = 28;
    constexpr int SPEED_ROT = 25;
    constexpr int ACCEL_STEP = 1;

    constexpr int PULSOS_90 = 310;
    constexpr int AVANCO_CENTRO = 150;

        // Controlo de velocidade dos motores
    constexpr float SPEED_PID_KP = 0.4f;
    constexpr float SPEED_PID_KI = 0.002f;
    constexpr float SPEED_PID_KD = 0.03f;

    // Limite do integral para evitar acumulação excessiva.
    constexpr float SPEED_PID_INTEGRAL_LIMIT = 100.0f;

    constexpr uint32_t CONTROL_INTERVAL_MS = 10;

}