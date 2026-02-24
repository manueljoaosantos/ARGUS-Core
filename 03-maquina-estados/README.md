# ARGUS-Core – Advanced Motion Control 🚀

Base locomotora da plataforma robótica **ARGUS**, desenvolvida com **ESP32-S3**, controlo por encoders e arquitetura não bloqueante baseada em máquina de estados.

Esta versão implementa controlo inteligente de movimento com:

- Máquina de estados não bloqueante
- Correção proporcional (P)
- Aceleração progressiva (rampa)
- Desaceleração antecipada (soft stop)
- Navegação por pulsos de encoder (sem delay)

---

## 🎬 Demonstração

📺 Vídeo da demo:

👉 https://www.youtube.com/watch?v=HQcKYufmBrE

Neste vídeo o ARGUS executa movimento controlado com:

- Arranque suave
- Correção automática de trajetória
- Travagem progressiva antes do alvo
- Movimento repetível e estável

---

## 🧠 Arquitetura Implementada

### 🔹 Máquina de Estados

Estados principais:

- `ANDAR`
- `RODAR`

Sem uso de:

- `delay()`
- `while()` bloqueante

---

### 🔹 Correção Proporcional

Durante o movimento reto:

- erro = contadorDir - contadorEsq
- correção = Kp \* erro

A velocidade de cada roda é ajustada dinamicamente para manter trajetória estável.

---

### 🔹 Aceleração Progressiva

Sistema de rampa:

- velAtual → aproxima-se gradualmente de velAlvo

Evita:

- Patinagem
- Picos de corrente
- Oscilação mecânica

---

### 🔹 Desaceleração Antecipada

Zona de travagem:
ZONA_TRAVAGEM = 400 pulsos

A velocidade é reduzida proporcionalmente à distância restante antes do alvo.

Resultado:

- Paragem suave
- Alta precisão
- Movimento profissional

---

## ⚙️ Hardware Utilizado

- ESP32-S3
- Driver TB6612
- 2 Motores DC
- 2 Encoders quadratura

### 📌 Mapeamento de Pinos

| Função             | GPIO |
| ------------------ | ---- |
| AIN1               | 4    |
| AIN2               | 5    |
| PWMA               | 12   |
| BIN1               | 13   |
| BIN2               | 14   |
| PWMB               | 15   |
| Encoder Direito A  | 16   |
| Encoder Direito B  | 17   |
| Encoder Esquerdo A | 21   |
| Encoder Esquerdo B | 42   |

---

## 🚀 Próximas Etapas

- Integração do sensor de linha **Pololu QTR-8A**
- Implementação de PID completo
- Navegação por coordenadas
- Integração com Raspberry Pi
- Inteligência artificial embarcada

---

## 📈 Estado do Projeto

ARGUS-Core encontra-se agora numa fase estável e modular, preparado para receber sensores e camadas de inteligência superiores sem necessidade de refatoração estrutural.

---

**Projeto ARGUS**  
Plataforma robótica evolutiva com arquitetura limpa, modular e escalável.
