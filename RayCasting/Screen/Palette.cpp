#include <algorithm>
#include <simd/simd.h>

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
    uint32_t af = (fg & maskAlpha) >> 24;

    simd::uint3 rgbB{
        (bg & maskRed) >> 16,
        (bg & maskGreen) >> 8,
        (bg & maskBlue)
    };
    simd::uint3 rgbF{
        (fg & maskRed) >> 16,
        (fg & maskGreen) >> 8,
        (fg & maskBlue)
    };

    uint32_t a;
    simd::uint3 rgb;

    switch (mode) {
        case BlendMode::normal:
            a = std::min(FF, ab + af);
            rgb = (rgbF * af + rgbB * (FF - af)) / FF;
            break;
        case BlendMode::add:
            a = std::min(FF, ab + af);
            rgb = simd::min(simd::uint3{FF, FF, FF}, rgbF * af / FF + rgbB);
            break;
        case BlendMode::multipy:
            a = std::min(FF, ab + af);
            rgb = (FF - (FF - rgbF) * af / FF) * rgbB / FF;
            break;
    }

    return (a << 24) | (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
}

} // namespace RC::Palette
