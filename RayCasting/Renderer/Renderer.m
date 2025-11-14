#import <ModelIO/ModelIO.h>
#import <os/log.h>
#import <os/signpost.h>
#import <simd/simd.h>

#import "Renderer.h"

#import "Config.h"
#import "RCBridge.h"
#import "ShaderTypes.h"

#define BYTES_PER_PIXEL 4

static const NSUInteger kMaxBuffersInFlight = 3;

static const size_t kAlignedUniformsSize = (sizeof(Uniforms) & ~0xFF) + 0x100;

static os_log_t rendererLog;

@implementation Renderer {
    dispatch_semaphore_t _inFlightSemaphore;
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLBuffer> _dynamicUniformBuffer;
    id<MTLRenderPipelineState> _renderPipelineState;
    id<MTLDepthStencilState> _depthState;
    id<MTLTexture> _canvasTexture;
    MTLVertexDescriptor* _mtlVertexDescriptor;
    id<MTLComputePipelineState> _computePipelineState;

    uint32_t _uniformBufferOffset;
    uint8_t _uniformBufferIndex;
    void* _uniformBufferAddress;
    simd_float4x4 _projectionMatrix;
    MTKMesh* _canvasMesh;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)view {
    self = [super init];
    if (self) {
        rendererLog = os_log_create("com.alexb.RayCasting", "PointsOfInterest");
        _inFlightSemaphore = dispatch_semaphore_create(kMaxBuffersInFlight);
        _device = view.device;
        [self _loadMetalWithView:view];
        [self _loadAssets];
    }
    return self;
}

- (void)_loadMetalWithView:(nonnull MTKView*)view {
    /// Load Metal state objects and initialize renderer dependent view properties

    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.sampleCount = 1;

    _mtlVertexDescriptor = [[MTLVertexDescriptor alloc] init];

    _mtlVertexDescriptor.attributes[VertexAttributePosition].format = MTLVertexFormatFloat3;
    _mtlVertexDescriptor.attributes[VertexAttributePosition].offset = 0;
    _mtlVertexDescriptor.attributes[VertexAttributePosition].bufferIndex = BufferIndexMeshPositions;

    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].format = MTLVertexFormatFloat2;
    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].offset = 0;
    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].bufferIndex = BufferIndexMeshGenerics;

    _mtlVertexDescriptor.layouts[BufferIndexMeshPositions].stride = 12;
    _mtlVertexDescriptor.layouts[BufferIndexMeshPositions].stepRate = 1;
    _mtlVertexDescriptor.layouts[BufferIndexMeshPositions].stepFunction = MTLVertexStepFunctionPerVertex;

    _mtlVertexDescriptor.layouts[BufferIndexMeshGenerics].stride = 8;
    _mtlVertexDescriptor.layouts[BufferIndexMeshGenerics].stepRate = 1;
    _mtlVertexDescriptor.layouts[BufferIndexMeshGenerics].stepFunction = MTLVertexStepFunctionPerVertex;

    id<MTLLibrary> library = [_device newDefaultLibrary];
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexShader"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"];

    MTLRenderPipelineDescriptor* pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"RCPipeline";
    pipelineStateDescriptor.rasterSampleCount = view.sampleCount;
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.vertexDescriptor = _mtlVertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;

    NSError* error = nil;
    _renderPipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_renderPipelineState) {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }

    MTLDepthStencilDescriptor* depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    depthStateDesc.depthWriteEnabled = YES;
    _depthState = [_device newDepthStencilStateWithDescriptor:depthStateDesc];

    NSUInteger uniformBufferSize = kAlignedUniformsSize * kMaxBuffersInFlight;

    _dynamicUniformBuffer = [_device newBufferWithLength:uniformBufferSize
                                                 options:MTLResourceStorageModeShared];

    _dynamicUniformBuffer.label = @"UniformBuffer";

    _commandQueue = [_device newCommandQueue];

    id<MTLFunction> kernelFunction = [library newFunctionWithName:@"transformTexture"];

    _computePipelineState = [_device newComputePipelineStateWithFunction:kernelFunction error:&error];
    if (!_computePipelineState) {
        NSLog(@"Failed to create compute pipeline state: %@", error);
    }
}

