//
//  Player.hpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 21.09.25.
//

#ifndef Player_hpp
#define Player_hpp

#include "Geometry.hpp"

namespace RC::Player {

extern Point position;
extern float angle;

void draw();

void update();

} // namespace RC::Player

#endif /* Player_hpp */
