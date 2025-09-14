//
//  Renderer.m
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 30.08.25.
//

#import <ModelIO/ModelIO.h>
#import <os/log.h>
#import <os/signpost.h>
#import <simd/simd.h>

#import "Config.h"
#import "Painter.h"
#import "Renderer.h"
#import "ShaderTypes.h"
#import "Text.h"

#define BYTES_PER_PIXEL 4

static const NSUInteger kMaxBuffersInFlight = 3;

static const size_t kAlignedUniformsSize = (sizeof(Uniforms) & ~0xFF) + 0x100;

static os_log_t rendererLog;

@implementation Renderer {
    dispatch_semaphore_t _inFlightSemaphore;
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLBuffer> _dynamicUniformBuffer;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLDepthStencilState> _depthState;
    id<MTLTexture> _canvasTexture;
    MTLVertexDescriptor* _mtlVertexDescriptor;

    uint32_t _uniformBufferOffset;
    uint8_t _uniformBufferIndex;
    void* _uniformBufferAddress;
    matrix_float4x4 _projectionMatrix;
    MTKMesh* _mesh;
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

    id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];

    id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];

    id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];

    MTLRenderPipelineDescriptor* pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"MyPipeline";
    pipelineStateDescriptor.rasterSampleCount = view.sampleCount;
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.vertexDescriptor = _mtlVertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;

    NSError* error = NULL;
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState) {
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
}

- (void)_loadAssets {
    /// Canvas Mesh

    NSError* error;
    MTKMeshBufferAllocator* metalAllocator = [[MTKMeshBufferAllocator alloc] initWithDevice:_device];

    MDLMesh* mdlMesh = [MDLMesh newPlaneWithDimensions:(vector_float2){2.f * CANVAS_WIDTH / CANVAS_HEIGHT, 2}
                                              segments:(vector_uint2){2, 2}
                                          geometryType:MDLGeometryTypeTriangles
                                             allocator:metalAllocator];

    MDLVertexDescriptor* mdlVertexDescriptor = MTKModelIOVertexDescriptorFromMetal(_mtlVertexDescriptor);
    mdlVertexDescriptor.attributes[VertexAttributePosition].name = MDLVertexAttributePosition;
    mdlVertexDescriptor.attributes[VertexAttributeTexcoord].name = MDLVertexAttributeTextureCoordinate;
    mdlMesh.vertexDescriptor = mdlVertexDescriptor;
    _mesh = [[MTKMesh alloc] initWithMesh:mdlMesh device:_device error:&error];

    if (!_mesh || error) {
        NSLog(@"Error creating mesh: %@", error.localizedDescription);
        error = NULL;
    }

    /// Canvas Texture

    _canvasTexture = [self _createTexture];

    if (!_canvasTexture || error) {
        NSLog(@"Error creating canvas texture: %@", error.localizedDescription);
        error = NULL;
    }

    // Font Texture

    id<MTLTexture> fontTexture = [self _loadTexture:@"FontSFMono" error:&error];

    if (!fontTexture || error) {
        NSLog(@"Error loading font texture: %@", error.localizedDescription);
        error = NULL;
    }

    void* fontData = [Text fontData];
    [fontTexture getBytes:fontData
              bytesPerRow:BYTES_PER_PIXEL * fontTexture.width
               fromRegion:MTLRegionMake2D(0, 0, fontTexture.width, fontTexture.height)
              mipmapLevel:0];
    [self _flipVertically:fontData width:fontTexture.width height:fontTexture.height];
}

