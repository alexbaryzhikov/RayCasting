#include <numbers>

#include "Map.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Geometry.hpp"
#include "Keyboard.hpp"
#include "MathUtils.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Viewport.hpp"

namespace RC::Map {

constexpr Frame fullFrame = {0, 0, CANVAS_WIDTH, CANVAS_HEIGHT};
constexpr Frame miniFrame = {CANVAS_WIDTH - CANVAS_HEIGHT / 3.0f, 0, CANVAS_HEIGHT / 3.0f, CANVAS_HEIGHT / 3.0f};
constexpr float fullDefaultZoom = MAP_ZOOM_DEFAULT;
constexpr float miniDefaultZoom = MAP_ZOOM_DEFAULT / 2.0f;

const std::vector<Segment> playerGeometry = Geometry::makePlayer();
const std::vector<Segment> wallGeometry = Geometry::makeWall();
std::vector<Segment> gridGeometry;

std::vector<Tile> tiles;
size_t tilesWidth;
size_t tilesHeight;
float width;
float height;
Frame frame = fullFrame;
float zoomFactor = fullDefaultZoom;
bool isVisible = false;

bool isFullScreen() {
    return frame == fullFrame;
}

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
    gridGeometry = Geometry::makeGrid(tilesWidth, tilesHeight);
}

void drawGeometry(const std::vector<Segment>& geometry, simd::float3x3 transform, uint32_t color) {
    Palette::color = color;
    for (Segment segment : geometry) {
        simd::float3 a = matrix_multiply(transform, segment.a);
        simd::float3 b = matrix_multiply(transform, segment.b);
        Canvas::line(a.x, a.y, b.x, b.y);
    }
}

void drawGrid() {
    simd::float3 playerPosition = Player::position * zoomFactor;
    simd::float3x3 translate = makeTranslationMatrix(frame.centerX() - playerPosition.x, frame.centerY() - playerPosition.y);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, scale);
    drawGeometry(gridGeometry, transform, Palette::mapGridColor);
}

void drawWall(size_t row, size_t col) {
    simd::float3 playerPosition = Player::position * zoomFactor;
    simd::float3 wallPosition = simd::float3{float(col), float(row), 1.0f} * MAP_TILE_SIZE * zoomFactor;
    simd::float3x3 translate = makeTranslationMatrix(
        frame.centerX() + wallPosition.x - playerPosition.x,
        frame.centerY() + wallPosition.y - playerPosition.y);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, scale);
    drawGeometry(wallGeometry, transform, Palette::mapWallColor);
}

void drawWalls() {
    for (size_t row = 0; row < tilesHeight; ++row) {
        for (size_t col = 0; col < tilesWidth; ++col) {
            if (tiles[row * tilesWidth + col] == Tile::wall) {
                drawWall(row, col);
            }
        }
    }
}

void drawPlayer() {
    simd::float3x3 translate = makeTranslationMatrix(frame.centerX(), frame.centerY());
    simd::float3x3 rotate = makeRotationMatrix(Player::angle);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, -zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, matrix_multiply(rotate, scale));
    drawGeometry(playerGeometry, transform, Palette::green);
}

void drawRays() {
    simd::float2 rayR = Viewport::castRay(-CAMERA_FOV / 2.0f).xy;
    simd::float2 rayG = Viewport::castRay(0.0f).xy;
    simd::float2 rayB = Viewport::castRay(CAMERA_FOV / 2.0f).xy;
    Segment segR = Geometry::makeSegment(Player::position.x, Player::position.y, Player::position.x + rayR.x, Player::position.y + rayR.y);
    Segment segG = Geometry::makeSegment(Player::position.x, Player::position.y, Player::position.x + rayG.x, Player::position.y + rayG.y);
    Segment segB = Geometry::makeSegment(Player::position.x, Player::position.y, Player::position.x + rayB.x, Player::position.y + rayB.y);
    simd::float3 playerPosition = Player::position * zoomFactor;
    simd::float3x3 translate = makeTranslationMatrix(frame.centerX() - playerPosition.x, frame.centerY() - playerPosition.y);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, scale);
    drawGeometry({segR}, transform, Palette::red);
    drawGeometry({segG}, transform, Palette::green);
    drawGeometry({segB}, transform, Palette::blue);
}

void draw() {
    if (isVisible) {
        Canvas::setClipFrame(frame);
        Canvas::fill(Palette::mapBackgroundColor);
        drawGrid();
        drawWalls();
        drawRays();
    }
}

void updateVisibility() {
    static bool updated = false;
    if (!updated && Keyboard::keys[Keyboard::keyM]) {
        updated = true;
        isVisible = !isVisible;
    }
    if (!Keyboard::keys[Keyboard::keyM]) {
        updated = false;
    }
}

void updateFrame() {
    if (Keyboard::keys[Keyboard::keyShift] && Keyboard::keys[Keyboard::keyMinus]) {
        frame = miniFrame;
        zoomFactor = miniDefaultZoom;
    }
    if (Keyboard::keys[Keyboard::keyShift] && Keyboard::keys[Keyboard::keyEquals]) {
        frame = fullFrame;
        zoomFactor = fullDefaultZoom;
    }
}

void updateZoom() {
    if (!Keyboard::keys[Keyboard::keyShift] && Keyboard::keys[Keyboard::keyEquals]) {
        zoomFactor *= MAP_ZOOM_SPEED;
    }
    if (!Keyboard::keys[Keyboard::keyShift] && Keyboard::keys[Keyboard::keyMinus]) {
        zoomFactor /= MAP_ZOOM_SPEED;
    }
    if (Keyboard::keys[Keyboard::key0]) {
        zoomFactor = frame == fullFrame ? fullDefaultZoom : miniDefaultZoom;
    }
}

void update() {
    updateVisibility();
    if (isVisible) {
        updateFrame();
        updateZoom();
    }
}

} // namespace RC::Map
