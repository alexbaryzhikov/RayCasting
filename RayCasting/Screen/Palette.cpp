#include <algorithm>

#include "Palette.hpp"

namespace RC::Palette {

uint32_t color = white;

BlendMode blendMode = BlendMode::normal;

void setColor(uint32_t c) { color = c; }

void setRgb(uint32_t rgb) { color = withRGB(rgb, color); }

void setAlpha(uint32_t a) { color = withAlpha(a, color); }

void setBlendMode(BlendMode mode) { blendMode = mode; }

uint32_t withRGB(uint32_t rgb, uint32_t c) { return (rgb & maskRGB) | (c & maskAlpha); }

uint32_t withAlpha(uint32_t a, uint32_t c) { return (c & maskRGB) | ((a << 24) & maskAlpha); }

uint32_t blend(uint32_t bg, uint32_t fg, BlendMode mode) {
    constexpr uint32_t FF = 0xFF;

    uint32_t ab = (bg & maskAlpha) >> 24;
    uint32_t rb = (bg & maskRed) >> 16;
    uint32_t gb = (bg & maskGreen) >> 8;
    uint32_t bb = (bg & maskBlue);

    uint32_t af = (fg & maskAlpha) >> 24;
    uint32_t rf = (fg & maskRed) >> 16;
    uint32_t gf = (fg & maskGreen) >> 8;
    uint32_t bf = (fg & maskBlue);

    uint32_t a, r, g, b;

    switch (mode) {
        case BlendMode::normal:
            a = std::min(FF, ab + af);
            r = (af * rf + (FF - af) * rb) / FF;
            g = (af * gf + (FF - af) * gb) / FF;
            b = (af * bf + (FF - af) * bb) / FF;
            break;
        case BlendMode::add:
            a = std::min(FF, ab + af);
            r = std::min(FF, af * rf / FF + rb);
            g = std::min(FF, af * gf / FF + gb);
            b = std::min(FF, af * bf / FF + bb);
            break;
    }
    return (a << 24) | (r << 16) | (g << 8) | b;
}

} // namespace RC::Palette
