# 01 - Base Motores + Encoders ⚙️

Primeira fase do desenvolvimento do **ARGUS-Core**.

Nesta etapa é implementado:

- Controlo básico de motores DC via TB6612FNG
- Leitura de encoders em quadratura
- Movimento frente e trás
- Contagem de pulsos para validação de velocidade e direção

---

## 🎯 Objetivo

Estabelecer uma base estável e validada de:

- Controlo de direção dos motores
- Geração de PWM
- Leitura correta dos encoders
- Alinhamento de sinais (positivo para frente)

Esta fase serve como fundação para:

- Movimento por distância
- Correção proporcional
- Máquina de estados
- Segue-linha

---

## 🛠 Hardware Utilizado

- ESP32-S3 (Keyestudio KS5019)
- Driver TB6612FNG
- 2x Motores DC com encoder quadratura
- Fonte 6–7.4V para motores

---

## 🔌 Mapeamento de Pinos

### Motores (TB6612FNG)

| Função | GPIO |
| ------ | ---- |
| AIN1   | 4    |
| AIN2   | 5    |
| PWMA   | 12   |
| BIN1   | 13   |
| BIN2   | 14   |
| PWMB   | 15   |

Canal A → Roda Direita  
Canal B → Roda Esquerda

---

### Encoders

| Encoder  | Canal A | Canal B |
| -------- | ------- | ------- |
| Direito  | 16      | 17      |
| Esquerdo | 21      | 42      |

---

## ⚙️ Funcionamento

- Interrupção configurada apenas no Canal A de cada encoder
- Direção determinada pela leitura do Canal B
- Contadores reiniciados antes de cada movimento
- Movimento controlado por tempo (delay) nesta fase inicial

Exemplo de saída:

```
Frente 3s
Direito: 2422 | Esquerdo: 2374

Tras 3s
Direito: -2389 | Esquerdo: -2393
```

Valores positivos → frente  
Valores negativos → trás

---

## 📈 Resultado Esperado

- Ambos os motores movem-se corretamente
- Contagens semelhantes entre direita e esquerda
- Sistema estável e previsível

---

## 🚀 Próxima Etapa

➡ 02 - Máquina de Estados  
Remover `delay()` e implementar controlo temporal com `millis()`.

---

## 🧠 Estado do Projeto

Base locomotora validada e pronta para evolução.
