#import <Foundation/Foundation.h>

#import "RCBridge.h"

#include "Canvas.hpp"
#include "Keyboard.hpp"
#include "Map.hpp"
#include "Mouse.hpp"
#include "Text.hpp"
#include "Textures.hpp"
#include "World.hpp"

@implementation RCBridge

+ (const void*)canvasBytes {
    return RC::Canvas::canvas.data();
}

+ (void*)fontBytes {
    return RC::Text::font.data();
}

+ (void*)textureBytes_DungeonCeilingRock {
    return RC::Textures::dungeonCeilingRock.data();
}

+ (void*)textureBytes_DungeonFloorDirt {
    return RC::Textures::dungeonFloorDirt.data();
}

+ (void*)textureBytes_DungeonWallBase {
    return RC::Textures::dungeonWallBase.data();
}

+ (void*)textureBytes_DungeonWallRock {
    return RC::Textures::dungeonWallRock.data();
}

+ (void*)textureBytes_DungeonWallTopBeam {
    return RC::Textures::dungeonWallTopBeam.data();
}

+ (void*)textureBytes_DungeonWallTorch {
    return RC::Textures::dungeonWallTorch.data();
}

+ (void*)textureBytes_DungeonWallVerticalBeam {
    return RC::Textures::dungeonWallVerticalBeam.data();
}

+ (void*)textureBytes_DungeonWallWindow {
    return RC::Textures::dungeonWallWindow.data();
}

+ (void)loadMap:(const void*)bytes size:(NSUInteger)size {
    RC::Map::load(bytes, size);
}

+ (void)startWorld {
    RC::World::start();
}

+ (void)drawWorld {
    RC::World::draw();
}

+ (void)updateWorld {
    RC::World::update();
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
