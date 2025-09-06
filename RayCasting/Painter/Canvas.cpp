//
//  Canvas.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 04.09.25.
//

#include "Canvas.hpp"
#include "Palette.hpp"

std::array<uint32_t, CANVAS_WIDTH * CANVAS_HEIGHT> canvas;

void point(int x, int y) {
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT) {
        uint32_t& c = canvas[y * CANVAS_WIDTH + x];
        c = blend(c);
    }
}

void point(int x, int y, uint32_t alpha) {
    setAlpha(alpha);
    point(x, y);
}
