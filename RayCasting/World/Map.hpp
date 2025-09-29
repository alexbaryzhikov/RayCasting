//
//  Map.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 20.09.25.
//

#ifndef Map_hpp
#define Map_hpp

#include <vector>

namespace RC {

struct MapTile {
    bool isWall;
};

struct Map {
    std::vector<std::vector<MapTile>> tiles;
};

void loadMap(const void* bytes, size_t size);

} // namespace RC

#endif /* Map_hpp */
