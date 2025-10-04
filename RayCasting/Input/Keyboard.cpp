#include "Keyboard.hpp"

namespace RC::Keyboard {

std::array<bool, 128> keys;

void keyDown(uint16_t code) {
    keys[code] = true;
}

void keyUp(uint16_t code) {
    keys[code] = false;
}

} // namespace RC::Keyboard
