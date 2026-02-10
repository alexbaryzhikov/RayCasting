#ifndef Viewport_hpp
#define Viewport_hpp

#include <simd/simd.h>

namespace RC {

struct Ray {
    simd::float2 position;
    float length;
    int tileIndex;

    bool isMiss() const { return tileIndex == -1; }
};

} // namespace RC

namespace RC::Viewport {

extern float cameraPositionZ;

void update();

Ray castRay(float playerSpaceAngle);

} // namespace RC::Viewport

#endif /* Viewport_hpp */
