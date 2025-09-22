//
//  CanvasBridge.mm
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 21.09.25.
//

#import <Foundation/Foundation.h>

#import "CanvasBridge.h"

#include "Canvas.hpp"

@implementation CanvasBridge

+ (const void*)getData {
    return RC::canvasData();
}

@end
