#import <Foundation/Foundation.h>

#include <simd/simd.h>

#include "GPUShaderTypes.h"

@interface RCBridge : NSObject

+ (const void*)canvasBytes;
+ (void*)fontBytes;
+ (void*)textureBytes_ceiling;
+ (void*)textureBytes_door;
+ (void*)textureBytes_floor;
+ (void*)textureBytes_wall;
+ (void*)textureBytes_wallFortified;
+ (void*)textureBytes_wallFortifiedDecal1;
+ (void*)textureBytes_wallFortifiedDecal2;
+ (void*)textureBytes_wallFortifiedTorch;
+ (void*)textureBytes_wallIndestructible;
+ (void)generateMap;
+ (void)startWorld;
+ (void)drawWorld;
+ (void)updateWorld;
+ (void)keyDown:(unsigned short)code;
+ (void)keyUp:(unsigned short)code;
+ (void)flagsChanged:(unsigned long)flags;
+ (void)mouseDown;
+ (void)mouseUp;
+ (void)rightMouseDown;
+ (void)rightMouseUp;
+ (void)mouseMoved:(CGVector)offset;
+ (void)mouseWheel:(CGVector)offset;
+ (void)copyCamera:(Camera*)dst;
+ (void)copyTiles:(Tile*)dst;

@end
