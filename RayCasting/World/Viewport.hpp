#ifndef Viewport_hpp
#define Viewport_hpp

#include <simd/simd.h>

namespace RC::Viewport {

extern simd::float2 rayL;
extern simd::float2 rayC;
extern simd::float2 rayR;

void draw();

void update();

}

#endif /* Viewport_hpp */
