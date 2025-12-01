#ifndef Map_hpp
#define Map_hpp

#include <array>
#include <map>

#include "Config.h"

namespace RC {

enum class Tile {
    doorH,
    doorV,
    floor,
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
    DoorState state = DoorState::idle;
    float progress = 1; // from 0 (fully open) to 1 (fully closed)

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
