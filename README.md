# ARGUS-Core 🤖

Plataforma robótica móvel baseada em ESP32-S3, núcleo locomotor do ecossistema ARGUS.

---

## 🎯 Objetivo

Desenvolver a base móvel inteligente da plataforma ARGUS com arquitetura modular e evolutiva:

- Controlo de motores DC (TB6612FNG)
- Encoders em quadratura
- Sensor de linha (QTR-8A)
- Controlo de velocidade com PID + Feedforward
- Segue linha com deteção de base e recuperação
- **Telemetria Wi-Fi (Modo Estação + fallback AP)**
- Máquina de estados (sem uso de delay)
- Integração futura com Raspberry Pi e Inteligência Artificial

---

## 📂 Estrutura do Projeto

```text
ARGUS-Core/
│
├── README.md
├── LICENSE
├── .gitignore
│
├── include/
│   └── argus/
│       ├── ArgusConfig.h
│       ├── ArgusMotors.h
│       ├── ArgusEncoders.h
│       ├── ArgusLineSensors.h
│       ├── ArgusLineFollower.h
│       ├── ArgusMaze.h
│       ├── ArgusTelemetry.h
│       ├── secrets.h            (ignorado pelo Git)
│       └── secrets.h.example    (modelo)
│
├── lib/
│   ├── ArgusConfig/
│   ├── ArgusMotors/
│   ├── ArgusEncoders/
│   ├── ArgusLineSensors/
│   ├── ArgusLineFollower/
│   ├── ArgusMaze/
│   └── ArgusTelemetry/
│
├── src/
│   └── main.cpp
│
└── docs/
    └── (documentação adicional)
```

---

## 🔌 Ligações de Hardware

### 1. Controlador de motores TB6612FNG (HW-048)

| Sinal TB6612FNG | GPIO ESP32-S3 | Função                              |
| --------------- | ------------: | ----------------------------------- |
| PWMA            |       GPIO 15 | PWM do canal A (motor DIREITO)      |
| AIN1            |       GPIO 13 | Direção do canal A (motor DIREITO)  |
| AIN2            |       GPIO 14 | Direção do canal A (motor DIREITO)  |
| PWMB            |       GPIO 12 | PWM do canal B (motor ESQUERDO)     |
| BIN1            |        GPIO 4 | Direção do canal B (motor ESQUERDO) |
| BIN2            |        GPIO 5 | Direção do canal B (motor ESQUERDO) |

> ⚠️ **Nota:** Os canais A e B estão fisicamente trocados em relação ao código original. A configuração atual reflete a ligação real.

#### Alimentação

| Pino | Ligação     | Observações                                                                                             |
| ---- | ----------- | ------------------------------------------------------------------------------------------------------- |
| VCC  | 3,3 V       | Alimentação lógica                                                                                      |
| VM   | 5 V         | Alimentação dos motores; confirmar a compatibilidade com os motores e a capacidade de corrente da fonte |
| GND  | Massa comum | Referência comum entre ESP32-S3 e controlador                                                           |
| NC   | Sem ligação | Não ligar                                                                                               |

#### Saídas para os motores

| Saída     | Motor/lado     |
| --------- | -------------- |
| AO1 / AO2 | Motor DIREITO  |
| BO1 / BO2 | Motor ESQUERDO |

### 2. Encoders

| Encoder  | Sinal | GPIO ESP32-S3 |
| -------- | ----- | ------------: |
| Esquerdo | C1    |       GPIO 21 |
| Esquerdo | C2    |       GPIO 42 |
| Direito  | C1    |       GPIO 16 |
| Direito  | C2    |       GPIO 17 |

> ⚠️ **Nota:** Os encoders estão fisicamente ligados aos motores corretos. A inversão de sinal foi corrigida no ISR.

### 3. Sensor de linha QTR-8A

| Sensor | GPIO ESP32-S3 |
| ------ | ------------: |
| QTR1   |        GPIO 1 |
| QTR2   |        GPIO 2 |
| QTR3   |        GPIO 3 |
| QTR4   |        GPIO 6 |
| QTR5   |        GPIO 7 |
| QTR6   |        GPIO 8 |
| QTR7   |        GPIO 9 |
| QTR8   |       GPIO 10 |

