#ifndef Player_hpp
#define Player_hpp

#include <simd/simd.h>

namespace RC::Player {

extern simd::float3 position;
extern simd::float3 velocity;
extern float angle;

void update();

} // namespace RC::Player

#endif /* Player_hpp */
