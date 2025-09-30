//
//  Map.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 20.09.25.
//

#ifndef Map_hpp
#define Map_hpp

#include <vector>

namespace RC::Map {

enum class Tile {
    FLOOR,
    WALL,
};

extern std::vector<std::vector<Tile>> tiles;

void load(const void* bytes, size_t size);

} // namespace RC::Map

#endif /* Map_hpp */
