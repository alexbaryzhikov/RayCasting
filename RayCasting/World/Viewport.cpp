#include <array>
#include <numbers>

#include "Viewport.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Keyboard.hpp"
#include "Map.hpp"
#include "MathUtils.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Textures.hpp"

namespace RC::Viewport {

constexpr TileHit tileMiss = {-1};
constexpr float epsilon = std::numeric_limits<float>::epsilon() * 128;
constexpr float bigFloat = 1.0e+6f;
constexpr float horizonHeight = CANVAS_HEIGHT / 2.0f;
constexpr size_t floorHeight = horizonHeight;
constexpr size_t ceilingHeight = CANVAS_HEIGHT - floorHeight;
const float projectionDistance = (CANVAS_WIDTH / 2.0f) / tan(CAMERA_FOV / 2.0f);
constexpr float maxDrawDistance = 800.0f;

std::array<float, CANVAS_WIDTH> rayAnglesHorizontal;
std::array<float, CANVAS_WIDTH> rayTansHorizontal;
std::array<float, floorHeight> rayTansFloor;
std::array<float, ceilingHeight> rayTansCeiling;

float cameraHeight = MAP_TILE_SIZE / 2.0f;
bool wallsVisible = true;

void initialize() {
    for (size_t i = 0; i < rayAnglesHorizontal.size(); ++i) {
        float x = float(i) + 0.5f - CANVAS_WIDTH / 2.0f;
        rayAnglesHorizontal[i] = atan(x / projectionDistance);
    }
    for (size_t i = 0; i < rayTansHorizontal.size(); ++i) {
        float x = float(i) + 0.5f - CANVAS_WIDTH / 2.0f;
        rayTansHorizontal[i] = x / projectionDistance;
    }
    for (size_t i = 0; i < rayTansFloor.size(); ++i) {
        float y = float(i) + 0.5f;
        rayTansFloor[i] = y / projectionDistance;
    }
    for (size_t i = 0; i < rayTansCeiling.size(); ++i) {
        float y = ceilingHeight - (float(i) + 0.5f);
        rayTansCeiling[i] = y / projectionDistance;
    }
}

uint32_t sampleTexture(uint32_t* texture, float x, float y) {
    using Textures::dimension;
    float row = round(y * (dimension - 1));
    float col = round(x * (dimension - 1));
    return texture[int(row * dimension + col)];
}

void drawFloor() {
    const float cameraDistanceToSurface = cameraHeight;
    simd::float3 mapTile = {MAP_TILE_SIZE, MAP_TILE_SIZE, 1.0f};
    simd::float3x3 mapSpaceTransform = matrix_multiply(makeTranslationMatrix(Player::position.x, Player::position.y),
                                                       makeRotationMatrix(Player::angle));
    for (size_t i = 0; i < rayTansHorizontal.size(); ++i) {
        for (size_t j = 0; j < rayTansFloor.size(); ++j) {
            float hitX = cameraDistanceToSurface / rayTansFloor[j];
            if (hitX > maxDrawDistance) {
                Canvas::point(i, j + ceilingHeight, Palette::fogColor);
                continue;
            }
            float hitY = hitX * rayTansHorizontal[i];
            simd::float3 hit = matrix_multiply(mapSpaceTransform, simd::float3{hitX, hitY, 1.0f});
            if (hit.x < 0 || hit.x > Map::width || hit.y < 0 || hit.y > Map::height) {
                continue;
            }
            simd::float3 texturePos = simd::fmod(hit, mapTile) / mapTile;
            uint32_t color = sampleTexture(Textures::floor.data(), texturePos.x, texturePos.y);
            float distanceCoef = hitX * 2 / maxDrawDistance;
            if (distanceCoef < 1) {
                color = Palette::blend(color, Palette::lightColor, (1 - distanceCoef) * 0x70, BlendMode::add);
            } else {
                color = Palette::blend(color, Palette::shadowColor, (distanceCoef - 1) * 0xA0, BlendMode::multipy);
            }
            color = Palette::blend(color, Palette::fogColor, hitX / maxDrawDistance * 0xFF, BlendMode::normal);
            Canvas::point(i, j + ceilingHeight, color);
        }
    }
}

void drawCeiling() {
    const float cameraDistanceToSurface = MAP_TILE_SIZE - cameraHeight;
    simd::float3 mapTile = {MAP_TILE_SIZE, MAP_TILE_SIZE, 1.0f};
    simd::float3x3 mapSpaceTransform = matrix_multiply(makeTranslationMatrix(Player::position.x, Player::position.y),
                                                       makeRotationMatrix(Player::angle));
    for (size_t i = 0; i < rayTansHorizontal.size(); ++i) {
        for (size_t j = 0; j < rayTansCeiling.size(); ++j) {
            float hitX = cameraDistanceToSurface / rayTansCeiling[j];
            if (hitX > maxDrawDistance) {
                Canvas::point(i, j, Palette::fogColor);
                continue;
            }
            float hitY = hitX * rayTansHorizontal[i];
            simd::float3 hit = matrix_multiply(mapSpaceTransform, simd::float3{hitX, hitY, 1.0f});
            if (hit.x < 0 || hit.x > Map::width || hit.y < 0 || hit.y > Map::height) {
                continue;
            }
            simd::float3 texturePos = simd::fmod(hit, mapTile) / mapTile;
            uint32_t color = sampleTexture(Textures::ceiling.data(), texturePos.x, texturePos.y);
            float distanceCoef = hitX * 2 / maxDrawDistance;
            if (distanceCoef < 1) {
                color = Palette::blend(color, Palette::lightColor, (1 - distanceCoef) * 0x70, BlendMode::add);
            } else {
                color = Palette::blend(color, Palette::shadowColor, (distanceCoef - 1) * 0xA0, BlendMode::multipy);
            }
            color = Palette::blend(color, Palette::fogColor, hitX / maxDrawDistance * 0xFF, BlendMode::normal);
            Canvas::point(i, j, color);
        }
    }
}

float sign(float x) {
    return x < 0 ? -1 : 1;
}

Ray castRay(float playerSpaceAngle) {
    float mapSpaceAngle = playerSpaceAngle + Player::angle;
    float sinA = sin(mapSpaceAngle);
    float cosA = cos(mapSpaceAngle);

    // Scan columns
    simd::float2 rayCol = {bigFloat, bigFloat};
    int tileIndexCol = -1;
    if (fabs(cosA) > epsilon) {
        rayCol.x = (cosA < 0 ? floor(Player::position.x / MAP_TILE_SIZE) : ceil(Player::position.x / MAP_TILE_SIZE)) * MAP_TILE_SIZE;
        rayCol.y = Player::position.y + (rayCol.x - Player::position.x) * sinA / cosA;
        simd::float2 d = {MAP_TILE_SIZE * sign(cosA), fabs(MAP_TILE_SIZE * sinA / cosA) * sign(sinA)};
        for (; rayCol.x > 0 && rayCol.x < Map::width && rayCol.y > 0 && rayCol.y < Map::height; rayCol += d) {
            int row = floor(rayCol.y / MAP_TILE_SIZE);
            int col = floor(rayCol.x / MAP_TILE_SIZE) - float(cosA < 0);
            size_t tileIndex = row * Map::tilesWidth + col;
            if (Map::tiles[tileIndex] != Tile::floor) {
                tileIndexCol = int(tileIndex);
                break;
            }
        }
        rayCol -= Player::position.xy;
    }

    // Scan rows
    simd::float2 rayRow = {bigFloat, bigFloat};
    int tileIndexRow = -1;
    if (fabs(sinA) > epsilon) {
        rayRow.y = (sinA < 0 ? floor(Player::position.y / MAP_TILE_SIZE) : ceil(Player::position.y / MAP_TILE_SIZE)) * MAP_TILE_SIZE;
        rayRow.x = Player::position.x + (rayRow.y - Player::position.y) * cosA / sinA;
        simd::float2 d = {fabs(MAP_TILE_SIZE * cosA / sinA) * sign(cosA), MAP_TILE_SIZE * sign(sinA)};
        for (; rayRow.x > 0 && rayRow.x < Map::width && rayRow.y > 0 && rayRow.y < Map::height; rayRow += d) {
            int row = floor(rayRow.y / MAP_TILE_SIZE) - float(sinA < 0);
            int col = floor(rayRow.x / MAP_TILE_SIZE);
            size_t tileIndex = row * int(Map::tilesWidth) + col;
            if (Map::tiles[tileIndex] != Tile::floor) {
                tileIndexRow = int(tileIndex);
                break;
            }
        }
        rayRow -= Player::position.xy;
    }

    if (simd::length(rayCol) < simd::length(rayRow)) {
        float length = simd_dot(rayCol, simd::float2{cos(Player::angle), sin(Player::angle)});
        if (tileIndexCol != -1 && length > CAMERA_NEAR_CLIP) {
            float offset = fmod(rayCol.y + Player::position.y, MAP_TILE_SIZE) / MAP_TILE_SIZE;
            float angle = atan(fabs(sinA / cosA)) * 2 / std::numbers::pi;
            if (cosA > 0) {
                TileHit tileHit = {tileIndexCol, TileSide::left, offset, angle};
                return {rayCol, length, tileHit};
            } else {
                TileHit tileHit = {tileIndexCol, TileSide::right, 1 - offset, angle};
                return {rayCol, length, tileHit};
            }
        } else {
            return {rayCol, length, tileMiss};
        }
    } else {
        float length = simd_dot(rayRow, simd::float2{cos(Player::angle), sin(Player::angle)});
        if (tileIndexRow != -1 && length > CAMERA_NEAR_CLIP) {
            float offset = fmod(rayRow.x + Player::position.x, MAP_TILE_SIZE) / MAP_TILE_SIZE;
            float angle = atan(fabs(cosA / sinA)) * 2 / std::numbers::pi;
            if (sinA > 0) {
                TileHit tileHit = {tileIndexRow, TileSide::top, 1 - offset, angle};
                return {rayRow, length, tileHit};
            } else {
                TileHit tileHit = {tileIndexRow, TileSide::bottom, offset, angle};
                return {rayRow, length, tileHit};
            }
        } else {
            return {rayRow, length, tileMiss};
        }
    }
}

void drawWalls() {
    for (int x = 0; x < rayAnglesHorizontal.size(); ++x) {
        Ray ray = castRay(rayAnglesHorizontal[x]);
        if (ray.isMiss()) continue;
        float projectionCoef = projectionDistance / ray.length;
        float distanceCoef = ray.length * 2 / maxDrawDistance;
        float angleCoef = 1 - ray.hit.angle / 2;
        float yStart = ceil(horizonHeight - (MAP_TILE_SIZE - cameraHeight) * projectionCoef);
        float yEnd = floor(horizonHeight + cameraHeight * projectionCoef);
        for (float y = fmax(0, yStart), end = fmin(yEnd + 1, CANVAS_HEIGHT); y < end; ++y) {
            if (ray.length > maxDrawDistance) {
                Canvas::point(x, y, Palette::fogColor);
                continue;
            }
            uint32_t* texture;
            switch (Map::tiles[ray.hit.index]) {
                case Tile::floor:
                    texture = nullptr;
                    break;
                case Tile::wall:
                    texture = Textures::wall.data();
                    break;
                case Tile::wallFortified:
                    texture = Textures::wallFortified.data();
                    break;
                case Tile::wallIndestructible:
                    texture = Textures::wallIndestructible.data();
                    break;
            }
            uint32_t color = sampleTexture(texture, ray.hit.offset, (y - yStart) / (yEnd - yStart));
            if (distanceCoef < 1) {
                color = Palette::blend(color, Palette::lightColor, angleCoef * (1 - distanceCoef) * 0xA0, BlendMode::add);
            } else {
                color = Palette::blend(color, Palette::shadowColor, (distanceCoef - 1) * 0xA0, BlendMode::multipy);
            }
            color = Palette::blend(color, Palette::fogColor, ray.length / maxDrawDistance * 0xFF, BlendMode::normal);
            Canvas::point(x, y, color);
        }
    }
}

void draw() {
    if (Map::isVisible() && Map::isFullScreen()) return;
    drawFloor();
    drawCeiling();
    if (wallsVisible) drawWalls();
}

void handleInputs() {
    static bool updatedWallsVisibility = false;
    if (!updatedWallsVisibility && Keyboard::keys[Keyboard::keyB]) {
        updatedWallsVisibility = true;
        wallsVisible = !wallsVisible;
    }
    if (!Keyboard::keys[Keyboard::keyB]) {
        updatedWallsVisibility = false;
    }
}

void bounceCamera() {
    const float freqency = 0.01f;
    const float amplitude = 2.0f;

    static float phase = 0;
    phase += simd::length(Player::velocity) * freqency;
    if (phase > 1) phase = 0;

    cameraHeight = MAP_TILE_SIZE / 2.0f - sin(phase * std::numbers::pi) * amplitude;
}

void testRay() {
    static float lastAngle = 0.123f;

    if (Player::angle == lastAngle) return;
    lastAngle = Player::angle;

    Ray ray = Viewport::castRay(0.0f);
    if (ray.isMiss()) return;

    size_t row = ray.hit.index / Map::tilesWidth;
    size_t col = ray.hit.index % Map::tilesWidth;
    const char* side;
    switch (ray.hit.side) {
        case TileSide::left:
            side = "left";
            break;
        case TileSide::right:
            side = "right";
            break;
        case TileSide::top:
            side = "top";
            break;
        case TileSide::bottom:
            side = "bottom";
            break;
    }
    printf("row: %zu, col: %zu, side: %s, offset: %f, angle: %f\n", row, col, side, ray.hit.offset, ray.hit.angle);
}

void update() {
    handleInputs();
    bounceCamera();
}

} // namespace RC::Viewport
