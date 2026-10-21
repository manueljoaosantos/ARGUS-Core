#include <Arduino.h>
#include "argus/ArgusConfig.h"
#include "argus/ArgusMotors.h"
#include "argus/ArgusEncoders.h"
#include "argus/ArgusLineSensors.h"
#include "argus/ArgusTelemetry.h"
#include "argus/ArgusMaze.h"

/*
 * ===================================================================
 *  ARGUS — Labirinto (Fase 1) + Telemetria Completa
 * ===================================================================
 *
 *  Arquitetura:
 *  - Sensores QTR-8A (8 canais analógicos) → posição da linha (0-7000)
 *  - Cérebro do labirinto (ArgusMaze) → decide as direções
 *  - Telemetria Wi-Fi (POS, erro, PWM, velocidade real)
 *  - Gráfico em tempo real no dashboard
 *
 *  Modos do ArgusMaze:
 *  - Follow, Wait, Check, TurnL, TurnR, Turn180, Stopped
 * ===================================================================
 */

namespace
{
    // ---------------------------------------------------------------
    // Parâmetros do Segue Linha (usados pelo ArgusMaze internamente)
    // ---------------------------------------------------------------
    constexpr float LINE_KP = 0.04f;
    constexpr float LINE_KD = 0.0f;

    // ---------------------------------------------------------------
    // Debug
    // ---------------------------------------------------------------
    enum class DebugMode
    {
        None,
        Motors,
        Sensors,
        All
    };
    constexpr DebugMode DEBUG_MODE = DebugMode::All;
    constexpr uint32_t DEBUG_INTERVAL_MS = 200;

    // ---------------------------------------------------------------
    // Estado da Velocidade Real
    // ---------------------------------------------------------------
    long prevEncL = 0;
    long prevEncR = 0;
    uint32_t prevVelMs = 0;
    float velL = 0.0f;
    float velR = 0.0f;

    uint32_t lastDebugMs = 0;
}

void setup()
{
    // Inicia a telemetria Wi-Fi (usa as credenciais de secrets.h)
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== ARGUS ===");

    // Iniciar telemetria (DEPOIS do Serial)
    ArgusTelemetry::begin();
    Serial.println("Telemetria OK");

    Serial.println();
    Serial.println("=== ARGUS LABIRINTO — FASE 1 ===");

    ArgusEncoders::begin();
    ArgusMotors::begin();
    ArgusLineSensors::begin();
    ArgusMaze::begin();

    ArgusMotors::setForward();
    ArgusMotors::stop();

    // Iniciar contagem de encoders para velocidade
    ArgusEncoders::reset();
    prevEncL = 0;
    prevEncR = 0;
    prevVelMs = millis();

    Serial.println("Pronto.");
}

void loop()
{
    // ---------------------------------------------------------------
    // 1. Leitura dos sensores (para a telemetria)
    // ---------------------------------------------------------------
    const bool lineDetected = ArgusLineSensors::hasLine();
    const bool wideLine = lineDetected && ArgusLineSensors::detectWideLine();

    float position = ArgusConfig::CENTER_POSITION;
    float error = 0.0f;
    float correction = 0.0f;

    if (lineDetected && !wideLine)
    {
        position = ArgusLineSensors::readLinePosition();
        error = position - ArgusConfig::CENTER_POSITION;
        correction = LINE_KP * error;
    }

    // ---------------------------------------------------------------
    // 2. O CÉREBRO DO LABIRINTO DECIDE E CONTROLA OS MOTORES
    // ---------------------------------------------------------------
    ArgusMaze::update();

    // ---------------------------------------------------------------
    // 3. Cálculo da velocidade real (pulsos/s)
    // ---------------------------------------------------------------
    {
        long r, l;
        ArgusEncoders::readCounts(r, l);

        const uint32_t now = millis();
        const uint32_t elapsed = now - prevVelMs;
        if (elapsed >= 100)
        {
            velL = (l - prevEncL) * 1000.0f / elapsed;
            velR = (r - prevEncR) * 1000.0f / elapsed;
            prevEncL = l;
            prevEncR = r;
            prevVelMs = now;
        }
    }

    // ---------------------------------------------------------------
    // 4. Debug
    // ---------------------------------------------------------------
    if (millis() - lastDebugMs >= DEBUG_INTERVAL_MS)
    {
        lastDebugMs = millis();

        auto state = ArgusMaze::getState();
        const char *stateName = "Follow";

        if (state == ArgusMaze::State::Wait)
            stateName = "Wait";
        else if (state == ArgusMaze::State::CheckDirection)
            stateName = "Check";
        else if (state == ArgusMaze::State::TurnLeft)
            stateName = "TurnL";
        else if (state == ArgusMaze::State::TurnRight)
            stateName = "TurnR";
        else if (state == ArgusMaze::State::TurnAround)
            stateName = "Turn180";
        else if (state == ArgusMaze::State::Stopped)
            stateName = "Stopped";

        if (DEBUG_MODE == DebugMode::All || DEBUG_MODE == DebugMode::Sensors)
        {
            Serial.printf("[%s] linha=%d wide=%d pos=%.0f err=%.0f corr=%.0f\n",
                          stateName, lineDetected, wideLine, position, error, correction);
        }

        if (DEBUG_MODE == DebugMode::All || DEBUG_MODE == DebugMode::Motors)
        {
            Serial.printf("[%s] VelL=%.0f VelR=%.0f pulsos/s\n", stateName, velL, velR);
        }
    }

    // ---------------------------------------------------------------
    // 5. Telemetria Wi-Fi (com velocidade real e posição)
    // ---------------------------------------------------------------
    // Para a telemetria saber o PWM, precisamos de expor os PWMs no ArgusMotors
    int pwmL = ArgusMotors::getPwmLeft();
    int pwmR = ArgusMotors::getPwmRight();

    // Envia para o dashboard: POS, erro, correção, PWM, velocidade e estado
    ArgusTelemetry::sendData(position, error, correction, pwmL, pwmR, velL, velR, "Maze");
}