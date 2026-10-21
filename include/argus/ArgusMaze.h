#pragma once
#include <Arduino.h>

namespace ArgusMaze
{

    // Estados da máquina do labirinto
    enum class State
    {
        FollowLine,     // Seguir a linha normalmente
        Wait,           // Parar um pouco para estabilizar no cruzamento
        CheckDirection, // Avançar devagar para ver para onde a linha vai
        TurnLeft,       // Rodar 90º à esquerda
        TurnRight,      // Rodar 90º à direita
        TurnAround,     // Rodar 180º (beco sem saída)
        AlignLine,
        RecoverLine, // <-- ADICIONADO: Corrigir a posição após a viragem
        Arrived,     // Chegou à base
        Stopped      // Parado por erro ou fim
    };

    void begin();
    void update();
    State getState();
    void reset();
}