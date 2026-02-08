#include <metal_stdlib>
using namespace metal;

#include "GPUShaderInternalTypes.hpp"

constant constexpr float pi = M_PI_F;
constant constexpr float eps = FLT_EPSILON;
constant constexpr float inf = 1e10;
constant constexpr float4 pointAtZero = {0, 0, 0, 1};
constant constexpr float4 pointAtInf = {inf, inf, inf, 1};

constant constexpr float horizonY = float(VIEWPORT_HEIGHT) / 2; // horizon coordinate in screen space
constant constexpr float2 viewportMaxCoord = float2(VIEWPORT_WIDTH, VIEWPORT_HEIGHT) - 1;

constant constexpr float mapWidth = MAP_WIDTH * MAP_TILE_SIZE;
constant constexpr float mapHeight = MAP_HEIGHT * MAP_TILE_SIZE;
constant constexpr float mapDepth = MAP_TILE_SIZE;
constant constexpr float4 mapBounds = {mapWidth, mapHeight, mapDepth, 1};


constant constexpr array<float2, 4> doorH = {
    float2(0, (MAP_TILE_SIZE - DOOR_DEPTH) / 2),
    float2(0, (MAP_TILE_SIZE + DOOR_DEPTH) / 2),
    float2(MAP_TILE_SIZE, (MAP_TILE_SIZE + DOOR_DEPTH) / 2),
    float2(MAP_TILE_SIZE, (MAP_TILE_SIZE - DOOR_DEPTH) / 2),
};

constant constexpr array<float2, 4> doorV = {
    float2((MAP_TILE_SIZE - DOOR_DEPTH) / 2, 0),
    float2((MAP_TILE_SIZE - DOOR_DEPTH) / 2, MAP_TILE_SIZE),
    float2((MAP_TILE_SIZE + DOOR_DEPTH) / 2, MAP_TILE_SIZE),
    float2((MAP_TILE_SIZE + DOOR_DEPTH) / 2, 0),
};

constant constexpr float3 colorFog = float3(0x07, 0x00, 0x16) / 0xFF;
constant constexpr float3 colorLight = float3(0xE6, 0xb5, 0x97) / 0xFF;
constant constexpr float3 colorShadow = float3(0x01, 0x5A, 0x00) / 0xFF;

float3 sRGBToLinear(float3 srgb) {
    return select(srgb / 12.92f, pow((srgb + 0.055f) / 1.055f, 2.4f), srgb > 0.04045f);
}

float3 applyIntensity(float3 color, float intensity) {
    return select(1.0f - (1.0f - color) / intensity, color * intensity, intensity <= 1.0f);
}

float4 blendNormal(float4 src, float4 dst) {
    return float4(dst.rgb * (1.0f - src.a) + src.rgb * src.a, 1.0f);
}

float4 blendAdd(float4 src, float4 dst) {
    return float4(dst.rgb + src.rgb * src.a, 1.0f);
}

float4 blendMultiply(float4 src, float4 dst) {
    return float4(dst.rgb * (1.0f - (1.0f - src.rgb) * src.a), 1.0f);
}

float4 blendDodge(float4 src, float4 dst) {
    return float4(dst.rgb * (1.0f - (1.0f - 1.0f / max(1.0f - src.rgb, 0.0001f)) * src.a), 1.0f);
}

float4x4 makeTranslationMatrix(float4 t) {
    return {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {t.x, t.y, t.z, 1},
    };
}

