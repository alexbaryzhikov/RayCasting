#import "RCBridge.h"

#include "Canvas.hpp"
#include "Keyboard.hpp"
#include "Map.hpp"
#include "MapGenerator.hpp"
#include "Mouse.hpp"
#include "Player.hpp"
#include "Text.hpp"
#include "Viewport.hpp"
#include "World.hpp"

@implementation RCBridge

+ (void)setCanvasBytes:(void*)bytes {
    RC::Canvas::canvas = (uint32_t*)bytes;
}

+ (void)setFontBytes:(void*)bytes {
    RC::Text::font = (uint32_t*)bytes;
}

+ (void)generateMap {
    RC::Map::generate();
}

+ (void)drawWorld {
    RC::World::draw();
}

+ (void)updateWorld {
    RC::World::update();
}

+ (void)copyCamera:(Camera*)dst {
    dst->position.xy = RC::Player::position.xy;
    dst->position.z = RC::Viewport::cameraPositionZ;
    dst->position.w = 1;
    dst->angle = RC::Player::angle;
}

+ (void)copyMap:(Map*)dst {
    TileType* tiles = dst->tiles;
    for (int i = 0; i < RC::Map::tiles.size(); ++i) {
        switch (RC::Map::tiles[i]) {
            case RC::TileType::doorH:
                tiles[i] = TileTypeDoorH;
                break;
            case RC::TileType::doorV:
                tiles[i] = TileTypeDoorV;
                break;
            case RC::TileType::empty:
                tiles[i] = TileTypeEmpty;
                break;
            case RC::TileType::wall:
                tiles[i] = TileTypeWall;
                break;
            case RC::TileType::wallFortified:
                tiles[i] = TileTypeWallFortified;
                break;
            case RC::TileType::wallIndestructible:
                tiles[i] = TileTypeWallIndestructible;
                break;
        }
    }
    Door* doors = dst->doors;
    for (auto [i, door] : RC::Map::doors) {
        doors[i].progress = door.progress;
    }
}

+ (void)keyDown:(unsigned short)code {
    RC::Keyboard::keyPressed(code);
}

+ (void)keyUp:(unsigned short)code {
    RC::Keyboard::keyReleased(code);
}

+ (void)flagsChanged:(unsigned long)flags {
    RC::Keyboard::flagsChanged(flags);
}

+ (void)mouseDown {
    RC::Mouse::leftButtonPressed();
}

+ (void)mouseUp {
    RC::Mouse::leftButtonReleased();
}

+ (void)rightMouseDown {
    RC::Mouse::rightButtonPressed();
}

+ (void)rightMouseUp {
    RC::Mouse::rightButtonReleased();
}

+ (void)mouseMoved:(CGVector)offset {
    RC::Mouse::moved(offset.dx, offset.dy);
}

+ (void)mouseWheel:(CGVector)offset {
    RC::Mouse::scrolled(offset.dx, offset.dy);
}

@end
