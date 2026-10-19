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
│       └── ArgusMaze.h
│
├── lib/
│   ├── ArgusConfig/
│   ├── ArgusMotors/
│   ├── ArgusEncoders/
│   ├── ArgusLineSensors/
│   ├── ArgusLineFollower/
│   └── ArgusMaze/
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

O controlo dos dois motores DC é realizado através do controlador TB6612FNG.

#### Sinais de controlo

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

Os encoders permitem medir a rotação das rodas e suportar o controlo de movimento por pulsos.

| Encoder  | Sinal | GPIO ESP32-S3 |
| -------- | ----- | ------------: |
| Esquerdo | C1    |       GPIO 21 |
| Esquerdo | C2    |       GPIO 42 |
| Direito  | C1    |       GPIO 16 |
| Direito  | C2    |       GPIO 17 |

> ⚠️ **Nota:** Os encoders estão fisicamente ligados aos motores corretos (esquerdo → motor esquerdo, direito → motor direito). A inversão de sinal foi corrigida no ISR.

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

A configuração dos pinos encontra-se em:

`include/argus/ArgusConfig.h`

A cablagem física e as definições do firmware devem permanecer sincronizadas. Qualquer alteração aos GPIO deve ser refletida neste documento.

### 5. Estado da validação do hardware

- [x] Firmware compilado e carregado no ESP32-S3.
- [x] Inicialização confirmada através do monitor série.
- [x] Movimento dos motores confirmado.
- [x] Leitura dos encoders durante o movimento confirmada.
- [x] Sentido de rotação de cada roda validado.
- [x] Encoders contam positivo ao avançar.
- [x] Motores mapeados corretamente (esquerdo = esquerdo, direito = direito).
- [x] PID de velocidade funcional (erro < ±5 pulsos/s).
- [ ] Alimentação dos motores e corrente disponível verificadas.

---

## 🧩 Fases de Desenvolvimento

### 01 - Base Motores + Encoders ✅

Controlo direto dos motores com leitura de pulsos e movimento por distância.
Implementação inicial de aceleração simples e testes de direção.

---

### 02 - 8 Digital Controlado por Encoder ♾️ ✅

Execução de trajetória em forma de "8" utilizando controlo por pulsos.
Validação de simetria, estabilidade mecânica e consistência dos encoders.

---

### 03 - Máquina de Estados ✅

Remoção de `delay()` e introdução de controlo baseado em `millis()`.
Separação de comportamentos (ANDAR / RODAR) e organização da lógica do movimento.

---

### 04 - Motion Control Core (Refatoração Estrutural) ✅

Reestruturação do sistema de movimento com:

- Separação entre `baseSpeedTarget` e `baseSpeedCurrent`
- Implementação de rampa independente da correção
- Estrutura preparada para PID
- Aplicação final desacoplada:
  `left = baseSpeed - correction`
  `right = baseSpeed + correction`

Esta etapa estabelece a fundação matemática do controlo diferencial.

---

### 05 - Control Loop Não Bloqueante ✅

Introdução de scheduler fixo (~100Hz) para:

- Atualização consistente de controlo
- Preparação para sensores de alta frequência
- Estabilidade futura do PID
- Arquitetura modular para múltiplos subsistemas

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

**Parâmetros atuais:**

- `LINE_KP = 0.04` (força da correção)
- `PWM_BASE = 55` (velocidade base)
- `MAX_PWM_CORRECTION = 40` (correção máxima)

> ⚠️ **Nota crítica:** O sinal da correção foi **invertido** no eixo (`pwmL = PWM_BASE - correction`), após testes empíricos.

---

### 07 - Integração com Raspberry Pi ⏳

Comunicação serial com Raspberry Pi.
Delegação de processamento avançado (AI, visão, estratégia).
Arquitetura híbrida ESP32 + processamento externo.

---

## 🚀 Próximos Passos

- [ ] Testar curvas mais apertadas (raio pequeno)
- [ ] Testar cruzamentos (`WideLine`)
- [ ] Testar base (`Arrived`)
- [ ] Integrar com `ArgusMaze` (máquina de estados para labirinto)
- [ ] Adicionar comunicação com Raspberry Pi
- [ ] Implementar visão computacional / IA

---

## 🧪 Testes Realizados

| Teste                         | Resultado      |
| ----------------------------- | -------------- |
| Motores (PWM direto)          | ✅             |
| Encoders (contagem)           | ✅             |
| PID de velocidade (alvo 220)  | ✅ ±5 pulsos/s |
| Segue linha reta              | ✅             |
| Segue linha com curvas suaves | ✅             |
| Deteção de base (`Arrived`)   | ✅             |
| Recuperação (`Searching`)     | ✅             |
| Sinal da correção             | ✅ (invertido) |

---

## 📜 Licença

MIT License
