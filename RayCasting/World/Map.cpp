#include "Map.hpp"

namespace RC::Map {

std::vector<std::vector<Tile>> tiles;

void load(const void* bytes, size_t size) {
    std::printf("Loading map: %p size: %lu\n", bytes, size);
}

} // namespace RC::Map
