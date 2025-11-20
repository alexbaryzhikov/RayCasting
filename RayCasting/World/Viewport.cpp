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
constexpr float pi = std::numbers::pi_v<float>;
constexpr float epsilon = std::numeric_limits<float>::epsilon() * 128;
constexpr float bigFloat = 1e6;
constexpr float horizonHeight = CANVAS_HEIGHT / 2.0f;
constexpr size_t floorHeight = horizonHeight;
constexpr size_t ceilingHeight = CANVAS_HEIGHT - floorHeight;
const float projectionDistance = (CANVAS_WIDTH / 2.0f) / tan(CAMERA_FOV / 2.0f);
constexpr float maxDrawDistance = 800.0f;
constexpr float distanceToDoor = (MAP_TILE_SIZE - DOOR_DEPTH) / 2;

std::array<float, CANVAS_WIDTH> rayAnglesHorizontal;
std::array<float, CANVAS_WIDTH> rayTansHorizontal;
std::array<float, floorHeight> rayTansFloor;
std::array<float, ceilingHeight> rayTansCeiling;
std::array<simd::float2, 4> doorH;
std::array<simd::float2, 4> doorV;

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
    doorH = {
        simd::float2{0, (MAP_TILE_SIZE - DOOR_DEPTH) / 2},
        simd::float2{0, (MAP_TILE_SIZE + DOOR_DEPTH) / 2},
        simd::float2{MAP_TILE_SIZE, (MAP_TILE_SIZE + DOOR_DEPTH) / 2},
        simd::float2{MAP_TILE_SIZE, (MAP_TILE_SIZE - DOOR_DEPTH) / 2},
    };
    doorV = {
        simd::float2{(MAP_TILE_SIZE - DOOR_DEPTH) / 2, 0},
        simd::float2{(MAP_TILE_SIZE - DOOR_DEPTH) / 2, MAP_TILE_SIZE},
        simd::float2{(MAP_TILE_SIZE + DOOR_DEPTH) / 2, MAP_TILE_SIZE},
        simd::float2{(MAP_TILE_SIZE + DOOR_DEPTH) / 2, 0},
    };
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

struct Intersection {
    simd::float2 point;
    int segmentIndex;
    float segmentOffset;
};

/**
 * Find intersection point between two line segments.
 */
bool findIntersection(simd::float2 a1, simd::float2 b1, simd::float2 a2, simd::float2 b2, simd::float2& point, float& offset) {
    // Solve parametric equation
    // 't' is the parameter for the first line segment (a1, b1)
    // 'u' is the parameter for the second line segment (a2, b2)
    // a1 + t(b1 - a1) = a2 + u(b2 - a2)
    // Rearrange and separate by component:
    // t(b1.x - a1.x) - u(b2.x - a2.x) = a2.x - a1.x
    // t(b1.y - a1.y) - u(b2.y - a2.y) = a2.y - a1.y

    // Calculate coefficients
    simd::float2 d1 = b1 - a1;
    simd::float2 d2 = b2 - a2;
    simd::float2 delta = a2 - a1;

    // Determinant of the main system:
    float determinant = d2.x * d1.y - d2.y * d1.x;

    if (fabs(determinant) < epsilon) {
        // Lines are parallel
        return false;
    }

    // Calculate 't' and 'u'
    float t = (d2.x * delta.y - d2.y * delta.x) / determinant;
    float u = (d1.x * delta.y - d1.y * delta.x) / determinant;

    // Check if the intersection point lies on both line segments
    // The parameters 't' and 'u' must be in the range [0, 1]
    if (t > -epsilon && t < 1 + epsilon && u > -epsilon && u < 1 + epsilon) {
        point = a1 + d1 * t;
        offset = u;
        return true;
    }
    return false;
}

/**
 * Find intersection point between ray and polygon that is closest to the ray start.
 */
template <size_t N>
bool findClosestIntersection(std::array<simd::float2, 2> ray, std::array<simd::float2, N> polygon, Intersection& intersection) {
    float minLength = bigFloat;
    simd::float2 point;
    float offset;
    for (int i = 0; i < N; ++i) {
        if (findIntersection(ray[0], ray[1], polygon[i], polygon[(i + 1) % N], point, offset)) {
            float length = simd::length(point - ray[0]);
            if (length < minLength) {
                minLength = length;
                intersection.point = point;
                intersection.segmentIndex = i;
                intersection.segmentOffset = offset;
            }
        }
    }
    return minLength < bigFloat;
}

float doorSegmentOffset(Tile door, int segmentIndex, float segmentOffset) {
    if (door == Tile::doorH) {
        return segmentIndex == 1 || segmentIndex == 3 ? segmentOffset : 0;
    } else {
        return segmentIndex == 0 || segmentIndex == 2 ? segmentOffset : 0;
    }
}

