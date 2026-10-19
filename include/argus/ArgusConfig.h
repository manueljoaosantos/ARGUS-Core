#pragma once
#include <Arduino.h>

namespace ArgusConfig
{
    // TB6612FNG — motores
    // Canal A → motor DIREITO
    // Canal B → motor ESQUERDO
    constexpr uint8_t AIN1 = 13;
    constexpr uint8_t AIN2 = 14;
    constexpr uint8_t PWMA = 15;
    constexpr uint8_t BIN1 = 4;
    constexpr uint8_t BIN2 = 5;
    constexpr uint8_t PWMB = 12;

    constexpr uint32_t PWM_FREQ = 20000;
    constexpr uint8_t PWM_RES = 8;
    constexpr uint8_t PWM_LIMIT = 100; // era 80

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

    // ---------------------------------------------------------------
    // Controlo de velocidade dos motores (PID + feedforward)
    // ---------------------------------------------------------------
    // Ganhos do PID — suaves para evitar oscilação
    constexpr float SPEED_PID_KP = 0.12f; // era 0.20 (reduzido)
    constexpr float SPEED_PID_KI = 0.03f; // era 0.05 (reduzido)
    constexpr float SPEED_PID_KD = 0.0f;

    // Limite do integral para evitar windup
    constexpr float SPEED_PID_INTEGRAL_LIMIT = 40.0f; // era 60 (reduzido)

    // Feedforward individual por roda (recalibrado)
    constexpr float SPEED_FF_GAIN_LEFT = 0.28f;  // era 0.35
    constexpr float SPEED_FF_GAIN_RIGHT = 0.25f; // era 0.30

    // Nota: o offset mínimo (FF_MIN) foi removido para evitar
    // overshoot em velocidades baixas. O PID agora corrige sozinho.

    // Alvo máximo de velocidade em pulsos/s.
    constexpr float SPEED_MAX_TARGET = 300.0f;

    // Período do controlador de velocidade.
    constexpr uint32_t CONTROL_INTERVAL_MS = 10;
}