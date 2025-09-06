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

void drawAnimatedBox() {
    const float size = 32;
    static float x = 0.f;
    static float y = 0.f;
    static float dh = 1.f;
    static float dv = 1.f;
    static uint32_t color = RED;
    setColor(color);
    boxFill(x, y, size, size);
    x += dh;
    y += dv;
    bool bounced = false;
    if (x <= 0 || x + size >= CANVAS_WIDTH) {
        dh *= -1.f;
        bounced = true;
    }
    if (y <= 0 || y + size >= CANVAS_HEIGHT) {
        dv *= -1.f;
        bounced = true;
    }
    if (bounced) {
        switch (color) {
            case RED:
                color = GREEN;
                break;
            case GREEN:
                color = BLUE;
                break;
            case BLUE:
                color = RED;
                break;
        }
    }
}

void drawCheckers() {
    setColor(CYAN);
    const float size = 32;
    int x = 0;
    int y = 0;
    bool flag = true;
    while (y < CANVAS_HEIGHT) {
        while (x < CANVAS_WIDTH) {
            if (flag) {
                boxFill(x, y, size, size);
            }
            flag = !flag;
            x += size;
        }
        flag = !flag;
        x = 0;
        y += size;
    }
}

const void* drawFrame() {
    canvas.fill(BLACK);
    drawAnimatedBox();
    return canvas.data();
}
