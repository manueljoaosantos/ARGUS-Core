# 05 - Non-Blocking Control Loop

## 🎯 Objetivo

Introduzir um sistema de controlo temporal determinístico baseado em `millis()`,
garantindo uma frequência fixa de atualização (~100Hz) para o núcleo de movimento.

Esta fase elimina a dependência da velocidade natural do `loop()` e prepara o ARGUS
para sensores de alta frequência e controlo PID estável.

---

## 🧠 Problema da Fase Anterior

Na fase 04, apesar da arquitetura de movimento estar corretamente estruturada,
o controlo ainda dependia da velocidade do `loop()`.

Isso significa que:

- O tempo entre atualizações variava
- Prints no Serial alteravam o comportamento
- Sensores futuros poderiam introduzir jitter
- O PID não teria estabilidade temporal

Para sistemas de navegação autónoma, isso é inaceitável.

---

## 🏗 Solução Implementada

Introdução de um scheduler baseado em `millis()`:

````cpp
unsigned long lastControlUpdate = 0;
const unsigned long CONTROL_INTERVAL = 10; // 100Hz

void loop() {
    unsigned long now = millis();

    if (now - lastControlUpdate >= CONTROL_INTERVAL) {
        lastControlUpdate = now;
        controlUpdate();
    }
}

## ⏱ Agora

- O controlo executa a cada **10ms**
- A frequência é previsível
- O comportamento é determinístico

---

## 🔄 Nova Arquitetura

Separação clara entre:

### `loop()`

Responsável apenas pelo agendamento.

### `controlUpdate()`

Responsável por:

- Máquina de estados
- Cálculo de correção
- Atualização da rampa
- Aplicação diferencial aos motores

### 📌 Fluxo

```text
loop()
└── scheduler (100Hz)
└── controlUpdate()
├── stateManager()
├── computeCorrection()
├── updateBaseSpeedRamp()
└── applyMotorOutput()

````

---

## 🚀 O Que Esta Fase Garante

- ✅ Frequência de controlo fixa
- ✅ Comportamento determinístico
- ✅ Estabilidade futura do PID
- ✅ Separação clara entre agendamento e lógica
- ✅ Base sólida para navegação autónoma

---

## 📈 Benefícios Obtidos

- Redução de jitter
- Maior previsibilidade dinâmica
- Arquitetura modular escalável
- Preparação para sensores rápidos (QTR-8A)
- Compatibilidade com integração AI futura

---

## ⚙ Frequência de Controlo

Atualmente configurado para:

- 100Hz (intervalo de 10ms)

Este valor oferece:

- Resposta suficientemente rápida
- Estabilidade mecânica
- Boa base para PID de line-follow

---

## 🔜 Próxima Fase

### 06 - Line Following (QTR-8A)

Integração do sensor Pololu QTR-8A para:

- Leitura da posição da linha
- Cálculo do erro relativo ao centro
- Implementação de controlo PID
- Navegação autónoma em pista

---

## 🧩 Papel Desta Fase na Evolução do ARGUS

Se a fase 04 estruturou o modelo matemático do movimento,

a fase 05 introduz o **tempo como elemento controlado**.

O ARGUS passa de:

> **Sistema reativo**  
> para  
> **Sistema de controlo determinístico**

Esta é a base de qualquer robô competitivo.
