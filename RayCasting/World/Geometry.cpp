#include "Geometry.hpp"

namespace RC {

Segment makeSegment(float ax, float ay, float bx, float by) {
    return {simd::float3{ax, ay, 1.0f},
            simd::float3{bx, by, 1.0f}};
}

} // namespace RC
