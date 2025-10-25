#ifndef Map_hpp
#define Map_hpp

#include <vector>

namespace RC::Map {

enum class Tile {
    floor,
    wall,
};

extern std::vector<Tile> tiles;
extern size_t width;
extern size_t height;
extern float zoomFactor;
extern bool isVisible;

bool isFullScreen();

void load(const void* bytes, size_t size);

void draw();

void update();

} // namespace RC::Map

#endif /* Map_hpp */
