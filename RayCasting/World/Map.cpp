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

const std::vector<Segment> playerGeometry = Geometry::makePlayer();
const std::vector<Segment> wallGeometry = Geometry::makeWall();
const std::vector<Segment> fortifiedWallGeometry = Geometry::makeWallFortified();
const std::vector<Segment> indestructibleWallGeometry = Geometry::makeWallIndestuctible();

std::vector<Segment> gridGeometry;
std::vector<Tile> tiles;
size_t tilesWidth;
size_t tilesHeight;
float width;
float height;
Frame frame = fullFrame;
float zoomFactor = MAP_ZOOM_DEFAULT;
simd::float2 positionOffset = -Player::position.xy;
bool visible = false;
bool follow = true;

bool isVisible() {
    return visible;
}

bool isFullScreen() {
    return frame == fullFrame;
}

bool isFollowing() {
    return follow;
}

void initialize() {
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
    simd::float2 offset = positionOffset * zoomFactor;
    simd::float3x3 translate = makeTranslationMatrix(frame.centerX() + offset.x, frame.centerY() + offset.y);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, scale);
    drawGeometry(gridGeometry, transform, Palette::mapGridColor);
}

void drawWall(size_t row, size_t col) {
    simd::float2 offset = positionOffset * zoomFactor;
    simd::float3 wallPosition = simd::float3{float(col), float(row), 1.0f} * MAP_TILE_SIZE * zoomFactor;
    simd::float3x3 translate = makeTranslationMatrix(
        frame.centerX() + wallPosition.x + offset.x,
        frame.centerY() + wallPosition.y + offset.y);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, scale);
    switch (tiles[row * MAP_WIDTH + col]) {
        case Tile::wall:
            drawGeometry(wallGeometry, transform, Palette::mapWallColor);
            break;
        case Tile::wallFortified:
            drawGeometry(fortifiedWallGeometry, transform, Palette::mapWallColor);
            break;
        case Tile::wallIndestructible:
            drawGeometry(indestructibleWallGeometry, transform, Palette::mapWallColor);
            break;
        default:
            break;
    }
}

void drawWalls() {
    for (size_t row = 0; row < tilesHeight; ++row) {
        for (size_t col = 0; col < tilesWidth; ++col) {
            if (tiles[row * tilesWidth + col] != Tile::floor) {
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
    simd::float2 offset = positionOffset * zoomFactor;
    simd::float3x3 translate = makeTranslationMatrix(frame.centerX() + offset.x, frame.centerY() + offset.y);
    simd::float3x3 scale = makeScaleMatrix(zoomFactor, zoomFactor);
    simd::float3x3 transform = matrix_multiply(translate, scale);
    drawGeometry({segR}, transform, Palette::red);
    drawGeometry({segG}, transform, Palette::green);
    drawGeometry({segB}, transform, Palette::blue);
}

void draw() {
    if (visible) {
        Canvas::setClipFrame(frame);
        Canvas::fill(Palette::mapBackgroundColor);
        drawGrid();
        drawWalls();
        drawRays();
        Canvas::resetClipFrame();
    }
}

void updateVisibility() {
    static bool updated = false;
    if (!updated && Keyboard::keys[Keyboard::keyM]) {
        updated = true;
        Frame targetFrame = Keyboard::keys[Keyboard::keyShift] ? miniFrame : fullFrame;
        if (visible && frame == targetFrame) {
            visible = false;
        } else {
            visible = true;
            frame = targetFrame;
        }
    }
    if (!Keyboard::keys[Keyboard::keyM]) {
        updated = false;
    }
}

void updateFollowMode() {
    static bool updated = false;
    if (!updated && Keyboard::keys[Keyboard::keyF]) {
        updated = true;
        follow = !follow;
    }
    if (!Keyboard::keys[Keyboard::keyF]) {
        updated = false;
    }
}

void updatePositionOffset() {
    if (follow) {
        positionOffset = -Player::position.xy;
    } else {
        if (Keyboard::keys[Keyboard::keyLeft]) {
            positionOffset.x += MAP_SCROLL_SPEED / zoomFactor;
        }
        if (Keyboard::keys[Keyboard::keyRight]) {
            positionOffset.x -= MAP_SCROLL_SPEED / zoomFactor;
        }
        if (Keyboard::keys[Keyboard::keyUp]) {
            positionOffset.y += MAP_SCROLL_SPEED / zoomFactor;
        }
        if (Keyboard::keys[Keyboard::keyDown]) {
            positionOffset.y -= MAP_SCROLL_SPEED / zoomFactor;
        }
    }
}

void updateZoom() {
    if (Keyboard::keys[Keyboard::keyEquals]) {
        zoomFactor *= MAP_ZOOM_SPEED;
    }
    if (Keyboard::keys[Keyboard::keyMinus]) {
        zoomFactor /= MAP_ZOOM_SPEED;
    }
    if (Keyboard::keys[Keyboard::key0]) {
        zoomFactor = MAP_ZOOM_DEFAULT;
    }
}

void update() {
    updateVisibility();
    if (visible) {
        updateFollowMode();
        updatePositionOffset();
        updateZoom();
    }
}

} // namespace RC::Map
