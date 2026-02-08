#import "CanvasRenderer.h"
#import "Config.h"
#import "RCBridge.h"

static const NSUInteger bufferCount = 3;

@implementation CanvasRenderer {
    dispatch_semaphore_t frameSemaphore;
    NSUInteger frameIndex;

    NSMutableArray<id<MTLBuffer>>* buffers;
    NSMutableArray<id<MTLTexture>>* textures;
}

- (nonnull instancetype)initWithDevice:(nonnull id<MTLDevice>)device {
    self = [super init];
    if (self) {
        frameSemaphore = dispatch_semaphore_create(bufferCount);
        frameIndex = 0;
        [self createBuffersWithDevice:device];
    }
    return self;
}

- (void)createBuffersWithDevice:(id<MTLDevice>)device {
    NSUInteger alignment = [device minimumLinearTextureAlignmentForPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = (bytesPerPixel * CANVAS_WIDTH + (alignment - 1)) & ~(alignment - 1);
    NSUInteger totalBytes = bytesPerRow * CANVAS_HEIGHT;

    buffers = [NSMutableArray arrayWithCapacity:bufferCount];
    textures = [NSMutableArray arrayWithCapacity:bufferCount];

    MTLTextureDescriptor* textureDescriptor = [self makeTextureDescriptor];

    for (int i = 0; i < bufferCount; ++i) {
        [self makeBufferWithDevice:device descriptor:textureDescriptor bytesPerRow:bytesPerRow totalBytes:totalBytes];
    }
}

- (MTLTextureDescriptor*)makeTextureDescriptor {
    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    [descriptor setWidth:CANVAS_WIDTH];
    [descriptor setHeight:CANVAS_HEIGHT];
    [descriptor setPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    [descriptor setTextureType:MTLTextureType2D];
    [descriptor setStorageMode:MTLStorageModeShared];
    [descriptor setUsage:MTLTextureUsageShaderRead];
    return descriptor;
}

- (void)makeBufferWithDevice:(id<MTLDevice>)device
                  descriptor:(MTLTextureDescriptor*)descriptor
                 bytesPerRow:(NSUInteger)bytesPerRow
                  totalBytes:(NSUInteger)totalBytes {
    id<MTLBuffer> buffer = [device newBufferWithLength:totalBytes options:MTLResourceStorageModeShared];
    if (!buffer) {
        NSLog(@"Failed to create canvas buffer");
        return;
    }

    id<MTLTexture> texture = [buffer newTextureWithDescriptor:descriptor offset:0 bytesPerRow:bytesPerRow];
    if (!texture) {
        NSLog(@"Error creating canvas texture");
        return;
    }

    [buffers addObject:buffer];
    [textures addObject:texture];
}

- (nonnull id<MTLTexture>)drawFrameWithCommandBuffer:(nonnull id<MTLCommandBuffer>)commandBuffer {
    dispatch_semaphore_wait(frameSemaphore, DISPATCH_TIME_FOREVER);

    __block dispatch_semaphore_t semaphore = frameSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer>) {
      dispatch_semaphore_signal(semaphore);
    }];

    id<MTLBuffer> buffer = buffers[frameIndex];
    id<MTLTexture> texture = textures[frameIndex];

    frameIndex = (frameIndex + 1) % bufferCount;

    [RCBridge setCanvasBytes:[buffer contents]];
    [RCBridge drawWorld];

    return texture;
}

@end
