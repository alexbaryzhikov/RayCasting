//
//  Canvas.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 04.09.25.
//

#ifndef Canvas_hpp
#define Canvas_hpp

#include <array>

#include "Config.h"

extern std::array<uint32_t, CANVAS_WIDTH * CANVAS_HEIGHT> canvas;

void point(int x, int y);

void point(int x, int y, uint32_t alpha);

#endif /* Canvas_hpp */