Ray castRay(float playerSpaceAngle, bool tracer) {
    float mapSpaceAngle = playerSpaceAngle + Player::angle;
    float sinA = sin(mapSpaceAngle);
    float cosA = cos(mapSpaceAngle);

    // Scan columns
    simd::float2 rayH = {bigFloat, bigFloat};
    TileHit tileHitH = tileMiss;
    if (fabs(cosA) > epsilon) {
        rayH.x = (cosA < 0 ? floor(Player::position.x / MAP_TILE_SIZE) : ceil(Player::position.x / MAP_TILE_SIZE)) * MAP_TILE_SIZE;
        rayH.y = Player::position.y + (rayH.x - Player::position.x) * sinA / cosA;
        simd::float2 d = {MAP_TILE_SIZE * sign(cosA), fabs(MAP_TILE_SIZE * sinA / cosA) * sign(sinA)};
        for (; rayH.x > 0 && rayH.x < Map::width && rayH.y > 0 && rayH.y < Map::height; rayH += d) {
            int row = floor(rayH.y / MAP_TILE_SIZE);
            int col = floor(rayH.x / MAP_TILE_SIZE) - float(cosA < 0);
            int tileIndex = row * MAP_WIDTH + col;
            Tile tile = Map::tiles[tileIndex];
            if (tile == Tile::doorH || tile == Tile::doorV) {
                simd::float2 tilePosition = simd::float2{float(col), float(row)} * MAP_TILE_SIZE;
                std::array<simd::float2, 2> ray = {rayH - tilePosition, rayH - tilePosition + d};
                Intersection intersection;
                if (findClosestIntersection(ray, tile == Tile::doorH ? doorH : doorV, intersection)) {
                    rayH += intersection.point - ray[0];
                    tileHitH = {
                        .index = tileIndex,
                        .offset = doorSegmentOffset(tile, intersection.segmentIndex, intersection.segmentOffset),
                        .angle = atan(fabs(tile == Tile::doorH ? cosA / sinA : sinA / cosA)) * 2 / pi,
                        .side = cosA < 0 ? TileSide::right : TileSide::left,
                    };
                    break;
                }
            } else if (tile != Tile::floor) {
                tileHitH = {
                    .index = tileIndex,
                    .offset = invertIf(cosA < 0, (rayH.y - row * MAP_TILE_SIZE) / MAP_TILE_SIZE),
                    .angle = atan(fabs(sinA / cosA)) * 2 / pi,
                    .side = cosA < 0 ? TileSide::right : TileSide::left,
                };
                break;
            }
        }
        rayH -= Player::position.xy;
    }

    // Scan rows
    simd::float2 rayV = {bigFloat, bigFloat};
    TileHit tileHitV = tileMiss;
    if (fabs(sinA) > epsilon) {
        rayV.y = (sinA < 0 ? floor(Player::position.y / MAP_TILE_SIZE) : ceil(Player::position.y / MAP_TILE_SIZE)) * MAP_TILE_SIZE;
        rayV.x = Player::position.x + (rayV.y - Player::position.y) * cosA / sinA;
        simd::float2 d = {fabs(MAP_TILE_SIZE * cosA / sinA) * sign(cosA), MAP_TILE_SIZE * sign(sinA)};
        for (; rayV.x > 0 && rayV.x < Map::width && rayV.y > 0 && rayV.y < Map::height; rayV += d) {
            int row = floor(rayV.y / MAP_TILE_SIZE) - float(sinA < 0);
            int col = floor(rayV.x / MAP_TILE_SIZE);
            int tileIndex = row * MAP_WIDTH + col;
            Tile tile = Map::tiles[tileIndex];
            if (tile == Tile::doorH || tile == Tile::doorV) {
                simd::float2 tilePosition = simd::float2{float(col), float(row)} * MAP_TILE_SIZE;
                std::array<simd::float2, 2> ray = {rayV - tilePosition, rayV - tilePosition + d};
                Intersection intersection;
                if (findClosestIntersection(ray, tile == Tile::doorH ? doorH : doorV, intersection)) {
                    rayV += intersection.point - ray[0];
                    tileHitV = {
                        .index = tileIndex,
                        .offset = doorSegmentOffset(tile, intersection.segmentIndex, intersection.segmentOffset),
                        .angle = atan(fabs(tile == Tile::doorH ? cosA / sinA : sinA / cosA)) * 2 / pi,
                        .side = sinA < 0 ? TileSide::bottom : TileSide::top,
                    };
                    break;
                }
            } else if (tile != Tile::floor) {
                tileHitV = {
                    .index = tileIndex,
                    .offset = invertIf(sinA > 0, (rayV.x - col * MAP_TILE_SIZE) / MAP_TILE_SIZE),
                    .angle = atan(fabs(cosA / sinA)) * 2 / pi,
                    .side = sinA < 0 ? TileSide::bottom : TileSide::top,
                };
                break;
            }
        }
        rayV -= Player::position.xy;
    }

    // Choose the shortest ray
    if (simd::length(rayH) < simd::length(rayV)) {
        float lengthH = simd::dot(rayH, simd::float2{cos(Player::angle), sin(Player::angle)});
        return {rayH, lengthH, lengthH > CAMERA_NEAR_CLIP ? tileHitH : tileMiss};
    } else {
        float lengthV = simd::dot(rayV, simd::float2{cos(Player::angle), sin(Player::angle)});
        return {rayV, lengthV, lengthV > CAMERA_NEAR_CLIP ? tileHitV : tileMiss};
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
                case Tile::doorH:
                case Tile::doorV:
                    texture = Textures::door.data();
                    break;
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

    cameraHeight = MAP_TILE_SIZE / 2.0f - sin(phase * pi) * amplitude;
}

void update() {
    handleInputs();
    bounceCamera();
}

} // namespace RC::Viewport
