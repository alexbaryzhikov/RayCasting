#ifndef Text_hpp
#define Text_hpp

#include <string>

#include "Palette.hpp"

namespace RC::Text {

void* fontBytes();

void drawFontMap();

void drawChar(float x, float y, const char c);

void draw(float x, float y, const std::string& text, uint32_t textColor = Palette::color);

} // namespace RC::Text

#endif /* Text_hpp */
