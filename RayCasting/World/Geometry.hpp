#ifndef Geometry_hpp
#define Geometry_hpp

#include <simd/simd.h>

namespace RC {

struct Segment {
    simd::float3 a;
    simd::float3 b;
};

Segment makeSegment(float ax, float ay, float bx, float by);

} // namespace RC

#endif /* Geometry_hpp */
