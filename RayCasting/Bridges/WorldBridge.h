//
//  WorldBridge.h
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 20.09.25.
//

@interface WorldBridge : NSObject

+ (void)loadMap:(const void*)data size:(NSUInteger)size;

+ (void)start;

+ (void)draw;

@end
