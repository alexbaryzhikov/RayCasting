#include "Textures.hpp"

#include "Canvas.hpp"
#include "Palette.hpp"

namespace RC::Textures {

std::array<uint32_t, dimension * dimension> ceiling;
std::array<uint32_t, dimension * dimension> door;
std::array<uint32_t, dimension * dimension> floor;
std::array<uint32_t, dimension * dimension> wall;
std::array<uint32_t, dimension * dimension> wallFortified;
std::array<uint32_t, dimension * dimension> wallFortifiedDecal1;
std::array<uint32_t, dimension * dimension> wallFortifiedDecal2;
std::array<uint32_t, dimension * dimension> wallFortifiedTorch;
std::array<uint32_t, dimension * dimension> wallIndestructible;

void drawTexture() {
    for (int y = 0; y < dimension; ++y) {
        for (int x = 0; x < dimension; ++x) {
            Canvas::point(x, y, wall[y * dimension + x]);
        }
    }
}

} // namespace RC::Textures
