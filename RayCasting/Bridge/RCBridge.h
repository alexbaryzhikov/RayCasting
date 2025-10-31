@interface RCBridge : NSObject

+ (const void*)canvasBytes;
+ (void*)fontBytes;
+ (void*)textureBytes_DungeonCeilingRock;
+ (void*)textureBytes_DungeonFloorDirt;
+ (void*)textureBytes_DungeonWallBase;
+ (void*)textureBytes_DungeonWallRock;
+ (void*)textureBytes_DungeonWallTopBeam;
+ (void*)textureBytes_DungeonWallTorch;
+ (void*)textureBytes_DungeonWallVerticalBeam;
+ (void*)textureBytes_DungeonWallWindow;
+ (void)loadMap:(const void*)bytes size:(NSUInteger)size;
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

@end
