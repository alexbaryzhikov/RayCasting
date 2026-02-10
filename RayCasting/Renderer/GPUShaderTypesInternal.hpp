#ifndef GPUShaderTypesInternal_hpp
#define GPUShaderTypesInternal_hpp

#include "GPUShaderTypesShared.h"

struct Intersection {
    float2 point;
    int segmentIndex;
    float segmentOffset;
};

enum TileSide {
    TileSideLeft,
    TileSideRight,
    TileSideTop,
    TileSideBottom,
    TileSideCeiling,
    TileSideFloor,
};

struct Tile {
    TileType type;
    TileSide side;
    float2 offset; // 0 to 1 (left to right for x, top to bottom for y)
    float slope;   // 0 to 1 (slope of the tile face relative to the ray)
};

struct Ray {
    float4 position;
    float length;
    Tile tile;
    bool miss;
};

struct RayComponent {
    float4 position; // ray position in world space
    float4 step;     // offset between adjacent grid-aligned positions
};

struct RayState {
    float2 normalH;    // ray horizontal direction in world space
    float2 normalV;    // ray vertical direction in world space
    RayComponent rayX; // tracks intersecitons with YZ walls
    RayComponent rayY; // tracks intersections with XZ walls
    RayComponent rayZ; // tracks intersections with XY surfaces
};

#endif /* GPUShaderTypesInternal_hpp */
