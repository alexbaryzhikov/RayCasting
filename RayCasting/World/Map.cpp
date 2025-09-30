//
//  Map.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 20.09.25.
//

#include <print>

#include "Map.hpp"

namespace RC::Map {

std::vector<std::vector<Tile>> tiles;

void load(const void* bytes, size_t size) {
    std::println("Loading map: {} size: {}", bytes, size);
}

} // namespace RC::Map
