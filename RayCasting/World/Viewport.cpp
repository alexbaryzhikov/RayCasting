#include <array>
#include <numbers>

#include "Viewport.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Map.hpp"
#include "MathUtils.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Textures.hpp"

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
    float textureOffset; // 0 to 1
    RayHit hit;
};

std::array<float, CANVAS_WIDTH> rayAngles;
std::array<RayCast, CANVAS_WIDTH> rayCasts;
float cameraHeight = MAP_BLOCK_SIZE / 2.0f;

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

uint32_t sampleTexture(float x, float y) {
    using Textures::dimension;
    float row = round(y * (dimension - 1));
    float col = round(x * (dimension - 1));
    return Textures::dungeonWallTopBeam[row * dimension + col];
}

void drawWalls() {
    for (int x = 0; x < rayCasts.size(); ++x) {
        RayCast rayCast = rayCasts[x];
        if (rayCast.hit == RayHit::none) continue;
        float projectionCoef = projectionDistance / rayCast.length;
        float yStart = ceil(horizonHeight + (cameraHeight - MAP_BLOCK_SIZE) * projectionCoef);
        float yEnd = floor(horizonHeight + (cameraHeight * projectionCoef));
        for (float y = fmax(0, yStart), end = fmin(yEnd + 1, CANVAS_HEIGHT); y < end; ++y) {
            uint32_t diffuse = sampleTexture(rayCast.textureOffset, (y - yStart) / (yEnd - yStart));
            if (rayCast.hit == RayHit::horizontal) {
                Palette::setColor(diffuse);
            } else {
                uint32_t color = Palette::blend(diffuse, Palette::withAlpha(0x80, Palette::gunmetalDarker), BlendMode::multipy);
                Palette::setColor(color);
            }
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

RayCast castRay(float playerSpaceAngle, const float mapWidth, const float mapHeight) {
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
        float length = simd_dot(rayA, simd::float2{cos(Player::angle), sin(Player::angle)});
        float offset = fmod(rayA.y + Player::position.y, MAP_BLOCK_SIZE) / MAP_BLOCK_SIZE;
        RayHit hit = hitA && length > CAMERA_NEAR_CLIP ? RayHit::vertical : RayHit::none;
        return {rayA, length, offset, hit};
    } else {
        float length = simd_dot(rayB, simd::float2{cos(Player::angle), sin(Player::angle)});
        float offset = fmod(rayB.x + Player::position.x, MAP_BLOCK_SIZE) / MAP_BLOCK_SIZE;
        RayHit hit = hitB && length > CAMERA_NEAR_CLIP ? RayHit::horizontal : RayHit::none;
        return {rayB, length, offset, hit};
    }
}

void castRays() {
    const float mapWidth = Map::width() * MAP_BLOCK_SIZE;
    const float mapHeight = Map::height() * MAP_BLOCK_SIZE;

    rayR = castRay(-CAMERA_FOV / 2.0f, mapWidth, mapHeight).ray;
    rayG = castRay(0.0f, mapWidth, mapHeight).ray;
    rayB = castRay(CAMERA_FOV / 2.0f, mapWidth, mapHeight).ray;

    for (size_t i = 0; i < rayCasts.size(); ++i) {
        rayCasts[i] = castRay(rayAngles[i], mapWidth, mapHeight);
    }
}

void bounceCamera() {
    const float freqency = 0.01f;
    const float amplitude = 2.0f;

    static float phase = 0;
    phase += simd::length(Player::velocity) * freqency;
    if (phase > 1) phase = 0;

    cameraHeight = MAP_BLOCK_SIZE / 2.0f - sin(phase * std::numbers::pi) * amplitude;
}

void update() {
    castRays();
    bounceCamera();
}

} // namespace RC::Viewport
