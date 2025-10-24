#ifndef Palette_hpp
#define Palette_hpp

#include <cstdint>

namespace RC {

enum class BlendMode {
    normal,
    add,
    multipy,
};

}

namespace RC::Palette {

constexpr uint32_t black = 0xFF000000;
constexpr uint32_t white = 0xFFFFFFFF;
constexpr uint32_t red = 0xFFFF0000;
constexpr uint32_t green = 0xFF00FF00;
constexpr uint32_t blue = 0xFF0000FF;
constexpr uint32_t yellow = 0xFFFFFF00;
constexpr uint32_t cyan = 0xFF00FFFF;
constexpr uint32_t magenta = 0xFFFF00FF;

constexpr uint32_t maskAlpha = 0xFF000000;
constexpr uint32_t maskRGB = 0xFFFFFF;
constexpr uint32_t maskRed = 0xFF0000;
constexpr uint32_t maskGreen = 0xFF00;
constexpr uint32_t maskBlue = 0xFF;

constexpr uint32_t gunmetalLighter = 0xFFB9D9EB;
constexpr uint32_t gunmetalLight = 0xFF628699;
constexpr uint32_t gunmetalDark = 0xFF353E43;
constexpr uint32_t gunmetalDarker = 0xFF041E2B;

extern uint32_t color;

extern BlendMode blendMode;

void setColor(uint32_t c);

void setRGB(uint32_t rgb);

void setAlpha(uint32_t a);

void setBlendMode(BlendMode mode);

uint32_t withRGB(uint32_t rgb, uint32_t c = color);

uint32_t withAlpha(uint32_t a, uint32_t c = color);

uint32_t blend(uint32_t bg, uint32_t fg = color, BlendMode mode = blendMode);

} // namespace RC::Palette

#endif /* Palette_hpp */
