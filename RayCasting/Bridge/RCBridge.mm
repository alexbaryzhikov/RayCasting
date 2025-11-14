#import <Foundation/Foundation.h>

#import "RCBridge.h"

#include "Canvas.hpp"
#include "Keyboard.hpp"
#include "Map.hpp"
#include "MapGenerator.hpp"
#include "MapLoader.hpp"
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

+ (void*)textureBytes_ceiling {
    return RC::Textures::ceiling.data();
}

+ (void*)textureBytes_door {
    return RC::Textures::door.data();
}

+ (void*)textureBytes_floor {
    return RC::Textures::floor.data();
}

+ (void*)textureBytes_wall {
    return RC::Textures::wall.data();
}

+ (void*)textureBytes_wallFortified {
    return RC::Textures::wallFortified.data();
}

+ (void*)textureBytes_wallFortifiedDecal1 {
    return RC::Textures::wallFortifiedDecal1.data();
}

+ (void*)textureBytes_wallFortifiedDecal2 {
    return RC::Textures::wallFortifiedDecal2.data();
}

+ (void*)textureBytes_wallFortifiedTorch {
    return RC::Textures::wallFortifiedTorch.data();
}

+ (void*)textureBytes_wallIndestructible {
    return RC::Textures::wallIndestructible.data();
}

+ (void)loadMap:(const void*)bytes size:(NSUInteger)size {
    RC::Map::load(bytes, size);
}

+ (void)generateMap {
    RC::Map::generate();
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
