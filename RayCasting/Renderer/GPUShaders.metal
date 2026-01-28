#include <metal_stdlib>
using namespace metal;

#include "GPUShaderInternalTypes.hpp"
#include "GPUShaderTypes.h"

constant constexpr float pi = M_PI_F;
constant constexpr float eps = FLT_EPSILON;
constant constexpr float inf = 1e10;
constant constexpr float4 pointAtZero = {0, 0, 0, 1};
constant constexpr float4 pointAtInf = {inf, inf, inf, 1};

constant constexpr float pixelAngle = float(CAMERA_FOV) / float(CANVAS_WIDTH); // angle between rays cast from camera to adjacent pixels of projection plane
constant constexpr float horizonY = float(CANVAS_HEIGHT) / 2;                  // horizon coordinate in screen space

constant constexpr float mapWidth = MAP_WIDTH * MAP_TILE_SIZE;
constant constexpr float mapHeight = MAP_HEIGHT * MAP_TILE_SIZE;
constant constexpr float4 mapBounds = {mapWidth, mapHeight, MAP_TILE_SIZE, 1};

constant constexpr TileHit tileMiss = {-1};

constant constexpr float2 doorH[4] = {
    {0, (MAP_TILE_SIZE - DOOR_DEPTH) / 2},
    {0, (MAP_TILE_SIZE + DOOR_DEPTH) / 2},
    {MAP_TILE_SIZE, (MAP_TILE_SIZE + DOOR_DEPTH) / 2},
    {MAP_TILE_SIZE, (MAP_TILE_SIZE - DOOR_DEPTH) / 2},
};

constant constexpr float2 doorV[4] = {
    {(MAP_TILE_SIZE - DOOR_DEPTH) / 2, 0},
    {(MAP_TILE_SIZE - DOOR_DEPTH) / 2, MAP_TILE_SIZE},
    {(MAP_TILE_SIZE + DOOR_DEPTH) / 2, MAP_TILE_SIZE},
    {(MAP_TILE_SIZE + DOOR_DEPTH) / 2, 0},
};

constant constexpr float3 colorFog = float3(0x07, 0x00, 0x16) / 0xFF;
constant constexpr float3 colorLight = float3(0xB7, 0x46, 0x40) / 0xFF;
constant constexpr float3 colorShadow = float3(0x01, 0x5A, 0x00) / 0xFF;

float3 sRGBToLinear(float3 srgb) {
    return select(srgb / 12.92f,
                  pow((srgb + 0.055f) / 1.055f, 2.4f),
                  srgb > 0.04045f);
}

float4 blendNormal(float4 src, float4 dst) {
    return float4(dst.rgb * (1 - src.a) + src.rgb * src.a, src.a + dst.a * (1 - src.a));
}

float4 blendAdd(float4 src, float4 dst) {
    return float4(dst.rgb + src.rgb * src.a, src.a + dst.a * (1 - src.a));
}

