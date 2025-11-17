#ifndef MathUtils_hpp
#define MathUtils_hpp

#include <simd/simd.h>

namespace RC {

/**
 * Returns 1 with the sign of the value.
 */
float sign(float value);

/**
 * Returns complementary of the value. Assumes value is within range [0, 1].
 */
float invertIf(bool condition, float value);

/**
 * Force angle to (-pi, pi] range.
 */
float normalizeAngle(float angle);

/**
 * Creates a normalized 2D direction vector from x and y components.
 * The third component (w) is set to 0.0f to represent a direction, which is unaffected by translations.
 */
simd::float3 makeNormal(float x, float y);

/**
 * Creates a 3x3 translation matrix for 2D transformations.
 */
simd::float3x3 makeTranslationMatrix(float tx, float ty);

/**
 * Creates a 3x3 rotation matrix for 2D transformations (rotation around the Z-axis).
 * The matrix will rotate the positive X-axis to align with the given angle.
 */
simd::float3x3 makeRotationMatrix(float angle);

/**
 * Creates a 3x3 rotation matrix for 2D transformations (rotation around the Z-axis).
 * The matrix will rotate the positive X-axis to align with the given direction (x, y).
 */
simd::float3x3 makeRotationMatrix(float rx, float ry);

/**
 * Creates a 3x3 scaling matrix for 2D transformations.
 */
simd::float3x3 makeScaleMatrix(float sx, float sy);

void printVector(const simd::float3& v);

void printMatrix(const simd::float3x3& m);

} // namespace RC

#endif /* MathUtils_hpp */
