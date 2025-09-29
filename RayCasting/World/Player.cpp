//
//  Player.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 21.09.25.
//

#include <simd/simd.h>
#include <vector>

#include "Player.hpp"

#include "Config.h"
#include "MathUtils.hpp"
#include "Palette.hpp"
#include "Primitives.hpp"

namespace RC {

Player player;

static std::vector<Segment> playerGeometry = {
    {{-5, -5}, {5, -5}},
    {{-5, -5}, {0, 10}},
    {{5, -5}, {0, 10}},
};

static const matrix_float3x3 worldTransform = matrix_multiply(
    createTranslationMatrix(CANVAS_WIDTH / 2.0f, CANVAS_HEIGHT / 2.0f),
    createRotationMatrix(M_PI));

vector_float3 vf3(Point p) {
    return {p.x, p.y, 1.0f};
}

void drawPlayer() {
    const Player& player = RC::player;

    matrix_float3x3 translation = createTranslationMatrix(player.position.x, player.position.y);
    matrix_float3x3 rotation = createRotationMatrix(player.angle);
    matrix_float3x3 geometryTransform = matrix_multiply(translation, rotation);
    matrix_float3x3 transform = matrix_multiply(worldTransform, geometryTransform);

    setColor(GREEN);
    for (Segment segment : playerGeometry) {
        vector_float3 a = matrix_multiply(transform, vf3(segment.a));
        vector_float3 b = matrix_multiply(transform, vf3(segment.b));
        line(a.x, a.y, b.x, b.y);
    }
}

void updatePlayer() {
    player.angle += 0.01;
}

} // namespace RC
