﻿// BasicComputeShader.slang

[[vk::binding(0, 0)]]
Sampler2D InTex;

[[vk::binding(1, 0)]]
RWTexture2D<float4> OutTex;

[shader("compute")]
[numthreads(16, 16, 1)]
void mainCS(uint2 dtid : SV_DispatchThreadID)
{
    OutTex[dtid.xy] = InTex[dtid.xy];
}
