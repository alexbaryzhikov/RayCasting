//
//  Palette.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 04.09.25.
//

#include <algorithm>

#include "Palette.hpp"

uint32_t color = WHITE;

BlendMode blendMode = BlendMode::normal;

void setColor(uint32_t c) { color = c; }

void setRgb(uint32_t c) { color = rgb(color, c); }

void setAlpha(uint32_t a) { color = alpha(color, a); }

void setBlendMode(BlendMode mode) { blendMode = mode; }

uint32_t rgb(uint32_t bg, uint32_t fg) { return (fg & ~MASK_ALPHA) | (bg & MASK_ALPHA); }

uint32_t alpha(uint32_t c, uint32_t a) { return (c & ~MASK_ALPHA) | (a << 24); }

uint32_t blend(uint32_t bg, uint32_t fg, BlendMode mode) {
    constexpr uint32_t FF = 0xFF;

    uint32_t ab = (bg & MASK_ALPHA) >> 24;
    uint32_t rb = (bg & MASK_RED) >> 16;
    uint32_t gb = (bg & MASK_GREEN) >> 8;
    uint32_t bb = (bg & MASK_BLUE);

    uint32_t af = (fg & MASK_ALPHA) >> 24;
    uint32_t rf = (fg & MASK_RED) >> 16;
    uint32_t gf = (fg & MASK_GREEN) >> 8;
    uint32_t bf = (fg & MASK_BLUE);

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
