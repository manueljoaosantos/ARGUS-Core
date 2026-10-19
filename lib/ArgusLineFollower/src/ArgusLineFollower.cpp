#include <Arduino.h>

#include "argus/ArgusConfig.h"
#include "argus/ArgusLineFollower.h"

namespace
{

    constexpr float DEFAULT_KP = 0.8f;
    constexpr float DEFAULT_KD = 6.0f;
    constexpr float MAX_CORRECTION = 60.0f;

    float kp = DEFAULT_KP;
    float kd = DEFAULT_KD;
    float previousError = 0.0f;

}

namespace ArgusLineFollower
{

    void reset()
    {
        previousError = 0.0f;
    }

    void setGains(float newKp, float newKd)
    {
        kp = newKp;
        kd = newKd;
    }

    int update(float linePosition)
    {
        const float error =
            linePosition - ArgusConfig::CENTER_POSITION;

        const float derivative = error - previousError;
        previousError = error;

        const float output =
            -(kp * error + kd * derivative);

        return static_cast<int>(
            constrain(output, -MAX_CORRECTION, MAX_CORRECTION));
    }

}