- (void)_loadAssets {
    /// Load textures and initialize world

    [self _createCanvas];

    [self _loadTexture:@(FONT_MAP) to:[RCBridge fontBytes]];
    [self _loadTexture:@"Ceiling" to:[RCBridge textureBytes_ceiling]];
    [self _loadTexture:@"Door" to:[RCBridge textureBytes_door]];
    [self _loadTexture:@"Floor" to:[RCBridge textureBytes_floor]];
    [self _loadTexture:@"Wall" to:[RCBridge textureBytes_wall]];
    [self _loadTexture:@"WallFortified" to:[RCBridge textureBytes_wallFortified]];
    [self _loadTexture:@"WallFortifiedDecal1" to:[RCBridge textureBytes_wallFortifiedDecal1]];
    [self _loadTexture:@"WallFortifiedDecal2" to:[RCBridge textureBytes_wallFortifiedDecal2]];
    [self _loadTexture:@"WallFortifiedTorch" to:[RCBridge textureBytes_wallFortifiedTorch]];
    [self _loadTexture:@"WallIndestructible" to:[RCBridge textureBytes_wallIndestructible]];

    if (MAP_LOAD) {
        [self _loadMap];
    } else {
        [RCBridge generateMap];
    }

    [RCBridge startWorld];
}

- (void)_createCanvas {
    MTKMeshBufferAllocator* metalAllocator = [[MTKMeshBufferAllocator alloc] initWithDevice:_device];

    MDLMesh* mdlMesh = [MDLMesh newPlaneWithDimensions:(simd_float2){2.f * CANVAS_WIDTH / CANVAS_HEIGHT, 2}
                                              segments:(simd_uint2){2, 2}
                                          geometryType:MDLGeometryTypeTriangles
                                             allocator:metalAllocator];

    MDLVertexDescriptor* mdlVertexDescriptor = MTKModelIOVertexDescriptorFromMetal(_mtlVertexDescriptor);
    mdlVertexDescriptor.attributes[VertexAttributePosition].name = MDLVertexAttributePosition;
    mdlVertexDescriptor.attributes[VertexAttributeTexcoord].name = MDLVertexAttributeTextureCoordinate;
    mdlMesh.vertexDescriptor = mdlVertexDescriptor;

    NSError* error;
    _canvasMesh = [[MTKMesh alloc] initWithMesh:mdlMesh device:_device error:&error];

    if (!_canvasMesh || error) {
        NSLog(@"Error creating canvas mesh: %@", error.localizedDescription);
        return;
    }

    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    [descriptor setWidth:CANVAS_WIDTH];
    [descriptor setHeight:CANVAS_HEIGHT];
    [descriptor setPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    [descriptor setTextureType:MTLTextureType2D];
    [descriptor setStorageMode:MTLStorageModeShared];
    [descriptor setUsage:MTLTextureUsageShaderRead];
    _canvasTexture = [_device newTextureWithDescriptor:descriptor];

    if (!_canvasTexture) {
        NSLog(@"Error creating canvas texture");
        return;
    }
}

- (void)_loadTexture:(nonnull NSString*)name to:(nonnull void*)bytes {
    NSError* error = nil;
    id<MTLTexture> texture = [self _loadTexture:name error:&error];
    if (!texture || error) {
        NSLog(@"Error loading texture '%@': %@", name, error.localizedDescription);
        return;
    }

    texture = [self createTransformedTextureFrom:texture];
    if (!texture || error) {
        NSLog(@"Error converting texture '%@': %@", name, error.localizedDescription);
        return;
    }

    [texture getBytes:bytes
          bytesPerRow:texture.width * BYTES_PER_PIXEL
           fromRegion:MTLRegionMake2D(0, 0, texture.width, texture.height)
          mipmapLevel:0];
}

- (nullable id<MTLTexture>)_loadTexture:(nonnull NSString*)name error:(NSError**)error {
    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:_device];
    NSDictionary<MTKTextureLoaderOption, id>* options = @{
        MTKTextureLoaderOptionTextureUsage : @(MTLTextureUsageUnknown),
        MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModeShared),
    };
    return [textureLoader newTextureWithName:name
                                 scaleFactor:1.0
                                      bundle:nil
                                     options:options
                                       error:error];
}

