# 04 - Motion Control Core (Refatoração Estrutural)

## 🎯 Objetivo

Reestruturar o núcleo de controlo de movimento do ARGUS, separando claramente:

- Velocidade base do robô
- Correção diferencial
- Aplicação final aos motores
- Sistema de rampa (aceleração/desaceleração)

Esta fase estabelece a fundação matemática necessária para a futura implementação de PID e Line Following.

---

## 🧠 Problema da Versão Anterior

Nas fases anteriores:

- A rampa de aceleração afetava diretamente as velocidades individuais dos motores.
- A correção proporcional estava misturada com a lógica de PWM.
- Não existia separação clara entre decisão de movimento e aplicação física.

Isso dificultava:

- Implementação de PID
- Ajustes finos
- Evolução do sistema

---

## 🏗 Nova Arquitetura do Movimento

Nesta fase introduzimos três conceitos fundamentais:

### 1️⃣ `baseSpeedTarget`

Velocidade desejada definida pela máquina de estados.

### 2️⃣ `baseSpeedCurrent`

Velocidade real aplicada após rampa progressiva.

### 3️⃣ `correction`

Valor diferencial aplicado após a rampa.

---

## 📐 Modelo Matemático Diferencial

A aplicação final aos motores passa a seguir a fórmula:

- left = baseSpeedCurrent - correction
- right = baseSpeedCurrent + correction

Esta separação permite que:

- A rampa atue apenas sobre a velocidade base
- A correção (futuro PID) não seja distorcida pela aceleração
- O comportamento seja previsível e estável

---

## 🚀 Sistema de Rampa

A rampa atua exclusivamente sobre `baseSpeedCurrent`:

```cpp
if (baseSpeedCurrent < baseSpeedTarget)
    baseSpeedCurrent += passoAceleracao;
else if (baseSpeedCurrent > baseSpeedTarget)
    baseSpeedCurrent -= passoAceleracao;
```

## 🚀 O Que Esta Estrutura Garante

- ✅ Aceleração progressiva
- ✅ Desaceleração suave
- ✅ Eliminação de transições abruptas

---

## 🔄 Fluxo de Execução

1. Máquina de estados define `baseSpeedTarget`
2. Calcula-se `correction`
3. Atualiza-se a rampa (`baseSpeedCurrent`)
4. Aplica-se a fórmula diferencial
5. Envia-se PWM para os motores

---

## 📈 Benefícios Obtidos

- Separação clara de responsabilidades
- Base sólida para PID
- Código mais modular
- Melhor previsibilidade dinâmica
- Preparação para sensores de alta frequência

---

## 🔜 Próxima Fase

### 05 - Control Loop Architecture

Introdução de loop não bloqueante com frequência fixa (~100Hz),  
criando um scheduler de controlo determinístico.

---

## 🧩 Papel Desta Fase na Evolução do ARGUS

Esta etapa marca a transição de:

> **Controlo funcional**  
> para  
> **Arquitetura de movimento profissional**

O ARGUS deixa de ser apenas um robô que anda  
e passa a ter um núcleo de controlo preparado para navegação autónoma.
