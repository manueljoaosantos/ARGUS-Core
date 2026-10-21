#include "argus/ArgusMaze.h"
#include "argus/ArgusMotors.h"
#include "argus/ArgusLineSensors.h"
#include "argus/ArgusEncoders.h"
#include "argus/ArgusConfig.h"

namespace ArgusMaze
{

    namespace
    {
        State currentState = State::FollowLine;
        uint32_t stateTimer = 0;

        int targetTicks = 0;
        long startTicks = 0;
        uint32_t turnStartTime = 0;
        uint32_t lostLineTime = 0;
        float lastKnownPosition = ArgusConfig::CENTER_POSITION;

        bool sawRight = false;
        bool sawLeft = false;
        bool sawStraight = false;

        constexpr int PWM_FOLLOW = 70;
        constexpr int PWM_SPIN = 65;
        constexpr int PWM_CHECK = 60;
        constexpr int PWM_RECOVER = 40; // Força para recuperar
        constexpr int TICKS_90 = 310;
        constexpr int TICKS_180 = TICKS_90 * 2;
        constexpr uint32_t TURN_TIMEOUT_MS = 1500;
        constexpr uint32_t MIN_TURN_TIME_MS = 100;
        constexpr uint32_t LOST_LINE_TIMEOUT_MS = 300;
        constexpr uint32_t BRAKE_TIME_MS = 20;
        constexpr uint32_t RECOVER_TIMEOUT_MS = 500;

        void setState(State newState)
        {
            currentState = newState;
            const uint32_t now = millis();
            stateTimer = now;

            if (newState == State::TurnLeft || newState == State::TurnRight || newState == State::TurnAround)
            {
                turnStartTime = now;
                lostLineTime = 0;

                if (newState == State::TurnAround)
                {
                    targetTicks = TICKS_180;
                    startTicks = ArgusEncoders::leftCount();
                }
            }
        }

        void stopMotors()
        {
            ArgusMotors::stop();
        }

        void handleTurn(int spinPwm, long encoderCount, uint32_t minTimeOffset,
                        const bool lineDetected, const bool wideLine, const float position)
        {
            if (millis() - turnStartTime > TURN_TIMEOUT_MS)
            {
                stopMotors();
                setState(State::FollowLine);
                return;
            }

            if (!lineDetected)
            {
                if (lostLineTime == 0)
                    lostLineTime = millis();
                if (millis() - lostLineTime > LOST_LINE_TIMEOUT_MS)
                    lostLineTime = 0;
            }
            else
            {
                lostLineTime = 0;
            }

            ArgusMotors::setSpin(spinPwm);

            if (millis() - turnStartTime > (MIN_TURN_TIME_MS + minTimeOffset))
            {
                if (lineDetected && !wideLine && position > 2500 && position < 4500)
                {
                    // Solução 2: Contra-spin para travar
                    ArgusMotors::setSpin(-spinPwm);
                    delay(BRAKE_TIME_MS);

                    stopMotors();
                    setState(State::FollowLine);
                    return;
                }
            }

            if (abs(encoderCount - startTicks) >= targetTicks)
            {
                ArgusMotors::setSpin(-spinPwm);
                delay(BRAKE_TIME_MS);

                stopMotors();
                setState(State::FollowLine);
            }
        }
    }

    void begin()
    {
        currentState = State::FollowLine;
        stateTimer = millis();
        lastKnownPosition = ArgusConfig::CENTER_POSITION;
    }

    State getState()
    {
        return currentState;
    }

    void reset()
    {
        currentState = State::FollowLine;
        stateTimer = millis();
        lastKnownPosition = ArgusConfig::CENTER_POSITION;
    }

