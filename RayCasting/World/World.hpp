//
//  World.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 20.09.25.
//

#ifndef World_hpp
#define World_hpp

#include "Map.hpp"
#include "Player.hpp"

namespace RC {

struct World {
    Map map;
    Player player;

    static World& get();

    void loadMap(const void* data, size_t size);

    void start();

    void draw();
};

} // namespace RC

#endif /* World_hpp */
