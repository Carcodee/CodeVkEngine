// Declare textures for painting layers
[[vk::binding(0, 0)]]
RWTexture2D<float4> OutImage;

[[vk::binding(2, 0)]]
RWStructuredBuffer <float> SimulationBuffer;

// Entry point for compute shader
[numthreads(1, 1, 1)]
void mainCS(uint3 dtid : SV_DispatchThreadID) {
  // Calculate position

  uint x = dtid.x;
  uint y = dtid.y;
  float value = SimulationBuffer[y * 1024 + x];
  OutImage[uint2(x, y)] = float4(value, value, value, value);
  
}