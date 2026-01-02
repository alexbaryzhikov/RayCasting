#include "../Config.h"
#include "GPUShaderTypes.h"
#include <metal_stdlib>

using namespace metal;

constant constexpr float inf = 1e10;
constant constexpr float4 pointAtInf = {inf, inf, inf, 1};

constant constexpr float pixelAngle = float(CAMERA_FOV) / float(CANVAS_WIDTH); // angle between two adjacent pixels of projection plane
constant constexpr float horizonY = float(CANVAS_HEIGHT) / 2; // horizon coordinate in screen space

constant constexpr float4 colorFog = float4(0x07, 0x00, 0x16, 0xFF) / 0xFF;
constant constexpr float4 colorLight = float4(0xB7, 0x46, 0x40, 0xFF) / 0xFF;
constant constexpr float4 colorShadow = float4(0x01, 0x5A, 0x00, 0xFF) / 0xFF;

/**
 * Convert sRGB color to Linear space.
 */
inline float3 sRGBToLinear(float3 sRGBColor) {
    float3 cutoff = float3(0.04045);
    float3 higher = pow((sRGBColor + 0.055) / 1.055, 2.4);
    float3 lower  = sRGBColor / 12.92;
    return select(lower, higher, sRGBColor > cutoff);
}

inline float4 sRGBToLinear(float4 sRGBColor) {
    return float4(sRGBToLinear(sRGBColor.rgb), sRGBColor.a);
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
        { c, s, 0, 0},
        {-s, c, 0, 0},
        { 0, 0, 1, 0},
        { 0, 0, 0, 1},
    };
}

/**
 * Returns matrix that transforms point in camera space to point in world space.
 */
float4x4 makeCameraToWorldTransform(float4 cameraPosition, float cameraAngleZ) {
    return makeTranslationMatrix(cameraPosition) * makeRotationZMatrix(cameraAngleZ);
}

/**
 * Returns angle in camera space of the ray cast from camera to pixel (x, y) of projection plane.
 */
float2 getRayAngle(uint2 pixel) {
    return {
        (pixel.x + 0.5f - CANVAS_WIDTH / 2.0f) * pixelAngle,
        (horizonY - pixel.y - 0.5f) * pixelAngle
    };
}

/**
 * Returns point in camera space where the ray cast from camera at given angle hits the ceiling.
 */
float4 getCeilingHit(float2 rayAngle, float cameraHeight, float ceilingHeight) {
    if (rayAngle.y <= 0 || cameraHeight >= ceilingHeight) return pointAtInf;
    float z = (ceilingHeight - cameraHeight);
    float x = z / tan(rayAngle.y);
    float y = x * tan(rayAngle.x);
    return {x, y, z, 1.0f};
}

/**
 * Returns point in camera space where the ray cast from camera at given angle hits the floor.
 */
float4 getFloorHit(float2 rayAngle, float cameraHeight, float floorHeight) {
    if (rayAngle.y >= 0 || cameraHeight <= floorHeight) return pointAtInf;
    float z = (floorHeight - cameraHeight);
    float x = z / tan(rayAngle.y);
    float y = x * tan(rayAngle.x);
    return {x, y, z, 1.0f};
}

kernel void castRays(texture2d<float, access::write> outputTexture [[texture(0)]],
                     array<texture2d<float, access::sample>, TEXTURE_HEAP_SIZE> textures [[texture(1)]],
                     constant Camera& camera [[buffer(0)]],
                     uint2 gid [[thread_position_in_grid]]) {
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    float2 rayAngle = getRayAngle(gid);
    bool isCeiling = rayAngle.y > 0;
    float4 hitCamera;
    if (isCeiling) {
        hitCamera = getCeilingHit(rayAngle, camera.placement.z, MAP_TILE_SIZE);
    } else {
        hitCamera = getFloorHit(rayAngle, camera.placement.z, 0);
    }
    if (hitCamera.x >= CAMERA_FAR_CLIP) {
        outputTexture.write(sRGBToLinear(colorFog), gid);
        return;
    }
    float4 cameraPosition = float4(camera.placement.xyz, 1);
    float cameraAngle = camera.placement.w;
    float4x4 transform = makeCameraToWorldTransform(cameraPosition, cameraAngle);
    float4 hitWorld = transform * hitCamera;
    texture2d<float, access::sample> texture = isCeiling ? textures[TextureCeiling] : textures[TextureFloor];
    constexpr sampler textureSampler(coord::normalized, address::repeat, filter::nearest);
    float2 readCoord = hitWorld.xy / float(MAP_TILE_SIZE);
    float4 color = texture.sample(textureSampler, readCoord);
    outputTexture.write(color, gid);
}
