#include "GPUShaderTypes.h"
#include <metal_stdlib>

using namespace metal;

constant constexpr float inf = 1e10;
constant constexpr float4 pointAtInf = {inf, inf, inf, 1};

constant constexpr float pixelAngle = float(CAMERA_FOV) / float(CANVAS_WIDTH); // angle between rays cast from camera to adjacent pixels of projection plane
constant constexpr float horizonY = float(CANVAS_HEIGHT) / 2;                  // horizon coordinate in screen space

constant constexpr float mapWidth = MAP_WIDTH * MAP_TILE_SIZE;
constant constexpr float mapHeight = MAP_HEIGHT * MAP_TILE_SIZE;

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
    return float4(dst.rgb + src.rgb * src.a, 1);
}

float4 blendMultiply(float4 src, float4 dst) {
    return float4(dst.rgb * (1 - (1 - src.rgb) * src.a), 1);
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
 * Returns angle in camera space of the ray cast from camera to pixel of projection plane.
 */
float2 getRayAngle(uint2 pixel) {
    return {
        (pixel.x + 0.5f - CANVAS_WIDTH / 2.0f) * pixelAngle,
        (horizonY - pixel.y - 0.5f) * pixelAngle
    };
}

/**
 * Returns point in camera space where the ray cast from camera at given angle hits horizontal surface.
 */
float4 getSurfaceHit(float2 rayAngle, float cameraZ, float surfaceZ) {
    if (rayAngle.y >= 0 && cameraZ >= surfaceZ) return pointAtInf;
    if (rayAngle.y <= 0 && cameraZ <= surfaceZ) return pointAtInf;
    float z = (surfaceZ - cameraZ);
    float x = z / tan(rayAngle.y);
    float y = x * tan(rayAngle.x);
    return {x, y, z, 1};
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
    bool isCeiling = rayAngle.y > 0;
    float4 hitCamera = getSurfaceHit(rayAngle, camera.placement.z, isCeiling ? MAP_TILE_SIZE : 0);
    if (hitCamera.x >= CAMERA_FAR_CLIP) {
        outputTexture.write(float4(sRGBToLinear(colorFog), 1), gid);
        return;
    }
    float4 cameraPosition = float4(camera.placement.xyz, 1);
    float cameraAngleZ = camera.placement.w;
    float4x4 cameraToWorld = makeTranslationMatrix(cameraPosition) * makeRotationZMatrix(cameraAngleZ);
    float4 hitWorld = cameraToWorld * hitCamera;
    texture2d<float, access::sample> texture = isCeiling ? textures[TextureIndexCeiling] : textures[TextureIndexFloor];
    constexpr sampler textureSampler(coord::normalized, address::repeat, filter::nearest);
    float2 readCoord = hitWorld.xy / float(MAP_TILE_SIZE);
    float4 outColor = texture.sample(textureSampler, readCoord);
    float lightFalloff = 1.0f - min(1.0f, hitCamera.x / PLAYER_LIGHT_RADIUS);
    float4 lightColor = float4(sRGBToLinear(colorLight), lightFalloff * PLAYER_LIGHT_INTENSITY);
    outColor = blendAdd(lightColor, outColor);
    float4 shadowColor = float4(sRGBToLinear(colorShadow), hitCamera.x / CAMERA_FAR_CLIP);
    outColor = blendMultiply(shadowColor, outColor);
    float4 fogColor = float4(sRGBToLinear(colorFog), hitCamera.x / CAMERA_FAR_CLIP);
    outColor = blendNormal(fogColor, outColor);
    outputTexture.write(outColor, gid);
}
