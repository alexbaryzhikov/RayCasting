#ifndef Geometry_hpp
#define Geometry_hpp

namespace RC {

struct Point {
    float x = 0.f;
    float y = 0.f;
};

struct Segment {
    Point a;
    Point b;
};

} // namespace RC

#endif /* Geometry_hpp */
