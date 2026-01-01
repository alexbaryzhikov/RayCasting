#ifndef GPUShaderTypes_h
#define GPUShaderTypes_h

#define TEXTURE_HEAP_SIZE 8

enum TextureIndex {
    TextureCeiling = 0,
    TextureFloor = 1,
    TextureDoor = 2,
    TextureWall = 3,
    TextureWallFortified = 4,
    TextureWallIndestructible = 5,
};

#ifdef __METAL_VERSION_
#define FLOAT4 float4
#else
#include <simd/simd.h>
#define FLOAT4 simd_float4
#endif

typedef struct Camera {
    FLOAT4 placement; // xyz + angle
} Camera;

#endif /* GPUShaderTypes_h */