- (nullable id<MTLTexture>)_createTexture {
    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    [descriptor setWidth:CANVAS_WIDTH];
    [descriptor setHeight:CANVAS_HEIGHT];
    [descriptor setPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    [descriptor setTextureType:MTLTextureType2D];
    [descriptor setStorageMode:MTLStorageModeShared];
    [descriptor setUsage:MTLTextureUsageShaderRead];
    return [_device newTextureWithDescriptor:descriptor];
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

- (void)_flipVertically:(void*)buffer width:(NSUInteger)width height:(NSUInteger)height {
    if (!buffer || width == 0 || height == 0) {
        return;
    }

    NSUInteger bytesPerRow = BYTES_PER_PIXEL * width;
    uint8_t tempRow[bytesPerRow];
    uint8_t* byteBuffer = (uint8_t*)buffer;

    for (NSUInteger i = 0; i < height / 2; ++i) {
        NSUInteger j = height - 1 - i;
        uint8_t* topRow = byteBuffer + (i * bytesPerRow);
        uint8_t* bottomRow = byteBuffer + (j * bytesPerRow);

        memcpy(tempRow, topRow, bytesPerRow);
        memcpy(topRow, bottomRow, bytesPerRow);
        memcpy(bottomRow, tempRow, bytesPerRow);
    }
}

- (void)drawInMTKView:(nonnull MTKView*)view {
    /// Per frame updates here

    os_signpost_id_t signpostID = os_signpost_id_generate(rendererLog);
    os_signpost_interval_begin(rendererLog, signpostID, "Renderer::draw", "Started");

    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MyCommand";

    __block dispatch_semaphore_t block_sema = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
      dispatch_semaphore_signal(block_sema);
    }];

    [self _updateDynamicBufferState];

    [self _updateGameState];

    /// Delay getting the currentRenderPassDescriptor until we absolutely need it to avoid
    ///   holding onto the drawable and blocking the display pipeline any longer than necessary
    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;

    if (renderPassDescriptor != nil) {

        /// Final pass rendering code here

        id<MTLRenderCommandEncoder> renderEncoder =
            [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"MyRenderEncoder";

        [renderEncoder pushDebugGroup:@"DrawBox"];

        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [renderEncoder setCullMode:MTLCullModeBack];
        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder setDepthStencilState:_depthState];

        [renderEncoder setVertexBuffer:_dynamicUniformBuffer
                                offset:_uniformBufferOffset
                               atIndex:BufferIndexUniforms];

        [renderEncoder setFragmentBuffer:_dynamicUniformBuffer
                                  offset:_uniformBufferOffset
                                 atIndex:BufferIndexUniforms];

        for (NSUInteger bufferIndex = 0; bufferIndex < _mesh.vertexBuffers.count; bufferIndex++) {
            MTKMeshBuffer* vertexBuffer = _mesh.vertexBuffers[bufferIndex];
            if ((NSNull*)vertexBuffer != [NSNull null]) {
                [renderEncoder setVertexBuffer:vertexBuffer.buffer
                                        offset:vertexBuffer.offset
                                       atIndex:bufferIndex];
            }
        }

        [renderEncoder setFragmentTexture:_canvasTexture
                                  atIndex:TextureIndexColor];

        for (MTKSubmesh* submesh in _mesh.submeshes) {
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
    /// Update the state of our uniform buffers before rendering

    _uniformBufferIndex = (_uniformBufferIndex + 1) % kMaxBuffersInFlight;

    _uniformBufferOffset = kAlignedUniformsSize * _uniformBufferIndex;

    _uniformBufferAddress = ((uint8_t*)_dynamicUniformBuffer.contents) + _uniformBufferOffset;
}

- (void)_updateGameState {
    /// Update any game state before encoding rendering commands to our drawable
    [self _placeMesh];
    [self _updateTexture];
}

- (void)_placeMesh {
    Uniforms* uniforms = (Uniforms*)_uniformBufferAddress;

    uniforms->projectionMatrix = _projectionMatrix;

    vector_float3 rotationAxis = {1, 0, 0};
    matrix_float4x4 modelMatrix = matrix4x4_rotation(M_PI_2, rotationAxis);
    matrix_float4x4 viewMatrix = matrix4x4_translation(0, 0, -1.f / tanf(M_PI_4));

    uniforms->modelViewMatrix = matrix_multiply(viewMatrix, modelMatrix);
}

- (void)_updateTexture {
    const void* frameData = [Painter drawFrame];
    [_canvasTexture replaceRegion:MTLRegionMake2D(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT)
                      mipmapLevel:0
                        withBytes:frameData
                      bytesPerRow:BYTES_PER_PIXEL * CANVAS_WIDTH];
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
    /// Respond to drawable size or orientation changes here

    float aspect = size.width / (float)size.height;
    _projectionMatrix = matrix_perspective_right_hand(M_PI_2, aspect, 0.1f, 100.0f);
}

#pragma mark Matrix Math Utilities

matrix_float4x4 matrix4x4_translation(float tx, float ty, float tz) {
    return (matrix_float4x4){{{1, 0, 0, 0},
                              {0, 1, 0, 0},
                              {0, 0, 1, 0},
                              {tx, ty, tz, 1}}};
}

static matrix_float4x4 matrix4x4_rotation(float radians, vector_float3 axis) {
    axis = vector_normalize(axis);
    float ct = cosf(radians);
    float st = sinf(radians);
    float ci = 1 - ct;
    float x = axis.x, y = axis.y, z = axis.z;

    return (matrix_float4x4){{{ct + x * x * ci, y * x * ci + z * st, z * x * ci - y * st, 0},
                              {x * y * ci - z * st, ct + y * y * ci, z * y * ci + x * st, 0},
                              {x * z * ci + y * st, y * z * ci - x * st, ct + z * z * ci, 0},
                              {0, 0, 0, 1}}};
}

matrix_float4x4 matrix_perspective_right_hand(float fovyRadians, float aspect, float nearZ, float farZ) {
    float ys = 1 / tanf(fovyRadians * 0.5);
    float xs = ys / aspect;
    float zs = farZ / (nearZ - farZ);

    return (matrix_float4x4){{{xs, 0, 0, 0},
                              {0, ys, 0, 0},
                              {0, 0, zs, -1},
                              {0, 0, nearZ * zs, 0}}};
}

@end
