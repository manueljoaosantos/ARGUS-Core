# ARGUS-Core 🤖

Plataforma robótica móvel baseada em ESP32-S3, núcleo locomotor do ecossistema ARGUS.

---

## 🎯 Objetivo

Desenvolver a base móvel inteligente da plataforma ARGUS com arquitetura modular e evolutiva:

- Controlo de motores DC (TB6612FNG)
- Encoders em quadratura
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
├── 02-maquina-estados/
│   ├── README.md
│   └── src/
│       └── main.cpp
│
├── 03-qtr-8a-line-follow/
│   ├── README.md
│   └── src/
│       └── main.cpp
│
└── 04-raspberry-ai-integration/
    ├── README.md
    └── docs/
```

---

## 🧩 Fases de Desenvolvimento

### 01 - Base Motores + Encoders

Controlo direto dos motores com leitura de pulsos e movimento por distância.

### 02 - Máquina de Estados

Remoção de `delay()` e implementação de controlo temporal com `millis()`.

### 03 - Line Following

Integração do sensor Pololu QTR-8A para navegação autónoma.

### 04 - Integração com Raspberry Pi

Comunicação e processamento inteligente externo.

---

## 📜 Licença

MIT License
