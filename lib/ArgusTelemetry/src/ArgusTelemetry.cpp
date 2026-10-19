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
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ARGUS Telemetry</title>
  <style>
    body { font-family: monospace; background: #111; color: #0f0; padding: 20px; }
    .data { font-size: 1.2em; margin: 5px 0; }
    .label { color: #888; }
    .ok { color: #0f0; }
    .err { color: #f00; }
    button {
      background: #c00; color: white; border: none;
      padding: 10px 20px; font-size: 1em; cursor: pointer;
      border-radius: 4px; margin-top: 20px;
    }
    button:hover { background: #f00; }
  </style>
</head>
<body>
  <h1>ARGUS Telemetry</h1>
  <div class="data"><span class="label">Ligação:</span> <span id="status">-</span></div>
  <hr>
  <div class="data"><span class="label">Modo:</span> <span id="mode">-</span></div>
  <div class="data"><span class="label">POS:</span> <span id="pos">-</span></div>
  <div class="data"><span class="label">Erro:</span> <span id="err">-</span></div>
  <div class="data"><span class="label">Correção:</span> <span id="corr">-</span></div>
  <div class="data"><span class="label">PWM Esq:</span> <span id="pwmL">-</span></div>
  <div class="data"><span class="label">PWM Dir:</span> <span id="pwmR">-</span></div>
  <hr>
  <button id="btnReset">🔄 Reiniciar</button>
  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    window.addEventListener('load', onLoad);
    function onLoad(event) { initWebSocket(); }
    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onopen = function() {
        document.getElementById('status').innerText = 'Ligado';
        document.getElementById('status').className = 'ok';
      };
      websocket.onclose = function() {
        document.getElementById('status').innerText = 'Desligado (a reconectar...)';
        document.getElementById('status').className = 'err';
        setTimeout(initWebSocket, 2000);
      };
      websocket.onmessage = function(event) {
        try {
          var data = JSON.parse(event.data);
          document.getElementById('mode').innerText = data.mode;
          document.getElementById('pos').innerText = data.pos;
          document.getElementById('err').innerText = data.err;
          document.getElementById('corr').innerText = data.corr;
          document.getElementById('pwmL').innerText = data.pwmL;
          document.getElementById('pwmR').innerText = data.pwmR;
        } catch(e) { console.error(e); }
      };
    }
    document.getElementById('btnReset').addEventListener('click', function() {
      if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send('reset');
        alert('Reset enviado! O robô vai reiniciar.');
      } else {
        alert('WebSocket não está ligado.');
      }
    });
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
    else if (type == WS_EVT_DATA)
    {
      // Recebeu dados do cliente (ex: "reset")
      String msg = "";
      for (size_t i = 0; i < len; i++)
        msg += (char)data[i];
      Serial.printf("WebSocket message: %s\n", msg.c_str());
      if (msg == "reset")
      {
        Serial.println("Comando de RESET recebido! A reiniciar...");
        client->text("A reiniciar...");
        delay(100);
        ESP.restart();
      }
    }
  }
}

namespace ArgusTelemetry
{
  void begin()
  {
    Serial.println();
    Serial.println("=== Telemetria Wi-Fi ===");

    // Tentar ligar à rede de casa (Modo Estação)
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("A ligar ao Wi-Fi '");
    Serial.print(WIFI_SSID);
    Serial.print("'");

    uint32_t timeout = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - timeout < 15000)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print("Ligado! IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("Acede em: http://");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println("Falha ao ligar à rede de casa. A criar Access Point...");
      WiFi.mode(WIFI_AP);
      WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
      Serial.print("AP IP: ");
      Serial.println(WiFi.softAPIP());
      Serial.print("Liga-te a '");
      Serial.print(WIFI_SSID);
      Serial.print("' e acede em: http://");
      Serial.println(WiFi.softAPIP());
    }

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