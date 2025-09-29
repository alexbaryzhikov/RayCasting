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
    return RC::canvasBytes();
}

+ (void*)fontBytes {
    return RC::fontBytes();
}

+ (void)loadMap:(const void*)bytes size:(NSUInteger)size {
    RC::loadMap(bytes, size);
}

+ (void)startWorld {
    RC::startWorld();
}

+ (void)drawWorld {
    RC::drawWorld();
}

+ (void)updateWorld {
    RC::updateWorld();
}

@end
