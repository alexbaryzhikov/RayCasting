#include "Player.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Keyboard.hpp"
#include "Map.hpp"
#include "MathUtils.hpp"
#include "Mouse.hpp"
#include "Palette.hpp"

namespace RC::Player {

simd::float3 position = {0.0f, 0.0f, 1.0f};
simd::float3 velocity = {0.0f, 0.0f, 0.0f};
float angle = 0.0f;
bool wallCollision = true;

void updateWallCollision() {
    static bool updated = false;
    if (!updated && Keyboard::keys[Keyboard::keyN]) {
        updated = true;
        wallCollision = !wallCollision;
    }
    if (!Keyboard::keys[Keyboard::keyN]) {
        updated = false;
    }
}

void updateAngle() {
    if (Map::isFollowing() && Keyboard::keys[Keyboard::keyLeft]) {
        angle -= PLAYER_TURN_SPEED;
    }
    if (Map::isFollowing() && Keyboard::keys[Keyboard::keyRight]) {
        angle += PLAYER_TURN_SPEED;
    }
}

void applyAcceleration(float accelerationMagnitude) {
    float dx = 0.0f;
    float dy = 0.0f;

    if (Keyboard::keys[Keyboard::keyW] || (Map::isFollowing() && Keyboard::keys[Keyboard::keyUp])) {
        dx += 1.0f;
    }
    if (Keyboard::keys[Keyboard::keyS] || (Map::isFollowing() && Keyboard::keys[Keyboard::keyDown])) {
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

void applyWallCollision(float radius) {
    if (!wallCollision || simd::all(velocity == 0)) return;

    int mapX = position.x / MAP_TILE_SIZE;
    int mapY = position.y / MAP_TILE_SIZE;
    simd::float3 newPosition = position + velocity;

    // West
    if (mapX == 0 || Map::tiles[mapY * Map::tilesWidth + mapX - 1] != Tile::floor) {
        newPosition.x = fmax(newPosition.x, mapX * MAP_TILE_SIZE + radius);
    }
    // East
    if (mapX + 1 == Map::tilesWidth || Map::tiles[mapY * Map::tilesWidth + mapX + 1] != Tile::floor) {
        newPosition.x = fmin(newPosition.x, (mapX + 1) * MAP_TILE_SIZE - radius);
    }
    // North
    if (mapY == 0 || Map::tiles[(mapY - 1) * Map::tilesWidth + mapX] != Tile::floor) {
        newPosition.y = fmax(newPosition.y, mapY * MAP_TILE_SIZE + radius);
    }
    // South
    if (mapY + 1 == Map::tilesHeight || Map::tiles[(mapY + 1) * Map::tilesWidth + mapX] != Tile::floor) {
        newPosition.y = fmin(newPosition.y, (mapY + 1) * MAP_TILE_SIZE - radius);
    }
    // NW
    if (mapX == 0 || mapY == 0 || Map::tiles[(mapY - 1) * Map::tilesWidth + mapX - 1] != Tile::floor) {
        simd::float2 corner = simd::float2{float(mapX), float(mapY)} * MAP_TILE_SIZE;
        simd::float2 cornerToPlayer = newPosition.xy - corner;
        float cornerToPlayerDistance = simd::length(cornerToPlayer);
        if (cornerToPlayerDistance > 0 && cornerToPlayerDistance < radius) {
            newPosition.xy = corner + simd::normalize(cornerToPlayer) * radius;
        }
    }
    // SW
    if (mapX == 0 || mapY + 1 == Map::tilesHeight || Map::tiles[(mapY + 1) * Map::tilesWidth + mapX - 1] != Tile::floor) {
        simd::float2 corner = simd::float2{float(mapX), float(mapY + 1)} * MAP_TILE_SIZE;
        simd::float2 cornerToPlayer = newPosition.xy - corner;
        float cornerToPlayerDistance = simd::length(cornerToPlayer);
        if (cornerToPlayerDistance > 0 && cornerToPlayerDistance < radius) {
            newPosition.xy = corner + simd::normalize(cornerToPlayer) * radius;
        }
    }
    // NE
    if (mapX + 1 == Map::tilesWidth || mapY == 0 || Map::tiles[(mapY - 1) * Map::tilesWidth + mapX + 1] != Tile::floor) {
        simd::float2 corner = simd::float2{float(mapX + 1), float(mapY)} * MAP_TILE_SIZE;
        simd::float2 cornerToPlayer = newPosition.xy - corner;
        float cornerToPlayerDistance = simd::length(cornerToPlayer);
        if (cornerToPlayerDistance > 0 && cornerToPlayerDistance < radius) {
            newPosition.xy = corner + simd::normalize(cornerToPlayer) * radius;
        }
    }
    // SE
    if (mapX + 1 == Map::tilesWidth || mapY + 1 == Map::tilesHeight || Map::tiles[(mapY + 1) * Map::tilesWidth + mapX + 1] != Tile::floor) {
        simd::float2 corner = simd::float2{float(mapX + 1), float(mapY + 1)} * MAP_TILE_SIZE;
        simd::float2 cornerToPlayer = newPosition.xy - corner;
        float cornerToPlayerDistance = simd::length(cornerToPlayer);
        if (cornerToPlayerDistance > 0 && cornerToPlayerDistance < radius) {
            newPosition.xy = corner + simd::normalize(cornerToPlayer) * radius;
        }
    }

    velocity = newPosition - position;
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
    applyWallCollision(PLAYER_RADIUS);
    applyMaxSpeed(PLAYER_MOVE_SPEED);
}

void updatePosition() {
    position += velocity;
}

void update() {
    updateWallCollision();
    updateAngle();
    updateVelocity();
    updatePosition();
}

} // namespace RC::Player
