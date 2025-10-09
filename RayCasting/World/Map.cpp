#include <print>

#include "Map.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Frame.hpp"
#include "Geometry.hpp"
#include "Keyboard.hpp"
#include "MathUtils.hpp"
#include "Palette.hpp"
#include "Player.hpp"

namespace RC::Map {

const std::vector<Segment> playerGeometry = {
    makeSegment(-8.0f, 0.0f, 8.0f, 0.0f),
    makeSegment(0.0f, -8.0f, 8.0f, 0.0f),
    makeSegment(0.0f, 8.0f, 8.0f, 0.0f),
};

std::vector<std::vector<Tile>> tiles;
std::vector<Segment> grid;
Frame frame = {0.0f, 0.0f, CANVAS_WIDTH, CANVAS_HEIGHT};
float zoomFactor = MAP_ZOOM_DEFAULT;

size_t width() {
    return tiles.empty() ? 0 : tiles[0].size();
}

size_t height() {
    return tiles.size();
}

bool isValid() {
    if (tiles.empty()) {
        return false;
    }
    for (size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i].size() != width()) {
            return false;
        }
    }
    return true;
}

void makeGrid() {
    float gridWidth = width() * MAP_BLOCK_SIZE;
    float gridHeight = height() * MAP_BLOCK_SIZE;
    for (size_t i = 0; i <= width(); ++i) {
        float offset = i * MAP_BLOCK_SIZE;
        grid.push_back(makeSegment(offset, 0.0f, offset, gridHeight));
        grid.push_back(makeSegment(0.0f, offset, gridWidth, offset));
    }
}

void load(const void* bytes, size_t size) {
    const char* chars = static_cast<const char*>(bytes);
    std::vector<Tile> row;
    for (size_t i = 0; i < size; ++i) {
        size_t x = row.size();
        size_t y = tiles.size();
        switch (chars[i]) {
            case '#':
                row.push_back(Tile::WALL);
                break;
            case '.':
                row.push_back(Tile::FLOOR);
                break;
            case '>':
                Player::angle = 0.0f;
                Player::position = simd_float3{x + 0.5f, y + 0.5f, 1.0f} * MAP_BLOCK_SIZE;
                row.push_back(Tile::FLOOR);
                break;
            case '<':
                Player::angle = M_PI;
                Player::position = simd_float3{x + 0.5f, y + 0.5f, 1.0f} * MAP_BLOCK_SIZE;
                row.push_back(Tile::FLOOR);
                break;
            case '^':
                Player::angle = -M_PI_2;
                Player::position = simd_float3{x + 0.5f, y + 0.5f, 1.0f} * MAP_BLOCK_SIZE;
                row.push_back(Tile::FLOOR);
                break;
            case 'v':
                Player::angle = M_PI_2;
                Player::position = simd_float3{x + 0.5f, y + 0.5f, 1.0f} * MAP_BLOCK_SIZE;
                row.push_back(Tile::FLOOR);
                break;
            case '\n':
                tiles.push_back(std::move(row));
                break;
            default:
                break;
        }
    }
    if (isValid()) {
        std::println("Loaded map of size {} x {}", width(), height());
        makeGrid();
    } else {
        std::println("Invalid map data");
        return;
    }
}

void drawGrid() {
    simd::float3 playerPosition = Player::position * zoomFactor;
    simd::float3x3 translate = makeTranslationMatrix(frame.centerX() - playerPosition.x, frame.centerY() - playerPosition.y);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, scale);
    Palette::setColor(Palette::GUNMETAL_GRAY_DARK);
    for (Segment segment : grid) {
        simd::float3 a = matrix_multiply(transform, segment.a);
        simd::float3 b = matrix_multiply(transform, segment.b);
        Canvas::line(a.x, a.y, b.x, b.y);
    }
}

void drawPlayer() {
    simd::float3x3 translate = makeTranslationMatrix(frame.centerX(), frame.centerY());
    simd::float3x3 rotate = makeRotationMatrix(Player::angle);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, -zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, matrix_multiply(rotate, scale));
    Palette::setColor(Palette::GREEN);
    for (Segment segment : playerGeometry) {
        simd::float3 a = matrix_multiply(transform, segment.a);
        simd::float3 b = matrix_multiply(transform, segment.b);
        Canvas::line(a.x, a.y, b.x, b.y);
    }
}

void draw() {
    drawGrid();
    drawPlayer();
}

void updateZoom() {
    if (Keyboard::keys[Keyboard::KEY_EQUALS]) {
        zoomFactor *= MAP_ZOOM_SPEED;
    }
    if (Keyboard::keys[Keyboard::KEY_MINUS]) {
        zoomFactor /= MAP_ZOOM_SPEED;
    }
    if (Keyboard::keys[Keyboard::KEY_0]) {
        zoomFactor = MAP_ZOOM_DEFAULT;
    }
}

void update() {
    updateZoom();
}

} // namespace RC::Map
