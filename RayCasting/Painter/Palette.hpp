//
//  Palette.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 04.09.25.
//

#ifndef Palette_hpp
#define Palette_hpp

#include <cstdint>

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

enum class BlendMode {
    normal,
    add
};

extern uint32_t fgColor;

extern BlendMode blendMode;

void setColor(uint32_t c);

void setRGB(uint32_t c);

void setAlpha(uint32_t a);

void setBlendMode(BlendMode mode);

uint32_t rgb(uint32_t bg, uint32_t fg);

uint32_t alpha(uint32_t c, uint32_t a);

uint32_t blend(uint32_t bg, uint32_t fg = fgColor, BlendMode mode = blendMode);

#endif /* Palette_hpp */
