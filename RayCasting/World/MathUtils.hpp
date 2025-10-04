#ifndef MathUtils_hpp
#define MathUtils_hpp

#include <simd/simd.h>

namespace RC {

/**
 * @brief Calculates the angle in radians of a 2D vector {x, y}.
 * The angle is measured counter-clockwise from the positive X-axis.
 * @param x The x-component of the vector.
 * @param y The y-component of the vector.
 * @return float The angle in radians.
 */
float radians(float x, float y) {
    return atan2f(y, x);
}

/**
 * @brief Creates a normalized 2D direction vector from x and y components.
 * This is returned as a vector_float3 suitable for use with homogeneous coordinate matrices.
 * The third component (w) is set to 0.0f to represent a direction, which is unaffected by translations.
 * @param x The x-component of the vector.
 * @param y The y-component of the vector.
 * @return vector_float3 A normalized direction vector (x, y, 0).
 */
vector_float3 createNormalizedVector(float x, float y) {
    float magnitude = sqrtf(x * x + y * y);
    if (magnitude > 0.0f) {
        return {x / magnitude, y / magnitude, 0.0f};
    } else {
        return {0.0f, 0.0f, 0.0f};
    }
}

/**
 * @brief Creates a 3x3 identity matrix for 2D transformations.
 * This is suitable for use with 2D homogeneous coordinates (x, y, 1).
 * @return matrix_float3x3 A 3x3 identity matrix.
 */
constexpr matrix_float3x3 createIdentityMatrix() {
    return matrix_float3x3({{1.0f, 0.0f, 0.0f},
                            {0.0f, 1.0f, 0.0f},
                            {0.0f, 0.0f, 1.0f}});
}

/**
 * @brief Creates a 3x3 translation matrix for 2D transformations.
 * This is suitable for use with 2D homogeneous coordinates (x, y, 1).
 * @param tx The translation amount along the x-axis.
 * @param ty The translation amount along the y-axis.
 * @return matrix_float3x3 A 3x3 translation matrix.
 */
constexpr matrix_float3x3 createTranslationMatrix(float tx, float ty) {
    return matrix_float3x3({{1.0f, 0.0f, 0.0f},
                            {0.0f, 1.0f, 0.0f},
                            {tx, ty, 1.0f}});
}

/**
 * @brief Creates a 3x3 rotation matrix for 2D transformations (rotation around the Z-axis).
 * This is suitable for use with 2D homogeneous coordinates (x, y, 1).
 * @param angleRad The angle of rotation in radians.
 * @return matrix_float3x3 A 3x3 rotation matrix.
 */
constexpr matrix_float3x3 createRotationMatrix(float angleRad) {
    float c = cosf(angleRad);
    float s = sinf(angleRad);
    return matrix_float3x3({{c, s, 0.0f},
                            {-s, c, 0.0f},
                            {0, 0, 1.0f}});
}

/**
 * @brief Creates a 3x3 rotation matrix from a 2D direction vector.
 * The matrix will rotate the positive X-axis to align with the given direction {x, y}.
 * @param x The x-component of the direction vector.
 * @param y The y-component of the direction vector.
 * @return matrix_float3x3 A 3x3 rotation matrix.
 */
matrix_float3x3 createRotationMatrix(float x, float y) {
    vector_float3 dir = createNormalizedVector(x, y);
    float c = dir[0]; // cos(angle)
    float s = dir[1]; // sin(angle)

    // Create a 3x3 rotation matrix for 2D homogeneous coordinates.
    // The simd library uses column-major order for matrix constructors.
    return matrix_float3x3({{c, s, 0.0f},
                            {-s, c, 0.0f},
                            {0, 0, 1.0f}});
}

/**
 * @brief Creates a 3x3 scaling matrix for 2D transformations.
 * This is suitable for use with 2D homogeneous coordinates (x, y, 1).
 * @param sx The scaling factor along the x-axis.
 * @param sy The scaling factor along the y-axis.
 * @return matrix_float3x3 A 3x3 scaling matrix.
 */
matrix_float3x3 createScaleMatrix(float sx, float sy) {
    // Create a 3x3 scaling matrix for 2D homogeneous coordinates.
    // The simd library uses column-major order for matrix constructors.
    return matrix_float3x3({{sx, 0.0f, 0.0f},
                            {0.0f, sy, 0.0f},
                            {0.0f, 0.0f, 1.0f}});
}

void printMatrix(const matrix_float3x3& m) {
    printf("[%f, %f, %f]\n", m.columns[0][0], m.columns[1][0], m.columns[2][0]);
    printf("[%f, %f, %f]\n", m.columns[0][1], m.columns[1][1], m.columns[2][1]);
    printf("[%f, %f, %f]\n", m.columns[0][2], m.columns[1][2], m.columns[2][2]);
}

} // namespace RC

#endif /* MathUtils_hpp */
