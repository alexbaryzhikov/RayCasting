//
//  Canvas.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 04.09.25.
//

#ifndef Canvas_hpp
#define Canvas_hpp

#include <cstdint>

void* canvasData();

void fillCanvas(uint32_t color);

void point(int x, int y);

void point(int x, int y, uint32_t alpha);

#endif /* Canvas_hpp */
