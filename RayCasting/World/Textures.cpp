#include "Textures.hpp"

#include "Canvas.hpp"
#include "Palette.hpp"

namespace RC::Textures {

std::array<uint32_t, dimension * dimension> dungeonWallBase;
std::array<uint32_t, dimension * dimension> dungeonWallTopBeam;
std::array<uint32_t, dimension * dimension> dungeonWallTorch;
std::array<uint32_t, dimension * dimension> dungeonWallVerticalBeam;
std::array<uint32_t, dimension * dimension> dungeonWallWindow;

void drawTexture() {
    for (int y = 0; y < dimension; ++y) {
        for (int x = 0; x < dimension; ++x) {
            Palette::setColor(dungeonWallTopBeam[y * dimension + x]);
            Canvas::point(x, y);
        }
    }
}

} // namespace RC::Textures
