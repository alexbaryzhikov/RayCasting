#ifndef Viewport_hpp
#define Viewport_hpp

#include <simd/simd.h>

namespace RC {

enum class TileSide {
    left,
    right,
    top,
    bottom,
};

struct TileHit {
    int index; // -1 for miss
    TileSide side;
    float offset; // 0 to 1
};

struct Ray {
    simd::float2 xy;
    float length;
    TileHit hit;

    bool isMiss() const { return hit.index == -1; }
};

} // namespace RC

namespace RC::Viewport {

void initialize();

void draw();

void update();

Ray castRay(float playerSpaceAngle);

} // namespace RC::Viewport

#endif /* Viewport_hpp */
