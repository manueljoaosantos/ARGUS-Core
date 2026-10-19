#include <Arduino.h>
#include "argus/ArgusConfig.h"
#include "argus/ArgusMotors.h"
#include "argus/ArgusEncoders.h"
#include "argus/ArgusLineSensors.h"
#include "argus/ArgusTelemetry.h"

/*
 * ===================================================================
 *  ARGUS — Segue Linha (v1.3 — Rampa Progressiva + PWM Alto)
 * ===================================================================
 *
 *  Arquitetura:
 *  - Sensores QTR-8A (8 canais analógicos) → posição da linha (0-7000)
 *  - Controlador P (Proporcional) contínuo → reage a qualquer desvio
 *  - PWM direto nos motores (sem PID de velocidade no Follow)
 *  - Modos: Follow, WideLine, Searching, Stopped, Arrived
 *  - Deteção de base: linha larga durante >1s
 *  - Recuperação: procura a linha durante 2s antes de parar
 *  - Velocidade Adaptativa: retas rápidas, curvas lentas
 *  - Rampa Progressiva: aceleração suave para evitar trancos
 *  - Telemetria Wi-Fi com velocidade real
 *
 *  Parâmetros afinados:
 *  - LINE_KP = 0.04 (força da correção)
 *  - PWM_BASE_HIGH = 85 (velocidade máxima nas retas) -> PWM_LIMIT = 100
 *  - PWM_BASE_LOW = 55 (velocidade mínima nas curvas)
 *  - RETA_THRESHOLD = 800 (limiar de erro para considerar reta)
 *  - MAX_PWM_CORRECTION = 45 (garante que a roda lenta nunca para)
 * ===================================================================
 */

namespace
{
    // ---------------------------------------------------------------
    // Parâmetros do Segue Linha
    // ---------------------------------------------------------------
    constexpr float LINE_KP = 0.04f;
    constexpr float LINE_KD = 0.0f;

    // Velocidade nas Curvas (MANTIDA)
    constexpr int PWM_BASE_LOW = 45;

    // Velocidade nas Retas (AUMENTADA progressivamente)
    // Vamos usar o limite máximo permitido (PWM_LIMIT = 100 no ArgusConfig.h)
    constexpr int PWM_BASE_HIGH = 95;

    // Limiar de erro para considerar "reta".
    constexpr float RETA_THRESHOLD = 800.0f;

    // Correção máxima (MANTIDA para as curvas)
    constexpr int MAX_PWM_CORRECTION = 40;

    // ---------------------------------------------------------------
    // Parâmetros da Rampa Progressiva (Apenas para as retas)
    // ---------------------------------------------------------------
    constexpr int ACCEL_STEP = 3;             // Sobe 3 PWM por ciclo (mais rápido a chegar aos 95)
    constexpr int DECEL_STEP = 8;             // Desce 8 PWM por ciclo (travagem forte ao entrar na curva)
    constexpr uint32_t RAMP_INTERVAL_MS = 40; // A cada 40ms

    // ---------------------------------------------------------------
    // Parâmetros de Recuperação
    // ---------------------------------------------------------------
    constexpr uint32_t SEARCH_TIMEOUT_MS = 2000;
    constexpr int PWM_SEARCH = 50;

    // ---------------------------------------------------------------
    // Deteção de Base
    // ---------------------------------------------------------------
    constexpr uint32_t BASE_TIMEOUT_MS = 1000;

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
    // Estado Global
    // ---------------------------------------------------------------
    float lastKnownPosition = ArgusConfig::CENTER_POSITION;
    uint32_t lastKnownMs = 0;
    bool wasDetected = false;
    bool searching = false;
    uint32_t searchingSince = 0;
    bool inWideLine = false;
    uint32_t wideLineSince = 0;
    bool arrived = false;
    uint32_t lastDebugMs = 0;

    // Estado da Rampa
    int currentPwmBase = PWM_BASE_LOW;
    uint32_t lastRampMs = 0;

    // ---------------------------------------------------------------
    // Estado da Velocidade Real
    // ---------------------------------------------------------------
    long prevEncL = 0;
    long prevEncR = 0;
    uint32_t prevVelMs = 0;
    float velL = 0.0f;
    float velR = 0.0f;

    // ---------------------------------------------------------------
    // Função de Debug
    // ---------------------------------------------------------------
    void debugPrint(
        DebugMode mode,
        const char *modeName,
        bool lineDetected,
        bool wideLine,
        float position,
        float error,
        float correction,
        int pwmL,
        int pwmR)
    {
        if (mode == DebugMode::None)
            return;

        const uint32_t now = millis();
        if (now - lastDebugMs < DEBUG_INTERVAL_MS)
            return;
        lastDebugMs = now;

        if (mode == DebugMode::Sensors || mode == DebugMode::All)
        {
            Serial.printf("[%s] linha=%d wide=%d pos=%.0f err=%.0f corr=%.0f\n",
                          modeName, lineDetected, wideLine, position, error, correction);
        }

        if (mode == DebugMode::Motors || mode == DebugMode::All)
        {
            Serial.printf("[%s] PWM_L=%d PWM_R=%d (Base=%d) | VelL=%.0f VelR=%.0f pulsos/s\n",
                          modeName, pwmL, pwmR, currentPwmBase, velL, velR);
        }
    }
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
    Serial.println("=== ARGUS v1.4 — COM TELEMETRIA + GRÁFICO ===");

    ArgusEncoders::begin();
    ArgusMotors::begin();
    ArgusLineSensors::begin();

    ArgusMotors::setForward();
    ArgusMotors::stop();

