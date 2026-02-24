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
