#include <vector>
#include <numbers>

#include "Player.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Keyboard.hpp"
#include "MathUtils.hpp"
#include "Mouse.hpp"
#include "Palette.hpp"

namespace RC::Player {

simd::float3 position = {0.0f, 0.0f, 1.0f};
simd::float3 velocity = {0.0f, 0.0f, 0.0f};
float angle = 0.0f;

void updateAngle() {
    if (Keyboard::keys[Keyboard::keyLeft]) {
        angle -= PLAYER_TURN_SPEED;
    }
    if (Keyboard::keys[Keyboard::keyRight]) {
        angle += PLAYER_TURN_SPEED;
    }
}

void applyAcceleration(float accelerationMagnitude) {
    float dx = 0.0f;
    float dy = 0.0f;

    if (Keyboard::keys[Keyboard::keyW] || Keyboard::keys[Keyboard::keyUp]) {
        dx += 1.0f;
    }
    if (Keyboard::keys[Keyboard::keyS] || Keyboard::keys[Keyboard::keyDown]) {
        dx -= 1.0f;
    }
    if (Keyboard::keys[Keyboard::keyA]) {
        dy -= 1.0f;
    }
    if (Keyboard::keys[Keyboard::keyD]) {
        dy += 1.0f;
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
