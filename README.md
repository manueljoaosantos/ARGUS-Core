# ARGUS-Core 🤖

Plataforma robótica móvel baseada em ESP32-S3, núcleo locomotor do ecossistema ARGUS.

---

## 🎯 Objetivo

Desenvolver a base móvel inteligente da plataforma ARGUS com arquitetura modular e evolutiva:

- Controlo de motores DC (TB6612FNG)
- Encoders em quadratura
- 8 Digital por Encoder
- Máquina de estados (sem uso de delay)
- Sensor de linha (QTR-8A)
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
├── 01-base-motores-encoders/
│   ├── README.md
│   └── src/
│       └── main.cpp
│
├── 02-8-digital-encoder/
│   ├── README.md
│   └── src/
│       └── main.cpp
│
├── 03-maquina-estados/
│   ├── README.md
│   └── src/
│       └── main.cpp
│
├── 04-motion-core-refactor/
│   ├── README.md
│   └── src/
│       └── main.cpp
│
├── 05-non-blocking-control-loop/
│   ├── README.md
│   └── src/
│       └── main.cpp
│
├── 06-qtr-8a-line-follow/
│   ├── README.md
│   └── src/
│       └── main.cpp
│
└── 07-raspberry-ai-integration/
    ├── README.md
    └── docs/
```

---

## 🔌 Ligações de Hardware

### 1. Controlador de motores TB6612FNG (HW-048)

O controlo dos dois motores DC é realizado através do controlador TB6612FNG.

#### Sinais de controlo

| Sinal TB6612FNG | GPIO ESP32-S3 | Função             |
| --------------- | ------------: | ------------------ |
| PWMA            |       GPIO 12 | PWM do canal A     |
| AIN1            |        GPIO 4 | Direção do canal A |
| AIN2            |        GPIO 5 | Direção do canal A |
| PWMB            |       GPIO 15 | PWM do canal B     |
| BIN1            |       GPIO 13 | Direção do canal B |
| BIN2            |       GPIO 14 | Direção do canal B |

#### Alimentação

| Pino | Ligação     | Observações                                                                                             |
| ---- | ----------- | ------------------------------------------------------------------------------------------------------- |
| VCC  | 3,3 V       | Alimentação lógica                                                                                      |
| VM   | 5 V         | Alimentação dos motores; confirmar a compatibilidade com os motores e a capacidade de corrente da fonte |
| GND  | Massa comum | Referência comum entre ESP32-S3 e controlador                                                           |
| NC   | Sem ligação | Não ligar                                                                                               |

**Nota:** a origem dos 5 V aplicados a VM deve ser documentada quando estiver confirmada.

#### Saídas para os motores

| Saída     | Motor/lado     |
| --------- | -------------- |
| AO1 / AO2 | Motor direito  |
| BO1 / BO2 | Motor esquerdo |

A correspondência do sentido de rotação deve ser validada durante os testes de movimento.

### 2. Encoders

Os encoders permitem medir a rotação das rodas e suportar o controlo de movimento por pulsos.

| Encoder  | Sinal | GPIO ESP32-S3 |
| -------- | ----- | ------------: |
| Esquerdo | C1    |       GPIO 21 |
| Esquerdo | C2    |       GPIO 42 |
| Direito  | C1    |       GPIO 16 |
| Direito  | C2    |       GPIO 17 |

### 3. Configuração no firmware

A configuração dos pinos encontra-se em:

`include/argus/ArgusConfig.h`

A cablagem física e as definições do firmware devem permanecer sincronizadas. Qualquer alteração aos GPIO deve ser refletida neste documento.

### 4. Estado da validação do hardware

- [x] Firmware compilado e carregado no ESP32-S3.
- [x] Inicialização confirmada através do monitor série.
- [x] Movimento dos motores confirmado.
- [x] Leitura dos encoders durante o movimento confirmada.
- [ ] Sentido de rotação de cada roda validado.
- [ ] Alimentação dos motores e corrente disponível verificadas.

## 🧩 Fases de Desenvolvimento

### 01 - Base Motores + Encoders

Controlo direto dos motores com leitura de pulsos e movimento por distância.
Implementação inicial de aceleração simples e testes de direção.

---

### 02 - 8 Digital Controlado por Encoder ♾️

Execução de trajetória em forma de "8" utilizando controlo por pulsos.
Validação de simetria, estabilidade mecânica e consistência dos encoders.

---

### 03 - Máquina de Estados

Remoção de `delay()` e introdução de controlo baseado em `millis()`.
Separação de comportamentos (ANDAR / RODAR) e organização da lógica do movimento.

---

### 04 - Motion Control Core (Refatoração Estrutural)

Reestruturação do sistema de movimento com:

- Separação entre `baseSpeedTarget` e `baseSpeedCurrent`
- Implementação de rampa independente da correção
- Estrutura preparada para PID
- Aplicação final desacoplada:  
  `left = baseSpeed - correction`  
  `right = baseSpeed + correction`

Esta etapa estabelece a fundação matemática do controlo diferencial.

---

### 05 - Control Loop Não Bloqueante

Introdução de scheduler fixo (~100Hz) para:

- Atualização consistente de controlo
- Preparação para sensores de alta frequência
- Estabilidade futura do PID
- Arquitetura modular para múltiplos subsistemas

---

### 06 - Line Following (QTR-8A)

Integração do sensor Pololu QTR-8A.
Cálculo de posição da linha, erro relativo ao centro e correção dinâmica.
Implementação futura de controlo PID.

---

### 07 - Integração com Raspberry Pi

Comunicação serial com Raspberry Pi.
Delegação de processamento avançado (AI, visão, estratégia).
Arquitetura híbrida ESP32 + processamento externo.

---

## 📜 Licença

MIT License
