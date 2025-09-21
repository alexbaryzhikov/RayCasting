//
//  Player.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 21.09.25.
//

#ifndef Player_hpp
#define Player_hpp

#include "Geometry.hpp"

namespace RC {

struct Player {
    Point position;
    float angle = 0.f;
};

} // namespace RC

#endif /* Player_hpp */
