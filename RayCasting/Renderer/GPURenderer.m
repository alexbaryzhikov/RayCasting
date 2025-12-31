#import "GPURenderer.h"
#import "Config.h"
#import "GPUShaderTypes.h"

@implementation GPURenderer {
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLComputePipelineState> _computePipelineState;

    id<MTLTexture> _textures[TEXTURE_HEAP_SIZE];
    NSUInteger _texturesCount;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)view {
    self = [super init];
    if (self) {
        _device = view.device;
        [self setupView:view];
        [self loadMetal];
        [self loadTextures];
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

- (void)loadMetal {
    _commandQueue = [_device newCommandQueue];

    id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
    id<MTLFunction> kernelFunction = [defaultLibrary newFunctionWithName:@"castRays"];

    NSError* error = nil;
    _computePipelineState = [_device newComputePipelineStateWithFunction:kernelFunction error:&error];
    if (!_computePipelineState) {
        NSLog(@"Failed to create compute pipeline state: %@", error);
    }
}

- (void)loadTextures {
    _textures[0] = [self loadTexture:@"door"];
    _texturesCount = 1;
}

- (nullable id<MTLTexture>)loadTexture:(nonnull NSString*)name {
    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:_device];
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

- (void)drawInMTKView:(MTKView*)view {
    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (!drawable) {
        return;
    }

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLComputeCommandEncoder> commandEncoder = [commandBuffer computeCommandEncoder];
    [commandEncoder setComputePipelineState:_computePipelineState];
    [commandEncoder setTexture:drawable.texture atIndex:0];
    if (_texturesCount > 0) {
        [commandEncoder setTextures:_textures withRange:NSMakeRange(1, _texturesCount)];
    }

    NSUInteger width = _computePipelineState.threadExecutionWidth;
    NSUInteger height = _computePipelineState.maxTotalThreadsPerThreadgroup / width;
    MTLSize threadsPerThreadgroup = MTLSizeMake(width, height, 1);
    MTLSize threadsPerGrid = MTLSizeMake(drawable.texture.width, drawable.texture.height, 1);

    [commandEncoder dispatchThreads:threadsPerGrid threadsPerThreadgroup:threadsPerThreadgroup];

    [commandEncoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size {
}

@end