- (id<MTLTexture>)createTransformedTextureFrom:(id<MTLTexture>)sourceTexture {
    MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB
                                     width:sourceTexture.width
                                    height:sourceTexture.height
                                 mipmapped:NO];

    descriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;

    id<MTLTexture> destinationTexture = [_device newTextureWithDescriptor:descriptor];
    if (!destinationTexture) {
        NSLog(@"Failed to create destination texture");
        return nil;
    }

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLComputeCommandEncoder> computeCommandEncoder = [commandBuffer computeCommandEncoder];

    [computeCommandEncoder setComputePipelineState:_computePipelineState];
    [computeCommandEncoder setTexture:sourceTexture atIndex:0];
    [computeCommandEncoder setTexture:destinationTexture atIndex:1];

    NSUInteger w = _computePipelineState.threadExecutionWidth;
    NSUInteger h = _computePipelineState.maxTotalThreadsPerThreadgroup / w;
    MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
    MTLSize threadsPerGrid = MTLSizeMake(sourceTexture.width, sourceTexture.height, 1);

    [computeCommandEncoder dispatchThreads:threadsPerGrid
                     threadsPerThreadgroup:threadsPerThreadgroup];
    [computeCommandEncoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];

    return destinationTexture;
}

- (void)_loadMap {
    NSString* mapName = @"map001";
    NSString* mapType = @"map";
    NSString* mapPath = [[NSBundle mainBundle] pathForResource:mapName ofType:mapType];

    if (!mapPath) {
        NSLog(@"Map not found: %@.%@", mapName, mapType);
    }

    NSData* mapData = [NSData dataWithContentsOfFile:mapPath];

    if (!mapData) {
        NSLog(@"Error loading map data: %@", mapPath);
    }

    [RCBridge loadMap:[mapData bytes] size:[mapData length]];
}

#pragma mark MTRViewDelegate Methods

- (void)drawInMTKView:(nonnull MTKView*)view {
    /// Per frame updates

    os_signpost_id_t signpostID = os_signpost_id_generate(rendererLog);
    os_signpost_interval_begin(rendererLog, signpostID, "Renderer::draw", "Started");

    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"RCCommandBuffer";

    __block dispatch_semaphore_t block_sema = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
      dispatch_semaphore_signal(block_sema);
    }];

    [self _updateDynamicBufferState];
    [self _updateWorldState];

    /// Delay getting the currentRenderPassDescriptor until we absolutely need it to avoid
    /// holding onto the drawable and blocking the display pipeline any longer than necessary
    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;

    if (renderPassDescriptor != nil) {
        id<MTLRenderCommandEncoder> renderEncoder =
            [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"RCRenderEncoder";

        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [renderEncoder setCullMode:MTLCullModeBack];
        [renderEncoder setRenderPipelineState:_renderPipelineState];
        [renderEncoder setDepthStencilState:_depthState];

        [renderEncoder setVertexBuffer:_dynamicUniformBuffer
                                offset:_uniformBufferOffset
                               atIndex:BufferIndexUniforms];

        [renderEncoder setFragmentBuffer:_dynamicUniformBuffer
                                  offset:_uniformBufferOffset
                                 atIndex:BufferIndexUniforms];

        for (NSUInteger bufferIndex = 0; bufferIndex < _canvasMesh.vertexBuffers.count; bufferIndex++) {
            MTKMeshBuffer* vertexBuffer = _canvasMesh.vertexBuffers[bufferIndex];
            if ((NSNull*)vertexBuffer != [NSNull null]) {
                [renderEncoder setVertexBuffer:vertexBuffer.buffer
                                        offset:vertexBuffer.offset
                                       atIndex:bufferIndex];
            }
        }

        [renderEncoder setFragmentTexture:_canvasTexture
                                  atIndex:TextureIndexColor];

        for (MTKSubmesh* submesh in _canvasMesh.submeshes) {
            [renderEncoder drawIndexedPrimitives:submesh.primitiveType
                                      indexCount:submesh.indexCount
                                       indexType:submesh.indexType
                                     indexBuffer:submesh.indexBuffer.buffer
                               indexBufferOffset:submesh.indexBuffer.offset];
        }

        [renderEncoder popDebugGroup];

        [renderEncoder endEncoding];

        [commandBuffer presentDrawable:view.currentDrawable];
    }

    [commandBuffer commit];

    os_signpost_interval_end(rendererLog, signpostID, "Renderer::draw", "Finished");
}

