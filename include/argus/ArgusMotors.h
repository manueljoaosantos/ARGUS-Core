
#pragma once

namespace ArgusMotors
{
    // Inicializa os pinos de direção e o PWM.
    bool begin();

    // Define o sentido de rotação dos motores.
    void setForward();
    void setRightRotation();

    // Aplica PWM diretamente, limitado pela configuração.
    void setSpeeds(int leftPwm, int rightPwm);

    // Define os alvos de velocidade das rodas.
    void setTargetSpeeds(float leftTarget, float rightTarget);

    // Atualiza a rampa e o controlo PID.
    // Deve ser chamada periodicamente.
    void updateSpeedControl();

    // Configura os ganhos do PID de velocidade.
    void setSpeedPidGains(
        float kp,
        float ki,
        float kd);

    // Reinicia o estado do controlador.
    void resetSpeedControl();

    // Para os motores e cancela os alvos de velocidade.
    void stop();
}
