#include <metal_stdlib>

// Including header shared between this Metal shader code and Swift/C code executing Metal API commands
#import "ShaderTypes.h"

using namespace metal;

typedef struct {
    float3 position [[attribute(VertexAttributePosition)]];
    float2 texCoord [[attribute(VertexAttributeTexcoord)]];
} Vertex;

typedef struct {
    float4 position [[position]];
    float2 texCoord;
} ColorInOut;

vertex ColorInOut vertexShader(Vertex in [[stage_in]],
                               constant Uniforms & uniforms [[ buffer(BufferIndexUniforms) ]]) {
    ColorInOut out;

    float4 position = float4(in.position, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;
    out.texCoord = in.texCoord;

    return out;
}

fragment float4 fragmentShader(ColorInOut in [[stage_in]],
                               constant Uniforms & uniforms [[ buffer(BufferIndexUniforms) ]],
                               texture2d<half> colorMap     [[ texture(TextureIndexColor) ]]) {
    constexpr sampler colorSampler(mip_filter::nearest,
                                   mag_filter::nearest,
                                   min_filter::nearest);

    half4 colorSample  = colorMap.sample(colorSampler, in.texCoord.xy);

    return float4(colorSample);
}

kernel void transformTexture(texture2d<float, access::read>  sourceTexture  [[texture(0)]],
                             texture2d<float, access::write> destinationTexture [[texture(1)]],
                             uint2 gid [[thread_position_in_grid]]) {
    if (gid.x >= destinationTexture.get_width() || gid.y >= destinationTexture.get_height()) {
        return;
    }
    uint2 readCoord = uint2(gid.x, sourceTexture.get_height() - gid.y - 1);
    float4 inColor = sourceTexture.read(readCoord);
    destinationTexture.write(inColor, gid);
}
