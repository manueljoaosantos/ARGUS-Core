#include "argus/ArgusTelemetry.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

namespace
{
  AsyncWebServer server(80);
  AsyncWebSocket ws("/ws");

  // Página HTML embutida
  const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ARGUS Telemetry</title>
  <style>
    body { font-family: monospace; background: #111; color: #0f0; padding: 20px; }
    .data { font-size: 1.2em; margin: 5px 0; }
    .label { color: #888; }
  </style>
</head>
<body>
  <h1>ARGUS Telemetry</h1>
  <div id="data">
    <div class="data"><span class="label">Modo:</span> <span id="mode">-</span></div>
    <div class="data"><span class="label">POS:</span> <span id="pos">-</span></div>
    <div class="data"><span class="label">Erro:</span> <span id="err">-</span></div>
    <div class="data"><span class="label">Correção:</span> <span id="corr">-</span></div>
    <div class="data"><span class="label">PWM Esq:</span> <span id="pwmL">-</span></div>
    <div class="data"><span class="label">PWM Dir:</span> <span id="pwmR">-</span></div>
  </div>
  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    window.addEventListener('load', onLoad);
    function onLoad(event) { initWebSocket(); }
    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage;
    }
    function onOpen(event) { console.log('Connection opened'); }
    function onClose(event) { setTimeout(initWebSocket, 2000); }
    function onMessage(event) {
      try {
        var data = JSON.parse(event.data);
        document.getElementById('mode').innerText = data.mode;
        document.getElementById('pos').innerText = data.pos;
        document.getElementById('err').innerText = data.err;
        document.getElementById('corr').innerText = data.corr;
        document.getElementById('pwmL').innerText = data.pwmL;
        document.getElementById('pwmR').innerText = data.pwmR;
      } catch(e) { console.error(e); }
    }
  </script>
</body>
</html>
)rawliteral";

  void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
  {
    if (type == WS_EVT_CONNECT)
    {
      Serial.printf("WebSocket client #%u connected\n", client->id());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
  }
}

namespace ArgusTelemetry
{
  void begin()
  {
    // Usa as credenciais do secrets.h
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP: ");
    Serial.println(IP);

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    server.begin();
    Serial.println("Telemetry server started.");
  }

  void sendData(float pos, float err, float corr, int pwmL, int pwmR, const char *mode)
  {
    if (ws.count() == 0)
      return;

    char json[128];
    snprintf(json, sizeof(json),
             "{\"mode\":\"%s\",\"pos\":%.0f,\"err\":%.0f,\"corr\":%.0f,\"pwmL\":%d,\"pwmR\":%d}",
             mode, pos, err, corr, pwmL, pwmR);
    ws.textAll(json);
  }
}