float4x4 makeRotationZMatrix(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return {
        {c, s, 0, 0},
        {-s, c, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    };
}

TextureIndex getTextureIndex(thread const Tile& tile) {
    switch (tile.side) {
        case TileSideLeft:
        case TileSideRight:
        case TileSideTop:
        case TileSideBottom:
            switch (tile.type) {
                case TileTypeDoorH:
                case TileTypeDoorV:
                    return TextureIndexDoor;
                case TileTypeEmpty:
                case TileTypeWall:
                    return TextureIndexWall;
                case TileTypeWallFortified:
                    return TextureIndexWallFortified;
                case TileTypeWallIndestructible:
                    return TextureIndexWallIndestructible;
            }
        case TileSideCeiling:
            return TextureIndexCeiling;
        case TileSideFloor:
            return TextureIndexFloor;
    }
}

float4 getRayColor(texture2d<float, access::sample> tileTexture, thread const Ray& ray) {
    if (ray.length > CAMERA_FAR_CLIP) {
        return float4(sRGBToLinear(colorFog), 1);
    }
    const sampler textureSampler(coord::normalized, address::repeat, filter::nearest);
    float2 readCoord = ray.tile.offset;
    float4 outColor = tileTexture.sample(textureSampler, readCoord);
    float4 shadowColor = float4(sRGBToLinear(colorShadow), ray.length / CAMERA_FAR_CLIP);
    outColor = blendMultiply(shadowColor, outColor);
    float lightFalloff = 1.0f - min(1.0f, ray.length / PLAYER_LIGHT_RADIUS);
    float lightSlope = 1.0f - ray.tile.slope * 0.8f;
    float4 lightColor = float4(applyIntensity(sRGBToLinear(colorLight), PLAYER_LIGHT_INTENSITY), lightFalloff * lightSlope);
    outColor = blendDodge(lightColor, outColor);
    float4 fogColor = float4(sRGBToLinear(colorFog), ray.length / CAMERA_FAR_CLIP);
    outColor = blendNormal(fogColor, outColor);
    return outColor;
}

float4 getOverlayColor(texture2d<float, access::sample> overlayTexture, uint2 pixel) {
    const sampler textureSampler(coord::normalized, address::clamp_to_edge, filter::nearest);
    float2 readCoord = float2(pixel) / viewportMaxCoord;
    return overlayTexture.sample(textureSampler, readCoord);
}

/**
 * Returns a direction vector in world space of the ray cast from camera to pixel of projection plane.
 */
float4 getRayDirection(float cameraAngle, uint2 pixel) {
    const float projectionDistance = (VIEWPORT_WIDTH / 2.0f) / tan(CAMERA_FOV / 2.0f);
    float u = (pixel.x + 0.5f) - VIEWPORT_WIDTH / 2.0f;
    float v = (pixel.y + 0.5f) - horizonY;
    float4 direction = {projectionDistance, u, -v, 0.0f};
    return normalize(makeRotationZMatrix(cameraAngle) * direction);
}

/**
 * Returns a traversal state for ray cast.
 */
RayState makeRayState(constant const Camera& camera, uint2 pixel) {
    float4 direction = getRayDirection(camera.angle, pixel);
    float2 normalH = normalize(direction.xy);
    float2 normalV = {length(direction.xy), direction.z};

    RayState state = {
        .normalH = normalH,
        .normalV = normalV,
        .rayX = {
            .position = pointAtInf,
            .step = {0, 0, 0, 0},
        },
        .rayY = {
            .position = pointAtInf,
            .step = {0, 0, 0, 0},
        },
        .rayZ = {
            .position = pointAtInf,
            .step = {0, 0, 0, 0},
        },
    };

    if (fabs(normalH.x) > eps && fabs(normalV.x) > eps) {
        state.rayX.position.x = select(floor(camera.position.x / MAP_TILE_SIZE) * MAP_TILE_SIZE,
                                       ceil(camera.position.x / MAP_TILE_SIZE) * MAP_TILE_SIZE,
                                       normalH.x > 0);
        float positionDXY = fabs((state.rayX.position.x - camera.position.x) / normalH.x);
        float positionDXYZ = fabs(positionDXY / normalV.x);
        state.rayX.position.y = camera.position.y + positionDXY * normalH.y;
        state.rayX.position.z = camera.position.z + positionDXYZ * normalV.y;
        float stepDXY = fabs(MAP_TILE_SIZE / normalH.x);
        float stepDXYZ = fabs(stepDXY / normalV.x);
        state.rayX.step.x = MAP_TILE_SIZE * sign(normalH.x);
        state.rayX.step.y = stepDXY * normalH.y;
        state.rayX.step.z = stepDXYZ * normalV.y;
    }

    if (fabs(normalH.y) > eps && fabs(normalV.x) > eps) {
        state.rayY.position.y = select(floor(camera.position.y / MAP_TILE_SIZE) * MAP_TILE_SIZE,
                                       ceil(camera.position.y / MAP_TILE_SIZE) * MAP_TILE_SIZE,
                                       normalH.y > 0);
        float positionDXY = fabs((state.rayY.position.y - camera.position.y) / normalH.y);
        float positionDXYZ = fabs(positionDXY / normalV.x);
        state.rayY.position.x = camera.position.x + positionDXY * normalH.x;
        state.rayY.position.z = camera.position.z + positionDXYZ * normalV.y;
        float stepDXY = fabs(MAP_TILE_SIZE / normalH.y);
        float stepDXYZ = fabs(stepDXY / normalV.x);
        state.rayY.step.x = stepDXY * normalH.x;
        state.rayY.step.y = MAP_TILE_SIZE * sign(normalH.y);
        state.rayY.step.z = stepDXYZ * normalV.y;
    }

    if (fabs(normalV.y) > eps) {
        state.rayZ.position.z = select(floor(camera.position.z / MAP_TILE_SIZE) * MAP_TILE_SIZE,
                                   ceil(camera.position.z / MAP_TILE_SIZE) * MAP_TILE_SIZE,
                                   normalV.y > 0);
        float positionDXYZ = fabs((state.rayZ.position.z - camera.position.z) / normalV.y);
        float positionDXY = fabs(positionDXYZ * normalV.x);
        state.rayZ.position.x = camera.position.x + positionDXY * normalH.x;
        state.rayZ.position.y = camera.position.y + positionDXY * normalH.y;
        float stepDXYZ = fabs(MAP_TILE_SIZE / normalV.y);
        float stepDXY = fabs(stepDXYZ * normalV.x);
        state.rayZ.step.x = stepDXY * normalH.x;
        state.rayZ.step.y = stepDXY * normalH.y;
        state.rayZ.step.z = MAP_TILE_SIZE * sign(normalV.y);
    }

    return state;
}

/**
 * Returns a ray at infinity.
 */
Ray makeRay() {
    return {
        .position = pointAtInf,
        .length = inf,
        .tile = {
            .type = TileTypeEmpty,
            .side = TileSideFloor,
            .offset = {0, 0},
            .slope = 0,
        },
        .miss = true,
    };
}

/**
 *  Returns full translation vector based on horizontal component and slope normal.
 *  Assumes normal is not strictly vertical.
 */
float4 getDelta(float2 offsetH, float2 normalV) {
    float4 delta = {0, 0, 0, 0};
    delta.xy = offsetH;
    delta.z = length(offsetH) * normalV.y / normalV.x;
    return delta;
}

/**
 * Returns tile's upper left corner position in world space.
 */
float2 makeTilePosition(int col, int row) {
    return float2{float(col), float(row)} * MAP_TILE_SIZE;
}

/**
 * Returns true if point is inside or on map boundary.
 */
bool inMapBounds(float4 point) {
    return all(point >= pointAtZero - eps) && all(point <= mapBounds + eps);
}

/**
 * Returns true if tile coordinates are valid.
 */
bool inMapBounds(int row, int col) {
    return row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH;
}

bool isDoor(TileType type) {
    switch (type) {
        case TileTypeDoorH:
        case TileTypeDoorV:
            return true;
        default:
            return false;
    }
}

bool isWall(TileType type) {
    switch (type) {
        case TileTypeWall:
        case TileTypeWallFortified:
        case TileTypeWallIndestructible:
            return true;
        default:
            return false;
    }
}

float invertIf(float value, bool condition) {
    return select(value, 1 - value, condition);
}

/**
 * Find intersection point between two line segments.
 */
bool findIntersection(float2 a1, float2 b1, float2 a2, float2 b2, thread float2& point, thread float& offset) {
    // Solve parametric equation:
    // 't' is the parameter for the first line segment (a1, b1)
    // 'u' is the parameter for the second line segment (a2, b2)
    // a1 + t(b1 - a1) = a2 + u(b2 - a2)
    // Rearrange and separate by component:
    // t(b1.x - a1.x) - u(b2.x - a2.x) = a2.x - a1.x
    // t(b1.y - a1.y) - u(b2.y - a2.y) = a2.y - a1.y

    // Calculate coefficients
    float2 d1 = b1 - a1;
    float2 d2 = b2 - a2;
    float2 delta = a2 - a1;

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
 * Find intersection point between ray and polygon segment closest to the ray start.
 */
template <size_t N>
bool findClosestIntersection(thread const array<float2, 2>& ray,
                             constant const array<float2, N>& polygon,
                             thread Intersection& intersection) {
    float minLength = inf;
    float2 point;
    float offset;
    for (size_t i = 0; i < N; ++i) {
        if (findIntersection(ray[0], ray[1], polygon[i], polygon[(i + 1) % N], point, offset)) {
            float len = length(point - ray[0]);
            if (len < minLength) {
                minLength = len;
                intersection.point = point;
                intersection.segmentIndex = i;
                intersection.segmentOffset = offset;
            }
        }
    }
    return minLength < inf;
}

/**
 * Checks if the point belongs to the polygon. Assumes polygon has CCW winding.
 */
template <size_t N>
bool inPolygonBounds(float2 point, constant const array<float2, N>& polygon) {
    for (size_t i = 0; i < N; ++i) {
        size_t j = (i + 1) % N;
        float2 a = polygon[j] - polygon[i];
        float2 b = point - polygon[i];
        if (a.x * b.y - a.y * b.x > 0) {
            return false;
        }
    }
    return true;
}

float doorSegmentOffset(TileType door, int segmentIndex, float segmentOffset) {
    if (door == TileTypeDoorH) {
        return select(0.0f, segmentOffset, segmentIndex == 1 || segmentIndex == 3);
    } else {
        return select(0.0f, segmentOffset, segmentIndex == 0 || segmentIndex == 2);
    }
}

bool castRay(constant Camera& camera, constant Map& map, thread RayState& state, thread Ray& ray) {
    const float2 normalH = state.normalH;
    const float2 normalV = state.normalV;

    float lengthX = select(inf, length(state.rayX.position - camera.position), inMapBounds(state.rayX.position));
    float lengthY = select(inf, length(state.rayY.position - camera.position), inMapBounds(state.rayY.position));
    float lengthZ = select(inf, length(state.rayZ.position - camera.position), inMapBounds(state.rayZ.position));

    if (lengthX < lengthY && lengthX < lengthZ) {
        int row = floor(state.rayX.position.y / MAP_TILE_SIZE);
        int col = floor(state.rayX.position.x / MAP_TILE_SIZE) - float(normalH.x < 0);
        if (!inMapBounds(row, col)) {
            return false;
        }
        int tileIndex = row * MAP_WIDTH + col;
        TileType tileType = map.tiles[tileIndex];
        if (isWall(tileType)) {
            ray.position = state.rayX.position;
            ray.length = lengthX;
            ray.tile = {
                .type = tileType,
                .side = normalH.x < 0 ? TileSideRight : TileSideLeft,
                .offset = {
                    invertIf((ray.position.y - row * MAP_TILE_SIZE) / MAP_TILE_SIZE, normalH.x < 0),
                    1 - ray.position.z / MAP_TILE_SIZE
                },
                .slope = atan(fabs(normalH.y / normalH.x)) * 2 / pi,
            };
            ray.miss = false;
        } else if (isDoor(tileType)) {
            float2 tilePosition = makeTilePosition(col, row);
            array<float2, 2> raySegment = {
                state.rayX.position.xy - tilePosition,
                state.rayX.position.xy - tilePosition + state.rayX.step.xy
            };
            constant const array<float2, 4>& doorPolygon = tileType == TileTypeDoorH ? doorH : doorV;
            Intersection intersection;
            if (findClosestIntersection(raySegment, doorPolygon, intersection)) {
                float4 rayPosition = state.rayX.position + getDelta(intersection.point - raySegment[0], normalV);
                float rayLength = length(rayPosition - camera.position);
                if (rayLength < lengthZ) {
                    float doorBottom = (1 - map.doors[tileIndex].progress) * MAP_TILE_SIZE;
                    if (doorBottom < rayPosition.z) {
                        ray.position = rayPosition;
                        ray.length = rayLength;
                        ray.tile = {
                            .type = tileType,
                            .side = normalH.x < 0 ? TileSideRight : TileSideLeft,
                            .offset = {
                                doorSegmentOffset(tileType, intersection.segmentIndex, intersection.segmentOffset),
                                1 - (ray.position.z - doorBottom) / MAP_TILE_SIZE,
                            },
                            .slope = atan(fabs(tileType == TileTypeDoorH ? normalH.x / normalH.y : normalH.y / normalH.x)) * 2 / pi,
                        };
                        ray.miss = false;
                    } else if (normalV.y > 0) {
                        float dZ = fabs(doorBottom - state.rayX.position.z);
                        float dXYZ = fabs(dZ / normalV.y);
                        float dXY = fabs(dXYZ * normalV.x);
                        float4 rayOffset = float4(normalH * dXY, dZ, 0);
                        float2 tilePoint = raySegment[0] + rayOffset.xy;
                        if (inPolygonBounds(tilePoint, doorPolygon)) {
                            ray.position = state.rayX.position + rayOffset;
                            ray.length = lengthX + dXYZ;
                            ray.tile = {
                                .type = tileType,
                                .side = normalH.x < 0 ? TileSideRight : TileSideLeft,
                                .offset = {(tileType == TileTypeDoorH ? tilePoint.x : tilePoint.y) / MAP_TILE_SIZE, 1},
                                .slope = atan(fabs(normalV.x / normalV.y)) * 2 / pi,
                            };
                            ray.miss = false;
                        }
                    }
                }
            }
        }
        state.rayX.position += state.rayX.step;
        return true;
    }

    if (lengthY < lengthZ) {
        int row = floor(state.rayY.position.y / MAP_TILE_SIZE) - float(normalH.y < 0);
        int col = floor(state.rayY.position.x / MAP_TILE_SIZE);
        if (!inMapBounds(row, col)) {
            return false;
        }
        int tileIndex = row * MAP_WIDTH + col;
        TileType tileType = map.tiles[tileIndex];
        if (isWall(tileType)) {
            ray.position = state.rayY.position;
            ray.length = lengthY;
            ray.tile = {
                .type = tileType,
                .side = normalH.y < 0 ? TileSideBottom : TileSideTop,
                .offset = {
                    invertIf((ray.position.x - col * MAP_TILE_SIZE) / MAP_TILE_SIZE, normalH.y > 0),
                    1 - ray.position.z / MAP_TILE_SIZE
                },
                .slope = atan(fabs(normalH.x / normalH.y)) * 2 / pi,
            };
            ray.miss = false;
        } else if (isDoor(tileType)) {
            float2 tilePosition = makeTilePosition(col, row);
            array<float2, 2> raySegment = {
                state.rayY.position.xy - tilePosition,
                state.rayY.position.xy - tilePosition + state.rayY.step.xy
            };
            constant const array<float2, 4>& doorPolygon = tileType == TileTypeDoorH ? doorH : doorV;
            Intersection intersection;
            if (findClosestIntersection(raySegment, doorPolygon, intersection)) {
                float4 rayPosition = state.rayY.position + getDelta(intersection.point - raySegment[0], normalV);
                float rayLength = length(rayPosition - camera.position);
                if (rayLength < lengthZ) {
                    float doorBottom = (1 - map.doors[tileIndex].progress) * MAP_TILE_SIZE;
                    if (doorBottom < rayPosition.z) {
                        ray.position = rayPosition;
                        ray.length = rayLength;
                        ray.tile = {
                            .type = tileType,
                            .side = normalH.y < 0 ? TileSideBottom : TileSideTop,
                            .offset = {
                                doorSegmentOffset(tileType, intersection.segmentIndex, intersection.segmentOffset),
                                1 - (ray.position.z - doorBottom) / MAP_TILE_SIZE,
                            },
                            .slope = atan(fabs(tileType == TileTypeDoorH ? normalH.x / normalH.y : normalH.y / normalH.x)) * 2 / pi,
                        };
                        ray.miss = false;
                    } else if (normalV.y > 0) {
                        float dZ = fabs(doorBottom - state.rayY.position.z);
                        float dXYZ = fabs(dZ / normalV.y);
                        float dXY = fabs(dXYZ * normalV.x);
                        float4 rayOffset = float4(normalH * dXY, dZ, 0);
                        float2 tilePoint = raySegment[0] + rayOffset.xy;
                        if (inPolygonBounds(tilePoint, doorPolygon)) {
                            ray.position = state.rayY.position + rayOffset;
                            ray.length = lengthY + dXYZ;
                            ray.tile = {
                                .type = tileType,
                                .side = normalH.y < 0 ? TileSideBottom : TileSideTop,
                                .offset = {(tileType == TileTypeDoorH ? tilePoint.x : tilePoint.y) / MAP_TILE_SIZE, 1},
                                .slope = atan(fabs(normalV.x / normalV.y)) * 2 / pi,
                            };
                            ray.miss = false;
                        }
                    }
                }
            }
        }
        state.rayY.position += state.rayY.step;
        return true;
    }

    if (lengthZ == inf) {
        return false;
    }
    int row = floor(state.rayZ.position.y / MAP_TILE_SIZE);
    int col = floor(state.rayZ.position.x / MAP_TILE_SIZE);
    if (!inMapBounds(row, col)) {
        return false;
    }
    float2 tilePosition = makeTilePosition(col, row);
    ray.position = state.rayZ.position;
    ray.length = lengthZ;
    ray.tile = {
        .type = TileTypeEmpty,
        .side = normalV.y < 0 ? TileSideFloor : TileSideCeiling,
        .offset = (ray.position.xy - tilePosition) / MAP_TILE_SIZE,
        .slope = atan(fabs(normalV.x / normalV.y)) * 2 / pi,
    };
    ray.miss = false;
    state.rayZ.position += state.rayZ.step;
    return true;
}

Ray castRay(constant Camera& camera, constant Map& map, uint2 pixel) {
    RayState state = makeRayState(camera, pixel);
    Ray ray = makeRay();
    while (castRay(camera, map, state, ray) && ray.miss);
    return ray;
}

kernel void render(texture2d<float, access::write> outputTexture [[texture(0)]],
                   texture2d<float, access::sample> overlayTexture [[texture(1)]],
                   array<texture2d<float, access::sample>, TEXTURE_HEAP_SIZE> textures [[texture(2)]],
                   constant Camera& camera [[buffer(0)]],
                   constant Map& map [[buffer(1)]],
                   uint2 gid [[thread_position_in_grid]]) {
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    Ray ray = castRay(camera, map, gid);
    float4 rayColor = getRayColor(textures[getTextureIndex(ray.tile)], ray);
    float4 overlayColor = getOverlayColor(overlayTexture, gid);
    float4 outColor = blendNormal(overlayColor, rayColor);
    outputTexture.write(outColor, gid);
}
