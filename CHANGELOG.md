# Changelog

Todas as mudanças notáveis neste projeto serão documentadas neste ficheiro.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/),
e este projeto adere ao [Versionamento Semântico](https://semver.org/lang/pt-BR/).

---

## [Não lançado]

### Adicionado

- Integração com `ArgusMaze` (máquina de estados para labirinto)
- Comunicação serial com Raspberry Pi
- Visão computacional / IA

### Alterado

- Ajuste fino dos parâmetros do segue linha para curvas apertadas
- Otimização do PID de velocidade

---

## [v1.2.3-velocidade-real] - 2026-10-19

### 🏎️ Velocidade Real no Dashboard

### Adicionado

- Cálculo da velocidade real (pulsos/s) a partir dos encoders
- Envio de `velL` e `velR` no JSON da telemetria
- Campos `Vel Esq` e `Vel Dir` no dashboard HTML
- Atualização da velocidade a cada 100ms (suavização)

### Alterado

- `sendData()` agora recebe `velL` e `velR`
- `ArgusTelemetry.h` atualizado com a nova assinatura
- `main.cpp` calcula a velocidade real no `loop()`
- `debugPrint` mostra `velL` e `velR` no Serial

### Testes realizados

- ✅ Velocidade real aparece no dashboard
- ✅ Valores atualizam em tempo real
- ✅ Consistência com o Serial Monitor

---

## [v1.2.2-botao-reset] - 2026-10-19

### 🔄 Botão de Reset na Página Web

### Adicionado

- Botão "🔄 Reiniciar" no dashboard HTML
- WebSocket envia `"reset"` quando clicado
- `onWsEvent` trata `WS_EVT_DATA` e chama `ESP.restart()`
- Permite reiniciar o robô sem carregar no botão EN

### Testes realizados

- ✅ Botão aparece no dashboard
- ✅ ESP32 reinicia ao clicar
- ✅ Serial mostra "Comando de RESET recebido!"

---

## [v1.2.1-telemetria-wifi] - 2026-10-19

### 📡 Telemetria Wi-Fi Funcional (STA + fallback AP)

### Adicionado

- **Modo Estação (STA):** o ESP32 liga-se à rede de casa
- **Fallback AP:** se falhar, cria `ARGUS-Telemetry` como rede de emergência
- **Página HTML com UTF-8** (acentos corrigidos)
- **Status de ligação** no dashboard (Ligado/Desligado)
- **Dados em tempo real** (POS, erro, correção, PWM, modo)

### Alterado

- `ArgusTelemetry::begin()` agora tenta STA primeiro, depois AP
- `sendData()` é chamado no `loop()` principal e no modo `Arrived`

### Testes realizados

- ✅ Ligação à rede de casa (STA)
- ✅ Fallback para Access Point
- ✅ Página HTML abre no browser
- ✅ Dados atualizam em tempo real
- ✅ Acentos corrigidos (UTF-8)

---

## [v1.1-velocidade-reta] - 2026-10-19

### 🏎️ Velocidade Adaptativa com Rampa Progressiva

### Adicionado

- **Velocidade adaptativa:**
  - `PWM_BASE_HIGH = 95` (retas)
  - `PWM_BASE_LOW = 45` (curvas)
  - `RETA_THRESHOLD = 800` (limiar de erro)
  - O robô acelera nas retas e abranda nas curvas

- **Rampa progressiva:**
  - `ACCEL_STEP = 3` (sobe 3 PWM por ciclo de 40ms)
  - `DECEL_STEP = 8` (desce 8 PWM por ciclo de 40ms)
  - Aceleração suave (evita trancos)
  - Travagem rápida (entra nas curvas a tempo)

- **PWM_LIMIT aumentado:**
  - De `70` para `100` (mais margem de velocidade)

### Alterado

- Parâmetros do segue linha:
  - `PWM_BASE` substituído por `PWM_BASE_HIGH` e `PWM_BASE_LOW`
  - `MAX_PWM_CORRECTION = 40` (mantido)
  - `LINE_KP = 0.04` (mantido)

### Testes realizados

- ✅ Retas rápidas (PWM 95)
- ✅ Curvas lentas (PWM 45)
- ✅ Transições suaves (rampa)
- ✅ Sem oscilação nas retas
- ✅ Curvas mantidas (sem alterações)

---

## [v1.0-segue-linha] - 2026-10-19

### 🎉 Primeira versão funcional do segue linha

### Adicionado

- **Segue linha com QTR-8A:**
  - Controlador Proporcional (P) contínuo (`LINE_KP = 0.04`)
  - Reação imediata a qualquer desvio (mesmo 3 sensores vs 2)
  - PWM direto nos motores (`setSpeeds`)
  - Modos de operação: `Follow`, `WideLine`, `Searching`, `Stopped`, `Arrived`
- **Deteção de base:**
  - Linha larga durante >1s → `Arrived` (para permanentemente)
- **Recuperação:**
  - Procura a linha durante 2s antes de parar (`Searching`)
  - Gira na direção da última posição conhecida
- **Debug configurável:**
  - `DebugMode::None` (silencioso)
  - `DebugMode::Motors` (PWM)
  - `DebugMode::Sensors` (posição da linha)
  - `DebugMode::All` (tudo)

- **PID de velocidade:**
  - Feedforward individual por roda (`SPEED_FF_GAIN_LEFT/RIGHT`)
  - Correção PID para assimetria dos motores
  - Erro < ±5 pulsos/s em velocidade constante

### Corrigido

- **Mapeamento de motores:**
  - Canal A = motor DIREITO
  - Canal B = motor ESQUERDO
  - Sentido de rotação corrigido no `setForward()` e `setRightRotation()`

- **Encoders:**
  - Contagem positiva ao avançar (sinal invertido no ISR)
  - Pinos mapeados corretamente (`ENC_ESQ_*` → motor esquerdo, `ENC_DIR_*` → motor direito)

- **Bug crítico do PID:**
  - `previousUpdateMs = millis()` removido de `setTargetSpeeds()`
  - O PID nunca corria porque o timer era resetado a cada loop
  - Motores ficavam parados mesmo com alvos > 0

- **Sinal da correção do segue linha:**
  - Invertido no eixo: `pwmL = PWM_BASE - correction` e `pwmR = PWM_BASE + correction`
  - O robô virava para o lado errado antes da inversão

### Alterado

- **Parâmetros afinados:**
  - `LINE_KP = 0.04` (força da correção)
  - `PWM_BASE = 55` (velocidade base)
  - `MAX_PWM_CORRECTION = 40` (correção máxima)
  - `LINE_KD = 0.0` (derivativo desligado para evitar oscilação)

- **Arquitetura modular:**
  - `ArgusConfig` (configuração centralizada)
  - `ArgusMotors` (controlo de motores com PID)
  - `ArgusEncoders` (leitura de encoders)
  - `ArgusLineSensors` (leitura do QTR-8A)
  - `ArgusLineFollower` (controlador PD)
  - `ArgusMaze` (máquina de estados para labirinto)

### Testes realizados

- ✅ Motores (PWM direto)
- ✅ Encoders (contagem positiva)
- ✅ PID de velocidade (alvo 220 pulsos/s → erro ±5)
- ✅ Segue linha reta
- ✅ Segue linha com curvas suaves
- ✅ Deteção de base (`Arrived`)
- ✅ Recuperação (`Searching`)
- ✅ Sinal da correção (invertido)

---

## [v0.9-arquitetura-modular] - 2026-10-15

### Adicionado

- Arquitetura modular com libs separadas:
  - `ArgusConfig`
  - `ArgusMotors`
  - `ArgusEncoders`
  - `ArgusLineSensors`
  - `ArgusLineFollower`
  - `ArgusMaze`
- Headers em `include/argus/`
- Namespaces para cada módulo

### Alterado

- Estrutura do projeto reorganizada
- `platformio.ini` atualizado com `build_src_filter`

---

## [v0.8-maze-examples] - 2026-10-14

### Adicionado

- Exemplos de navegação de labirinto (`maze navigation examples`)
- Documentação inicial para integração futura

---

## [v0.7-maze-safe-control] - 2026-02-24

### Alterado

- Estratégia de controlo do segue linha transita para "maze-safe"
- Prioridade à estabilidade e fiabilidade em curvas de 90°

---

## [v0.6-loop-nao-bloqueante] - 2026-02-24

### Adicionado

- Loop não bloqueante a ~100Hz
- Preparação para sensores de alta frequência

---

## [v0.5-motion-core] - 2026-02-24

### Adicionado

- Modelo diferencial (`baseSpeed ± correction`)
- Separação entre `baseSpeedTarget` e `baseSpeedCurrent`
- Rampa independente da correção

---

## [v0.4-maquina-estados] - 2026-02-24

### Adicionado

- Máquina de estados com aceleração suave
- Desaceleração suave (soft deceleration)
- Remoção de `delay()` no controlo principal

---

## [v0.3-8-digital] - 2026-02-24

### Adicionado

- Movimento em "8" controlado por pulsos dos encoders
- Validação de simetria e estabilidade mecânica

### Corrigido

- Sentido de rotação dos motores ajustado

---

## [v0.2-base-motores] - 2026-02-24

### Adicionado

- README do módulo base motores e encoders
- Estrutura inicial do projeto ajustada

---

## [v0.1-inicial] - 2026-02-24

### Adicionado

- Estrutura inicial do ARGUS-Core
- Controlo básico de motores (TB6612FNG)
- Leitura de encoders em quadratura
- Inicialização do ESP32-S3

---

## Tipos de mudanças

- `Adicionado` para novas funcionalidades.
- `Alterado` para mudanças em funcionalidades existentes.
- `Descontinuado` para funcionalidades que serão removidas.
- `Removido` para funcionalidades removidas.
- `Corrigido` para correção de bugs.
- `Segurança` em caso de vulnerabilidades.

---

## Versionamento Semântico

Este projeto segue o [Versionamento Semântico](https://semver.org/lang/pt-BR/):

- **MAJOR** (ex: v2.0.0): mudanças incompatíveis com versões anteriores
- **MINOR** (ex: v1.1.0): novas funcionalidades compatíveis
- **PATCH** (ex: v1.0.1): correções de bugs compatíveis

---

## Tags do projeto

| Tag                        | Data       | Descrição                                   |
| -------------------------- | ---------- | ------------------------------------------- |
| `v1.2.3-velocidade-real`   | 2026-10-19 | Velocidade real no dashboard                |
| `v1.2.2-botao-reset`       | 2026-10-19 | Botão de reset na página web                |
| `v1.2.1-telemetria-wifi`   | 2026-10-19 | Telemetria Wi-Fi (STA + fallback AP)        |
| `v1.2-telemetria-wifi`     | 2026-10-19 | Telemetria Wi-Fi (versão inicial)           |
| `v1.1-velocidade-reta`     | 2026-10-19 | Velocidade adaptativa + rampa progressiva   |
| `v1.0-segue-linha`         | 2026-10-19 | Primeira versão funcional do segue linha    |
| `v0.9-arquitetura-modular` | 2026-10-15 | Arquitetura modular (libs separadas)        |
| `v0.8-maze-examples`       | 2026-10-14 | Exemplos de navegação de labirinto          |
| `v0.7-maze-safe-control`   | 2026-02-24 | Controlo maze-safe (estabilidade em 90°)    |
| `v0.6-loop-nao-bloqueante` | 2026-02-24 | Loop não bloqueante a 100Hz                 |
| `v0.5-motion-core`         | 2026-02-24 | Modelo diferencial (baseSpeed ± correction) |
| `v0.4-maquina-estados`     | 2026-02-24 | Máquina de estados com aceleração suave     |
| `v0.3-8-digital`           | 2026-02-24 | Movimento em "8" controlado por pulsos      |
| `v0.2-base-motores`        | 2026-02-24 | README do módulo base motores               |
| `v0.1-inicial`             | 2026-02-24 | Estrutura inicial do ARGUS-Core             |
