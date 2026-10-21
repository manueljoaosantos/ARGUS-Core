#include <Arduino.h>

#include "argus/ArgusConfig.h"
#include "argus/ArgusMotors.h"
#include "argus/ArgusEncoders.h"

namespace
{
    constexpr float DEFAULT_KP = ArgusConfig::SPEED_PID_KP;
    constexpr float DEFAULT_KI = ArgusConfig::SPEED_PID_KI;
    constexpr float DEFAULT_KD = ArgusConfig::SPEED_PID_KD;

    float kp = DEFAULT_KP;
    float ki = DEFAULT_KI;
    float kd = DEFAULT_KD;

    float targetLeft = 0.0f;
    float targetRight = 0.0f;

    float integralLeft = 0.0f;
    float integralRight = 0.0f;

    float previousErrorLeft = 0.0f;
    float previousErrorRight = 0.0f;

    long previousLeftTicks = 0;
    long previousRightTicks = 0;

    uint32_t previousUpdateMs = 0;

    bool controlEnabled = false;

    int currentLeftPwm = 0;
    int currentRightPwm = 0;

    int clampPwm(float value)
    {
        return constrain(
            static_cast<int>(lroundf(value)),
            0,
            static_cast<int>(ArgusConfig::PWM_LIMIT));
    }

    void applyPwm(int leftPwm, int rightPwm)
    {
        currentLeftPwm = constrain(
            leftPwm, 0, static_cast<int>(ArgusConfig::PWM_LIMIT));

        currentRightPwm = constrain(
            rightPwm, 0, static_cast<int>(ArgusConfig::PWM_LIMIT));

        ledcWrite(ArgusConfig::PWMA, currentLeftPwm);
        ledcWrite(ArgusConfig::PWMB, currentRightPwm);
    }

    float calculatePid(
        float target,
        float measured,
        float &integral,
        float &previousError,
        float dt)
    {
        const float error = target - measured;

        integral += error * dt;

        integral = constrain(
            integral,
            -ArgusConfig::SPEED_PID_INTEGRAL_LIMIT,
            ArgusConfig::SPEED_PID_INTEGRAL_LIMIT);

        const float derivative = (error - previousError) / dt;
        previousError = error;

        return kp * error + ki * integral + kd * derivative;
    }

    void clearControllerState()
    {
        integralLeft = 0.0f;
        integralRight = 0.0f;

        previousErrorLeft = 0.0f;
        previousErrorRight = 0.0f;

        targetLeft = 0.0f;
        targetRight = 0.0f;
    }
}

namespace ArgusMotors
{
    bool begin()
    {
        pinMode(ArgusConfig::AIN1, OUTPUT);
        pinMode(ArgusConfig::AIN2, OUTPUT);
        pinMode(ArgusConfig::BIN1, OUTPUT);
        pinMode(ArgusConfig::BIN2, OUTPUT);

        const bool leftReady = ledcAttach(
            ArgusConfig::PWMA,
            ArgusConfig::PWM_FREQ,
            ArgusConfig::PWM_RES);

        const bool rightReady = ledcAttach(
            ArgusConfig::PWMB,
            ArgusConfig::PWM_FREQ,
            ArgusConfig::PWM_RES);

        if (!leftReady || !rightReady)
            return false;

        applyPwm(0, 0);
        clearControllerState();

        previousUpdateMs = millis();
        controlEnabled = false;

        return true;
    }

    void setForward()
    {
        // Canal A = motor DIREITO
        digitalWrite(ArgusConfig::AIN1, HIGH);
        digitalWrite(ArgusConfig::AIN2, LOW);

        // Canal B = motor ESQUERDO
        digitalWrite(ArgusConfig::BIN1, LOW);
        digitalWrite(ArgusConfig::BIN2, HIGH);
    }

    void setRightRotation()
    {
        digitalWrite(ArgusConfig::AIN1, HIGH);
        digitalWrite(ArgusConfig::AIN2, LOW);

        digitalWrite(ArgusConfig::BIN1, HIGH);
        digitalWrite(ArgusConfig::BIN2, LOW);
    }

    void setSpeeds(int leftPwm, int rightPwm)
    {
        controlEnabled = false;
        clearControllerState();
        applyPwm(leftPwm, rightPwm);
    }

    void setTargetSpeeds(float leftTarget, float rightTarget)
    {
        targetLeft = constrain(
            leftTarget, 0.0f, ArgusConfig::SPEED_MAX_TARGET);

        targetRight = constrain(
            rightTarget, 0.0f, ArgusConfig::SPEED_MAX_TARGET);

        if (targetLeft > 0.0f || targetRight > 0.0f)
        {
            controlEnabled = true;
        }
        else
        {
            stop();
            return;
        }
    }

