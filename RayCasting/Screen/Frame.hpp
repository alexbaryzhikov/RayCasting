#ifndef Frame_hpp
#define Frame_hpp

namespace RC {

struct Frame {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    float centerX() { return x + w / 2.0f; }
    float centerY() { return y + h / 2.0f; }
};

} // namespace RC

#endif /* Frame_hpp */
