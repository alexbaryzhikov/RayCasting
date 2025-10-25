#ifndef Text_hpp
#define Text_hpp

#include <array>
#include <string>

#include "Config.h"
#include "Palette.hpp"

namespace RC::Text {

extern std::array<uint32_t, FONT_MAP_SIZE> font;

void drawFontMap();

void drawChar(float x, float y, const char c);

void draw(float x, float y, const std::string& text, uint32_t textColor = Palette::color);

} // namespace RC::Text

#endif /* Text_hpp */
