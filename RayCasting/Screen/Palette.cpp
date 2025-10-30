#include <simd/simd.h>

#include "Palette.hpp"

namespace RC::Palette {

constexpr uint32_t maskRed = 0xFF0000;
constexpr uint32_t maskGreen = 0xFF00;
constexpr uint32_t maskBlue = 0xFF;

constexpr simd::uint3 maxRGB = {0xFF, 0xFF, 0xFF};

uint32_t color = white;

uint32_t blend(uint32_t baseColor, uint32_t blendColor, uint32_t alpha, BlendMode mode) {
    simd::uint3 baseRGB{(baseColor & maskRed) >> 16,
                        (baseColor & maskGreen) >> 8,
                        (baseColor & maskBlue)};

    simd::uint3 blendRGB{(blendColor & maskRed) >> 16,
                         (blendColor & maskGreen) >> 8,
                         (blendColor & maskBlue)};

    simd::uint3 rgb;

    switch (mode) {
        case BlendMode::normal:
            rgb = (baseRGB * (0xFF - alpha) + blendRGB * alpha) / 0xFF;
            break;
        case BlendMode::add:
            rgb = simd::min(maxRGB, baseRGB + blendRGB * alpha / 0xFF);
            break;
        case BlendMode::multipy:
            rgb = baseRGB * (0xFF - (0xFF - blendRGB) * alpha / 0xFF) / 0xFF;
            break;
    }

    return (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
}

} // namespace RC::Palette