float4 blendMultiply(float4 src, float4 dst) {
    return float4(dst.rgb * (1 - (1 - src.rgb) * src.a), dst.a);
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

/**
 * Returns angle in camera space of the ray cast from camera to pixel of projection plane.
 */
float2 getRayAngle(uint2 pixel) {
    return {
        (pixel.x + 0.5f - CANVAS_WIDTH / 2.0f) * pixelAngle,
        (horizonY - pixel.y - 0.5f) * pixelAngle};
}

/**
 * Returns point in camera space where the ray cast from camera at given angle hits horizontal surface.
 */
float4 castRayToSurface(float2 rayAngle, float cameraZ, float surfaceZ) {
    if (fabs(rayAngle.y) < eps) return pointAtInf;
    if (rayAngle.y > 0 && cameraZ > surfaceZ) return pointAtInf;
    if (rayAngle.y < 0 && cameraZ < surfaceZ) return pointAtInf;
    float z = surfaceZ - cameraZ;
    float x = z / tan(rayAngle.y);
    float y = x * tan(rayAngle.x);
    return {x, y, z, 1};
}

/**
 * Draws horizontal surface color into the output texture pixel.
 */
void drawSurface(float4 raySurfaceHit,  // coordinate of ray and surface intersection in camera space
                 float4 cameraPosition, // camera position in world space
                 float cameraAngle,     // camera horizontal angle in world space
                 texture2d<float, access::sample> surfaceTexture,
                 texture2d<float, access::write> outputTexture,
                 uint2 pixel) {
    if (raySurfaceHit.x >= CAMERA_FAR_CLIP) {
        outputTexture.write(float4(sRGBToLinear(colorFog), 1), pixel);
        return;
    }
    float4 raySurfaceHitWorld = makeTranslationMatrix(cameraPosition) * makeRotationZMatrix(cameraAngle) * raySurfaceHit;
    constexpr sampler textureSampler(coord::normalized, address::repeat, filter::nearest);
    float2 readCoord = raySurfaceHitWorld.xy / float(MAP_TILE_SIZE);
    float4 outColor = surfaceTexture.sample(textureSampler, readCoord);
    float lightFalloff = 1.0f - min(1.0f, raySurfaceHit.x / PLAYER_LIGHT_RADIUS);
    float4 lightColor = float4(sRGBToLinear(colorLight), lightFalloff * PLAYER_LIGHT_INTENSITY);
    outColor = blendAdd(lightColor, outColor);
    float4 shadowColor = float4(sRGBToLinear(colorShadow), raySurfaceHit.x / CAMERA_FAR_CLIP);
    outColor = blendMultiply(shadowColor, outColor);
    float4 fogColor = float4(sRGBToLinear(colorFog), raySurfaceHit.x / CAMERA_FAR_CLIP);
    outColor = blendNormal(fogColor, outColor);
    outputTexture.write(outColor, pixel);
}

void drawPixel(uint2 pixel,
               Ray ray,
               texture2d<float, access::sample> tileTexture,
               texture2d<float, access::write> outputTexture) {
    constexpr sampler textureSampler(coord::normalized, address::repeat, filter::nearest);
    float2 readCoord = ray.tile.offset;
    float4 outColor = tileTexture.sample(textureSampler, readCoord);
    outputTexture.write(outColor, pixel);
}

/**
 * Returns a traversal state for ray cast.
 */
RayState makeRayState(constant Camera& camera, float2 rayAngle) {
    float cosH = cos(camera.angle + rayAngle.x);
    float sinH = sin(camera.angle + rayAngle.x);
    float cosV = cos(rayAngle.y);
    float sinV = sin(rayAngle.y);

    RayState state = {
        .normalH = {cosH, sinH},
        .normalV = {cosV, sinV},
        .rayX = {
            .position = camera.position,
            .next = pointAtInf,
            .step = {0, 0, 0, 0},
            .tile = tileMiss,
            .miss = true,
        },
        .rayY = {
            .position = camera.position,
            .next = pointAtInf,
            .step = {0, 0, 0, 0},
            .tile = tileMiss,
            .miss = true,
        },
        .rayZ = {
            .position = camera.position,
            .next = pointAtInf,
            .step = {0, 0, 0, 0},
            .tile = tileMiss,
            .miss = true,
        }
    };

    if (fabs(cosH) > eps && fabs(cosV) > eps) {
        state.rayX.next.x = select(floor(camera.position.x / MAP_TILE_SIZE), ceil(camera.position.x / MAP_TILE_SIZE), cosH > 0) * MAP_TILE_SIZE;
        float nextDXY = fabs((state.rayX.next.x - camera.position.x) / cosH);
        float nextDXYZ = fabs(nextDXY / cosV);
        state.rayX.next.y = camera.position.y + nextDXY * sinH;
        state.rayX.next.z = camera.position.z + nextDXYZ * sinV;
        float stepDXY = fabs(MAP_TILE_SIZE / cosH);
        float stepDXYZ = fabs(stepDXY / cosV);
        state.rayX.step.x = MAP_TILE_SIZE * sign(cosH);
        state.rayX.step.y = stepDXY * sinH;
        state.rayX.step.z = stepDXYZ * sinV;
        state.rayX.miss = false;
    }

    if (fabs(sinH) > eps && fabs(cosV) > eps) {
        state.rayY.next.y = select(floor(camera.position.y / MAP_TILE_SIZE), ceil(camera.position.y / MAP_TILE_SIZE), sinH > 0) * MAP_TILE_SIZE;
        float nextDXY = fabs((state.rayY.next.y - camera.position.y) / sinH);
        float nextDXYZ = fabs(nextDXY / cosV);
        state.rayY.next.x = camera.position.x + nextDXY * cosH;
        state.rayY.next.z = camera.position.z + nextDXYZ * sinV;
        float stepDXY = fabs(MAP_TILE_SIZE / sinH);
        float stepDXYZ = fabs(stepDXY / cosV);
        state.rayY.step.x = stepDXY * cosH;
        state.rayY.step.y = MAP_TILE_SIZE * sign(sinH);
        state.rayY.step.z = stepDXYZ * sinV;
        state.rayY.miss = false;
    }

    if (fabs(sinV) > eps) {
        state.rayZ.next.z = select(floor(camera.position.z / MAP_TILE_SIZE), ceil(camera.position.z / MAP_TILE_SIZE), sinV > 0) * MAP_TILE_SIZE;
        float nextDXYZ = fabs((state.rayZ.next.z - camera.position.z) / sinV);
        float nextDXY = fabs(nextDXYZ * cosV);
        state.rayZ.next.x = camera.position.x + nextDXY * cosH;
        state.rayZ.next.y = camera.position.y + nextDXY * sinH;
        float stepDXYZ = fabs(MAP_TILE_SIZE / sinV);
        float stepDXY = fabs(stepDXYZ * cosV);
        state.rayZ.step.x = stepDXY * cosH;
        state.rayZ.step.y = stepDXY * sinH;
        state.rayZ.step.z = MAP_TILE_SIZE * sign(sinV);
        state.rayZ.miss = false;
    }

    return state;
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

bool isDoor(Tile tile) {
    switch (tile) {
        case TileDoorH:
        case TileDoorV:
            return true;
        default:
            return false;
    }
}

bool isWall(Tile tile) {
    switch (tile) {
        case TileWall:
        case TileWallFortified:
        case TileWallIndestructible:
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
bool findClosestIntersection(float2 ray[2], constant float2 polygon[N], thread Intersection& intersection) {
    float minLength = inf;
    float2 point;
    float offset;
    for (int i = 0; i < N; ++i) {
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

float doorSegmentOffset(Tile door, int segmentIndex, float segmentOffset) {
    if (door == TileDoorH) {
        return select(0.0f, segmentOffset, segmentIndex == 1 || segmentIndex == 3);
    } else {
        return select(0.0f, segmentOffset, segmentIndex == 0 || segmentIndex == 2);
    }
}

bool castRay(constant Camera& camera, constant Map& map, thread RayState& state, thread Ray& ray) {
    if (state.rayX.miss && state.rayY.miss && state.rayZ.miss) return false;

    const float2 normalH = state.normalH;
    const float2 normalV = state.normalV;

    // Scan intersections with vertical grid lines.
    if (!state.rayX.miss) {
        thread RayComponent& rayX = state.rayX;
        rayX.position = rayX.next;
        rayX.next = pointAtInf;
        rayX.tile = tileMiss;
        rayX.miss = true;
        for (; inMapBounds(rayX.position); rayX.position += rayX.step) {
            int row = floor(rayX.position.y / MAP_TILE_SIZE);
            int col = floor(rayX.position.x / MAP_TILE_SIZE) - float(normalH.x < 0);
            if (!inMapBounds(row, col)) break;
            int tileIndex = row * MAP_WIDTH + col;
            Tile tile = map.tiles[tileIndex];
            if (isWall(tile)) {
                rayX.next = rayX.position + rayX.step;
                rayX.tile = {
                    .index = tileIndex,
                    .side = normalH.x < 0 ? TileSideRight : TileSideLeft,
                    .offset = {
                        invertIf((rayX.position.y - row * MAP_TILE_SIZE) / MAP_TILE_SIZE, normalH.x < 0),
                        1 - rayX.position.z / MAP_TILE_SIZE
                    },
                    .slope = atan(fabs(normalH.y / normalH.x)) * 2 / pi,
                };
                rayX.miss = false;
                break;
            } else if (isDoor(tile)) {
                float2 tilePosition = makeTilePosition(col, row);
                float2 raySegment[2] = {
                    rayX.position.xy - tilePosition,
                    rayX.position.xy - tilePosition + rayX.step.xy
                };
                Intersection intersection;
                if (findClosestIntersection<4>(raySegment, tile == TileDoorH ? doorH : doorV, intersection)) {
                    rayX.next = rayX.position + rayX.step;
                    rayX.position += getDelta(intersection.point - raySegment[0], normalV);
                    rayX.tile = {
                        .index = tileIndex,
                        .side = normalH.x < 0 ? TileSideRight : TileSideLeft,
                        .offset = {
                            doorSegmentOffset(tile, intersection.segmentIndex, intersection.segmentOffset),
                            1 - rayX.position.z / MAP_TILE_SIZE
                        },
                        .slope = atan(fabs(tile == TileDoorH ? normalH.x / normalH.y : normalH.y / normalH.x)) * 2 / pi,
                    };
                    rayX.miss = false;
                    break;
                }
            }
        }
    }

    // Scan intersections with horizontal grid lines.
    if (!state.rayY.miss) {
        thread RayComponent& rayY = state.rayY;
        rayY.position = rayY.next;
        rayY.next = pointAtInf;
        rayY.tile = tileMiss;
        rayY.miss = true;
        for (; inMapBounds(rayY.position); rayY.position += rayY.step) {
            int row = floor(rayY.position.y / MAP_TILE_SIZE) - float(normalH.y < 0);
            int col = floor(rayY.position.x / MAP_TILE_SIZE);
            if (!inMapBounds(row, col)) break;
            int tileIndex = row * MAP_WIDTH + col;
            Tile tile = map.tiles[tileIndex];
            if (isWall(tile)) {
                rayY.next = rayY.position + rayY.step;
                rayY.tile = {
                    .index = tileIndex,
                    .side = normalH.y < 0 ? TileSideBottom : TileSideTop,
                    .offset = {
                        invertIf((rayY.position.x - col * MAP_TILE_SIZE) / MAP_TILE_SIZE, normalH.y > 0),
                        1 - rayY.position.z / MAP_TILE_SIZE
                    },
                    .slope = atan(fabs(normalH.x / normalH.y)) * 2 / pi,
                };
                rayY.miss = false;
                break;
            } else if (isDoor(tile)) {
                float2 tilePosition = makeTilePosition(col, row);
                float2 raySegment[2] = {
                    rayY.position.xy - tilePosition,
                    rayY.position.xy - tilePosition + rayY.step.xy
                };
                Intersection intersection;
                if (findClosestIntersection<4>(raySegment, tile == TileDoorH ? doorH : doorV, intersection)) {
                    rayY.next = rayY.position + rayY.step;
                    rayY.position += getDelta(intersection.point - raySegment[0], normalV);
                    rayY.tile = {
                        .index = tileIndex,
                        .side = normalH.y < 0 ? TileSideBottom : TileSideTop,
                        .offset = {
                            doorSegmentOffset(tile, intersection.segmentIndex, intersection.segmentOffset),
                            1 - rayY.position.z / MAP_TILE_SIZE
                        },
                        .slope = atan(fabs(tile == TileDoorH ? normalH.x / normalH.y : normalH.y / normalH.x)) * 2 / pi,
                    };
                    rayY.miss = false;
                    break;
                }
            }
        }
    }

    // Calculate intersection with floor/ceiling.
    if (!state.rayZ.miss) {
        thread RayComponent& rayZ = state.rayZ;
        rayZ.position = rayZ.next;
        rayZ.next = pointAtInf;
        rayZ.tile = tileMiss;
        rayZ.miss = true;
        if (inMapBounds(rayZ.position)) {
            int row = floor(rayZ.position.y / MAP_TILE_SIZE);
            int col = floor(rayZ.position.x / MAP_TILE_SIZE);
            if (inMapBounds(row, col)) {
                int tileIndex = row * MAP_WIDTH + col;
                float2 tilePosition = makeTilePosition(col, row);
                rayZ.next = rayZ.position + rayZ.step;
                rayZ.tile = {
                    .index = tileIndex,
                    .side = normalV.y < 0 ? TileSideFloor : TileSideCeiling,
                    .offset = (rayZ.position.xy - tilePosition) / MAP_TILE_SIZE,
                    .slope = atan(fabs(normalV.x / normalV.y)) * 2 / pi,
                };
                rayZ.miss = false;
            }
        }
    }

    if (state.rayX.miss && state.rayY.miss && state.rayZ.miss) return false;

    // Choose the shortest ray component
    float lengthX = state.rayX.miss ? inf : length(state.rayX.position - camera.position);
    float lengthY = state.rayY.miss ? inf : length(state.rayY.position - camera.position);
    float lengthZ = state.rayZ.miss ? inf : length(state.rayZ.position - camera.position);
    if (lengthX < lengthY && lengthX < lengthZ) {
        ray.position = state.rayX.position;
        ray.length = lengthX;
        ray.tile = state.rayX.tile;
        return true;
    }
    if (lengthY < lengthZ) {
        ray.position = state.rayY.position;
        ray.length = lengthY;
        ray.tile = state.rayY.tile;
        return true;
    }
    ray.position = state.rayZ.position;
    ray.length = lengthZ;
    ray.tile = state.rayZ.tile;
    return true;
}

kernel void castRays(texture2d<float, access::write> outputTexture [[texture(0)]],
                     array<texture2d<float, access::sample>, TEXTURE_HEAP_SIZE> textures [[texture(1)]],
                     constant Camera& camera [[buffer(0)]],
                     constant Map& map [[buffer(1)]],
                     uint2 gid [[thread_position_in_grid]]) {
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    float2 rayAngle = getRayAngle(gid);
    RayState state = makeRayState(camera, rayAngle);
    Ray ray;
    if (castRay(camera, map, state, ray)) {
        texture2d<float, access::sample> tileTexture;
        switch (ray.tile.side) {
            case TileSideLeft:
            case TileSideRight:
            case TileSideTop:
            case TileSideBottom:
                switch (map.tiles[ray.tile.index]) {
                    case TileDoorH:
                    case TileDoorV:
                        tileTexture = textures[TextureIndexDoor];
                        break;
                    case TileFloor:
                        tileTexture = textures[TextureIndexFloor];
                        break;
                    case TileWall:
                        tileTexture = textures[TextureIndexWall];
                        break;
                    case TileWallFortified:
                        tileTexture = textures[TextureIndexWallFortified];
                        break;
                    case TileWallIndestructible:
                        tileTexture = textures[TextureIndexWallIndestructible];
                        break;
                }
                break;
            case TileSideCeiling:
                tileTexture = textures[TextureIndexCeiling];
                break;
            case TileSideFloor:
                tileTexture = textures[TextureIndexFloor];
                break;
        }
        drawPixel(gid, ray, tileTexture, outputTexture);
    }
}
