#pragma once

#include <Arduino.h>

namespace ArgusLineSensors
{
    // Inicializa os oito sensores analogicos.
    bool begin();

    // Indica se existe sinal suficiente para considerar
    // que a linha preta foi detetada.
    bool hasLine();

    // Devolve a posicao estimada da linha: 0 a 7000.
    // O centro teorico e 3500.
    float readLinePosition();

    // Deteta uma linha larga, potencialmente um cruzamento.
    bool detectWideLine();
}