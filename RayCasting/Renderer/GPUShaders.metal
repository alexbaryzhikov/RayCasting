#include "GPUShaderTypes.h"
#include <metal_stdlib>
using namespace metal;

kernel void castRays(texture2d<float, access::write> outputTexture [[texture(0)]],
                     array<texture2d<float, access::sample>, TEXTURE_HEAP_SIZE> textures [[texture(1)]],
                     uint2 gid [[thread_position_in_grid]]) {
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    constexpr sampler textureSampler(coord::normalized, address::clamp_to_edge, filter::nearest);
    float2 readCoord = float2((float(gid.x) + 0.5) / float(textures[0].get_width()),
                              (float(gid.y) + 0.5) / float(textures[0].get_height()));
    float4 color = textures[0].sample(textureSampler, readCoord);
    outputTexture.write(color, gid);
}
