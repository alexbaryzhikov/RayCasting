//
//  Text.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 14.09.25.
//

#include <array>
#include <cmath>
#include <cstdint>

#include "Canvas.hpp"
#include "Config.h"
#include "Text.hpp"

namespace RC {

static std::array<uint32_t, FONT_MAP_SIZE> font;

void* fontData() {
    return font.data();
}

void drawFontMap() {
    for (int y = 0; y < FONT_MAP_HEIGHT; ++y) {
        for (int x = 0; x < FONT_MAP_WIDTH; ++x) {
            setColor(font[x + y * FONT_MAP_WIDTH]);
            point(x, y);
        }
    }
}

void drawChar(float x, float y, const char c) {
    int canvasX = round(x);
    int canvasY = round(y);
    int glyphX = (c & 0x0F) * GLYPH_WIDTH;
    int glyphY = (c >> 4) * GLYPH_HEIGHT;
    for (int j = 0; j < GLYPH_HEIGHT; ++j) {
        for (int i = 0; i < GLYPH_WIDTH; ++i) {
            uint32_t color = font[glyphX + i + (glyphY + j) * FONT_MAP_WIDTH];
            setAlpha(color & 0xFF);
            point(canvasX + i, canvasY + j);
        }
    }
}

void drawText(float x, float y, const std::string& text, uint32_t textColor) {
    uint32_t lastColor = fgColor;
    setColor(textColor);
    for (int i = 0; i < text.size(); ++i) {
        drawChar(x + i * GLYPH_WIDTH, y, text[i]);
    }
    setColor(lastColor);
}

} // namespace RC
