#include <array>
#include <cmath>
#include <cstdint>

#include "Text.hpp"

#include "Canvas.hpp"

namespace RC::Text {

std::array<uint32_t, FONT_MAP_SIZE> font;

void drawFontMap() {
    for (int y = 0; y < FONT_MAP_HEIGHT; ++y) {
        for (int x = 0; x < FONT_MAP_WIDTH; ++x) {
            Canvas::point(x, y, font[x + y * FONT_MAP_WIDTH]);
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
            if (font[glyphX + i + (glyphY + j) * FONT_MAP_WIDTH] != Palette::black) {
                Canvas::point(canvasX + i, canvasY + j);
            }
        }
    }
}

void draw(float x, float y, const std::string& text, uint32_t textColor) {
    Palette::color = textColor;
    for (int i = 0; i < text.size(); ++i) {
        drawChar(x + i * GLYPH_WIDTH, y, text[i]);
    }
}

} // namespace RC::Text
