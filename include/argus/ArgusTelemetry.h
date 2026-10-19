#pragma once
#include <Arduino.h>
#include "argus/secrets.h" // ← NOVO

namespace ArgusTelemetry
{
  // Inicia o Access Point e o servidor WebSocket
  // (usa as credenciais definidas em secrets.h)
  void begin();

  // Envia os dados para todos os clientes ligados
  void sendData(float pos, float err, float corr, int pwmL, int pwmR, const char *mode);
}