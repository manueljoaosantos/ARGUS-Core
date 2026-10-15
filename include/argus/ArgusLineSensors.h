
#pragma once

#include <Arduino.h>

namespace ArgusLineSensors
{

    // Inicializa os oito sensores analógicos.
    bool begin();

    // Devolve a posição estimada da linha: 0 a 7000.
    // O centro teórico é 3500.
    float readLinePosition();

    // Deteta uma linha larga, potencialmente um cruzamento.
    bool detectWideLine();

}
