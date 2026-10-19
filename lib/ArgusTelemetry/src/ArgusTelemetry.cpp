#include "argus/ArgusTelemetry.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

namespace
{
  AsyncWebServer server(80);
  AsyncWebSocket ws("/ws");

  const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ARGUS Telemetry</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
  <style>
    * { box-sizing: border-box; }
    body {
      font-family: monospace;
      background: #111;
      color: #0f0;
      padding: 10px;
      margin: 0 auto;
      max-width: 100%;
      overflow-x: hidden;
    }
    h1 { font-size: 1.5em; }
    .data { font-size: 1em; margin: 4px 0; }
    .label { color: #888; }
    .ok { color: #0f0; }
    .err { color: #f00; }
    button {
      background: #c00; color: white; border: none;
      padding: 10px 20px; font-size: 1em; cursor: pointer;
      border-radius: 4px; margin-top: 10px;
      width: 100%;
    }
    button:hover { background: #f00; }
    .chart-container {
      position: relative;
      width: 100%;
      height: 250px;  /* altura fixa para o gráfico */
      margin-top: 10px;
      background: #222;
      border-radius: 4px;
    }
    .data-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 5px;
      margin-top: 10px;
    }
    hr { border-color: #333; margin: 10px 0; }
    @media (min-width: 700px) {
      body { padding: 20px; max-width: 900px; }
      h1 { font-size: 2em; }
      .data { font-size: 1.2em; }
      .chart-container { height: 300px; }
      button { width: auto; }
    }
  </style>
</head>
<body>
  <h1>ARGUS Telemetry</h1>
  <div class="data"><span class="label">Ligação:</span> <span id="status">-</span></div>
  <hr>
  
  <!-- Gráfico em cima (responsivo) -->
  <div class="chart-container">
    <canvas id="chartPOS"></canvas>
  </div>
  <hr>

  <!-- Dados em baixo (2 colunas) -->
  <div class="data-grid">
    <div class="data"><span class="label">Modo:</span> <span id="mode">-</span></div>
    <div class="data"><span class="label">POS:</span> <span id="pos">-</span></div>
    <div class="data"><span class="label">Erro:</span> <span id="err">-</span></div>
    <div class="data"><span class="label">Correção:</span> <span id="corr">-</span></div>
    <div class="data"><span class="label">PWM Esq:</span> <span id="pwmL">-</span></div>
    <div class="data"><span class="label">PWM Dir:</span> <span id="pwmR">-</span></div>
    <div class="data"><span class="label">Vel Esq:</span> <span id="velL">-</span> p/s</div>
    <div class="data"><span class="label">Vel Dir:</span> <span id="velR">-</span> p/s</div>
  </div>

  <hr>
  <button id="btnReset">🔄 Reiniciar</button>

  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    var chartPOS = null;
    const MAX_POINTS = 100;

    function initChart() {
      const ctx = document.getElementById('chartPOS').getContext('2d');
      chartPOS = new Chart(ctx, {
        type: 'line',
        data: {
          labels: [],
          datasets: [{
            label: 'POS (0-7000)',
            data: [],
            borderColor: '#0f0',
            backgroundColor: 'rgba(0, 255, 0, 0.1)',
            borderWidth: 2,
            pointRadius: 0,
            tension: 0.1
          }, {
            label: 'Centro (3500)',
            data: [],
            borderColor: '#ff0',
            borderWidth: 1,
            borderDash: [5, 5],
            pointRadius: 0
          }]
        },
        options: {
          responsive: true,      // ← adapta-se ao container
          maintainAspectRatio: false,  // ← permite altura fixa em CSS
          animation: false,
          scales: {
            y: {
              min: 0,
              max: 7000,
              ticks: { color: '#888' },
              grid: { color: '#333' }
            },
            x: {
              ticks: { color: '#888', maxTicksLimit: 10 },
              grid: { color: '#333' }
            }
          },
          plugins: {
            legend: { labels: { color: '#0f0' } }
          }
        }
      });
    }

    function updateChart(pos) {
      if (!chartPOS) return;
      chartPOS.data.labels.push('');
      chartPOS.data.datasets[0].data.push(pos);
      chartPOS.data.datasets[1].data.push(3500);
      if (chartPOS.data.labels.length > MAX_POINTS) {
        chartPOS.data.labels.shift();
        chartPOS.data.datasets[0].data.shift();
        chartPOS.data.datasets[1].data.shift();
      }
      chartPOS.update('none');
    }

    window.addEventListener('load', function() {
      initChart();
      initWebSocket();
    });

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
          document.getElementById('velL').innerText = data.velL;
          document.getElementById('velR').innerText = data.velR;
          updateChart(data.pos);
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

  void sendData(float pos, float err, float corr, int pwmL, int pwmR, float velL, float velR, const char *mode)
  {
    if (ws.count() == 0)
      return;

    char json[192];
    snprintf(json, sizeof(json),
             "{\"mode\":\"%s\",\"pos\":%.0f,\"err\":%.0f,\"corr\":%.0f,\"pwmL\":%d,\"pwmR\":%d,\"velL\":%.0f,\"velR\":%.0f}",
             mode, pos, err, corr, pwmL, pwmR, velL, velR);
    ws.textAll(json);
  }
}