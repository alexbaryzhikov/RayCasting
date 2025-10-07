#ifndef MathUtils_hpp
#define MathUtils_hpp

#include <simd/simd.h>

namespace RC {

/**
 * @brief Calculates the angle in radians of a 2D vector (x, y).
 * The angle is measured counter-clockwise from the positive X-axis.
 * @param x The x-component of the vector.
 * @param y The y-component of the vector.
 * @return float The angle in radians.
 */
float makeAngle(float x, float y);

/**
 * @brief Creates a normalized 2D direction vector from x and y components.
 * The third component (w) is set to 0.0f to represent a direction, which is unaffected by translations.
 * @param x The x-component of the vector.
 * @param y The y-component of the vector.
 * @return float3 A normalized direction vector (x, y, 0).
 */
simd::float3 makeNormal(float x, float y);

/**
 * @brief Creates a 3x3 translation matrix for 2D transformations.
 * @param tx The translation amount along the x-axis.
 * @param ty The translation amount along the y-axis.
 * @return float3x3 A 3x3 translation matrix.
 */
simd::float3x3 makeTranslationMatrix(float tx, float ty);

/**
 * @brief Creates a 3x3 rotation matrix for 2D transformations (rotation around the Z-axis).
 * The matrix will rotate the positive X-axis to align with the given angle.
 * @param angle The angle of rotation in radians.
 * @return float3x3 A 3x3 rotation matrix.
 */
simd::float3x3 makeRotationMatrix(float angle);

/**
 * @brief Creates a 3x3 rotation matrix for 2D transformations (rotation around the Z-axis).
 * The matrix will rotate the positive X-axis to align with the given direction (x, y).
 * @param rx The x-component of the direction vector.
 * @param ry The y-component of the direction vector.
 * @return float3x3 A 3x3 rotation matrix.
 */
simd::float3x3 makeRotationMatrix(float rx, float ry);

/**
 * @brief Creates a 3x3 scaling matrix for 2D transformations.
 * @param sx The scaling factor along the x-axis.
 * @param sy The scaling factor along the y-axis.
 * @return float3x3 A 3x3 scaling matrix.
 */
simd::float3x3 makeScaleMatrix(float sx, float sy);

void printVector(const simd::float3& v);

void printMatrix(const simd::float3x3& m);

} // namespace RC

#endif /* MathUtils_hpp */
