#import <Foundation/Foundation.h>

#import "RCBridge.h"

#include "Canvas.hpp"
#include "Keyboard.hpp"
#include "Map.hpp"
#include "Text.hpp"
#include "World.hpp"

@implementation RCBridge

+ (const void*)canvasBytes {
    return RC::Canvas::bytes();
}

+ (void*)fontBytes {
    return RC::Text::fontBytes();
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

@end
