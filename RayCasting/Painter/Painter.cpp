//
//  Painter.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 05.09.25.
//

#include "Painter.hpp"
#include "Canvas.hpp"
#include "Helpers.hpp"
#include "Text.hpp"

void* drawFrame() {
    fillCanvas(BLACK);
    drawAnimatedBox();
    drawText(50, 30, "Ray Caster 2000", WHITE);
    return canvasData();
}
