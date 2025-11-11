#ifndef Map_hpp
#define Map_hpp

#include <vector>

namespace RC {

enum class Tile {
    floor,
    wall,
    wallFortified,
    wallIndestructible,
};

}

namespace RC::Map {

extern std::vector<Tile> tiles;
extern size_t tilesWidth;
extern size_t tilesHeight;
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
