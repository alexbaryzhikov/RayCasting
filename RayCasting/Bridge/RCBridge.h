#import <Foundation/Foundation.h>

#include "GPUShaderTypesShared.h"

@interface RCBridge : NSObject

+ (void)setCanvasBytes:(void*)bytes;
+ (void)setFontBytes:(void*)bytes;
+ (void)generateMap;
+ (void)drawWorld;
+ (void)updateWorld;
+ (void)copyCamera:(Camera*)dst;
+ (void)copyMap:(Map*)dst;
+ (void)keyDown:(unsigned short)code;
+ (void)keyUp:(unsigned short)code;
+ (void)flagsChanged:(unsigned long)flags;
+ (void)mouseDown;
+ (void)mouseUp;
+ (void)rightMouseDown;
+ (void)rightMouseUp;
+ (void)mouseMoved:(CGVector)offset;
+ (void)mouseWheel:(CGVector)offset;

@end