- (void)_updateDynamicBufferState {
    _uniformBufferIndex = (_uniformBufferIndex + 1) % kMaxBuffersInFlight;
    _uniformBufferOffset = kAlignedUniformsSize * _uniformBufferIndex;
    _uniformBufferAddress = ((uint8_t*)_dynamicUniformBuffer.contents) + _uniformBufferOffset;
}

- (void)_updateWorldState {
    [RCBridge updateWorld];
    [RCBridge drawWorld];
    [self _placeMesh];
    [self _updateTexture];
}

- (void)_placeMesh {
    Uniforms* uniforms = (Uniforms*)_uniformBufferAddress;

    uniforms->projectionMatrix = _projectionMatrix;

    simd_float3 rotationAxis = {1, 0, 0};
    simd_float4x4 modelMatrix = matrix4x4_rotation(M_PI_2, rotationAxis);
    simd_float4x4 viewMatrix = matrix4x4_translation(0, 0, -1.f / tanf(M_PI_4));

    uniforms->modelViewMatrix = matrix_multiply(viewMatrix, modelMatrix);
}

- (void)_updateTexture {
    const void* canvasBytes = [RCBridge canvasBytes];
    [_canvasTexture replaceRegion:MTLRegionMake2D(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT)
                      mipmapLevel:0
                        withBytes:canvasBytes
                      bytesPerRow:BYTES_PER_PIXEL * CANVAS_WIDTH];
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
    /// Respond to drawable size or orientation changes

    float aspect = size.width / (float)size.height;
    _projectionMatrix = matrix_perspective_right_hand(M_PI_2, aspect, 0.1f, 100.0f);
}

#pragma mark Matrix Math Utilities

static simd_float4x4 matrix4x4_translation(float tx, float ty, float tz) {
    return (simd_float4x4){{{1, 0, 0, 0},
                            {0, 1, 0, 0},
                            {0, 0, 1, 0},
                            {tx, ty, tz, 1}}};
}

static simd_float4x4 matrix4x4_rotation(float radians, simd_float3 axis) {
    axis = simd_normalize(axis);
    float ct = cosf(radians);
    float st = sinf(radians);
    float ci = 1 - ct;
    float x = axis.x, y = axis.y, z = axis.z;

    return (simd_float4x4){{{ct + x * x * ci, y * x * ci + z * st, z * x * ci - y * st, 0},
                            {x * y * ci - z * st, ct + y * y * ci, z * y * ci + x * st, 0},
                            {x * z * ci + y * st, y * z * ci - x * st, ct + z * z * ci, 0},
                            {0, 0, 0, 1}}};
}

static simd_float4x4 matrix_perspective_right_hand(float fovyRadians, float aspect, float nearZ, float farZ) {
    float ys = 1 / tanf(fovyRadians * 0.5);
    float xs = ys / aspect;
    float zs = farZ / (nearZ - farZ);

    return (simd_float4x4){{{xs, 0, 0, 0},
                            {0, ys, 0, 0},
                            {0, 0, zs, -1},
                            {0, 0, nearZ * zs, 0}}};
}

@end
