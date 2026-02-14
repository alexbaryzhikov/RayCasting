#ifndef Map_hpp
#define Map_hpp

#include <array>
#include <chrono>
#include <map>
#include <simd/simd.h>

#include "Config.h"

namespace RC {

enum class TileType {
    doorH,
    doorV,
    empty,
    wall,
    wallFortified,
    wallIndestructible,
};

bool isDoor(TileType type);

bool isWall(TileType type);

enum class DoorState {
    idle,
    opening,
    closing,
};

struct Door {
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    simd::float2 position;
    float progress; // from 0 (fully open) to 1 (fully closed)
    DoorState state;
    TimePoint closingTime;

    bool isPassable() { return progress < 1 - CAMERA_HEIGHT / MAP_TILE_SIZE; }
};

Door makeDoorAt(int col, int row);

} // namespace RC

namespace RC::Map {

extern std::array<TileType, MAP_WIDTH * MAP_HEIGHT> tiles;
extern std::map<int, Door> doors;
extern const float width;
extern const float height;
extern float zoomFactor;

bool isVisible();

bool isFullScreen();

bool isFollowing();

void draw();

void update();

} // namespace RC::Map

#endif /* Map_hpp */
