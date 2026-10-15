
#include <Arduino.h>

#include "argus/ArgusConfig.h"
#include "argus/ArgusLineSensors.h"

namespace
{

    constexpr int ADC_WHITE_REFERENCE = 600;
    constexpr int ADC_BLACK_REFERENCE = 4095;
    constexpr int SENSOR_MAX_VALUE = 1000;
    constexpr int MIN_TOTAL_SIGNAL = 100;
    constexpr int WIDE_LINE_THRESHOLD = 3000;
    constexpr uint8_t WIDE_LINE_MIN_ACTIVE = 7;

    int readNormalizedSensor(uint8_t pin)
    {
        const int raw = analogRead(pin);

        int value = map(
            raw,
            ADC_WHITE_REFERENCE,
            ADC_BLACK_REFERENCE,
            0,
            SENSOR_MAX_VALUE);

        return constrain(value, 0, SENSOR_MAX_VALUE);
    }

}

namespace ArgusLineSensors
{

    bool begin()
    {
        analogReadResolution(12);

        for (uint8_t i = 0; i < 8; ++i)
        {
            pinMode(ArgusConfig::QTR_PINS[i], INPUT);
        }

        return true;
    }

    float readLinePosition()
    {
        long weightedSum = 0;
        long totalSignal = 0;

        for (uint8_t i = 0; i < 8; ++i)
        {
            const int value =
                readNormalizedSensor(ArgusConfig::QTR_PINS[i]);

            weightedSum += static_cast<long>(value) * (i * 1000);
            totalSignal += value;
        }

        if (totalSignal < MIN_TOTAL_SIGNAL)
        {
            return static_cast<float>(ArgusConfig::CENTER_POSITION);
        }

        return static_cast<float>(weightedSum) / totalSignal;
    }

    bool detectWideLine()
    {
        uint8_t activeSensors = 0;

        for (uint8_t i = 0; i < 8; ++i)
        {
            if (analogRead(ArgusConfig::QTR_PINS[i]) >
                WIDE_LINE_THRESHOLD)
            {
                ++activeSensors;
            }
        }

        return activeSensors >= WIDE_LINE_MIN_ACTIVE;
    }

}
