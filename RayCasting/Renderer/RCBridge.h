//
//  RCBridge.h
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 28.09.25.
//

@interface RCBridge : NSObject

+ (const void*)canvasBytes;
+ (void*)fontBytes;
+ (void)loadMap:(const void*)bytes size:(NSUInteger)size;
+ (void)startWorld;
+ (void)drawWorld;
+ (void)updateWorld;

@end
