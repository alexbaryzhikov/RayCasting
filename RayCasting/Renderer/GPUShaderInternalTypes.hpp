#ifndef GPUShaderInternalTypes_hpp
#define GPUShaderInternalTypes_hpp

struct Intersection {
    float2 point;
    int segmentIndex;
    float segmentOffset;
};

enum class TileSide {
    left,
    right,
    top,
    bottom,
};

struct TileHit {
    int index;     // -1 for miss
    float2 offset; // 0 to 1
    float angle;   // 0 to 1 (0 to 90 degrees)
    TileSide side;
};

struct Ray {
    float2 position;
    float length;
    TileHit tile;

    bool isMiss() const { return tile.index == -1; }
};

struct RayComponent {
    float2 position; // ray position in world space
    float2 step;     // offset between adjacent grid-aligned positions
    float2 next;     // next grid-aligned position
    TileHit tile;    // tile hit data
    bool advance;    // true if ray can advance further
};

struct RayState {
    float2 normal;     // ray direction in map space
    RayComponent rayH; // horizontal component
    RayComponent rayV; // vertical component
};

#endif /* GPUShaderInternalTypes_hpp */
