#include <cmath>
#include <numbers>

#include "ScreenTests.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Palette.hpp"

namespace RC {

void drawFrame() {
    Canvas::fill(Palette::black);
    Canvas::line(0, 0, CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1, Palette::blue);
    Canvas::line(CANVAS_WIDTH - 1, 0, 0, CANVAS_HEIGHT - 1, Palette::green);
    Canvas::box(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, Palette::red);
}

void drawLines() {
    Canvas::fill(Palette::black);
    float step = std::numbers::pi / 32;
    float cx = CANVAS_WIDTH / 2.0f;
    float cy = CANVAS_HEIGHT / 2.0f;
    float radius = CANVAS_HEIGHT / 3.0f;
    uint32_t color = Palette::red;
    for (float angle = 0; angle <= std::numbers::pi / 4; angle += step) {
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        Canvas::line(cx + x / 10, cy + y / 10, cx + x, cy + y, color);
        Canvas::line(cx - x / 10, cy + y / 10, cx - x, cy + y, color);
        Canvas::line(cx + x / 10, cy - y / 10, cx + x, cy - y, color);
        Canvas::line(cx - x / 10, cy - y / 10, cx - x, cy - y, color);
        Canvas::line(cx + y / 10, cy + x / 10, cx + y, cy + x, color);
        Canvas::line(cx - y / 10, cy + x / 10, cx - y, cy + x, color);
        Canvas::line(cx + y / 10, cy - x / 10, cx + y, cy - x, color);
        Canvas::line(cx - y / 10, cy - x / 10, cx - y, cy - x, color);
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

void drawCheckers() {
    Canvas::fill(Palette::black);
    Palette::color = Palette::cyan;
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

void drawAnimatedBox() {
    const float size = 32;
    static float x = 0.f;
    static float y = 0.f;
    static float dx = 1.f;
    static float dy = 1.f;
    static uint32_t color = Palette::red;

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

} // namespace RC