    // Iniciar contagem de encoders para velocidade
    ArgusEncoders::reset();
    prevEncL = 0;
    prevEncR = 0;
    prevVelMs = millis();

    lastRampMs = millis();
    Serial.println("Pronto.");
}

void loop()
{
    if (arrived)
    {
        ArgusMotors::stop();
        delay(200);
        return;
    }

    // ---------------------------------------------------------------
    // 1. Leitura dos sensores
    // ---------------------------------------------------------------
    const bool lineDetected = ArgusLineSensors::hasLine();
    const bool wideLine = lineDetected && ArgusLineSensors::detectWideLine();

    // ---------------------------------------------------------------
    // 2. Determina o modo de operação
    // ---------------------------------------------------------------
    float position = lastKnownPosition;
    float error = 0.0f;
    float correction = 0.0f;

    enum class Mode
    {
        Follow,
        WideLine,
        Searching,
        Stopped,
        Arrived
    };
    Mode mode = Mode::Follow;

    if (lineDetected && !wideLine)
    {
        mode = Mode::Follow;
        inWideLine = false;

        position = ArgusLineSensors::readLinePosition();
        error = position - ArgusConfig::CENTER_POSITION;

        correction = LINE_KP * error;

        if (correction > MAX_PWM_CORRECTION)
            correction = MAX_PWM_CORRECTION;
        if (correction < -MAX_PWM_CORRECTION)
            correction = -MAX_PWM_CORRECTION;

        lastKnownPosition = position;
        lastKnownMs = millis();
        wasDetected = true;
        searching = false;
    }
    else if (lineDetected && wideLine)
    {
        if (!inWideLine)
        {
            inWideLine = true;
            wideLineSince = millis();
        }

        if (millis() - wideLineSince >= BASE_TIMEOUT_MS)
        {
            mode = Mode::Arrived;
            arrived = true;
        }
        else
        {
            mode = Mode::WideLine;
            correction = 0.0f;
        }

        lastKnownMs = millis();
        wasDetected = true;
        searching = false;
    }
    else
    {
        inWideLine = false;

        if (wasDetected && !searching)
        {
            searching = true;
            searchingSince = millis();
        }

        if (searching && (millis() - searchingSince) < SEARCH_TIMEOUT_MS)
        {
            mode = Mode::Searching;
            if (lastKnownPosition < ArgusConfig::CENTER_POSITION)
                correction = -PWM_SEARCH;
            else
                correction = +PWM_SEARCH;
        }
        else
        {
            mode = Mode::Stopped;
        }
    }

    // ---------------------------------------------------------------
    // 3. Rampa de velocidade (apenas no modo Follow)
    // ---------------------------------------------------------------
    if (mode == Mode::Follow)
    {
        const uint32_t now = millis();
        if (now - lastRampMs >= RAMP_INTERVAL_MS)
        {
            lastRampMs = now;

            // Qual é o alvo? (Reta = 95, Curva = 45)
            const int targetPwm = (fabs(error) < RETA_THRESHOLD) ? PWM_BASE_HIGH : PWM_BASE_LOW;

            if (currentPwmBase < targetPwm)
            {
                currentPwmBase += ACCEL_STEP;
                if (currentPwmBase > targetPwm)
                    currentPwmBase = targetPwm;
            }
            else if (currentPwmBase > targetPwm)
            {
                currentPwmBase -= DECEL_STEP;
                if (currentPwmBase < targetPwm)
                    currentPwmBase = targetPwm;
            }
        }
    }
    else
    {
        currentPwmBase = PWM_BASE_LOW;
    }

    // ---------------------------------------------------------------
    // 4. Aplica aos motores
    // ---------------------------------------------------------------
    int pwmL = 0, pwmR = 0;
    const char *modeName = "";

    switch (mode)
    {
    case Mode::Follow:
        // ⚠️ SINAL INVERTIDO
        pwmL = currentPwmBase - (int)correction;
        pwmR = currentPwmBase + (int)correction;
        modeName = "Follow";
        break;

    case Mode::WideLine:
        pwmL = PWM_BASE_LOW;
        pwmR = PWM_BASE_LOW;
        modeName = "WideLine";
        break;

    case Mode::Searching:
        if (correction < 0)
        {
            pwmL = 0;
            pwmR = PWM_SEARCH;
        }
        else
        {
            pwmL = PWM_SEARCH;
            pwmR = 0;
        }
        modeName = "Searching";
        break;

    case Mode::Arrived:
        pwmL = 0;
        pwmR = 0;
        modeName = "Arrived";
        break;

    case Mode::Stopped:
    default:
        pwmL = 0;
        pwmR = 0;
        modeName = "Stopped";
        break;
    }

    pwmL = constrain(pwmL, 0, (int)ArgusConfig::PWM_LIMIT);
    pwmR = constrain(pwmR, 0, (int)ArgusConfig::PWM_LIMIT);

    if (mode == Mode::Stopped || mode == Mode::Arrived)
    {
        ArgusMotors::stop();
    }
    else
    {
        ArgusMotors::setForward();
        ArgusMotors::setSpeeds(pwmL, pwmR);
    }

    // ---------------------------------------------------------------
    // 5. Cálculo da velocidade real (pulsos/s)
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
    // 6. Debug
    // ---------------------------------------------------------------
    debugPrint(DEBUG_MODE, modeName, lineDetected, wideLine,
               position, error, correction, pwmL, pwmR);

    // Envia telemetria para os clientes Wi-Fi (com velocidade real)
    ArgusTelemetry::sendData(position, error, correction, pwmL, pwmR, velL, velR, modeName);
}