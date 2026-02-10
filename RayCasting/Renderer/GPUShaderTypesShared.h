#ifndef GPUShaderTypesShared_h
#define GPUShaderTypesShared_h

#include "../Config.h"
#include <simd/simd.h>

#define TEXTURE_HEAP_SIZE 8

typedef enum {
    TextureIndexCeiling,
    TextureIndexFloor,
    TextureIndexDoor,
    TextureIndexWall,
    TextureIndexWallFortified,
    TextureIndexWallIndestructible,
} TextureIndex;

typedef struct {
    simd_float4 position;
    float angle;
} Camera;

typedef enum {
    TileTypeDoorH,
    TileTypeDoorV,
    TileTypeEmpty,
    TileTypeWall,
    TileTypeWallFortified,
    TileTypeWallIndestructible,
} TileType;

typedef struct {
    float progress;
} Door;

typedef struct {
    TileType tiles[MAP_WIDTH * MAP_HEIGHT];
    Door doors[MAP_WIDTH * MAP_HEIGHT];
} Map;

#endif /* GPUShaderTypesShared_h */
