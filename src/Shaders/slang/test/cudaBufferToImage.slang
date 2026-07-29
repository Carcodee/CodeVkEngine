// Declare textures for painting layers
[[vk::binding(0, 0)]]
RWTexture2D<float4> OutImage;

[[vk::binding(2, 0)]]
RWStructuredBuffer <float> SimulationBuffer;
float3 ColorMapSpeed(float speed)
{
    // Based on your observed velocity:
    // average magnitude roughly 0.08 to 0.10
    // typical high speed roughly 0.20
    // extreme speed roughly 0.40
    const float maxVisualSpeed = 0.50f;

    float t = saturate(speed / maxVisualSpeed);

    // Expands low and medium speeds while still preserving high-speed detail.
    t = pow(t, 0.55f);

    // Black-blue -> blue -> cyan -> green -> yellow -> orange -> red
    if (t < 0.12f)
    {
        return lerp(
            float3(0.005f, 0.008f, 0.025f),
            float3(0.02f, 0.08f, 0.35f),
            t / 0.12f
        );
    }

    if (t < 0.28f)
    {
        return lerp(
            float3(0.02f, 0.08f, 0.35f),
            float3(0.00f, 0.45f, 1.00f),
            (t - 0.12f) / 0.16f
        );
    }

    if (t < 0.45f)
    {
        return lerp(
            float3(0.00f, 0.45f, 1.00f),
            float3(0.00f, 0.95f, 0.85f),
            (t - 0.28f) / 0.17f
        );
    }

    if (t < 0.62f)
    {
        return lerp(
            float3(0.00f, 0.95f, 0.85f),
            float3(0.20f, 1.00f, 0.15f),
            (t - 0.45f) / 0.17f
        );
    }

    if (t < 0.78f)
    {
        return lerp(
            float3(0.20f, 1.00f, 0.15f),
            float3(1.00f, 0.95f, 0.00f),
            (t - 0.62f) / 0.16f
        );
    }

    if (t < 0.90f)
    {
        return lerp(
            float3(1.00f, 0.95f, 0.00f),
            float3(1.00f, 0.35f, 0.00f),
            (t - 0.78f) / 0.12f
        );
    }

    return lerp(
        float3(1.00f, 0.35f, 0.00f),
        float3(0.75f, 0.00f, 0.02f),
        (t - 0.90f) / 0.10f
);
}
// Entry point for compute shader
[numthreads(1, 1, 1)]
void mainCS(uint3 dtid : SV_DispatchThreadID) {
  // Calculate position

  uint x = dtid.x;
  uint y = dtid.y;
  uint pixelIndex = y * 1024 + x;
  uint baseIndex = pixelIndex * 3;

  float3 value = float3(
      SimulationBuffer[baseIndex + 0],
      SimulationBuffer[baseIndex + 1],
      SimulationBuffer[baseIndex + 2]
  );

  float2 uv = float2(float(x)/1023.0f, float(y)/1023.0f);
  //float rescalled_v = (value - 0.05f) / (0.8f- 0.15f);
  float3 valueInterp = ColorMapSpeed(length(value));
  /*
  float max_col = max(valueInterp.x, (max(valueInterp.y, valueInterp.z)));


  bool show = false;
  if (value < 0.1 && value > 0.005) {
    show = true;
  }
  if (value > 0.25 && value < 0.8) {
    show = true;
  }
  
  if (value > 35 && value < 0.7) {
    show = false;
  }
  */


//  float alpha = show ? 1.0 : 0.0;

  uint y_write = 1024 - y;
  OutImage[uint2(x, y_write)] = float4(value, 1.0);
}