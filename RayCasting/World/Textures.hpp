#ifndef Textures_hpp
#define Textures_hpp

#include <array>

namespace RC::Textures {

constexpr size_t dimension = 512;

extern std::array<uint32_t, dimension * dimension> dungeonFloorCobblestone;
extern std::array<uint32_t, dimension * dimension> dungeonFloorDirt;
extern std::array<uint32_t, dimension * dimension> dungeonWallBase;
extern std::array<uint32_t, dimension * dimension> dungeonWallTopBeam;
extern std::array<uint32_t, dimension * dimension> dungeonWallTorch;
extern std::array<uint32_t, dimension * dimension> dungeonWallVerticalBeam;
extern std::array<uint32_t, dimension * dimension> dungeonWallWindow;

void drawTexture();

} // namespace RC::Textures

#endif /* Textures_hpp */
