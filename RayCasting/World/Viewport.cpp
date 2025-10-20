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

constexpr float epsilon = std::numeric_limits<float>::epsilon() * 128;
constexpr float bigFloat = 1.0e+6f;
constexpr float horizonHeight = CANVAS_HEIGHT / 2.0f;
const float projectionDistance = (CANVAS_WIDTH / 2.0f) / tan(CAMERA_FOV / 2.0f);

simd::float2 rayR = {0.0f, 0.0f};
simd::float2 rayG = {0.0f, 0.0f};
simd::float2 rayB = {0.0f, 0.0f};

enum class RayHit {
    none,
    horizontal,
    vertical,
};

struct RayCast {
    simd::float2 ray;
    float length;
    RayHit hit;
};

std::array<float, CANVAS_WIDTH> rayAngles;
std::array<RayCast, CANVAS_WIDTH> rayCasts;

void initialize() {
    for (size_t i = 0; i < rayAngles.size(); ++i) {
        float x = float(i) - CANVAS_WIDTH / 2.0f;
        rayAngles[i] = atan(x / projectionDistance);
    }
}

void fillBackground() {
    Canvas::setClipFrame(0, 0, CANVAS_WIDTH, horizonHeight);
    Canvas::fill(Palette::gunmetalDarker);
    Canvas::setClipFrame(0, horizonHeight, CANVAS_WIDTH, CANVAS_HEIGHT - horizonHeight);
    Canvas::fill(Palette::gunmetalDark);
    Canvas::resetClipFrame();
}

void drawWalls() {
    for (int x = 0; x < rayCasts.size(); ++x) {
        RayCast rayCast = rayCasts[x];
        if (rayCast.hit == RayHit::none) continue;
        float wallHeight = MAP_BLOCK_SIZE * (projectionDistance / rayCast.length);
        int y = ceil(horizonHeight - wallHeight / 2.0f);
        int yEnd = floor(horizonHeight + wallHeight / 2.0f);
        Palette::setColor(rayCast.hit == RayHit::horizontal ? Palette::gunmetalLight : Palette::gunmetalLighter);
        for (; y <= yEnd; ++y) {
            Canvas::point(x, y);
        }
    }
}

void draw() {
    if (Map::isVisible && Map::isFullFrame()) return;
    fillBackground();
    drawWalls();
}

float sign(float x) {
    return x < 0 ? -1 : 1;
}

RayCast castRay(float playerSpaceAngle, float mapWidth, float mapHeight) {
    float mapSpaceAngle = playerSpaceAngle + Player::angle;
    float sinA = sin(mapSpaceAngle);
    float cosA = cos(mapSpaceAngle);

    // Scan columns
    simd::float2 rayA = {bigFloat, bigFloat};
    bool hitA = false;
    if (fabs(cosA) > epsilon) {
        rayA.x = (cosA < 0 ? floor(Player::position.x / MAP_BLOCK_SIZE) : ceil(Player::position.x / MAP_BLOCK_SIZE)) * MAP_BLOCK_SIZE;
        rayA.y = Player::position.y + (rayA.x - Player::position.x) * sinA / cosA;
        simd::float2 d = {MAP_BLOCK_SIZE * sign(cosA), fabs(MAP_BLOCK_SIZE * sinA / cosA) * sign(sinA)};
        for (; rayA.x > 0 && rayA.x < mapWidth && rayA.y > 0 && rayA.y < mapHeight; rayA += d) {
            int row = floor(rayA.y / MAP_BLOCK_SIZE);
            int col = floor(rayA.x / MAP_BLOCK_SIZE) - float(cosA < 0);
            if (Map::tiles[row][col] == Map::Tile::wall) {
                hitA = true;
                break;
            }
        }
        rayA -= Player::position.xy;
    }

    // Scan rows
    simd::float2 rayB = {bigFloat, bigFloat};
    bool hitB = false;
    if (fabs(sinA) > epsilon) {
        rayB.y = (sinA < 0 ? floor(Player::position.y / MAP_BLOCK_SIZE) : ceil(Player::position.y / MAP_BLOCK_SIZE)) * MAP_BLOCK_SIZE;
        rayB.x = Player::position.x + (rayB.y - Player::position.y) * cosA / sinA;
        simd::float2 d = {fabs(MAP_BLOCK_SIZE * cosA / sinA) * sign(cosA), MAP_BLOCK_SIZE * sign(sinA)};
        for (; rayB.x > 0 && rayB.x < mapWidth && rayB.y > 0 && rayB.y < mapHeight; rayB += d) {
            int row = floor(rayB.y / MAP_BLOCK_SIZE) - float(sinA < 0);
            int col = floor(rayB.x / MAP_BLOCK_SIZE);
            if (Map::tiles[row][col] == Map::Tile::wall) {
                hitB = true;
                break;
            }
        }
        rayB -= Player::position.xy;
    }

    if (simd::length(rayA) < simd::length(rayB)) {
        float lenA = simd_dot(rayA, simd::float2{cos(Player::angle), sin(Player::angle)});
        return {rayA, lenA, hitA && lenA > CAMERA_NEAR_CLIP ? RayHit::vertical : RayHit::none};
    } else {
        float lenB = simd_dot(rayB, simd::float2{cos(Player::angle), sin(Player::angle)});
        return {rayB, lenB, hitB && lenB > CAMERA_NEAR_CLIP ? RayHit::horizontal : RayHit::none};
    }
}

void castRays() {
    float mapWidth = Map::width() * MAP_BLOCK_SIZE;
    float mapHeight = Map::height() * MAP_BLOCK_SIZE;

    rayR = castRay(-CAMERA_FOV / 2.0f, mapWidth, mapHeight).ray;
    rayG = castRay(0.0f, mapWidth, mapHeight).ray;
    rayB = castRay(CAMERA_FOV / 2.0f, mapWidth, mapHeight).ray;

    for (size_t i = 0; i < rayCasts.size(); ++i) {
        rayCasts[i] = castRay(rayAngles[i], mapWidth, mapHeight);
    }
}

void update() {
    castRays();
}

} // namespace RC::Viewport
