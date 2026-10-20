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

        bool sawRight = false;
        bool sawLeft = false;
        bool sawStraight = false;

        constexpr int PWM_FOLLOW = 60;
        constexpr int PWM_SPIN = 60;
        constexpr int PWM_CHECK = 55;
        constexpr int PWM_ALIGN = 40; // Velocidade lenta para alinhar
        constexpr int TICKS_90 = 310;
        constexpr int TICKS_180 = TICKS_90 * 2;
        constexpr uint32_t TURN_TIMEOUT_MS = 1500;

        void setState(State newState)
        {
            currentState = newState;
            stateTimer = millis();
            if (newState == State::TurnLeft || newState == State::TurnRight || newState == State::TurnAround)
            {
                turnStartTime = millis();
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
                    startTicks = ArgusEncoders::leftCount();
                    targetTicks = TICKS_180;
                    setState(State::TurnAround);
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
                    startTicks = ArgusEncoders::leftCount();
                    targetTicks = TICKS_180;
                    setState(State::TurnAround);
                }
            }
            break;
        }

        case State::TurnRight:
        {
            if (millis() - turnStartTime > TURN_TIMEOUT_MS)
            {
                stopMotors();
                setState(State::AlignLine);
                break;
            }

            ArgusMotors::setSpin(PWM_SPIN);

            long currentTicks = abs(ArgusEncoders::leftCount() - startTicks);
            if (currentTicks >= targetTicks)
            {
                stopMotors();
                setState(State::AlignLine); // <-- Vai para alinhar
            }
            break;
        }

        case State::TurnLeft:
        {
            if (millis() - turnStartTime > TURN_TIMEOUT_MS)
            {
                stopMotors();
                setState(State::AlignLine);
                break;
            }

            ArgusMotors::setSpin(-PWM_SPIN);

            long currentTicks = abs(ArgusEncoders::rightCount() - startTicks);
            if (currentTicks >= targetTicks)
            {
                stopMotors();
                setState(State::AlignLine); // <-- Vai para alinhar
            }
            break;
        }

        case State::TurnAround:
        {
            if (millis() - turnStartTime > TURN_TIMEOUT_MS)
            {
                stopMotors();
                setState(State::AlignLine);
                break;
            }

            ArgusMotors::setSpin(PWM_SPIN);

            long currentTicks = abs(ArgusEncoders::leftCount() - startTicks);
            if (currentTicks >= targetTicks)
            {
                stopMotors();
                setState(State::AlignLine); // <-- Vai para alinhar
            }
            break;
        }

        case State::AlignLine:
        {
            // Se perdeu a linha de todo, para
            if (!lineDetected)
            {
                stopMotors();
                setState(State::Stopped);
                break;
            }

            // Se a linha está perfeitamente centrada (POS entre 3000 e 4000)
            // OU se o robô já andou o suficiente para se alinhar
            if (position > 3000 && position < 4000)
            {
                stopMotors();
                setState(State::FollowLine);
                break;
            }

            // Avança devagar para se alinhar
            ArgusMotors::setForward();
            ArgusMotors::setSpeeds(PWM_ALIGN, PWM_ALIGN);

            // Se demorar mais de 1s a alinhar, desiste e volta ao FollowLine
            if (millis() - stateTimer > 1000)
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