//
//  Text.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 14.09.25.
//

#ifndef Text_hpp
#define Text_hpp

#include <string>

#include "Palette.hpp"

namespace RC {

void* fontData();

void drawFontMap();

void drawChar(float x, float y, const char c);

void drawText(float x, float y, const std::string& text, uint32_t textColor = fgColor);

} // namespace RC

#endif /* Text_hpp */
