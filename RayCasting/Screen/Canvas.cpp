//
//  Canvas.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 04.09.25.
//

#include <array>

#include "Canvas.hpp"

#include "Config.h"
#include "Palette.hpp"

namespace RC {

std::array<uint32_t, CANVAS_SIZE> canvas;

const void* canvasBytes() {
    return canvas.data();
}

void fillCanvas(uint32_t color) {
    canvas.fill(color);
}

void point(int x, int y) {
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT) {
        uint32_t& c = canvas[x + y * CANVAS_WIDTH];
        c = blend(c);
    }
}

void point(int x, int y, uint32_t alpha) {
    setAlpha(alpha);
    point(x, y);
}

} // namespace RC
