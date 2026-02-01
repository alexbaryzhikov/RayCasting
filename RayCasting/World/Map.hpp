#ifndef Map_hpp
#define Map_hpp

#include <array>
#include <chrono>
#include <map>
#include <simd/simd.h>

#include "Config.h"

namespace RC {

enum class Tile {
    doorH,
    doorV,
    empty,
    wall,
    wallFortified,
    wallIndestructible,
};

bool isDoor(Tile tile);

bool isWall(Tile tile);

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

    static Door makeDoorAt(int col, int row) {
        return {
            .position = (simd::float2{float(col), float(row)} + 0.5) * MAP_TILE_SIZE,
            .progress = 1,
            .state = DoorState::idle,
            .closingTime = TimePoint(std::chrono::seconds(0)),
        };
    }

    bool isPassable() { return progress < 1 - CAMERA_HEIGHT / MAP_TILE_SIZE; }
};

} // namespace RC

namespace RC::Map {

extern std::array<Tile, MAP_WIDTH * MAP_HEIGHT> tiles;
extern std::map<int, Door> doors;
extern const float width;
extern const float height;
extern float zoomFactor;

bool isVisible();

bool isFullScreen();

bool isFollowing();

void initialize();

void draw();

void update();

} // namespace RC::Map

#endif /* Map_hpp */
