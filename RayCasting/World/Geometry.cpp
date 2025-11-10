#include "Geometry.hpp"

#include "Config.h"

namespace RC::Geometry {

Segment makeSegment(float ax, float ay, float bx, float by) {
    return {simd::float3{ax, ay, 1.0f},
            simd::float3{bx, by, 1.0f}};
}

std::vector<Segment> makeGrid(size_t width, size_t height) {
    std::vector<Segment> grid;
    float gridWidth = width * MAP_TILE_SIZE;
    float gridHeight = height * MAP_TILE_SIZE;
    for (size_t i = 0; i <= width; ++i) {
        float offset = i * MAP_TILE_SIZE;
        grid.push_back(makeSegment(offset, 0, offset, gridHeight));
    }
    for (size_t i = 0; i <= height; ++i) {
        float offset = i * MAP_TILE_SIZE;
        grid.push_back(makeSegment(0, offset, gridWidth, offset));
    }
    return grid;
}

std::vector<Segment> makePlayer() {
    return {
        makeSegment(-16, 0, 16, 0),
        makeSegment(8, -8, 16, 0),
        makeSegment(8, 8, 16, 0),
    };
}

std::vector<Segment> makeWall() {
    float i0 = 0;
    float i1 = MAP_TILE_SIZE / 8;
    float i2 = MAP_TILE_SIZE * 3 / 8;
    float i3 = MAP_TILE_SIZE * 5 / 8;
    float i4 = MAP_TILE_SIZE * 7 / 8;
    float i5 = MAP_TILE_SIZE;
    return {
        makeSegment(i0, i0, i5, i0),
        makeSegment(i0, i5, i5, i5),
        makeSegment(i0, i0, i0, i5),
        makeSegment(i5, i0, i5, i5),
        makeSegment(i0, i1, i1, i0),
        makeSegment(i0, i2, i2, i0),
        makeSegment(i0, i3, i3, i0),
        makeSegment(i0, i4, i4, i0),
        makeSegment(i1, i5, i5, i1),
        makeSegment(i2, i5, i5, i2),
        makeSegment(i3, i5, i5, i3),
        makeSegment(i4, i5, i5, i4),
    };
}

std::vector<Segment> makeWallFortified() {
    float i0 = 0;
    float i1 = MAP_TILE_SIZE / 4;
    float i2 = MAP_TILE_SIZE / 2;
    float i3 = MAP_TILE_SIZE * 3 / 4;
    float i4 = MAP_TILE_SIZE;
    return {
        makeSegment(i0, i0, i4, i0),
        makeSegment(i0, i1, i4, i1),
        makeSegment(i0, i2, i4, i2),
        makeSegment(i0, i3, i4, i3),
        makeSegment(i0, i4, i4, i4),
        makeSegment(i0, i0, i0, i4),
        makeSegment(i1, i1, i1, i2),
        makeSegment(i1, i3, i1, i4),
        makeSegment(i2, i0, i2, i1),
        makeSegment(i2, i2, i2, i3),
        makeSegment(i3, i1, i3, i2),
        makeSegment(i3, i3, i3, i4),
        makeSegment(i4, i0, i4, i4),
    };
}

std::vector<Segment> makeWallIndestuctible() {
    float i0 = 0;
    float i1 = MAP_TILE_SIZE;
    return {
        makeSegment(i0, i0, i1, i0),
        makeSegment(i0, i1, i1, i1),
        makeSegment(i0, i0, i0, i1),
        makeSegment(i1, i0, i1, i1),
        makeSegment(i0, i0, i1, i1),
        makeSegment(i0, i1, i1, i0),
    };
}

} // namespace RC::Geometry
