#ifndef Palette_hpp
#define Palette_hpp

#include <cstdint>

namespace RC {

enum class BlendMode {
    normal,
    add,
    multipy,
};

}

namespace RC::Palette {

constexpr uint32_t black = 0xFF000000;
constexpr uint32_t white = 0xFFFFFFFF;
constexpr uint32_t red = 0xFFFF0000;
constexpr uint32_t green = 0xFF00FF00;
constexpr uint32_t blue = 0xFF0000FF;
constexpr uint32_t cyan = 0xFF00FFFF;
constexpr uint32_t magenta = 0xFFFF00FF;
constexpr uint32_t yellow = 0xFFFFFF00;

constexpr uint32_t mapWallColor = 0xFF628699;
constexpr uint32_t mapGridColor = 0xFF353E43;
constexpr uint32_t mapBackgroundColor = 0xFF041E2B;

constexpr uint32_t fogColor = 0xFF070016;
constexpr uint32_t lightColor = 0xFFDE2728;
constexpr uint32_t shadowColor = 0xFF015A00;

extern uint32_t color;

uint32_t blend(uint32_t baseColor, uint32_t blendColor, uint32_t alpha, BlendMode mode);

} // namespace RC::Palette

#endif /* Palette_hpp */
