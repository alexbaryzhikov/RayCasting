#include "Keyboard.hpp"

namespace RC::Keyboard {

std::array<bool, 128> keys;

void keyDown(uint16_t code) {
    keys[code] = true;
}

void keyUp(uint16_t code) {
    keys[code] = false;
}

void flagsChanged(uint64_t flags) {
    keys[KEY_COMMAND] = flags & FLAG_COMMAND;
    keys[KEY_SHIFT] = flags & FLAG_SHIFT;
    keys[KEY_OPTION] = flags & FLAG_OPTION;
    keys[KEY_CONTROL] = flags & FLAG_CONTROL;
}

} // namespace RC::Keyboard
