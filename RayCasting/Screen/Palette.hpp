#ifndef Palette_hpp
#define Palette_hpp

#include <cstdint>

namespace RC::Palette {

constexpr uint32_t BLACK = 0xFF000000;
constexpr uint32_t WHITE = 0xFFFFFFFF;
constexpr uint32_t RED = 0xFFFF0000;
constexpr uint32_t GREEN = 0xFF00FF00;
constexpr uint32_t BLUE = 0xFF0000FF;
constexpr uint32_t YELLOW = 0xFFFFFF00;
constexpr uint32_t CYAN = 0xFF00FFFF;
constexpr uint32_t MAGENTA = 0xFFFF00FF;

constexpr uint32_t MASK_ALPHA = 0xFF000000;
constexpr uint32_t MASK_RED = 0xFF0000;
constexpr uint32_t MASK_GREEN = 0xFF00;
constexpr uint32_t MASK_BLUE = 0xFF;

constexpr uint32_t BEIGE_LIGHTER = 0xFFEDE8D0;
constexpr uint32_t BEIGE_LIGHT = 0xFFC9C5B1;
constexpr uint32_t BEIGE_DARK = 0xFF787569;
constexpr uint32_t BEIGE_DARKER = 0xFF4F4D46;

constexpr uint32_t GRAY_LIGHTER = 0xFFD3D3D3;
constexpr uint32_t GRAY_LIGHT = 0xFF878787;
constexpr uint32_t GRAY_DARK = 0xFF5E5E5E;
constexpr uint32_t GRAY_DARKER = 0xFF363636;

constexpr uint32_t GUNMETAL_GRAY_LIGHTER = 0xFFB9D9EB;
constexpr uint32_t GUNMETAL_GRAY_LIGHT = 0xFF628699;
constexpr uint32_t GUNMETAL_GRAY_DARK = 0xFF353E43;
constexpr uint32_t GUNMETAL_GRAY_DARKER = 0xFF041E2B;

enum class BlendMode {
    normal,
    add
};

extern uint32_t color;

extern BlendMode blendMode;

void setColor(uint32_t c);

void setRGB(uint32_t c);

void setAlpha(uint32_t a);

void setBlendMode(BlendMode mode);

uint32_t rgb(uint32_t bg, uint32_t fg);

uint32_t alpha(uint32_t c, uint32_t a);

uint32_t blend(uint32_t bg, uint32_t fg = color, BlendMode mode = blendMode);

} // namespace RC::Palette

#endif /* Palette_hpp */
