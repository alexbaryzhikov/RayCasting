#ifndef Keyboard_hpp
#define Keyboard_hpp

#include <array>

namespace RC::Keyboard {

// Numbers
constexpr uint16_t KEY_0 = 29;
constexpr uint16_t KEY_1 = 18;
constexpr uint16_t KEY_2 = 19;
constexpr uint16_t KEY_3 = 20;
constexpr uint16_t KEY_4 = 21;
constexpr uint16_t KEY_5 = 23;
constexpr uint16_t KEY_6 = 22;
constexpr uint16_t KEY_7 = 26;
constexpr uint16_t KEY_8 = 28;
constexpr uint16_t KEY_9 = 25;

// Letters
constexpr uint16_t KEY_A = 0;
constexpr uint16_t KEY_B = 11;
constexpr uint16_t KEY_C = 8;
constexpr uint16_t KEY_D = 2;
constexpr uint16_t KEY_E = 14;
constexpr uint16_t KEY_F = 3;
constexpr uint16_t KEY_G = 5;
constexpr uint16_t KEY_H = 4;
constexpr uint16_t KEY_I = 34;
constexpr uint16_t KEY_J = 38;
constexpr uint16_t KEY_K = 40;
constexpr uint16_t KEY_L = 37;
constexpr uint16_t KEY_M = 46;
constexpr uint16_t KEY_N = 45;
constexpr uint16_t KEY_O = 31;
constexpr uint16_t KEY_P = 35;
constexpr uint16_t KEY_Q = 12;
constexpr uint16_t KEY_R = 15;
constexpr uint16_t KEY_S = 1;
constexpr uint16_t KEY_T = 17;
constexpr uint16_t KEY_U = 32;
constexpr uint16_t KEY_V = 9;
constexpr uint16_t KEY_W = 13;
constexpr uint16_t KEY_X = 7;
constexpr uint16_t KEY_Y = 16;
constexpr uint16_t KEY_Z = 6;

// Special symbols
constexpr uint16_t KEY_EQUALS = 24;
constexpr uint16_t KEY_MINUS = 27;
constexpr uint16_t KEY_RIGHTBRACKET = 30;
constexpr uint16_t KEY_LEFTBRACKET = 33;
constexpr uint16_t KEY_APOSTROPHE = 39;
constexpr uint16_t KEY_SEMICOLON = 41;
constexpr uint16_t KEY_FRONTSLASH = 42;
constexpr uint16_t KEY_COMMA = 43;
constexpr uint16_t KEY_BACKSLASH = 44;
constexpr uint16_t KEY_PERIOD = 47;
constexpr uint16_t KEY_TAB = 48;
constexpr uint16_t KEY_SPACE = 49;
constexpr uint16_t KEY_BACKAPOSTROPHE = 50;

// Modifiers
constexpr uint16_t KEY_COMMAND = 55;
constexpr uint16_t KEY_SHIFT = 56;
constexpr uint16_t KEY_OPTION = 58;
constexpr uint16_t KEY_CONTROL = 59;

// Controls
constexpr uint16_t KEY_RETURN = 36;
constexpr uint16_t KEY_DELETE = 51;
constexpr uint16_t KEY_ESCAPE = 53;
constexpr uint16_t KEY_CAPSLOCK = 57;

// Arrows
constexpr uint16_t KEY_LEFT = 123;
constexpr uint16_t KEY_RIGHT = 124;
constexpr uint16_t KEY_DOWN = 125;
constexpr uint16_t KEY_UP = 126;

// Modifier flags
constexpr uint32_t FLAG_CAPSLOCK = 1 << 16;   // Set if Caps Lock key is pressed.
constexpr uint32_t FLAG_SHIFT = 1 << 17;      // Set if Shift key is pressed.
constexpr uint32_t FLAG_CONTROL = 1 << 18;    // Set if Control key is pressed.
constexpr uint32_t FLAG_OPTION = 1 << 19;     // Set if Option or Alternate key is pressed.
constexpr uint32_t FLAG_COMMAND = 1 << 20;    // Set if Command key is pressed.
constexpr uint32_t FLAG_NUMERICPAD = 1 << 21; // Set if any key in the numeric keypad is pressed.
constexpr uint32_t FLAG_HELP = 1 << 22;       // Set if the Help key is pressed.
constexpr uint32_t FLAG_FUNCTION = 1 << 23;   // Set if any function key is pressed.

extern std::array<bool, 128> keys;

void keyDown(uint16_t code);

void keyUp(uint16_t code);

void flagsChanged(uint64_t flags);

} // namespace RC::Keyboard

#endif /* Keyboard_hpp */
