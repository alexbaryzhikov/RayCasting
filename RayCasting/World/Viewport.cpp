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

std::array<float, CANVAS_WIDTH> rays;

void fillBackground() {
    Palette::setColor(Palette::BEIGE_DARK);
    Canvas::boxFill(0, 0, CANVAS_WIDTH, HORIZON_HEIGHT);
    Palette::setColor(Palette::BEIGE_LIGHT);
    Canvas::boxFill(0, HORIZON_HEIGHT, CANVAS_WIDTH, CANVAS_HEIGHT - HORIZON_HEIGHT);
}

void drawWalls() {
}

void draw() {
    fillBackground();
}

float sign(float x) {
    return x < 0 ? -1 : 1;
}

simd::float2 castRay(float angle, float mapWidth, float mapHeight) {
    float sinA = sin(angle);
    float cosA = cos(angle);

    // Scan columns
    simd::float2 rayA = {BIG_FLOAT, BIG_FLOAT};
    if (fabs(cosA) > EPSILON) {
        rayA.x = (cosA < 0 ? floor(Player::position.x / MAP_BLOCK_SIZE) : ceil(Player::position.x / MAP_BLOCK_SIZE)) * MAP_BLOCK_SIZE;
        rayA.y = Player::position.y + (rayA.x - Player::position.x) * sinA / cosA;
        simd::float2 d = {MAP_BLOCK_SIZE * sign(cosA), fabs(MAP_BLOCK_SIZE * sinA / cosA) * sign(sinA)};
        for (; rayA.x > 0 && rayA.x < mapWidth && rayA.y > 0 && rayA.y < mapHeight; rayA += d) {
            int row = floor(rayA.y / MAP_BLOCK_SIZE);
            int col = floor(rayA.x / MAP_BLOCK_SIZE) - float(cosA < 0);
            if (Map::tiles[row][col] == Map::Tile::WALL) {
                break;
            }
        }
    }

    // Scan rows
    simd::float2 rayB = {BIG_FLOAT, BIG_FLOAT};
    if (fabs(sinA) > EPSILON) {
        rayB.y = (sinA < 0 ? floor(Player::position.y / MAP_BLOCK_SIZE) : ceil(Player::position.y / MAP_BLOCK_SIZE)) * MAP_BLOCK_SIZE;
        rayB.x = Player::position.x + (rayB.y - Player::position.y) * cosA / sinA;
        simd::float2 d = {fabs(MAP_BLOCK_SIZE * cosA / sinA) * sign(cosA), MAP_BLOCK_SIZE * sign(sinA)};
        for (; rayB.x > 0 && rayB.x < mapWidth && rayB.y > 0 && rayB.y < mapHeight; rayB += d) {
            int row = floor(rayB.y / MAP_BLOCK_SIZE) - float(sinA < 0);
            int col = floor(rayB.x / MAP_BLOCK_SIZE);
            if (Map::tiles[row][col] == Map::Tile::WALL) {
                break;
            }
        }
    }

    return simd::length(rayA - Player::position.xy) < simd::length(rayB - Player::position.xy) ? rayA : rayB;
}

void castRays() {
    float mapWidth = Map::width() * MAP_BLOCK_SIZE;
    float mapHeight = Map::height() * MAP_BLOCK_SIZE;

    rayL = castRay(Player::angle - CAMERA_FOV / 2.0f, mapWidth, mapHeight);
    rayC = castRay(Player::angle, mapWidth, mapHeight);
    rayR = castRay(Player::angle + CAMERA_FOV / 2.0f, mapWidth, mapHeight);
}

void update() {
    castRays();
}

} // namespace RC::Viewport
