#ifndef Map_hpp
#define Map_hpp

#include <vector>

namespace RC::Map {

enum class Tile {
    floor,
    wall,
};

extern std::vector<std::vector<Tile>> tiles;
extern bool isVisible;

size_t width();

size_t height();

bool isFullFrame();

void load(const void* bytes, size_t size);

void draw();

void update();

} // namespace RC::Map

#endif /* Map_hpp */
