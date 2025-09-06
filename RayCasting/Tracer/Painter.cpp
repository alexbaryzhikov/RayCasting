//
//  Painter.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 05.09.25.
//

#include "Config.h"

#include "Canvas.hpp"
#include "Painter.hpp"
#include "Palette.hpp"
#include "Primitives.hpp"

const void* drawFrame() {
    canvas.fill(BLACK);
    setColor(RED);
    line(0, 0, CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1);
    setColor(BLUE);
    line(0, CANVAS_HEIGHT - 1, CANVAS_WIDTH - 1, 0);
    setColor(GREEN);
    box(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
    return canvas.data();
}
