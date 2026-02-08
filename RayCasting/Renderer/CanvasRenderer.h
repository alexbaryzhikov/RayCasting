#import <Metal/Metal.h>

@interface CanvasRenderer : NSObject

- (nonnull instancetype)initWithDevice:(nonnull id<MTLDevice>)device;
- (nonnull id<MTLTexture>)drawFrameWithCommandBuffer:(nonnull id<MTLCommandBuffer>)commandBuffer;

@end
