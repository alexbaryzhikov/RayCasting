#include <numbers>
#include <simd/simd.h>

#include "MapLoader.hpp"

#include "Config.h"
#include "Geometry.hpp"
#include "Player.hpp"

namespace RC::Map {

void load(const void* bytes, size_t size) {
    const char* chars = static_cast<const char*>(bytes);
    tilesWidth = 0;
    for (size_t i = 0; i < size && chars[i] != '\n'; ++i) {
        tilesWidth += chars[i] != ' ';
    }
    if (tilesWidth == 0) {
        printf("Invalid map\n");
        return;
    }
    for (size_t i = 0; i < size; ++i) {
        size_t x = tiles.size() % tilesWidth;
        size_t y = tiles.size() / tilesWidth;
        switch (chars[i]) {
            case '#':
                tiles.push_back(Tile::wall);
                break;
            case '.':
                tiles.push_back(Tile::floor);
                break;
            case '>':
                Player::angle = 0.0f;
                Player::position = simd_float3{x + 0.5f, y + 0.5f, 1.0f} * MAP_TILE_SIZE;
                tiles.push_back(Tile::floor);
                break;
            case '<':
                Player::angle = std::numbers::pi;
                Player::position = simd_float3{x + 0.5f, y + 0.5f, 1.0f} * MAP_TILE_SIZE;
                tiles.push_back(Tile::floor);
                break;
            case '^':
                Player::angle = -std::numbers::pi / 2.0f;
                Player::position = simd_float3{x + 0.5f, y + 0.5f, 1.0f} * MAP_TILE_SIZE;
                tiles.push_back(Tile::floor);
                break;
            case 'v':
                Player::angle = std::numbers::pi / 2.0f;
                Player::position = simd_float3{x + 0.5f, y + 0.5f, 1.0f} * MAP_TILE_SIZE;
                tiles.push_back(Tile::floor);
                break;
            default:
                break;
        }
    }
    tilesHeight = tiles.size() / tilesWidth;
    if (tilesHeight == 0 || tiles.size() % tilesWidth != 0) {
        printf("Invalid map\n");
        return;
    }
    printf("Loaded map of size %lu x %lu\n", tilesWidth, tilesHeight);
    width = tilesWidth * MAP_TILE_SIZE;
    height = tilesHeight * MAP_TILE_SIZE;
}

} // namespace RC::Map
