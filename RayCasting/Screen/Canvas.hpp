//
//  Canvas.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 04.09.25.
//

#ifndef Canvas_hpp
#define Canvas_hpp

#include <cstdint>

namespace RC::Canvas {

const void* bytes();

void fill(uint32_t color);

void point(int x, int y);

void point(int x, int y, uint32_t alpha);

void line(float x0, float y0, float x1, float y1);

void box(float x, float y, float w, float h);

void boxFill(float x, float y, float w, float h);

} // namespace RC::Canvas

#endif /* Canvas_hpp */
