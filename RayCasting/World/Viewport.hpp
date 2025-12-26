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
    int index;    // -1 for miss
    float offset; // 0 to 1
    float angle;  // 0 to 1
    TileSide side;
};

struct Ray {
    simd::float2 xy;
    float length;
    TileHit tile;

    bool isMiss() const { return tile.index == -1; }
};

} // namespace RC

namespace RC::Viewport {

extern simd::float2 rayA;
extern simd::float2 rayB;

void initialize();

void draw();

void update();

Ray castRay(float playerSpaceAngle, bool tracer = false);
Ray castRayToFirstHit(float playerSpaceAngle);

} // namespace RC::Viewport

#endif /* Viewport_hpp */