> **Nota:** Os sensores QTR-8A estão ligados a pinos ADC1 do ESP32-S3. Evitar GPIOs 1, 2, 3 para outros fins (são strapping pins).

### 4. Configuração no firmware

A configuração dos pinos encontra-se em: `include/argus/ArgusConfig.h`

A cablagem física e as definições do firmware devem permanecer sincronizadas.

### 5. Estado da validação do hardware

- [x] Firmware compilado e carregado no ESP32-S3.
- [x] Inicialização confirmada através do monitor série.
- [x] Movimento dos motores confirmado.
- [x] Leitura dos encoders durante o movimento confirmada.
- [x] Sentido de rotação de cada roda validado.
- [x] Encoders contam positivo ao avançar.
- [x] Motores mapeados corretamente.
- [x] PID de velocidade funcional (erro < ±5 pulsos/s).
- [ ] Alimentação dos motores e corrente disponível verificadas.

---

## 🔐 Credenciais Wi-Fi

O projeto usa um ficheiro `secrets.h` para guardar as credenciais Wi-Fi (SSID e password). Este ficheiro **não vai para o Git** (está no `.gitignore`).

**Para configurar:**

1. Copia o ficheiro de exemplo:

   ```bash
   cp include/argus/secrets.h.example include/argus/secrets.h
   ```

2. Edita o `secrets.h` com os teus dados:

   ```cpp
   #define WIFI_SSID     "NomeDaTuaRedeCasa"
   #define WIFI_PASSWORD "PasswordDaTuaRedeCasa"
   ```

3. **NÃO commites o `secrets.h`** (o `.gitignore` já o protege).

> ⚠️ **Nota:** O ESP32-S3 só suporta Wi-Fi **2.4 GHz**. Se o teu router tem redes separadas, usa a de 2.4 GHz.

---

## 📡 Telemetria Wi-Fi

O robô liga-se à **rede de casa** (Modo Estação) e serve uma página web com os dados em tempo real.

**Se não conseguir ligar à rede de casa** (ex: password errada), cria um **Access Point de fallback** (`ARGUS-Telemetry`) para não ficares sem acesso.

### Como usar (Modo Estação)

1. O ESP32 liga-se à rede de casa automaticamente.
2. O Serial Monitor mostra o IP atribuído:
   ```
   Ligado! IP: 192.168.1.42
   ```
3. Abre o browser em **`http://<IP>`** (no mesmo IP que o Serial mostrou).
4. Vês os dados a atualizar em tempo real.

### Como usar (Fallback AP)

Se o ESP32 não conseguir ligar à rede de casa:

1. O Serial Monitor mostra:
   ```
   Falha ao ligar à rede de casa. A criar Access Point...
   AP IP: 192.168.4.1
   ```
2. Liga-te ao Wi-Fi **`ARGUS-Telemetry`** (password definida no `secrets.h`).
3. Abre o browser em **`http://192.168.4.1`**.

### Tecnologia

- `ESPAsyncWebServer` + `WebSocket`
- Página HTML embutida (com UTF-8)
- Dados enviados a cada 200ms

---

## 🧩 Fases de Desenvolvimento

### 01 - Base Motores + Encoders ✅

Controlo direto dos motores com leitura de pulsos e movimento por distância.

---

### 02 - 8 Digital Controlado por Encoder ♾️ ✅

Execução de trajetória em forma de "8" utilizando controlo por pulsos.

---

### 03 - Máquina de Estados ✅

Remoção de `delay()` e introdução de controlo baseado em `millis()`.

---

### 04 - Motion Control Core (Refatoração Estrutural) ✅

Reestruturação do sistema de movimento com separação entre `baseSpeedTarget` e `baseSpeedCurrent`, rampa independente da correção e estrutura preparada para PID.

---

### 05 - Control Loop Não Bloqueante ✅

Introdução de scheduler fixo (~100Hz) para atualização consistente de controlo.

---

### 06 - Line Following (QTR-8A) ✅ **IMPLEMENTADO**

Integração do sensor Pololu QTR-8A.

**Funcionalidades implementadas:**

