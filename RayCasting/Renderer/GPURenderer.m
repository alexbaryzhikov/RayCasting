#import "GPURenderer.h"
#import "CanvasRenderer.h"
#import "GPUShaderTypes.h"
#import "RCBridge.h"

#define BYTES_PER_PIXEL 4

@implementation GPURenderer {
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLComputePipelineState> computePipelineState;

    CanvasRenderer* canvasRenderer;

    id<MTLTexture> textures[TEXTURE_HEAP_SIZE];
    NSUInteger texturesCount;

    id<MTLBuffer> cameraBuffer;
    id<MTLBuffer> mapBuffer;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)view {
    self = [super init];
    if (self) {
        device = view.device;
        canvasRenderer = [[CanvasRenderer alloc] initWithDevice:device];
        [self setupView:view];
        [self setupMetal];
        [self loadTextures];
        [self loadFont];
        [self setupWorld];
    }
    return self;
}

- (void)setupView:(nonnull MTKView*)view {
    view.framebufferOnly = NO;
    view.autoResizeDrawable = NO;
    view.drawableSize = CGSizeMake(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.layer.magnificationFilter = kCAFilterNearest;
    view.layer.contentsGravity = kCAGravityResizeAspect;
}

- (void)setupMetal {
    commandQueue = [device newCommandQueue];

    id<MTLLibrary> defaultLibrary = [device newDefaultLibrary];
    id<MTLFunction> kernelFunction = [defaultLibrary newFunctionWithName:@"render"];

    NSError* error = nil;
    computePipelineState = [device newComputePipelineStateWithFunction:kernelFunction error:&error];
    if (!computePipelineState) {
        NSLog(@"Failed to create compute pipeline state: %@", error);
    }

    cameraBuffer = [device newBufferWithLength:sizeof(Camera) options:MTLResourceStorageModeShared];
    mapBuffer = [device newBufferWithLength:sizeof(Map) options:MTLResourceStorageModeShared];
}

- (void)loadTextures {
    MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:device];
    NSDictionary<MTKTextureLoaderOption, id>* options = @{
        MTKTextureLoaderOptionTextureUsage : @(MTLTextureUsageShaderRead),
        MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate),
    };
    textures[TextureIndexCeiling] = [self loadTexture:@"basalt" loader:loader options:options];
    textures[TextureIndexFloor] = [self loadTexture:@"dirt" loader:loader options:options];
    textures[TextureIndexDoor] = [self loadTexture:@"door" loader:loader options:options];
    textures[TextureIndexWall] = [self loadTexture:@"wall_basalt" loader:loader options:options];
    textures[TextureIndexWallFortified] = [self loadTexture:@"wall_brick" loader:loader options:options];
    textures[TextureIndexWallIndestructible] = [self loadTexture:@"wall_metal" loader:loader options:options];
    texturesCount = 6;
}

- (nullable id<MTLTexture>)loadTexture:(nonnull NSString*)name
                                loader:(MTKTextureLoader*)loader
                               options:(NSDictionary<MTKTextureLoaderOption, id>*)options {
    NSURL* url = [NSBundle.mainBundle URLForResource:name withExtension:@"png"];
    if (!url) {
        NSLog(@"Could not find file '%@.png' in main bundle", name);
        return nil;
    }
    NSError* error = nil;
    id<MTLTexture> texture = [loader newTextureWithContentsOfURL:url
                                                         options:options
                                                           error:&error];
    if (!texture || error) {
        NSLog(@"Error loading texture '%@.png': %@", name, error);
        return nil;
    }
    return texture;
}

- (void)loadFont {
    MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:device];
    NSDictionary<MTKTextureLoaderOption, id>* options = @{
        MTKTextureLoaderOptionTextureUsage : @(MTLTextureUsageUnknown),
        MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModeShared),
    };
    id<MTLTexture> texture = [self loadTexture:@"font_sweet_sixteen" loader:loader options:options];
    if (!texture) {
        return;
    }
    [texture getBytes:[RCBridge fontBytes]
          bytesPerRow:texture.width * BYTES_PER_PIXEL
           fromRegion:MTLRegionMake2D(0, 0, texture.width, texture.height)
          mipmapLevel:0];
}

- (void)setupWorld {
    [RCBridge generateMap];
    [RCBridge startWorld];
}

- (void)drawInMTKView:(MTKView*)view {
    [self updateUniforms];

    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];

    id<MTLTexture> overlayTexture = [canvasRenderer drawFrameWithCommandBuffer:commandBuffer];
    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (!drawable) {
        return;
    }

    id<MTLComputeCommandEncoder> commandEncoder = [commandBuffer computeCommandEncoder];
    [commandEncoder setComputePipelineState:computePipelineState];
    [commandEncoder setTexture:drawable.texture atIndex:0];
    [commandEncoder setTexture:overlayTexture atIndex:1];
    if (texturesCount > 0) {
        [commandEncoder setTextures:textures withRange:NSMakeRange(2, texturesCount)];
    }
    [commandEncoder setBuffer:cameraBuffer offset:0 atIndex:0];
    [commandEncoder setBuffer:mapBuffer offset:0 atIndex:1];

    NSUInteger width = computePipelineState.threadExecutionWidth;
    NSUInteger height = computePipelineState.maxTotalThreadsPerThreadgroup / width;
    MTLSize threadsPerThreadgroup = MTLSizeMake(width, height, 1);
    MTLSize threadsPerGrid = MTLSizeMake(drawable.texture.width, drawable.texture.height, 1);
    [commandEncoder dispatchThreads:threadsPerGrid threadsPerThreadgroup:threadsPerThreadgroup];

    [commandEncoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

- (void)updateUniforms {
    [RCBridge updateWorld];
    [RCBridge copyCamera:cameraBuffer.contents];
    [RCBridge copyMap:mapBuffer.contents];
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size {
}

@end
