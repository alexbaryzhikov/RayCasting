#include "Geometry.hpp"

#include "Config.h"

namespace RC::Geometry {

Segment makeSegment(float ax, float ay, float bx, float by) {
    return {simd::float3{ax, ay, 1.0f},
            simd::float3{bx, by, 1.0f}};
}

std::vector<Segment> makeGrid(size_t width, size_t height) {
    std::vector<Segment> grid;
    float gridWidth = width * MAP_BLOCK_SIZE;
    float gridHeight = height * MAP_BLOCK_SIZE;
    for (size_t i = 0; i <= width; ++i) {
        float offset = i * MAP_BLOCK_SIZE;
        grid.push_back(makeSegment(offset, 0, offset, gridHeight));
    }
    for (size_t i = 0; i <= height; ++i) {
        float offset = i * MAP_BLOCK_SIZE;
        grid.push_back(makeSegment(0, offset, gridWidth, offset));
    }
    return grid;
}

std::vector<Segment> makePlayer() {
    return {
        makeSegment(-8, 0, 8, 0),
        makeSegment(0, -8, 8, 0),
        makeSegment(0, 8, 8, 0),
    };
}

std::vector<Segment> makeWall() {
    std::vector<Segment> wall = {
        makeSegment(0, 0, MAP_BLOCK_SIZE, 0),
        makeSegment(0, MAP_BLOCK_SIZE, MAP_BLOCK_SIZE, MAP_BLOCK_SIZE),
        makeSegment(0, 0, 0, MAP_BLOCK_SIZE),
        makeSegment(MAP_BLOCK_SIZE, 0, MAP_BLOCK_SIZE, MAP_BLOCK_SIZE),
    };
    for (size_t i = 8; i < MAP_BLOCK_SIZE; i += 16) {
        wall.push_back(makeSegment(0, i, i, 0));
        wall.push_back(makeSegment(i, MAP_BLOCK_SIZE, MAP_BLOCK_SIZE, i));
    }
    return wall;
}

} // namespace RC::Geometry
