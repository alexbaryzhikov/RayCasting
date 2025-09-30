//
//  Player.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 21.09.25.
//

#include <simd/simd.h>
#include <vector>

#include "Player.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "MathUtils.hpp"
#include "Palette.hpp"

namespace RC::Player {

static const std::vector<Segment> playerGeometry = {
    {{-5, -5}, {5, -5}},
    {{-5, -5}, {0, 10}},
    {{5, -5}, {0, 10}},
};

static const matrix_float3x3 worldTransform = matrix_multiply(
    createTranslationMatrix(CANVAS_WIDTH / 2.0f, CANVAS_HEIGHT / 2.0f),
    createRotationMatrix(M_PI));

Point position;
float angle = 0.0f;

vector_float3 vf3(Point p) {
    return {p.x, p.y, 1.0f};
}

void draw() {
    matrix_float3x3 translation = createTranslationMatrix(position.x, position.y);
    matrix_float3x3 rotation = createRotationMatrix(angle);
    matrix_float3x3 geometryTransform = matrix_multiply(translation, rotation);
    matrix_float3x3 transform = matrix_multiply(worldTransform, geometryTransform);

    Palette::setColor(Palette::GREEN);
    for (Segment segment : playerGeometry) {
        vector_float3 a = matrix_multiply(transform, vf3(segment.a));
        vector_float3 b = matrix_multiply(transform, vf3(segment.b));
        Canvas::line(a.x, a.y, b.x, b.y);
    }
}

void update() {
    angle += 0.01f;
}

} // namespace RC::Player
