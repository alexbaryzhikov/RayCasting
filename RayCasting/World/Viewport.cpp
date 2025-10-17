#include <array>
#include <cmath>
#include <print>

#include "Viewport.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Map.hpp"
#include "MathUtils.hpp"
#include "Palette.hpp"
#include "Player.hpp"

namespace RC::Viewport {

constexpr float EPSILON = std::numeric_limits<float>::epsilon() * 128;
constexpr float BIG_FLOAT = 1.0e+6f;
constexpr float HORIZON_HEIGHT = CANVAS_HEIGHT / 2.0f;
const float PROJECTION_DISTANCE = CANVAS_WIDTH / (2.0f * tan(CAMERA_FOV / 2.0f));

simd::float2 rayL = {0.0f, 0.0f};
simd::float2 rayC = {0.0f, 0.0f};
simd::float2 rayR = {0.0f, 0.0f};

enum class RayHit {
    none,
    horizontal,
    vertical
};

struct RayCast {
    simd::float2 ray;
    RayHit hit;
};

std::array<RayCast, CANVAS_WIDTH> rayCasts;

void fillBackground() {
    Canvas::setClipFrame(0, 0, CANVAS_WIDTH, HORIZON_HEIGHT);
    Canvas::fill(Palette::GUNMETAL_GRAY_DARKER);
    Canvas::setClipFrame(0, HORIZON_HEIGHT, CANVAS_WIDTH, CANVAS_HEIGHT - HORIZON_HEIGHT);
    Canvas::fill(Palette::GUNMETAL_GRAY_DARK);
    Canvas::resetClipFrame();
}

void drawWalls() {
    for (int x = 0; x < rayCasts.size(); ++x) {
        RayCast rayCast = rayCasts[x];
        if (rayCast.hit == RayHit::none) continue;
        float wallDistance = simd::length(rayCast.ray);
        float wallHeight = MAP_BLOCK_SIZE * PROJECTION_DISTANCE / wallDistance;
        int y = ceil((CANVAS_HEIGHT - wallHeight) / 2.0f);
        int yEnd = floor((CANVAS_HEIGHT + wallHeight) / 2.0f);
        Palette::setColor(rayCast.hit == RayHit::horizontal ? Palette::GUNMETAL_GRAY_LIGHT : Palette::GUNMETAL_GRAY_LIGHTER);
        for (; y <= yEnd; ++y) {
            Canvas::point(x, y);
        }
    }
}

void draw() {
    if (!Map::isVisible) {
        fillBackground();
        drawWalls();
    }
}

float sign(float x) {
    return x < 0 ? -1 : 1;
}

RayCast castRay(float angle, float mapWidth, float mapHeight) {
    float sinA = sin(angle);
    float cosA = cos(angle);

    // Scan columns
    simd::float2 rayA = {BIG_FLOAT, BIG_FLOAT};
    bool hitA = false;
    if (fabs(cosA) > EPSILON) {
        rayA.x = (cosA < 0 ? floor(Player::position.x / MAP_BLOCK_SIZE) : ceil(Player::position.x / MAP_BLOCK_SIZE)) * MAP_BLOCK_SIZE;
        rayA.y = Player::position.y + (rayA.x - Player::position.x) * sinA / cosA;
        simd::float2 d = {MAP_BLOCK_SIZE * sign(cosA), fabs(MAP_BLOCK_SIZE * sinA / cosA) * sign(sinA)};
        for (; rayA.x > 0 && rayA.x < mapWidth && rayA.y > 0 && rayA.y < mapHeight; rayA += d) {
            int row = floor(rayA.y / MAP_BLOCK_SIZE);
            int col = floor(rayA.x / MAP_BLOCK_SIZE) - float(cosA < 0);
            if (Map::tiles[row][col] == Map::Tile::WALL) {
                hitA = true;
                break;
            }
        }
        rayA -= Player::position.xy;
    }

    // Scan rows
    simd::float2 rayB = {BIG_FLOAT, BIG_FLOAT};
    bool hitB = false;
    if (fabs(sinA) > EPSILON) {
        rayB.y = (sinA < 0 ? floor(Player::position.y / MAP_BLOCK_SIZE) : ceil(Player::position.y / MAP_BLOCK_SIZE)) * MAP_BLOCK_SIZE;
        rayB.x = Player::position.x + (rayB.y - Player::position.y) * cosA / sinA;
        simd::float2 d = {fabs(MAP_BLOCK_SIZE * cosA / sinA) * sign(cosA), MAP_BLOCK_SIZE * sign(sinA)};
        for (; rayB.x > 0 && rayB.x < mapWidth && rayB.y > 0 && rayB.y < mapHeight; rayB += d) {
            int row = floor(rayB.y / MAP_BLOCK_SIZE) - float(sinA < 0);
            int col = floor(rayB.x / MAP_BLOCK_SIZE);
            if (Map::tiles[row][col] == Map::Tile::WALL) {
                hitB = true;
                break;
            }
        }
        rayB -= Player::position.xy;
    }

    if (hitA != hitB) {
        if (hitA) {
            return {rayA, RayHit::vertical};
        } else {
            return {rayB, RayHit::horizontal};
        }
    }
    if (simd::length(rayA) < simd::length(rayB)) {
        return {rayA, hitA ? RayHit::vertical : RayHit::none};
    } else {
        return {rayB, hitB ? RayHit::horizontal : RayHit::none};
    }
}

void castRays() {
    float mapWidth = Map::width() * MAP_BLOCK_SIZE;
    float mapHeight = Map::height() * MAP_BLOCK_SIZE;

    rayL = castRay(Player::angle - CAMERA_FOV / 2.0f, mapWidth, mapHeight).ray;
    rayC = castRay(Player::angle, mapWidth, mapHeight).ray;
    rayR = castRay(Player::angle + CAMERA_FOV / 2.0f, mapWidth, mapHeight).ray;

    for (size_t i = 0; i < rayCasts.size(); ++i) {
        float angle = Player::angle + CAMERA_FOV * (float(i) / rayCasts.size() - 0.5f);
        rayCasts[i] = castRay(angle, mapWidth, mapHeight);
    }
}

void update() {
    castRays();
}

} // namespace RC::Viewport
