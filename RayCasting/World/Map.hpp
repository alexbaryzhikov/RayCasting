#ifndef Map_hpp
#define Map_hpp

#include <vector>

namespace RC::Map {

enum class Tile {
    FLOOR,
    WALL,
};

extern std::vector<std::vector<Tile>> tiles;

size_t width();

size_t height();

void load(const void* bytes, size_t size);

void draw();

void update();

} // namespace RC::Map

#endif /* Map_hpp */
