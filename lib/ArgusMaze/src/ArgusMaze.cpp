#include "argus/ArgusMaze.h"
namespace ArgusMaze {
    namespace { State currentState = State::Stopped; }
    void begin() { currentState = State::Stopped; }
    void update() {
        // A mÃ¡quina de estados serÃ¡ migrada do exemplo original numa etapa posterior.
    }
}