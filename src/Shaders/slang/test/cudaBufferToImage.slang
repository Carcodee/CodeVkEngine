// Declare textures for painting layers
[[vk::binding(0, 0)]]
RWTexture2D<float4> OutImage;

[[vk::binding(2, 0)]]
RWStructuredBuffer <float> SimulationBuffer;
float3 ColorMap(float t)
{
    t = saturate(t);

    // Dark blue -> cyan -> green -> yellow -> red
    if (t < 0.25f)
    {
        return lerp(
            float3(0.0f, 0.0f, 0.25f),
            float3(0.0f, 0.8f, 1.0f),
            t / 0.25f
        );
    }

    if (t < 0.5f)
    {
        return lerp(
            float3(0.0f, 0.8f, 1.0f),
            float3(0.0f, 1.0f, 0.2f),
            (t - 0.25f) / 0.25f
        );
    }

    if (t < 0.75f)
    {
        return lerp(
            float3(0.0f, 1.0f, 0.2f),
            float3(1.0f, 1.0f, 0.0f),
            (t - 0.5f) / 0.25f
        );
    }

    return lerp(
        float3(1.0f, 1.0f, 0.0f),
        float3(1.0f, 0.0f, 0.0f),
        (t - 0.75f) / 0.25f
    );
}
// Entry point for compute shader
[numthreads(1, 1, 1)]
void mainCS(uint3 dtid : SV_DispatchThreadID) {
  // Calculate position

  uint x = dtid.x;
  uint y = dtid.y;
  float value = SimulationBuffer[y * 1024 + x];
  float rescalled_v = (value - 0.05f) / (0.8f- 0.15f);
  float3 valueInterp = ColorMap(value);
  OutImage[uint2(x, y)] = float4(valueInterp.xyz, value);
}