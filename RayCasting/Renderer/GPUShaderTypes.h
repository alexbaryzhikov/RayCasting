#ifndef GPUShaderTypes_h
#define GPUShaderTypes_h

#include "../Config.h"

#define TEXTURE_HEAP_SIZE 8

enum TextureIndex {
    TextureIndexCeiling,
    TextureIndexFloor,
    TextureIndexDoor,
    TextureIndexWall,
    TextureIndexWallFortified,
    TextureIndexWallIndestructible,
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

typedef enum Tile {
    TileDoorH,
    TileDoorV,
    TileFloor,
    TileWall,
    TileWallFortified,
    TileWallIndestructible,
} Tile;

typedef struct Map {
    Tile tiles[MAP_WIDTH * MAP_HEIGHT];
} Map;

#endif /* GPUShaderTypes_h */
