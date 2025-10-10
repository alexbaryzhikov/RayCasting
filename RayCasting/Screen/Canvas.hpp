#ifndef Canvas_hpp
#define Canvas_hpp

#include <cstdint>

namespace RC {

struct Frame {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    float centerX() { return x + w / 2.0f; }
    float centerY() { return y + h / 2.0f; }
    float maxX() { return x + w; }
    float maxY() { return y + h; }

    bool operator==(const Frame& o) const {
        return x == o.x && y == o.y && w == o.w && h == o.h;
    }
};

} // namespace RC

namespace RC::Canvas {

const void* bytes();

void setClipFrame(Frame frame);

void fill(uint32_t color);

void point(int x, int y);

void point(int x, int y, uint32_t alpha);

void line(float x0, float y0, float x1, float y1);

void box(float x, float y, float w, float h);

void boxFill(float x, float y, float w, float h);

} // namespace RC::Canvas

#endif /* Canvas_hpp */
