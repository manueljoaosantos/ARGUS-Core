# 02 - 8 Digital Controlado por Encoder ♾️

Segunda fase do desenvolvimento do ARGUS-Core.

Nesta etapa o robô executa um movimento em forma de "8" digital utilizando controlo por pulsos dos encoders.

---

## 🎯 Objetivo

- Implementar controlo de movimento por distância (pulsos)
- Criar curvas controladas por diferença de velocidade
- Validar repetibilidade do movimento
- Eliminar dependência exclusiva de tempo (delay)

---

## 🧠 Estratégia

O "8" é composto por:

1. Curva para a direita
2. Pequena reta
3. Curva para a esquerda
4. Pequena reta

Cada movimento termina quando o número de pulsos do encoder atinge um valor alvo.

---

## ⚙️ Parâmetros Ajustáveis

```cpp
int velReta = 150;
int velCurvaRapido = 160;
int velCurvaLento  = 80;
```

E:

```cpp
curvaDireita(1200);
andarReto(600);
curvaEsquerda(1200);
andarReto(600);
```

Ajustando estes valores é possível:

- Apertar ou alargar o 8
- Tornar curvas mais suaves
- Ajustar proporcionalidade

---

## 📈 Resultado Esperado

- Movimento repetível
- Curvas simétricas
- Diferença mínima entre contagens dos encoders
- Estabilidade mecânica validada

---

## ⚠️ Limitação Atual

Esta versão ainda utiliza:

- `while()` bloqueante
- Pequenos `delay()` de estabilização

Isto será removido na próxima fase.

---

## 🚀 Próxima Etapa

03 - Máquina de Estados  
Remover delays e loops bloqueantes e implementar controlo temporal com `millis()`.
