#import <MetalKit/MetalKit.h>

@protocol Renderer <NSObject, MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)view;

@end
