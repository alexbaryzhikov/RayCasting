#ifndef Geometry_hpp
#define Geometry_hpp

#include <simd/simd.h>
#include <vector>

namespace RC {

struct Segment {
    simd::float3 a;
    simd::float3 b;
};

} // namespace RC

namespace RC::Geometry {

Segment makeSegment(float ax, float ay, float bx, float by);

std::vector<Segment> makeGrid(size_t width, size_t height);

std::vector<Segment> makePlayer();

std::vector<Segment> makeDoorH();

std::vector<Segment> makeDoorV();

std::vector<Segment> makeWall();

std::vector<Segment> makeWallFortified();

std::vector<Segment> makeWallIndestuctible();

} // namespace RC::Geometry

#endif /* Geometry_hpp */
