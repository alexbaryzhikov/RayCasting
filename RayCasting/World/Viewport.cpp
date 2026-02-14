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

namespace RC::Viewport {

constexpr float pi = std::numbers::pi_v<float>;
constexpr float eps = std::numeric_limits<float>::epsilon();
constexpr float inf = 1e10;

std::array<simd::float2, 4> doorH = {
    simd::float2{0, (MAP_TILE_SIZE - DOOR_DEPTH) / 2},
    simd::float2{0, (MAP_TILE_SIZE + DOOR_DEPTH) / 2},
    simd::float2{MAP_TILE_SIZE, (MAP_TILE_SIZE + DOOR_DEPTH) / 2},
    simd::float2{MAP_TILE_SIZE, (MAP_TILE_SIZE - DOOR_DEPTH) / 2},
};

std::array<simd::float2, 4> doorV = {
    simd::float2{(MAP_TILE_SIZE - DOOR_DEPTH) / 2, 0},
    simd::float2{(MAP_TILE_SIZE - DOOR_DEPTH) / 2, MAP_TILE_SIZE},
    simd::float2{(MAP_TILE_SIZE + DOOR_DEPTH) / 2, MAP_TILE_SIZE},
    simd::float2{(MAP_TILE_SIZE + DOOR_DEPTH) / 2, 0},
};

float cameraPositionZ = MAP_TILE_SIZE / 2.0f; // map space

float sign(float value) {
    return value < 0 ? -1 : 1;
}

float invertIf(bool condition, float value) {
    return condition ? 1 - value : value;
}

bool inMapBounds(int row, int col) {
    return row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH;
}

bool atInf(simd::float2 point) {
    return point.x == inf || point.y == inf;
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

    if (fabs(determinant) < eps) {
        // Lines are parallel
        return false;
    }

    // Calculate 't' and 'u'
    float t = (d2.x * delta.y - d2.y * delta.x) / determinant;
    float u = (d1.x * delta.y - d1.y * delta.x) / determinant;

    // Check if the intersection point lies on both line segments
    // The parameters 't' and 'u' must be in the range [0, 1]
    if (t > -eps && t < 1 + eps && u > -eps && u < 1 + eps) {
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
    float minLength = inf;
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
    return minLength < inf;
}

struct RayComponent {
    simd::float2 position; // ray position in map space
    simd::float2 step;     // offset between adjacent grid-aligned positions
};

struct RayState {
    simd::float2 normal; // ray direction in map space
    RayComponent rayX;   // horizontal component
    RayComponent rayY;   // vertical component
};

RayState makeRayState(simd::float2 position, float angle) {
    RayState state = {
        .normal = {cos(angle), sin(angle)},
        .rayX = {
            .position = {inf, inf},
            .step = {0, 0},
        },
        .rayY = {
            .position = {inf, inf},
            .step = {0, 0},
        },
    };

    if (fabs(state.normal.x) > eps) {
        state.rayX.position.x = (state.normal.x < 0 ? floor(position.x / MAP_TILE_SIZE) : ceil(position.x / MAP_TILE_SIZE)) * MAP_TILE_SIZE;
        state.rayX.position.y = position.y + (state.rayX.position.x - position.x) * state.normal.y / state.normal.x;
        state.rayX.step = {MAP_TILE_SIZE * sign(state.normal.x), fabs(MAP_TILE_SIZE * state.normal.y / state.normal.x) * sign(state.normal.y)};
    }

    if (fabs(state.normal.y) > eps) {
        state.rayY.position.y = (state.normal.y < 0 ? floor(position.y / MAP_TILE_SIZE) : ceil(position.y / MAP_TILE_SIZE)) * MAP_TILE_SIZE;
        state.rayY.position.x = position.x + (state.rayY.position.y - position.y) * state.normal.x / state.normal.y;
        state.rayY.step = {fabs(MAP_TILE_SIZE * state.normal.x / state.normal.y) * sign(state.normal.x), MAP_TILE_SIZE * sign(state.normal.y)};
    }

    return state;
}

Ray makeRay() {
    return {
        .position = {0, 0},
        .length = 0,
        .tileIndex = -1,
    };
}

simd::float2 makeTilePosition(int col, int row) {
    return simd::float2{float(col), float(row)} * MAP_TILE_SIZE;
}

bool castRay(RayState& state, Ray& ray) {
    float lengthX = atInf(state.rayX.position) ? inf : simd::length(state.rayX.position - Player::position.xy);
    float lengthY = atInf(state.rayY.position) ? inf : simd::length(state.rayY.position - Player::position.xy);

    if (lengthX < lengthY) {
        int row = floor(state.rayX.position.y / MAP_TILE_SIZE);
        int col = floor(state.rayX.position.x / MAP_TILE_SIZE) - float(state.normal.x < 0);
        if (!inMapBounds(row, col)) {
            return false;
        }
        int tileIndex = row * MAP_WIDTH + col;
        TileType tileType = Map::tiles[tileIndex];
        if (isWall(tileType)) {
            ray.position = state.rayX.position;
            ray.length = lengthX;
            ray.tileIndex = tileIndex;
        } else if (isDoor(tileType)) {
            simd::float2 tilePosition = makeTilePosition(col, row);
            std::array<simd::float2, 2> raySegment = {state.rayX.position - tilePosition, state.rayX.position - tilePosition + state.rayX.step};
            std::array<simd::float2, 4>& doorPolygon = tileType == TileType::doorH ? doorH : doorV;
            Intersection intersection;
            if (findClosestIntersection(raySegment, doorPolygon, intersection) && !Map::doors[tileIndex].isPassable()) {
                ray.position = state.rayX.position + intersection.point - raySegment[0];
                ray.length = lengthX;
                ray.tileIndex = tileIndex;
            }
        }
        state.rayX.position += state.rayX.step;
        return true;
    }

    if (lengthY < inf) {
        int row = floor(state.rayY.position.y / MAP_TILE_SIZE) - float(state.normal.y < 0);
        int col = floor(state.rayY.position.x / MAP_TILE_SIZE);
        if (!inMapBounds(row, col)) {
            return false;
        }
        int tileIndex = row * MAP_WIDTH + col;
        TileType tileType = Map::tiles[tileIndex];
        if (isWall(tileType)) {
            ray.position = state.rayY.position;
            ray.length = lengthY;
            ray.tileIndex = tileIndex;
        } else if (isDoor(tileType)) {
            simd::float2 tilePosition = makeTilePosition(col, row);
            std::array<simd::float2, 2> raySegment = {state.rayY.position - tilePosition, state.rayY.position - tilePosition + state.rayY.step};
            std::array<simd::float2, 4>& doorPolygon = tileType == TileType::doorH ? doorH : doorV;
            Intersection intersection;
            if (findClosestIntersection(raySegment, doorPolygon, intersection) && !Map::doors[tileIndex].isPassable()) {
                ray.position = state.rayY.position + intersection.point - raySegment[0];
                ray.length = lengthY;
                ray.tileIndex = tileIndex;
            }
        }
        state.rayY.position += state.rayY.step;
        return true;
    }

    return false;
}

Ray castRay(float playerSpaceAngle) {
    RayState state = makeRayState(Player::position.xy, Player::angle + playerSpaceAngle);
    Ray ray = makeRay();
    while (castRay(state, ray)) {
        if (!ray.isMiss()) {
            ray.position -= Player::position.xy;
            break;
        }
    }
    return ray;
}

void bounceCamera() {
    const float freqency = 0.01f;
    const float amplitude = 2.0f;

    static float phase = 0;
    phase += simd::length(Player::velocity) * freqency;
    if (phase > 1) phase = 0;

    cameraPositionZ = MAP_TILE_SIZE / 2.0f + sin(phase * pi) * amplitude;
}

void update() {
    bounceCamera();
}

} // namespace RC::Viewport
