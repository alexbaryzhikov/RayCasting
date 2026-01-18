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

typedef struct Camera {
    simd_float4 position;
    float angle;
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
