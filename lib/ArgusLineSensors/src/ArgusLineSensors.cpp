#include <Arduino.h>

#include "argus/ArgusConfig.h"
#include "argus/ArgusLineSensors.h"

namespace
{
    constexpr int ADC_BLACK_REFERENCE = 4095;
    constexpr int SENSOR_MAX_VALUE = 1000;

    constexpr int MIN_TOTAL_SIGNAL = 100;
    constexpr int LINE_DETECTION_THRESHOLD = 200;
    constexpr int WIDE_LINE_THRESHOLD = 500;
    constexpr uint8_t WIDE_LINE_MIN_ACTIVE = 7;

    // Referencias brancas medidas anteriormente.
    // Ordem: QTR1 a QTR8.
    constexpr int ADC_WHITE_REFERENCE[8] = {
        2942, 2680, 2671, 2492,
        2591, 2450, 2644, 3078};

    int readNormalizedSensor(uint8_t index)
    {
        const int raw = analogRead(ArgusConfig::QTR_PINS[index]);
        const int whiteReference = ADC_WHITE_REFERENCE[index];

        if (raw <= whiteReference)
            return 0;

        if (raw >= ADC_BLACK_REFERENCE)
            return SENSOR_MAX_VALUE;

        const long numerator =
            static_cast<long>(raw - whiteReference) * SENSOR_MAX_VALUE;

        const int value = static_cast<int>(
            numerator / (ADC_BLACK_REFERENCE - whiteReference));

        return constrain(value, 0, SENSOR_MAX_VALUE);
    }

    int readLineSignal()
    {
        int totalSignal = 0;

        for (uint8_t i = 0; i < 8; ++i)
        {
            const int value = readNormalizedSensor(i);

            if (value >= LINE_DETECTION_THRESHOLD)
                totalSignal += value;
        }

        return totalSignal;
    }
}

namespace ArgusLineSensors
{
    bool begin()
    {
        analogReadResolution(12);

        for (uint8_t i = 0; i < 8; ++i)
            pinMode(ArgusConfig::QTR_PINS[i], INPUT);

        return true;
    }

    bool hasLine()
    {
        return readLineSignal() >= MIN_TOTAL_SIGNAL;
    }

    float readLinePosition()
    {
        long weightedSum = 0;
        long totalSignal = 0;

        for (uint8_t i = 0; i < 8; ++i)
        {
            const int value = readNormalizedSensor(i);

            if (value >= LINE_DETECTION_THRESHOLD)
            {
                weightedSum +=
                    static_cast<long>(value) * (i * 1000);

                totalSignal += value;
            }
        }

        if (totalSignal < MIN_TOTAL_SIGNAL)
            return static_cast<float>(ArgusConfig::CENTER_POSITION);

        return static_cast<float>(weightedSum / totalSignal);
    }

    bool detectWideLine()
    {
        uint8_t activeSensors = 0;

        for (uint8_t i = 0; i < 8; ++i)
        {
            if (readNormalizedSensor(i) >= WIDE_LINE_THRESHOLD)
                ++activeSensors;
        }

        return activeSensors >= WIDE_LINE_MIN_ACTIVE;
    }
}