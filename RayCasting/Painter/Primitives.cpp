//
//  Primitives.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 06.09.25.
//

#include <algorithm>
#include <cassert>

#include "Canvas.hpp"
#include "Primitives.hpp"

void line(float x0, float y0, float x1, float y1) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    if (abs(dx) < abs(dy)) {
        if (y0 > y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        } else if (y0 == y1) {
            point(floor(x1), floor(y1));
            return;
        }
        int y = ceil(y0);
        int yEnd = floor(y1);
        float slope = dx / dy;
        float _x = x0 + (y - y0) * slope + 0.5f;
        int x = round(_x * (1 << 16));
        int dx = round(slope * (1 << 16));
        do {
            point(x >> 16, y);
            x += dx;
        } while (++y <= yEnd);
    } else {
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        } else if (x0 == x1) {
            point(floor(x1), floor(y1));
            return;
        }
        int x = ceil(x0);
        int xEnd = floor(x1);
        float slope = dy / dx;
        float _y = y0 + (x - x0) * slope + 0.5f;
        int y = round(_y * (1 << 16));
        int dy = round(slope * (1 << 16));
        do {
            point(x, y >> 16);
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
        point(i, yStart);
        point(i, yEnd - 1);
    }
    for (int j = yStart + 1; j < yEnd - 1; ++j) {
        point(xStart, j);
        point(xEnd - 1, j);
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
            point(i, j);
        }
    }
}
