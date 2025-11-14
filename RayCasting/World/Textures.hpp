#ifndef Textures_hpp
#define Textures_hpp

#include <array>

namespace RC::Textures {

constexpr size_t dimension = 512;

extern std::array<uint32_t, dimension * dimension> ceiling;
extern std::array<uint32_t, dimension * dimension> door;
extern std::array<uint32_t, dimension * dimension> floor;
extern std::array<uint32_t, dimension * dimension> wall;
extern std::array<uint32_t, dimension * dimension> wallFortified;
extern std::array<uint32_t, dimension * dimension> wallFortifiedDecal1;
extern std::array<uint32_t, dimension * dimension> wallFortifiedDecal2;
extern std::array<uint32_t, dimension * dimension> wallFortifiedTorch;
extern std::array<uint32_t, dimension * dimension> wallIndestructible;

void drawTexture();

} // namespace RC::Textures

#endif /* Textures_hpp */