    void updateSpeedControl()
    {
        if (!controlEnabled)
            return;

        const uint32_t now = millis();
        const uint32_t elapsedMs = now - previousUpdateMs;

        if (elapsedMs < ArgusConfig::CONTROL_INTERVAL_MS)
            return;

        previousUpdateMs = now;

        const float dt = elapsedMs / 1000.0f;

        long rightTicks;
        long leftTicks;
        ArgusEncoders::readCounts(rightTicks, leftTicks);

        const long deltaLeft = leftTicks - previousLeftTicks;
        const long deltaRight = rightTicks - previousRightTicks;

        previousLeftTicks = leftTicks;
        previousRightTicks = rightTicks;

        const float measuredLeft = deltaLeft / dt;
        const float measuredRight = deltaRight / dt;

        const float corrLeft = calculatePid(
            targetLeft, measuredLeft,
            integralLeft, previousErrorLeft, dt);

        const float corrRight = calculatePid(
            targetRight, measuredRight,
            integralRight, previousErrorRight, dt);

        const float ffLeft = targetLeft * ArgusConfig::SPEED_FF_GAIN_LEFT;
        const float ffRight = targetRight * ArgusConfig::SPEED_FF_GAIN_RIGHT;

        applyPwm(
            clampPwm(ffLeft + corrLeft),
            clampPwm(ffRight + corrRight));
    }

    void setSpeedPidGains(float newKp, float newKi, float newKd)
    {
        kp = newKp;
        ki = newKi;
        kd = newKd;

        integralLeft = 0.0f;
        integralRight = 0.0f;
        previousErrorLeft = 0.0f;
        previousErrorRight = 0.0f;
    }

    void resetSpeedControl()
    {
        clearControllerState();

        long rightTicks;
        long leftTicks;
        ArgusEncoders::readCounts(rightTicks, leftTicks);

        previousLeftTicks = leftTicks;
        previousRightTicks = rightTicks;

        controlEnabled = false;
        applyPwm(0, 0);
    }

    void stop()
    {
        controlEnabled = false;
        clearControllerState();

        applyPwm(0, 0);

        long rightTicks;
        long leftTicks;
        ArgusEncoders::readCounts(rightTicks, leftTicks);

        previousLeftTicks = leftTicks;
        previousRightTicks = rightTicks;
    }

    void setSpin(int pwm)
    {
        controlEnabled = false;
        clearControllerState();

        int absPwm = constrain(abs(pwm), 0, (int)ArgusConfig::PWM_LIMIT);

        if (pwm > 0) // Virar à DIREITA
        {
            // Esquerda para a FRENTE, Direita para TRÁS
            // Compensação: Esquerda é mais fraca, damos +20 PWM
            int pwmLeft = constrain(absPwm + 20, 0, 100);
            int pwmRight = absPwm;

            digitalWrite(ArgusConfig::BIN1, LOW);
            digitalWrite(ArgusConfig::BIN2, HIGH); // Esquerda frente
            ledcWrite(ArgusConfig::PWMB, pwmLeft); // ESQUERDO -> PWMB

            digitalWrite(ArgusConfig::AIN1, LOW);
            digitalWrite(ArgusConfig::AIN2, HIGH);  // Direita trás
            ledcWrite(ArgusConfig::PWMA, pwmRight); // DIREITO -> PWMA
        }
        else if (pwm < 0) // Virar à ESQUERDA
        {
            // Esquerda para TRÁS, Direita para FRENTE
            // Compensação: Esquerda é mais fraca, damos +35 PWM (mais porque vai para trás)
            int pwmLeft = constrain(absPwm + 35, 0, 100);
            int pwmRight = absPwm;

            digitalWrite(ArgusConfig::BIN1, HIGH);
            digitalWrite(ArgusConfig::BIN2, LOW);  // Esquerda trás
            ledcWrite(ArgusConfig::PWMB, pwmLeft); // ESQUERDO -> PWMB

            digitalWrite(ArgusConfig::AIN1, HIGH);
            digitalWrite(ArgusConfig::AIN2, LOW);   // Direita frente
            ledcWrite(ArgusConfig::PWMA, pwmRight); // DIREITO -> PWMA
        }
        else
        {
            applyPwm(0, 0);
        }
    }

    int getPwmLeft() { return currentLeftPwm; }
    int getPwmRight() { return currentRightPwm; }
}