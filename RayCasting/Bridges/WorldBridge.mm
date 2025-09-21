//
//  WorldBridge.mm
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 20.09.25.
//

#import <Foundation/Foundation.h>

#import "WorldBridge.h"

#include "World.hpp"

@implementation WorldBridge

+ (void)loadMap:(const void*)data size:(NSUInteger)size {
    RC::World::get().loadMap(data, size);
}

+ (void)start {
    RC::World::get().start();
}

+ (void)draw {
    RC::World::get().draw();
}

@end
