#ifndef Viewport_hpp
#define Viewport_hpp

#include <simd/simd.h>

namespace RC::Viewport {

extern simd::float2 rayR;
extern simd::float2 rayG;
extern simd::float2 rayB;

void initialize();

void draw();

void update();

}

#endif /* Viewport_hpp */
