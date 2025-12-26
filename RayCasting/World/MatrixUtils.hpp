#ifndef MatrixUtils_hpp
#define MatrixUtils_hpp

#include <simd/simd.h>

namespace RC {

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

} // namespace RC

#endif /* MatrixUtils_hpp */