    void update()
    {
        const bool lineDetected = ArgusLineSensors::hasLine();
        const bool wideLine = lineDetected && ArgusLineSensors::detectWideLine();
        const float position = ArgusLineSensors::readLinePosition();
        const float error = position - ArgusConfig::CENTER_POSITION;

        switch (currentState)
        {

        case State::FollowLine:
        {
            // Solução 1: Recuperação se perder a linha
            if (!lineDetected)
            {
                if (lostLineTime == 0)
                    lostLineTime = millis();

                if (millis() - lostLineTime < RECOVER_TIMEOUT_MS)
                {
                    // Gira devagar para o lado da última posição conhecida
                    if (lastKnownPosition < ArgusConfig::CENTER_POSITION)
                    {
                        ArgusMotors::setSpin(-PWM_RECOVER);
                    }
                    else
                    {
                        ArgusMotors::setSpin(PWM_RECOVER);
                    }
                    break;
                }
                else
                {
                    stopMotors();
                    setState(State::Stopped);
                    break;
                }
            }
            else
            {
                lostLineTime = 0;
                lastKnownPosition = position;
            }

            if (wideLine)
            {
                sawRight = false;
                sawLeft = false;
                sawStraight = false;
                stopMotors();
                setState(State::Wait);
                break;
            }

            if (position < 500)
            {
                stopMotors();
                startTicks = ArgusEncoders::rightCount();
                targetTicks = TICKS_90;
                setState(State::TurnLeft);
                break;
            }
            else if (position > 6500)
            {
                stopMotors();
                startTicks = ArgusEncoders::leftCount();
                targetTicks = TICKS_90;
                setState(State::TurnRight);
                break;
            }

            int pwmBase = PWM_FOLLOW;
            float correction = 0.04f * error;
            correction = constrain(correction, -35, 35);

            int pwmL = pwmBase - (int)correction;
            int pwmR = pwmBase + (int)correction;

            ArgusMotors::setForward();
            ArgusMotors::setSpeeds(pwmL, pwmR);
            break;
        }

        case State::Wait:
        {
            if (millis() - stateTimer > 200)
            {
                ArgusMotors::setForward();
                ArgusMotors::setSpeeds(PWM_CHECK, PWM_CHECK);
                setState(State::CheckDirection);
            }
            break;
        }

        case State::CheckDirection:
        {
            if (lineDetected && !wideLine)
            {
                if (position < 2500)
                    sawLeft = true;
                else if (position > 4500)
                    sawRight = true;
                else
                    sawStraight = true;
            }

            if (millis() - stateTimer > 400)
            {
                stopMotors();

                if (sawRight)
                {
                    startTicks = ArgusEncoders::rightCount();
                    targetTicks = TICKS_90;
                    setState(State::TurnRight);
                }
                else if (sawStraight)
                {
                    setState(State::FollowLine);
                }
                else if (sawLeft)
                {
                    startTicks = ArgusEncoders::leftCount();
                    targetTicks = TICKS_90;
                    setState(State::TurnLeft);
                }
                else
                {
                    startTicks = ArgusEncoders::rightCount();
                    targetTicks = TICKS_90;
                    setState(State::TurnRight);
                }
            }
            else if (!lineDetected && (millis() - stateTimer > 300))
            {
                stopMotors();
                if (sawRight)
                {
                    startTicks = ArgusEncoders::rightCount();
                    targetTicks = TICKS_90;
                    setState(State::TurnRight);
                }
                else if (sawLeft)
                {
                    startTicks = ArgusEncoders::leftCount();
                    targetTicks = TICKS_90;
                    setState(State::TurnLeft);
                }
                else
                {
                    startTicks = ArgusEncoders::rightCount();
                    targetTicks = TICKS_90;
                    setState(State::TurnRight);
                }
            }
            break;
        }

        case State::TurnRight:
            handleTurn(PWM_SPIN, ArgusEncoders::leftCount(), 0,
                       lineDetected, wideLine, position);
            break;

        case State::TurnLeft:
            handleTurn(-PWM_SPIN, ArgusEncoders::rightCount(), 0,
                       lineDetected, wideLine, position);
            break;

        case State::TurnAround:
            handleTurn(PWM_SPIN, ArgusEncoders::leftCount(), 100,
                       lineDetected, wideLine, position);
            break;

        case State::Arrived:
        case State::Stopped:
        default:
            stopMotors();
            break;
        }
    }
}