#include <vector>

#include "Player.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Keyboard.hpp"
#include "MathUtils.hpp"
#include "Palette.hpp"

namespace RC::Player {

const std::vector<Segment> playerGeometry = {
    makeSegment(-5, -5, 5, -5),
    makeSegment(-5, -5, 0, 10),
    makeSegment(5, -5, 0, 10),
};

const simd::float3x3 worldTransform = matrix_multiply(
    makeTranslationMatrix(CANVAS_WIDTH / 2.0f, CANVAS_HEIGHT / 2.0f),
    makeScaleMatrix(1.0f, -1.0f));

simd::float3 position = {0.0f, 0.0f, 1.0f};
simd::float3 velocity = {0.0f, 0.0f, 0.0f};
float angle = 0.0f;

void draw() {
    simd::float3x3 translation = makeTranslationMatrix(position.x, position.y);
    simd::float3x3 rotation = makeRotationMatrix(angle);
    simd::float3x3 geometryTransform = matrix_multiply(translation, rotation);
    simd::float3x3 transform = matrix_multiply(worldTransform, geometryTransform);

    Palette::setColor(Palette::GREEN);
    for (Segment segment : playerGeometry) {
        simd::float3 a = matrix_multiply(transform, segment.a);
        simd::float3 b = matrix_multiply(transform, segment.b);
        Canvas::line(a.x, a.y, b.x, b.y);
    }
}

void updateAngle() {
    if (Keyboard::keys[Keyboard::KEY_LEFT]) {
        angle -= PLAYER_TURN_SPEED;
    }
    if (Keyboard::keys[Keyboard::KEY_RIGHT]) {
        angle += PLAYER_TURN_SPEED;
    }
}

void applyAcceleration(float accelerationMagnitude) {
    float dx = 0.0f;
    float dy = 0.0f;

    if (Keyboard::keys[Keyboard::KEY_W] || Keyboard::keys[Keyboard::KEY_UP]) {
        dx += 1.0f;
    }
    if (Keyboard::keys[Keyboard::KEY_S] || Keyboard::keys[Keyboard::KEY_DOWN]) {
        dx -= 1.0f;
    }
    if (Keyboard::keys[Keyboard::KEY_A]) {
        dy += 1.0f;
    }
    if (Keyboard::keys[Keyboard::KEY_D]) {
        dy -= 1.0f;
    }

    simd::float3 direction = matrix_multiply(makeRotationMatrix(angle), makeNormal(dx, dy));
    simd::float3 acceleration = direction * accelerationMagnitude;
    velocity += acceleration;
}

void applyFriction(float friction) {
    float speed = simd::length(velocity);
    if (speed > friction) {
        velocity *= (speed - friction) / speed;
    } else {
        velocity = {0.0f, 0.0f, 0.0f};
    }
}

void applyMaxSpeed(float maxSpeed) {
    float speed = simd::length(velocity);
    if (speed > maxSpeed) {
        velocity *= maxSpeed / speed;
    }
}

void updateVelocity() {
    applyAcceleration(PLAYER_ACCELERATION);
    applyFriction(PLAYER_FRICTION);
    applyMaxSpeed(PLAYER_MOVE_SPEED);
}

void updatePosition() {
    position += velocity;
}

void update() {
    updateAngle();
    updateVelocity();
    updatePosition();
}

} // namespace RC::Player
