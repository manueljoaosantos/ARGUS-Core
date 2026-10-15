
#include <Arduino.h>
#include "argus/ArgusConfig.h"
#include "argus/ArgusMotors.h"
#include "argus/ArgusEncoders.h"

uint32_t lastReportMs = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=== ARGUS-Core: Motores + Encoders ===");

    if (!ArgusEncoders::begin())
    {
        Serial.println("Erro ao inicializar encoders.");
        while (true)
            delay(1000);
    }

    if (!ArgusMotors::begin())
    {
        Serial.println("Erro ao inicializar motores.");
        while (true)
            delay(1000);
    }

    ArgusEncoders::reset();

    ArgusMotors::setForward();
    ArgusMotors::setSpeeds(
        ArgusConfig::PWM_LIMIT,
        ArgusConfig::PWM_LIMIT);

    Serial.println("Motores a avancar.");
    Serial.println("Leitura dos encoders a cada 500 ms.");
    Serial.println("Rodas devem estar levantadas.");
}

void loop()
{
    // Mantem os motores em movimento.
    ArgusMotors::setSpeeds(
        ArgusConfig::PWM_LIMIT,
        ArgusConfig::PWM_LIMIT);

    const uint32_t now = millis();

    if (now - lastReportMs >= 500)
    {
        lastReportMs = now;

        long right = 0;
        long left = 0;

        ArgusEncoders::readCounts(right, left);

        Serial.print("Direito: ");
        Serial.print(right);

        Serial.print(" | Esquerdo: ");
        Serial.println(left);
    }
}
