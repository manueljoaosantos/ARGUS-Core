#include <Arduino.h>
#include "argus/ArgusConfig.h"
#include "argus/ArgusMotors.h"
#include "argus/ArgusEncoders.h"
#include "argus/ArgusLineSensors.h"

/*
 * ===================================================================
 *  ARGUS — Segue Linha (versão funcional)
 * ===================================================================
 *
 *  Arquitetura:
 *  - Sensores QTR-8A (8 canais analógicos) → posição da linha (0-7000)
 *  - Controlador P (Proporcional) contínuo → reage a qualquer desvio
 *  - PWM direto nos motores (sem PID de velocidade no Follow)
 *  - Modos: Follow, WideLine, Searching, Stopped, Arrived
 *  - Deteção de base: linha larga durante >1s
 *  - Recuperação: procura a linha durante 2s antes de parar
 *
 *  Parâmetros afinados:
 *  - LINE_KP = 0.04 (força da correção)
 *  - PWM_BASE = 55 (velocidade base)
 *  - MAX_PWM_CORRECTION = 40 (garante que a roda lenta nunca para)
 * ===================================================================
 */

namespace
{
    // ---------------------------------------------------------------
    // Parâmetros do Segue Linha
    // ---------------------------------------------------------------
    // A "força" da correção. Com 0.04, um erro de 1000 (desvio pequeno)
    // gera uma correção de 40 PWM.
    constexpr float LINE_KP = 0.04f;

    // Derivativo desligado (evita oscilação causada por ruído dos sensores).
    constexpr float LINE_KD = 0.0f;

    // Velocidade base em PWM (0 a 70).
    // Ajusta para a velocidade desejada. Valores altos (>60) podem
    // dificultar as curvas.
    constexpr int PWM_BASE = 55;

    // Correção máxima em PWM.
    // Garante que a roda lenta nunca para: PWM mínimo = 55 - 40 = 15.
    // Isto evita o efeito de "travão" e a inércia que causava a perda da linha.
    constexpr int MAX_PWM_CORRECTION = 40;

    // ---------------------------------------------------------------
    // Parâmetros de Recuperação
    // ---------------------------------------------------------------
    // Tempo máximo que o robô procura a linha antes de parar.
    constexpr uint32_t SEARCH_TIMEOUT_MS = 2000;

    // PWM usado para girar durante a procura.
    constexpr int PWM_SEARCH = 50;

    // ---------------------------------------------------------------
    // Deteção de Base
    // ---------------------------------------------------------------
    // Tempo mínimo em linha larga para considerar que chegou à base.
    // Se a linha for larga durante >1s, é a base (não um cruzamento).
    constexpr uint32_t BASE_TIMEOUT_MS = 1000;

    // ---------------------------------------------------------------
    // Debug
    // ---------------------------------------------------------------
    enum class DebugMode
    {
        None,    // Sem output
        Motors,  // Só PWM
        Sensors, // Só posição da linha
        All      // Tudo
    };

    // ⚙️ ALTERA AQUI O QUE QUERES VER:
    constexpr DebugMode DEBUG_MODE = DebugMode::All;
    constexpr uint32_t DEBUG_INTERVAL_MS = 200;

    // ---------------------------------------------------------------
    // Estado Global
    // ---------------------------------------------------------------
    // Última posição válida (para recuperação).
    float lastKnownPosition = ArgusConfig::CENTER_POSITION;
    uint32_t lastKnownMs = 0;

    // Flags de estado.
    bool wasDetected = false;
    bool searching = false;
    uint32_t searchingSince = 0;
    bool inWideLine = false;
    uint32_t wideLineSince = 0;
    bool arrived = false;

    // Debug.
    uint32_t lastDebugMs = 0;

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

        // Sensores
        if (mode == DebugMode::Sensors || mode == DebugMode::All)
        {
            Serial.printf("[%s] linha=%d wide=%d pos=%.0f err=%.0f corr=%.0f\n",
                          modeName, lineDetected, wideLine, position, error, correction);
        }

