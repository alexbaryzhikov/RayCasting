#ifndef Map_hpp
#define Map_hpp

#include <array>

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

}

namespace RC::Map {

extern std::array<Tile, MAP_WIDTH * MAP_HEIGHT> tiles;
extern float width;
extern float height;
extern float zoomFactor;

bool isVisible();

bool isFullScreen();

bool isFollowing();

void initialize();

void draw();

void update();

} // namespace RC::Map

#endif /* Map_hpp */
