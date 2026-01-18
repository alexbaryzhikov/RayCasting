#include <array>
#include <numbers>

#include "Viewport.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Keyboard.hpp"
#include "Map.hpp"
#include "MatrixUtils.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Textures.hpp"

namespace RC::Viewport {

constexpr float pi = std::numbers::pi_v<float>;
constexpr float epsilon = std::numeric_limits<float>::epsilon() * 128;
constexpr float bigFloat = 1e6;
constexpr float horizonY = CANVAS_HEIGHT / 2.0f; // screen space
constexpr size_t ceilingSize = horizonY;
constexpr size_t floorSize = CANVAS_HEIGHT - ceilingSize;
const float projectionDistance = (CANVAS_WIDTH / 2.0f) / tan(CAMERA_FOV / 2.0f);
constexpr simd::float3 mapTile = {MAP_TILE_SIZE, MAP_TILE_SIZE, 1.0f};
constexpr simd::float3 pointAtInf = {bigFloat, bigFloat, 1.0f};
constexpr TileHit tileMiss = {-1};

std::array<float, CANVAS_WIDTH> rayAnglesHorizontal;
std::array<float, CANVAS_WIDTH> rayTansHorizontal;
std::array<float, floorSize> rayTansFloor;
std::array<float, ceilingSize> rayTansCeiling;
std::array<simd::float2, 4> doorH;
std::array<simd::float2, 4> doorV;

float cameraHeight = MAP_TILE_SIZE / 2.0f; // map space
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
        float y = ceilingSize - (float(i) + 0.5f);
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

float sign(float value) {
    return value < 0 ? -1 : 1;
}

float invertIf(bool condition, float value) {
    return condition ? 1 - value : value;
}

bool inMapBounds(simd::float2 point) {
    return point.x > 0 && point.x < Map::width && point.y > 0 && point.y < Map::height;
}

simd::float3x3 makePlayerToMapTransform() {
    return matrix_multiply(makeTranslationMatrix(Player::position.x, Player::position.y),
                           makeRotationMatrix(Player::angle));
}

/**
 * Returns a point in player space where ray cast from camera to screen pixel (x, y) hits the ceiling.
 */
simd::float3 ceilingHit(int x, int y, float ceilingHeight) {
    if (y > ceilingSize || cameraHeight > ceilingHeight) return pointAtInf;
    float hitX = (ceilingHeight - cameraHeight) / rayTansCeiling[y];
    float hitY = hitX * rayTansHorizontal[x];
    return {hitX, hitY, 1.0f};
}

/**
 * Returns a point in player space where ray cast from camera to screen pixel (x, y) hits the foor.
 */
simd::float3 floorHit(int x, int y, float floorHeight) {
    if (y < ceilingSize || cameraHeight < floorHeight) return pointAtInf;
    float hitX = (cameraHeight - floorHeight) / rayTansFloor[y - ceilingSize];
    float hitY = hitX * rayTansHorizontal[x];
    return {hitX, hitY, 1.0f};
}

/**
 * Returns texture color at point (x, y). Assumes x and y are in [0, 1] range.
 */
uint32_t sampleTexture(uint32_t* texture, float x, float y) {
    using Textures::dimension;
    float row = round(y * (dimension - 1));
    float col = round(x * (dimension - 1));
    return texture[int(row * dimension + col)];
}

void drawCeiling() {
    simd::float3x3 playerToMapTransform = makePlayerToMapTransform();
    for (int x = 0; x < CANVAS_WIDTH; ++x) {
        for (int y = 0; y < ceilingSize; ++y) {
            simd::float3 hitPlayer = ceilingHit(x, y, MAP_TILE_SIZE);
            if (hitPlayer.x > CAMERA_FAR_CLIP) {
                Canvas::point(x, y, Palette::fogColor);
                continue;
            }
            simd::float3 hitMap = matrix_multiply(playerToMapTransform, hitPlayer);
            if (!inMapBounds(hitMap.xy)) {
                continue;
            }
            simd::float3 texturePos = simd::fmod(hitMap, mapTile) / mapTile;
            uint32_t color = sampleTexture(Textures::ceiling.data(), texturePos.x, texturePos.y);
            float distanceCoef = hitPlayer.x * 2 / CAMERA_FAR_CLIP;
            if (distanceCoef < 1) {
                color = Palette::blend(color, Palette::lightColor, (1 - distanceCoef) * 0x70, BlendMode::add);
            } else {
                color = Palette::blend(color, Palette::shadowColor, (distanceCoef - 1) * 0xA0, BlendMode::multipy);
            }
            color = Palette::blend(color, Palette::fogColor, hitPlayer.x / CAMERA_FAR_CLIP * 0xFF, BlendMode::normal);
            Canvas::point(x, y, color);
        }
    }
}

void drawFloor() {
    simd::float3x3 playerToMapTransform = makePlayerToMapTransform();
    for (int x = 0; x < CANVAS_WIDTH; ++x) {
        for (int y = ceilingSize; y < CANVAS_HEIGHT; ++y) {
            simd::float3 hitPlayer = floorHit(x, y, 0);
            if (hitPlayer.x > CAMERA_FAR_CLIP) {
                Canvas::point(x, y, Palette::fogColor);
                continue;
            }
            simd::float3 hitMap = matrix_multiply(playerToMapTransform, hitPlayer);
            if (!inMapBounds(hitMap.xy)) {
                continue;
            }
            simd::float3 texturePos = simd::fmod(hitMap, mapTile) / mapTile;
            uint32_t color = sampleTexture(Textures::floor.data(), texturePos.x, texturePos.y);
            float distanceCoef = hitPlayer.x * 2 / CAMERA_FAR_CLIP;
            if (distanceCoef < 1) {
                color = Palette::blend(color, Palette::lightColor, (1 - distanceCoef) * 0x70, BlendMode::add);
            } else {
                color = Palette::blend(color, Palette::shadowColor, (distanceCoef - 1) * 0xA0, BlendMode::multipy);
            }
            color = Palette::blend(color, Palette::fogColor, hitPlayer.x / CAMERA_FAR_CLIP * 0xFF, BlendMode::normal);
            Canvas::point(x, y, color);
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

struct RayComponent {
    simd::float2 position; // ray position in map space
    simd::float2 step;     // offset between adjacent grid-aligned positions
    simd::float2 next;     // next grid-aligned position
    TileHit tile;          // tile hit data
    bool advance;          // true if ray can advance further
};

struct RayState {
    simd::float2 normal; // ray direction in map space
    RayComponent rayH;   // horizontal component
    RayComponent rayV;   // vertical component
};

RayState makeRayState(float playerSpaceAngle) {
    float mapSpaceAngle = playerSpaceAngle + Player::angle;
    float cosA = cos(mapSpaceAngle);
    float sinA = sin(mapSpaceAngle);
    simd::float2 nextH = pointAtInf.xy;
    simd::float2 stepH = {0, 0};
    bool advanceH = false;
    if (fabs(cosA) > epsilon) {
        nextH.x = (cosA < 0 ? floor(Player::position.x / MAP_TILE_SIZE) : ceil(Player::position.x / MAP_TILE_SIZE)) * MAP_TILE_SIZE;
        nextH.y = Player::position.y + (nextH.x - Player::position.x) * sinA / cosA;
        stepH = {MAP_TILE_SIZE * sign(cosA), fabs(MAP_TILE_SIZE * sinA / cosA) * sign(sinA)};
        advanceH = true;
    }
    simd::float2 nextV = pointAtInf.xy;
    simd::float2 stepV = {0, 0};
    bool advanceV = false;
    if (fabs(sinA) > epsilon) {
        nextV.y = (sinA < 0 ? floor(Player::position.y / MAP_TILE_SIZE) : ceil(Player::position.y / MAP_TILE_SIZE)) * MAP_TILE_SIZE;
        nextV.x = Player::position.x + (nextV.y - Player::position.y) * cosA / sinA;
        stepV = {fabs(MAP_TILE_SIZE * cosA / sinA) * sign(cosA), MAP_TILE_SIZE * sign(sinA)};
        advanceV = true;
    }
    return {
        .normal = {cosA, sinA},
        .rayH = {
            .position = pointAtInf.xy,
            .step = stepH,
            .next = nextH,
            .tile = tileMiss,
            .advance = advanceH,
        },
        .rayV = {
            .position = pointAtInf.xy,
            .step = stepV,
            .next = nextV,
            .tile = tileMiss,
            .advance = advanceV,
        },
    };
}

/**
 * Returns tile's upper left corner position in map space.
 */
simd::float2 makeTilePosition(int col, int row) {
    return simd::float2{float(col), float(row)} * MAP_TILE_SIZE;
}

bool castRay(RayState& state, Ray& ray) {
    if (!state.rayH.advance && !state.rayV.advance) return false;

    float cosA = state.normal.x;
    float sinA = state.normal.y;

    // Scan columns
    if (state.rayH.advance) {
        RayComponent& rayH = state.rayH;
        rayH.position = rayH.next;
        rayH.next = pointAtInf.xy;
        rayH.tile = tileMiss;
        for (; inMapBounds(rayH.position); rayH.position += rayH.step) {
            int row = floor(rayH.position.y / MAP_TILE_SIZE);
            int col = floor(rayH.position.x / MAP_TILE_SIZE) - float(cosA < 0);
            int tileIndex = row * MAP_WIDTH + col;
            Tile tile = Map::tiles[tileIndex];
            if (isDoor(tile)) {
                simd::float2 tilePosition = makeTilePosition(col, row);
                std::array<simd::float2, 2> raySegment = {rayH.position - tilePosition, rayH.position - tilePosition + rayH.step};
                Intersection intersection;
                if (findClosestIntersection(raySegment, tile == Tile::doorH ? doorH : doorV, intersection)) {
                    rayH.next = rayH.position + rayH.step;
                    rayH.position += intersection.point - raySegment[0];
                    rayH.tile = {
                        .index = tileIndex,
                        .offset = doorSegmentOffset(tile, intersection.segmentIndex, intersection.segmentOffset),
                        .angle = atan(fabs(tile == Tile::doorH ? cosA / sinA : sinA / cosA)) * 2 / pi,
                        .side = cosA < 0 ? TileSide::right : TileSide::left,
                    };
                    break;
                }
            } else if (isWall(tile)) {
                rayH.tile = {
                    .index = tileIndex,
                    .offset = invertIf(cosA < 0, (rayH.position.y - row * MAP_TILE_SIZE) / MAP_TILE_SIZE),
                    .angle = atan(fabs(sinA / cosA)) * 2 / pi,
                    .side = cosA < 0 ? TileSide::right : TileSide::left,
                };
                break;
            }
        }
    }

    // Scan rows
    if (state.rayV.advance) {
        RayComponent& rayV = state.rayV;
        rayV.position = rayV.next;
        rayV.next = pointAtInf.xy;
        rayV.tile = tileMiss;
        for (; inMapBounds(rayV.position); rayV.position += rayV.step) {
            int row = floor(rayV.position.y / MAP_TILE_SIZE) - float(sinA < 0);
            int col = floor(rayV.position.x / MAP_TILE_SIZE);
            int tileIndex = row * MAP_WIDTH + col;
            Tile tile = Map::tiles[tileIndex];
            if (isDoor(tile)) {
                simd::float2 tilePosition = makeTilePosition(col, row);
                std::array<simd::float2, 2> raySegment = {rayV.position - tilePosition, rayV.position - tilePosition + rayV.step};
                Intersection intersection;
                if (findClosestIntersection(raySegment, tile == Tile::doorH ? doorH : doorV, intersection)) {
                    rayV.next = rayV.position + rayV.step;
                    rayV.position += intersection.point - raySegment[0];
                    rayV.tile = {
                        .index = tileIndex,
                        .offset = doorSegmentOffset(tile, intersection.segmentIndex, intersection.segmentOffset),
                        .angle = atan(fabs(tile == Tile::doorH ? cosA / sinA : sinA / cosA)) * 2 / pi,
                        .side = sinA < 0 ? TileSide::bottom : TileSide::top,
                    };
                    break;
                }
            } else if (isWall(tile)) {
                rayV.tile = {
                    .index = tileIndex,
                    .offset = invertIf(sinA > 0, (rayV.position.x - col * MAP_TILE_SIZE) / MAP_TILE_SIZE),
                    .angle = atan(fabs(cosA / sinA)) * 2 / pi,
                    .side = sinA < 0 ? TileSide::bottom : TileSide::top,
                };
                break;
            }
        }
    }

    // Choose the shortest ray component
    simd::float2 posH = state.rayH.position - Player::position.xy;
    simd::float2 posV = state.rayV.position - Player::position.xy;
    if (simd::length(posH) < simd::length(posV)) {
        float lengthH = simd::dot(posH, simd::float2{cos(Player::angle), sin(Player::angle)});
        ray.xy = posH;
        ray.length = lengthH;
        ray.tile = lengthH > CAMERA_NEAR_CLIP ? state.rayH.tile : tileMiss;
        state.rayH.advance = state.rayH.next.x != bigFloat;
        state.rayV.advance = false;
        return !ray.isMiss();
    } else {
        float lengthV = simd::dot(posV, simd::float2{cos(Player::angle), sin(Player::angle)});
        ray.xy = posV;
        ray.length = lengthV;
        ray.tile = lengthV > CAMERA_NEAR_CLIP ? state.rayV.tile : tileMiss;
        state.rayH.advance = false;
        state.rayV.advance = state.rayV.next.x != bigFloat;
        return !ray.isMiss();
    }
}

Ray castRay(float playerSpaceAngle, bool tracer) {
    RayState state = makeRayState(playerSpaceAngle);
    Ray ray;
    while (castRay(state, ray)) {
    }
    return ray;
}

Ray castRayToFirstHit(float playerSpaceAngle) {
    RayState state = makeRayState(playerSpaceAngle);
    Ray ray;
    castRay(state, ray);
    return ray;
}

/**
 * Returns a polygon in map space for the specified door tile.
 */
std::array<simd::float2, 4> makeDoorPolygon(int tileIndex) {
    simd::float2 tilePosition = makeTilePosition(tileIndex % MAP_WIDTH, tileIndex / MAP_WIDTH);
    std::array<simd::float2, 4> doorPolygon = (Map::tiles[tileIndex] == Tile::doorH ? doorH : doorV);
    for (simd::float2& p : doorPolygon) {
        p += tilePosition;
    }
    return doorPolygon;
}

/**
 * Checks if the point belongs to the polygon. Assumes polygon has CCW winding.
 */
template <size_t N>
bool inPolygonBounds(simd::float2 point, const std::array<simd::float2, N>& polygon) {
    for (int i = 0; i < N; ++i) {
        int j = (i + 1) % N;
        float crossProduct = simd::cross(polygon[j] - polygon[i], point - polygon[i])[2];
        if (crossProduct > -epsilon) {
            return false;
        }
    }
    return true;
}

void drawWall(const Ray& ray, float beginY, float endY, float wallHeight, float textureOffsetY, int x, int& y) {
    y = fmax(y, ceil(beginY));
    int end = fmin(floor(endY) + 1, CANVAS_HEIGHT);
    if (y >= end) {
        // Wall is fully obstructed, nothing to draw.
        return;
    }
    if (ray.length > CAMERA_FAR_CLIP) {
        for (; y < end; ++y) {
            Canvas::point(x, y, Palette::fogColor);
        }
        return;
    }
    uint32_t* texture = Textures::getTexture(Map::tiles[ray.tile.index]);
    float distanceCoef = ray.length * 2 / CAMERA_FAR_CLIP;
    float angleCoef = 1 - ray.tile.angle / 2;
    for (; y < end; ++y) {
        uint32_t color = sampleTexture(texture, ray.tile.offset, (y - beginY) / wallHeight + textureOffsetY);
        if (distanceCoef < 1) {
            color = Palette::blend(color, Palette::lightColor, angleCoef * (1 - distanceCoef) * 0xA0, BlendMode::add);
        } else {
            color = Palette::blend(color, Palette::shadowColor, (distanceCoef - 1) * 0xA0, BlendMode::multipy);
        }
        color = Palette::blend(color, Palette::fogColor, ray.length / CAMERA_FAR_CLIP * 0xFF, BlendMode::normal);
        Canvas::point(x, y, color);
    }
}

void drawDoorBottom(const Ray& ray, int x, int& y) {
    float doorProgress = Map::doors[ray.tile.index].progress;
    float doorBottomHeight = MAP_TILE_SIZE * (1 - doorProgress);
    if (cameraHeight > doorBottomHeight) {
        // Door bottom is not visible, nothing to draw.
        return;
    }
    simd::float3x3 playerToMapTransform = makePlayerToMapTransform();
    std::array<simd::float2, 4> doorPolygon = makeDoorPolygon(ray.tile.index);
    uint32_t* doorTexture = Textures::getTexture(Map::tiles[ray.tile.index]);
    float distanceCoef = ray.length * 2 / CAMERA_FAR_CLIP;
    for (; y < horizonY; ++y) {
        simd::float3 hit = matrix_multiply(playerToMapTransform, ceilingHit(x, y, doorBottomHeight));
        if (!inPolygonBounds(hit.xy, doorPolygon)) {
            break;
        }
        simd::float3 texturePos = simd::fmod(hit, mapTile) / mapTile;
        uint32_t color = sampleTexture(doorTexture, Map::tiles[ray.tile.index] == Tile::doorH ? texturePos.x : texturePos.y, 1.0f);
        if (distanceCoef < 1) {
            color = Palette::blend(color, Palette::lightColor, (1 - distanceCoef) * 0x70, BlendMode::add);
        } else {
            color = Palette::blend(color, Palette::shadowColor, (distanceCoef - 1) * 0xA0, BlendMode::multipy);
        }
        color = Palette::blend(color, Palette::fogColor, ray.length / CAMERA_FAR_CLIP * 0xFF, BlendMode::normal);
        Canvas::point(x, y, color);
    }
}

void drawWalls() {
    for (int x = 0; x < rayAnglesHorizontal.size(); ++x) {
        RayState state = makeRayState(rayAnglesHorizontal[x]);
        Ray ray;
        for (int y = 0; y < CANVAS_HEIGHT && castRay(state, ray);) {
            float projectionCoef = projectionDistance / ray.length;
            float wallHeight = MAP_TILE_SIZE * projectionCoef;
            float beginY = horizonY - (MAP_TILE_SIZE - cameraHeight) * projectionCoef;
            if (!isDoor(Map::tiles[ray.tile.index])) {
                float endY = beginY + wallHeight;
                drawWall(ray, beginY, endY, wallHeight, 0, x, y);
            } else {
                float doorProgress = Map::doors[ray.tile.index].progress;
                float endY = beginY + wallHeight * doorProgress;
                float textureOffsetY = 1 - doorProgress;
                drawWall(ray, beginY, endY, wallHeight, textureOffsetY, x, y);
                drawDoorBottom(ray, x, y);
            }
        }
    }
}

void draw() {
    if (Map::isVisible() && Map::isFullScreen()) return;
    drawCeiling();
    drawFloor();
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

    cameraHeight = MAP_TILE_SIZE / 2.0f + sin(phase * pi) * amplitude;
}

void update() {
    handleInputs();
    bounceCamera();
}

} // namespace RC::Viewport
