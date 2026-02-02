#import "GPURenderer.h"
#import "GPUShaderTypes.h"
#import "RCBridge.h"

@implementation GPURenderer {
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLComputePipelineState> computePipelineState;

    id<MTLTexture> textures[TEXTURE_HEAP_SIZE];
    NSUInteger texturesCount;

    id<MTLBuffer> cameraBuffer;
    id<MTLBuffer> mapBuffer;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)view {
    self = [super init];
    if (self) {
        device = view.device;
        [self setupView:view];
        [self setupMetal];
        [self loadTextures];
        [self setupWorld];
    }
    return self;
}

- (void)setupView:(nonnull MTKView*)view {
    view.framebufferOnly = NO;
    view.autoResizeDrawable = NO;
    view.drawableSize = CGSizeMake(CANVAS_WIDTH, CANVAS_HEIGHT);
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.layer.magnificationFilter = kCAFilterNearest;
    view.layer.contentsGravity = kCAGravityResizeAspect;
}

- (void)setupMetal {
    commandQueue = [device newCommandQueue];

    id<MTLLibrary> defaultLibrary = [device newDefaultLibrary];
    id<MTLFunction> kernelFunction = [defaultLibrary newFunctionWithName:@"castRays"];

    NSError* error = nil;
    computePipelineState = [device newComputePipelineStateWithFunction:kernelFunction error:&error];
    if (!computePipelineState) {
        NSLog(@"Failed to create compute pipeline state: %@", error);
    }

    cameraBuffer = [device newBufferWithLength:sizeof(Camera) options:MTLResourceStorageModeShared];
    mapBuffer = [device newBufferWithLength:sizeof(Map) options:MTLResourceStorageModeShared];
}

- (void)loadTextures {
    textures[TextureIndexCeiling] = [self loadTexture:@"basalt"];
    textures[TextureIndexFloor] = [self loadTexture:@"dirt"];
    textures[TextureIndexDoor] = [self loadTexture:@"door"];
    textures[TextureIndexWall] = [self loadTexture:@"wall_basalt"];
    textures[TextureIndexWallFortified] = [self loadTexture:@"wall_brick"];
    textures[TextureIndexWallIndestructible] = [self loadTexture:@"wall_metal"];
    texturesCount = 6;
}

- (nullable id<MTLTexture>)loadTexture:(nonnull NSString*)name {
    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:device];
    NSURL* url = [NSBundle.mainBundle URLForResource:name withExtension:@"png"];
    if (!url) {
        NSLog(@"Could not find file '%@.png' in main bundle", name);
        return nil;
    }
    NSDictionary<MTKTextureLoaderOption, id>* options = @{
        MTKTextureLoaderOptionTextureUsage : @(MTLTextureUsageShaderRead),
        MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate),
    };
    NSError* error = nil;
    id<MTLTexture> texture = [textureLoader newTextureWithContentsOfURL:url
                                                                options:options
                                                                  error:&error];
    if (!texture || error) {
        NSLog(@"Error loading texture '%@.png': %@", name, error);
        return nil;
    }
    return texture;
}

- (void)setupWorld {
    [RCBridge generateMap];
    [RCBridge startWorld];
}

- (void)drawInMTKView:(MTKView*)view {
    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (!drawable) {
        return;
    }

    [self updateUniforms];

    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    id<MTLComputeCommandEncoder> commandEncoder = [commandBuffer computeCommandEncoder];
    [commandEncoder setComputePipelineState:computePipelineState];
    [commandEncoder setTexture:drawable.texture atIndex:0];
    if (texturesCount > 0) {
        [commandEncoder setTextures:textures withRange:NSMakeRange(1, texturesCount)];
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
