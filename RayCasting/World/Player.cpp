#include <simd/simd.h>
#include <vector>

#include "Player.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Keyboard.hpp"
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
    createScaleMatrix(1.0f, -1.0f));

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
    if (Keyboard::keys[Keyboard::KEY_LEFT]) {
        angle += PLAYER_TURN_SPEED;
    }
    if (Keyboard::keys[Keyboard::KEY_RIGHT]) {
        angle -= PLAYER_TURN_SPEED;
    }

    float dx = 0.0f;
    float dy = 0.0f;

    if (Keyboard::keys[Keyboard::KEY_W] || Keyboard::keys[Keyboard::KEY_UP]) {
        dy += 1.0f;
    }
    if (Keyboard::keys[Keyboard::KEY_S] || Keyboard::keys[Keyboard::KEY_DOWN]) {
        dy -= 1.0f;
    }
    if (Keyboard::keys[Keyboard::KEY_A]) {
        dx -= 1.0f;
    }
    if (Keyboard::keys[Keyboard::KEY_D]) {
        dx += 1.0f;
    }

    if (dx != 0.0f || dy != 0.0f) {
        float velocityAngle = radians(dx, dy);
        vector_float3 velocityVector = {PLAYER_MOVE_SPEED, 0.0f, 0.0f};
        vector_float3 translation = matrix_multiply(createRotationMatrix(angle + velocityAngle), velocityVector);

        position.x += translation.x;
        position.y += translation.y;
    }
}

} // namespace RC::Player
