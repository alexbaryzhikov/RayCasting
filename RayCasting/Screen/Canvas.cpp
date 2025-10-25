#include <cassert>
#include <cmath>

#include "Canvas.hpp"

namespace RC {

bool operator==(const Frame& a, const Frame& b) {
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

} // namespace RC

namespace RC::Canvas {

constexpr Frame fullFrame = {0, 0, CANVAS_WIDTH, CANVAS_HEIGHT};

std::array<uint32_t, CANVAS_SIZE> canvas;
Frame clipFrame = fullFrame;

const void* bytes() {
    return canvas.data();
}

void setClipFrame(float x, float y, float w, float h) {
    setClipFrame({x, y, w, h});
}

void setClipFrame(Frame frame) {
    assert(frame.x >= 0 && frame.y >= 0 && frame.h > 0 && frame.w > 0 &&
           frame.maxX() <= CANVAS_WIDTH && frame.maxY() <= CANVAS_HEIGHT);
    clipFrame = frame;
}

void resetClipFrame() {
    clipFrame = fullFrame;
}

void fill() {
    fill(Palette::color);
}

void fill(uint32_t color) {
    if (clipFrame == fullFrame) {
        canvas.fill(color);
    } else {
        for (size_t i = clipFrame.y; i < clipFrame.maxY(); ++i) {
            size_t offset = i * CANVAS_WIDTH + clipFrame.x;
            std::fill_n(canvas.begin() + offset, clipFrame.w, color);
        }
    }
}

void point(float x, float y) {
    point(x, y, Palette::color);
}

void point(float x, float y, uint32_t color) {
    if (clipFrame.contains(x, y)) {
        canvas[int(x) + int(y) * CANVAS_WIDTH] = color;
    }
}

void line(float x0, float y0, float x1, float y1) {
    line(x0, y0, x1, y1, Palette::color);
}

void line(float x0, float y0, float x1, float y1, uint32_t color) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    if (abs(dx) < abs(dy)) {
        if (y0 > y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        float slope = dx / dy;
        float y = ceil(y0);
        y1 = floor(y1);
        x0 += (y - y0) * slope + 0.5f;
        do {
            point(x0, y, color);
            x0 += slope;
        } while (++y <= y1);
    } else {
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        float slope = dy / dx;
        float x = ceil(x0);
        x1 = floor(x1);
        y0 += (x - x0) * slope + 0.5f;
        do {
            point(x, y0, color);
            y0 += slope;
        } while (++x <= x1);
    }
}

void box(float x, float y, float w, float h) {
    box(x, y, w, h, Palette::color);
}

void box(float x, float y, float w, float h, uint32_t color) {
    assert(w >= 0.f && h >= 0.f);
    if (w == 0.f || h == 0.f) return;
    w = (w < 1.f ? 1.f : w);
    h = (h < 1.f ? 1.f : h);
    float xStart = round(x);
    float xEnd = round(x + w) - 1;
    float yStart = round(y);
    float yEnd = round(y + h) - 1;
    for (x = xStart; x <= xEnd; ++x) {
        point(x, yStart, color);
        point(x, yEnd, color);
    }
    for (y = yStart + 1; y < yEnd; ++y) {
        point(xStart, y, color);
        point(xEnd, y, color);
    }
}

} // namespace RC::Canvas
