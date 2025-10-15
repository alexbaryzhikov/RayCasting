#ifndef Canvas_hpp
#define Canvas_hpp

#include <cstdint>

#include "Palette.hpp"

namespace RC {

struct Frame {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    float centerX() const { return x + w / 2.0f; }
    float centerY() const { return y + h / 2.0f; }
    float maxX() const { return x + w; }
    float maxY() const { return y + h; }
    bool contains(float px, float py) const { return px >= x && py >= y && px < maxX() && py < maxY(); }
};

bool operator==(const Frame& a, const Frame& b);

} // namespace RC

namespace RC::Canvas {

const void* bytes();

void setClipFrame(float x, float y, float w, float h);

void setClipFrame(Frame frame);

void resetClipFrame();

void fill();

void fill(uint32_t color);

void point(int x, int y, uint32_t color = Palette::color);

void pointBlend(int x, int y, uint32_t color = Palette::color, BlendMode mode = Palette::blendMode);

void line(float x0, float y0, float x1, float y1);

void box(float x, float y, float w, float h);

void boxFill(float x, float y, float w, float h);

} // namespace RC::Canvas

#endif /* Canvas_hpp */
