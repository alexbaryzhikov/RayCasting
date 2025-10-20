#include <cmath>

#include "ScreenTests.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Palette.hpp"

namespace RC {

void drawAnimatedBox() {
    const float size = 32;
    static float x = 0.f;
    static float y = 0.f;
    static float dx = 1.f;
    static float dy = 1.f;
    static uint32_t color = Palette::red;

    Canvas::resetClipFrame();
    Canvas::fill(Palette::black);
    Canvas::setClipFrame(x, y, size, size);
    Canvas::fill(color);
    Canvas::resetClipFrame();

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
            case Palette::red:
                color = Palette::green;
                break;
            case Palette::green:
                color = Palette::blue;
                break;
            case Palette::blue:
                color = Palette::red;
                break;
        }
    }
}

void drawFrame() {
    Palette::setColor(Palette::blue);
    Canvas::line(0, 0, CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1);
    Palette::setColor(Palette::green);
    Canvas::line(CANVAS_WIDTH - 1, 0, 0, CANVAS_HEIGHT - 1);
    Palette::setColor(Palette::red);
    Canvas::box(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
}

void drawCheckers() {
    Palette::setColor(Palette::cyan);
    const float size = CANVAS_WIDTH / 16.0f;
    const bool oddBoxesPerRow = int(ceil(CANVAS_WIDTH / size)) & 1;
    float x = 0;
    float y = 0;
    bool flag = true;
    while (y < CANVAS_HEIGHT) {
        while (x < CANVAS_WIDTH) {
            if (flag) {
                Canvas::setClipFrame(x, y, size, size);
                Canvas::fill();
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
    Canvas::resetClipFrame();
}

} // namespace RC
