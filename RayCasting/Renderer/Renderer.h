//
//  Renderer.h
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 30.08.25.
//

#import <MetalKit/MetalKit.h>

/// Platform independent renderer class. Implements the MTKViewDelegate protocol which
/// allows it to accept per-frame update and drawable resize callbacks.
@interface Renderer : NSObject <MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)view;

@end
