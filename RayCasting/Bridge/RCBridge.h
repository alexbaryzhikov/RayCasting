@interface RCBridge : NSObject

+ (const void*)canvasBytes;
+ (void*)fontBytes;
+ (void)loadMap:(const void*)bytes size:(NSUInteger)size;
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
