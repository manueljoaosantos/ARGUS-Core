#include <Arduino.h>
#include "argus/ArgusConfig.h"
#include "argus/ArgusEncoders.h"

namespace
{
    volatile long rightTicks = 0, leftTicks = 0;
    void IRAM_ATTR rightIsr()
    {
        if (digitalRead(ArgusConfig::ENC_DIR_B))
            rightTicks--;
        else
            rightTicks++; // mantém
    }
    void IRAM_ATTR leftIsr()
    {
        if (digitalRead(ArgusConfig::ENC_ESQ_B))
            leftTicks++;
        else
            leftTicks--; // ← inverte
    }
}
namespace ArgusEncoders
{
    bool begin()
    {
        pinMode(ArgusConfig::ENC_DIR_A, INPUT_PULLUP);
        pinMode(ArgusConfig::ENC_DIR_B, INPUT_PULLUP);
        pinMode(ArgusConfig::ENC_ESQ_A, INPUT_PULLUP);
        pinMode(ArgusConfig::ENC_ESQ_B, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(ArgusConfig::ENC_DIR_A), rightIsr, RISING);
        attachInterrupt(digitalPinToInterrupt(ArgusConfig::ENC_ESQ_A), leftIsr, RISING);
        reset();
        return true;
    }
    void reset()
    {
        noInterrupts();
        rightTicks = 0;
        leftTicks = 0;
        interrupts();
    }
    void readCounts(long &right, long &left)
    {
        noInterrupts();
        right = rightTicks;
        left = leftTicks;
        interrupts();
    }
    long rightCount()
    {
        long r, l;
        readCounts(r, l);
        return r;
    }
    long leftCount()
    {
        long r, l;
        readCounts(r, l);
        return l;
    }
}