        // Motores
        if (mode == DebugMode::Motors || mode == DebugMode::All)
        {
            Serial.printf("[%s] PWM_L=%d PWM_R=%d\n", modeName, pwmL, pwmR);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("=== ARGUS: Segue Linha (com Recuperação e Base) ===");

    // Inicialização dos subsistemas.
    ArgusEncoders::begin();
    ArgusMotors::begin();
    ArgusLineSensors::begin();

    // Configuração inicial dos motores.
    ArgusMotors::setForward();
    ArgusMotors::stop();

    Serial.println("Pronto.");
}

void loop()
{
    // Se chegou à base, para permanentemente.
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
        Follow,    // Seguimento normal
        WideLine,  // Linha larga (possível cruzamento)
        Searching, // À procura da linha
        Stopped,   // Parado (perdeu a linha há muito tempo)
        Arrived    // Chegou à base
    };
    Mode mode = Mode::Follow;

    if (lineDetected && !wideLine)
    {
        // ----- Seguimento Normal -----
        mode = Mode::Follow;
        inWideLine = false;

        // Lê a posição da linha e calcula o erro.
        position = ArgusLineSensors::readLinePosition();
        error = position - ArgusConfig::CENTER_POSITION;

        // Controlador Proporcional (P) contínuo.
        // Reage instantaneamente a QUALQUER desvio, por pequeno que seja.
        correction = LINE_KP * error;

        // Limita a correção para o motor lento nunca parar.
        // MAX = 40 → PWM mínimo = 55 - 40 = 15 (nunca para).
        if (correction > MAX_PWM_CORRECTION)
            correction = MAX_PWM_CORRECTION;
        if (correction < -MAX_PWM_CORRECTION)
            correction = -MAX_PWM_CORRECTION;

        // Atualiza memória (para recuperação).
        lastKnownPosition = position;
        lastKnownMs = millis();
        wasDetected = true;
        searching = false;
    }
    else if (lineDetected && wideLine)
    {
        // ----- Linha Larga (Cruzamento ou Base) -----
        if (!inWideLine)
        {
            inWideLine = true;
            wideLineSince = millis();
        }

        if (millis() - wideLineSince >= BASE_TIMEOUT_MS)
        {
            // Linha larga durante muito tempo → é a base.
            mode = Mode::Arrived;
            arrived = true;
        }
        else
        {
            // Linha larga por pouco tempo → é um cruzamento.
            // Continua em frente.
            mode = Mode::WideLine;
            correction = 0.0f;
        }

        lastKnownMs = millis();
        wasDetected = true;
        searching = false;
    }
    else
    {
        // ----- Sem Linha -----
        inWideLine = false;

        if (wasDetected && !searching)
        {
            // Acabou de perder a linha.
            searching = true;
            searchingSince = millis();
        }

        if (searching && (millis() - searchingSince) < SEARCH_TIMEOUT_MS)
        {
            // Procura a linha girando para o lado onde ela estava.
            mode = Mode::Searching;
            if (lastKnownPosition < ArgusConfig::CENTER_POSITION)
                correction = -PWM_SEARCH;
            else
                correction = +PWM_SEARCH;
        }
        else
        {
            // Desistiu de procurar.
            mode = Mode::Stopped;
        }
    }

    // ---------------------------------------------------------------
    // 3. Aplica aos motores
    // ---------------------------------------------------------------
    int pwmL = 0, pwmR = 0;
    const char *modeName = "";

    switch (mode)
    {
    case Mode::Follow:
        // ⚠️ SINAL INVERTIDO (descoberto em teste):
        // Se error > 0 (linha à direita), correction > 0.
        // Para virar à direita: PWM_L aumenta, PWM_R diminui.
        // Então INVERTEMOS: pwmL = PWM_BASE - correction; pwmR = PWM_BASE + correction.
        pwmL = PWM_BASE - (int)correction;
        pwmR = PWM_BASE + (int)correction;
        modeName = "Follow";
        break;

    case Mode::WideLine:
        // Cruzamento: segue em frente.
        pwmL = PWM_BASE;
        pwmR = PWM_BASE;
        modeName = "WideLine";
        break;

    case Mode::Searching:
        // Gira no próprio eixo para procurar.
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

    // Limita os PWMs ao máximo permitido pelo hardware.
    pwmL = constrain(pwmL, 0, (int)ArgusConfig::PWM_LIMIT);
    pwmR = constrain(pwmR, 0, (int)ArgusConfig::PWM_LIMIT);

    // Aplica o movimento.
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
    // 4. Debug
    // ---------------------------------------------------------------
    debugPrint(DEBUG_MODE, modeName, lineDetected, wideLine,
               position, error, correction, pwmL, pwmR);
}