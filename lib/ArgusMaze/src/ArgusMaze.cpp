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

        bool sawRight = false;
        bool sawLeft = false;
        bool sawStraight = false;

        constexpr int PWM_FOLLOW = 60;
        constexpr int PWM_SPIN = 60;
        constexpr int PWM_CHECK = 55;
        constexpr int TICKS_90 = 310;
        constexpr int TICKS_180 = TICKS_90 * 2;
        constexpr uint32_t TURN_TIMEOUT_MS = 1500;
        constexpr uint32_t MIN_TURN_TIME_MS = 100;
        constexpr uint32_t LOST_LINE_TIMEOUT_MS = 300;

        void setState(State newState)
        {
            currentState = newState;
            stateTimer = millis();
            if (newState == State::TurnLeft || newState == State::TurnRight || newState == State::TurnAround)
            {
                turnStartTime = millis();
                lostLineTime = 0;
            }
        }

        void stopMotors()
        {
            ArgusMotors::stop();
        }
    }

    void begin()
    {
        currentState = State::FollowLine;
        stateTimer = millis();
    }

    State getState()
    {
        return currentState;
    }

    void reset()
    {
        currentState = State::FollowLine;
        stateTimer = millis();
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
            if (!lineDetected)
            {
                stopMotors();
                setState(State::Stopped);
                break;
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
                    // Fallback: assume T e vira à direita (Regra da Mão Direita)
                    startTicks = ArgusEncoders::rightCount();
                    targetTicks = TICKS_90;
                    setState(State::TurnRight);
                }
            }

            if (!lineDetected && (millis() - stateTimer > 300))
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
                    // Fallback: assume T e vira à direita
                    startTicks = ArgusEncoders::rightCount();
                    targetTicks = TICKS_90;
                    setState(State::TurnRight);
                }
            }
            break;
        }

        case State::TurnRight:
        {
            if (millis() - turnStartTime > TURN_TIMEOUT_MS)
            {
                stopMotors();
                setState(State::FollowLine);
                break;
            }

            // Se perdeu a linha, continua a rodar
            if (!lineDetected)
            {
                if (lostLineTime == 0)
                    lostLineTime = millis();
                // Se perdeu a linha há muito tempo, tenta continuar a rodar (sem delay)
                if (millis() - lostLineTime > LOST_LINE_TIMEOUT_MS)
                {
                    lostLineTime = 0; // reset
                }
            }
            else
            {
                lostLineTime = 0;
            }

            ArgusMotors::setSpin(PWM_SPIN);

            if (millis() - turnStartTime > MIN_TURN_TIME_MS)
            {
                if (lineDetected && !wideLine && position > 2500 && position < 4500)
                {
                    stopMotors();
                    setState(State::FollowLine);
                    break;
                }
            }

            long currentTicks = abs(ArgusEncoders::leftCount() - startTicks);
            if (currentTicks >= targetTicks)
            {
                stopMotors();
                setState(State::FollowLine);
            }
            break;
        }

        case State::TurnLeft:
        {
            if (millis() - turnStartTime > TURN_TIMEOUT_MS)
            {
                stopMotors();
                setState(State::FollowLine);
                break;
            }

            if (!lineDetected)
            {
                if (lostLineTime == 0)
                    lostLineTime = millis();
                if (millis() - lostLineTime > LOST_LINE_TIMEOUT_MS)
                {
                    lostLineTime = 0;
                }
            }
            else
            {
                lostLineTime = 0;
            }

            ArgusMotors::setSpin(-PWM_SPIN);

            if (millis() - turnStartTime > MIN_TURN_TIME_MS)
            {
                if (lineDetected && !wideLine && position > 2500 && position < 4500)
                {
                    stopMotors();
                    setState(State::FollowLine);
                    break;
                }
            }

            long currentTicks = abs(ArgusEncoders::rightCount() - startTicks);
            if (currentTicks >= targetTicks)
            {
                stopMotors();
                setState(State::FollowLine);
            }
            break;
        }

        case State::TurnAround:
        {
            if (millis() - turnStartTime > TURN_TIMEOUT_MS)
            {
                stopMotors();
                setState(State::FollowLine);
                break;
            }

            if (!lineDetected)
            {
                if (lostLineTime == 0)
                    lostLineTime = millis();
                if (millis() - lostLineTime > LOST_LINE_TIMEOUT_MS)
                {
                    lostLineTime = 0;
                }
            }
            else
            {
                lostLineTime = 0;
            }

            ArgusMotors::setSpin(PWM_SPIN);

            if (millis() - turnStartTime > (MIN_TURN_TIME_MS + 100))
            {
                if (lineDetected && !wideLine && position > 2500 && position < 4500)
                {
                    stopMotors();
                    setState(State::FollowLine);
                    break;
                }
            }

            long currentTicks = abs(ArgusEncoders::leftCount() - startTicks);
            if (currentTicks >= targetTicks)
            {
                stopMotors();
                setState(State::FollowLine);
            }
            break;
        }

        case State::Arrived:
        case State::Stopped:
        default:
            stopMotors();
            break;
        }
    }
}