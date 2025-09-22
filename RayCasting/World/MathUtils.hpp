//
//  MathUtils.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 22.09.25.
//

#ifndef MathUtils_hpp
#define MathUtils_hpp

#include <simd/simd.h>

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

#endif /* MathUtils_hpp */