- Cálculo de posição da linha (0-7000, centro 3500)
- Controlador Proporcional (P) contínuo
- Modos: `Follow`, `WideLine`, `Searching`, `Stopped`, `Arrived`
- Deteção de base (linha larga > 1s)
- Recuperação (procura a linha durante 2s)
- Debug configurável (`Motors`, `Sensors`, `All`, `None`)
- **Velocidade Adaptativa:** retas rápidas (PWM 95), curvas lentas (PWM 45)
- **Rampa Progressiva:** aceleração suave (+3 PWM/ciclo) e travagem rápida (-8 PWM/ciclo)

**Parâmetros atuais:**

- `LINE_KP = 0.04` (força da correção)
- `PWM_BASE_HIGH = 95` (velocidade nas retas)
- `PWM_BASE_LOW = 45` (velocidade nas curvas)
- `MAX_PWM_CORRECTION = 40` (correção máxima)
- `RETA_THRESHOLD = 800` (limiar de erro para considerar reta)
- `PWM_LIMIT = 100` (limite máximo do hardware)

> ⚠️ **Nota crítica:** O sinal da correção foi **invertido** no eixo (`pwmL = PWM_BASE - correction`), após testes empíricos.

---

### 07 - Telemetria Wi-Fi ✅ **IMPLEMENTADO**

### Gráfico em tempo real

O dashboard inclui um gráfico (Chart.js) que mostra a evolução do `POS` ao longo do tempo:

- **Linha verde:** POS (0-7000)
- **Linha amarela tracejada:** centro (3500)
- **Scroll automático:** máximo de 100 pontos
- **Responsivo:** adapta-se ao telemóvel e ao PC

### Dados mostrados no dashboard

| Campo             | Descrição                                                            |
| ----------------- | -------------------------------------------------------------------- |
| **Ligação**       | Estado do WebSocket (Ligado/Desligado)                               |
| **Modo**          | Modo atual (`Follow`, `WideLine`, `Searching`, `Stopped`, `Arrived`) |
| **POS**           | Posição da linha (0-7000, centro 3500)                               |
| **Erro**          | `POS - 3500` (desvio do centro)                                      |
| **Correção**      | Valor da correção aplicada (`LINE_KP × erro`)                        |
| **PWM Esq / Dir** | PWM aplicado a cada roda (0-100)                                     |
| **Vel Esq / Dir** | Velocidade real em pulsos/s (medida pelos encoders)                  |

- Modo Estação (STA): liga à rede de casa
- Fallback AP: `ARGUS-Telemetry` se falhar
- Página HTML com UTF-8 (acentos corrigidos)
- Status de ligação no dashboard
- Dados em tempo real (POS, erro, correção, PWM, modo)
- **Botão de Reset** (reinicia o robô via WebSocket)
- **Velocidade real** (pulsos/s) de cada roda

---

### 08 - Integração com Raspberry Pi ⏳

Comunicação serial com Raspberry Pi. Delegação de processamento avançado (AI, visão, estratégia).

---

## 🚀 Próximos Passos

- [ ] Testar curvas mais apertadas (raio pequeno)
- [ ] Testar cruzamentos (`WideLine`)
- [ ] Testar base (`Arrived`)
- [ ] Integrar com `ArgusMaze` (máquina de estados para labirinto)
- [ ] Adicionar comunicação com Raspberry Pi
- [ ] Implementar visão computacional / IA
- [ ] Melhorias na telemetria (botão de reset, velocidade real, bateria, gráfico, ajuste remoto)

---

## 🧪 Testes Realizados

| Teste                          | Resultado      |
| ------------------------------ | -------------- |
| Motores (PWM direto)           | ✅             |
| Encoders (contagem)            | ✅             |
| PID de velocidade (alvo 220)   | ✅ ±5 pulsos/s |
| Segue linha reta               | ✅             |
| Segue linha com curvas suaves  | ✅             |
| Deteção de base (`Arrived`)    | ✅             |
| Recuperação (`Searching`)      | ✅             |
| Sinal da correção              | ✅ (invertido) |
| Telemetria Wi-Fi (STA)         | ✅             |
| Telemetria Wi-Fi (fallback AP) | ✅             |

---

## 📜 Licença

MIT License
