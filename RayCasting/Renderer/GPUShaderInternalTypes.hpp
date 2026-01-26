#ifndef GPUShaderInternalTypes_hpp
#define GPUShaderInternalTypes_hpp

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

struct TileHit {
    int index;     // -1 for miss
    TileSide side; // side of the cube tile hit by the ray
    float2 offset; // 0 to 1 (left to right for x, top to bottom for y)
    float slope;   // 0 to 1 (slope of the tile face relative to the ray)
};

struct Ray {
    float4 position;
    float length;
    TileHit tile;

    bool isMiss() const { return tile.index == -1; }
};

struct RayComponent {
    float4 position; // ray position in world space
    float4 next;     // next grid-aligned position
    float4 step;     // offset between adjacent grid-aligned positions
    TileHit tile;    // tile hit data
    bool miss;       // true if ray component leaves map bounds
};

struct RayState {
    float2 normalH;    // ray horizontal direction in world space
    float2 normalV;    // ray vertical direction in world space
    RayComponent rayX; // tracks intersecitons with YZ walls
    RayComponent rayY; // tracks intersections with XZ walls
    RayComponent rayZ; // tracks intersections with XY surfaces
};

#endif /* GPUShaderInternalTypes_hpp */
