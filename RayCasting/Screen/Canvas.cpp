#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>

#include "Canvas.hpp"

#include "Config.h"
#include "Palette.hpp"

namespace RC {

bool operator==(const Frame& a, const Frame& b) {
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

} // namespace RC

namespace RC::Canvas {

constexpr Frame FULL_FRAME = {0, 0, CANVAS_WIDTH, CANVAS_HEIGHT};

std::array<uint32_t, CANVAS_SIZE> canvas;
Frame clipFrame = FULL_FRAME;

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
    clipFrame = FULL_FRAME;
}

void fill() {
    fill(Palette::color);
}

void fill(uint32_t color) {
    if (clipFrame == FULL_FRAME) {
        canvas.fill(color);
    } else {
        for (size_t i = clipFrame.y; i < clipFrame.maxY(); ++i) {
            size_t offset = i * CANVAS_WIDTH + clipFrame.x;
            std::fill_n(canvas.begin() + offset, clipFrame.w, color);
        }
    }
}

void point(int x, int y) {
    if (x >= clipFrame.x && x < clipFrame.maxX() && y >= clipFrame.y && y < clipFrame.maxY()) {
        uint32_t& c = canvas[x + y * CANVAS_WIDTH];
        c = Palette::blend(c);
    }
}

void point(int x, int y, uint32_t alpha) {
    Palette::setAlpha(alpha);
    point(x, y);
}

void line(float x0, float y0, float x1, float y1) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    if (abs(dx) < abs(dy)) {
        if (y0 > y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        } else if (y0 == y1) {
            Canvas::point(floor(x1), floor(y1));
            return;
        }
        int y = ceil(y0);
        int yEnd = floor(y1);
        float slope = dx / dy;
        float _x = x0 + (y - y0) * slope + 0.5f;
        int x = round(_x * (1 << 16));
        int dx = round(slope * (1 << 16));
        do {
            Canvas::point(x >> 16, y);
            x += dx;
        } while (++y <= yEnd);
    } else {
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        } else if (x0 == x1) {
            Canvas::point(floor(x1), floor(y1));
            return;
        }
        int x = ceil(x0);
        int xEnd = floor(x1);
        float slope = dy / dx;
        float _y = y0 + (x - x0) * slope + 0.5f;
        int y = round(_y * (1 << 16));
        int dy = round(slope * (1 << 16));
        do {
            Canvas::point(x, y >> 16);
            y += dy;
        } while (++x <= xEnd);
    }
}

void box(float x, float y, float w, float h) {
    assert(w >= 0.f && h >= 0.f);
    if (w == 0.f || h == 0.f) return;
    w = (w < 1.f ? 1.f : w);
    h = (h < 1.f ? 1.f : h);
    int xStart = round(x);
    int xEnd = round(x + w);
    int yStart = round(y);
    int yEnd = round(y + h);
    for (int i = xStart; i < xEnd; ++i) {
        Canvas::point(i, yStart);
        Canvas::point(i, yEnd - 1);
    }
    for (int j = yStart + 1; j < yEnd - 1; ++j) {
        Canvas::point(xStart, j);
        Canvas::point(xEnd - 1, j);
    }
}

void boxFill(float x, float y, float w, float h) {
    assert(w >= 0.f && h >= 0.f);
    if (w == 0.f || h == 0.f) return;
    w = (w < 1.f ? 1.f : w);
    h = (h < 1.f ? 1.f : h);
    int xStart = round(x);
    int xEnd = round(x + w);
    int yStart = round(y);
    int yEnd = round(y + h);
    for (int j = yStart; j < yEnd; ++j) {
        for (int i = xStart; i < xEnd; ++i) {
            Canvas::point(i, j);
        }
    }
}

} // namespace RC::Canvas
