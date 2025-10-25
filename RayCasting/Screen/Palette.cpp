#include <algorithm>
#include <simd/simd.h>

#include "Palette.hpp"

namespace RC::Palette {

constexpr uint32_t maskRed = 0xFF0000;
constexpr uint32_t maskGreen = 0xFF00;
constexpr uint32_t maskBlue = 0xFF;

constexpr simd::uint3 maxRGB = {0xFF, 0xFF, 0xFF};

uint32_t color = white;

uint32_t blend(uint32_t colorA, uint32_t colorB, uint32_t alpha, BlendMode mode) {
    simd::uint3 rgbA{(colorA & maskRed) >> 16,
                     (colorA & maskGreen) >> 8,
                     (colorA & maskBlue)};

    simd::uint3 rgbB{(colorB & maskRed) >> 16,
                     (colorB & maskGreen) >> 8,
                     (colorB & maskBlue)};

    simd::uint3 rgb;

    switch (mode) {
        case BlendMode::normal:
            rgb = (rgbA * (0xFF - alpha) + rgbB * alpha) / 0xFF;
            break;
        case BlendMode::add:
            rgb = simd::min(maxRGB, rgbA + rgbB * alpha / 0xFF);
            break;
        case BlendMode::multipy:
            rgb = rgbA * (0xFF - (0xFF - rgbB) * alpha / 0xFF) / 0xFF;
            break;
    }

    return (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
}

} // namespace RC::Palette
