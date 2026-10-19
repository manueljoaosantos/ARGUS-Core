#pragma once

namespace ArgusLineFollower
{

    // Reinicia a memória do controlador.
    void reset();

    // Atualiza o controlador PD.
    // Recebe a posição da linha e devolve a correção de direção.
    int update(float linePosition);

    // Define os ganhos proporcional e derivativo.
    void setGains(float kp, float kd);

}