#include <print>

#include "MathUtils.hpp"

namespace RC {

float makeAngle(float x, float y) {
    return atan2f(y, x);
}

simd::float3 makeNormal(float x, float y) {
    if (x == 0.0f && y == 0.0f) {
        return {0.0f, 0.0f, 0.0f};
    }
    return simd_normalize(simd::float3{x, y, 0.0f});
}

simd::float3x3 makeTranslationMatrix(float tx, float ty) {
    return simd::float3x3{simd::float3{1.0f, 0.0f, 0.0f},
                          simd::float3{0.0f, 1.0f, 0.0f},
                          simd::float3{tx, ty, 1.0f}};
}

simd::float3x3 makeRotationMatrix(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return simd::float3x3{simd::float3{cosA, sinA, 0.0f},
                          simd::float3{-sinA, cosA, 0.0f},
                          simd::float3{0.0f, 0.0f, 1.0f}};
}

simd::float3x3 makeRotationMatrix(float rx, float ry) {
    simd::float3 dir = makeNormal(rx, ry);
    float cosA = dir[0];
    float sinA = dir[1];
    return simd::float3x3{simd::float3{cosA, sinA, 0.0f},
                          simd::float3{-sinA, cosA, 0.0f},
                          simd::float3{0.0f, 0.0f, 1.0f}};
}

simd::float3x3 makeScaleMatrix(float sx, float sy) {
    return simd::float3x3{simd::float3{sx, 0.0f, 0.0f},
                          simd::float3{0.0f, sy, 0.0f},
                          simd::float3{0.0f, 0.0f, 1.0f}};
}

void printVector(const simd::float3& v) {
    std::println("[{} {} {}]", v[0], v[1], v[2]);
}

void printMatrix(const simd::float3x3& m) {
    std::println("[[{} {} {}]", m.columns[0][0], m.columns[1][0], m.columns[2][0]);
    std::println(" [{} {} {}]", m.columns[0][1], m.columns[1][1], m.columns[2][1]);
    std::println(" [{} {} {}]]", m.columns[0][2], m.columns[1][2], m.columns[2][2]);
}

} // namespace RC
