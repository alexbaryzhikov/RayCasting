#ifndef Player_hpp
#define Player_hpp

#include "Geometry.hpp"

namespace RC::Player {

extern simd::float3 position;
extern float angle;

void draw();

void update();

} // namespace RC::Player

#endif /* Player_hpp */
