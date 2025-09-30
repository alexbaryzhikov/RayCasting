//
//  RCBridge.m
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 28.09.25.
//

#import <Foundation/Foundation.h>

#import "RCBridge.h"

#include "Canvas.hpp"
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

@end
