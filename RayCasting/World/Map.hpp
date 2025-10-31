#ifndef Map_hpp
#define Map_hpp

#include <vector>

namespace RC::Map {

enum class Tile {
    floor,
    wall,
};

extern std::vector<Tile> tiles;
extern size_t tilesWidth;
extern size_t tilesHeight;
extern float width;
extern float height;
extern float zoomFactor;
extern bool isVisible;

bool isFullScreen();

void initialize();

void draw();

void update();

} // namespace RC::Map

#endif /* Map_hpp */
