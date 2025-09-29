//
//  ScreenTests.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 14.09.25.
//

#include <cmath>

#include "ScreenTests.hpp"

#include "Config.h"
#include "Palette.hpp"
#include "Primitives.hpp"

namespace RC {

void drawAnimatedBox() {
    const float size = 32;
    static float x = 0.f;
    static float y = 0.f;
    static float dx = 1.f;
    static float dy = 1.f;
    static uint32_t color = RED;

    setColor(color);
    boxFill(x, y, size, size);

    x += dx;
    y += dy;
    bool bounced = false;
    if (x <= 0 || x + size >= CANVAS_WIDTH) {
        dx *= -1.f;
        bounced = true;
    }
    if (y <= 0 || y + size >= CANVAS_HEIGHT) {
        dy *= -1.f;
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

void drawFrame() {
    setColor(BLUE);
    line(0, 0, CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1);
    setColor(GREEN);
    line(CANVAS_WIDTH - 1, 0, 0, CANVAS_HEIGHT - 1);
    setColor(RED);
    box(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
}

void drawCheckers() {
    setColor(CYAN);
    const float size = 16;
    const bool oddBoxesPerRow = int(ceil(CANVAS_WIDTH / size)) & 1;
    float x = 0;
    float y = 0;
    bool flag = true;
    while (y < CANVAS_HEIGHT) {
        while (x < CANVAS_WIDTH) {
            if (flag) {
                boxFill(x, y, size, size);
            }
            flag = !flag;
            x += size;
        }
        if (!oddBoxesPerRow) {
            flag = !flag;
        }
        x = 0;
        y += size;
    }
}

} // namespace RC
