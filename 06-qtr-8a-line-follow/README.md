# ARGUS Line Follower – Demo Vídeo

Este diretório contém o vídeo demonstrativo da versão atual do ARGUS, o robô **line follower** baseado em ESP32-S3 com controlo PD e estrutura de movimento determinístico (100 Hz), otimizado para estabilidade máxima e navegação em curvas fechadas, incluindo curvas de 90°.

📹 **Demo Vídeo:**  
https://www.youtube.com/shorts/tEfyto_uwwY

---

## 🎯 Descrição da Demo

Neste vídeo mostramos:

- O ARGUS a seguir uma linha preta com alta estabilidade;
- Curvas de 90° executadas com controlo suave e sem perda de linha;
- Comportamento robusto mesmo em mudanças de direção súbitas;
- Sistema PD (Proporcional-Derivativo) para correção angular;
- Control loop não bloqueante a 100 Hz;
- Desaceleração antecipada em curvas para garantir zero perdas.

Este vídeo ilustra a eficácia da arquitetura atual no controlo de movimento do robô, especialmente em trajetórias desafiantes como esquinas de 90°.

---

## 📌 Objetivos da Versão

Esta implementação prioriza:

- ✔ Estabilidade máxima antes de velocidade;
- ✔ Reação suave ao erro lateral;
- ✔ Recuperação robusta quando a linha é perdida temporariamente;
- ✔ Desaceleração automática em curvas fortes;
- ✔ Base sólida para evolução para maze solving.

---

## 🧩 Arquitetura Técnica

O projeto ARGUS-Core está organizado em fases, e esta demo apresenta principalmente o comportamento das fases:

- **04 – Motion Control Core (Refactor):**
  - Separação de velocidades alvo e atuais
  - Layout de controlo diferencial
- **05 – Non-Blocking Control Loop:**
  - Loop determinístico de controlo a 100 Hz
- **06 – QTR-8A Line Follow (Estável):**
  - Leitura calibrada do array QTR-8A
  - Cálculo de erro de posição
  - PD control para correção dinâmica
  - Downspeed apropriado em curvas

---

## 📌 Como Reproduzir

1. **Ligar sensores e encoders** conforme o esquema do projeto.
2. **Carregar o firmware** no ESP32-S3.
3. **Ajustar sensor e altura** para ~7 – 8 mm acima da pista.
4. **Colocar fita preta (19 mm)** sobre fundo claro.
5. Testar e observar comportamento de seguimento de linha.

---

## 🧠 Observações da Demo

- O robô executa curvas sem perder a linha;
- A desaceleração em curvas é perceptível e eficaz;
- O controlo PD mantém o robô estável mesmo sob variações inesperadas;
- Não se observa “oversteer” ou oscilações excessivas.

---

## 🧭 Próximos Passos

Com esta base estável, o projeto está pronto para evoluções como:

- 🧩 Detecção de interseções e T-junctions
- 🧠 Algoritmos de maze solving (Left-Hand, Right-Hand, FloodFill)
- 📈 Telemetria e parâmetros ajustáveis em tempo real
- 🤖 Integração com Raspberry Pi ou outro co-processador

---

## 📜 Licença

Este projeto está licenciado sob os termos definidos no ficheiro LICENSE do repositório principal.
