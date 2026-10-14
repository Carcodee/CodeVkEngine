// Declare textures for painting layers
[[vk::binding(0, 0)]]
RWTexture2D<float4> out_image;

[[vk::binding(1, 0)]]
RWTexture2D<float4> out_image_simulation_info;

[[vk::binding(2, 0)]]
RWStructuredBuffer <float> simulation_buffer;

[[vk::binding(3, 0)]]
RWStructuredBuffer <float> simulation_buffer_info;
float3 color_map_speed(float speed)
{
    // Based on your observed velocity:
    // average magnitude roughly 0.08 to 0.10
    // typical high speed roughly 0.20
    // extreme speed roughly 0.40
    const float max_visual_speed = 0.50f;

    float t = saturate(speed / max_visual_speed);

    // Expands low and medium speeds while still preserving high-speed detail.
    t = pow(t, 0.85f);

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
[numthreads(8, 8, 1)]
void main_cs(uint3 dtid : SV_DispatchThreadID) {
  // Calculate position

  uint x = dtid.x;
  uint y = dtid.y;
  uint pixel_index = y * 1024 + x;
  uint base_index = pixel_index * 4;

  float4 value = float4(
      simulation_buffer[base_index + 0],
      simulation_buffer[base_index + 1],
      simulation_buffer[base_index + 2],
      simulation_buffer[base_index + 3],
  );
  
  float value_vel = float(
      simulation_buffer_info[pixel_index],
  );


  float2 uv = float2(float(x)/1023.0f, float(y)/1023.0f);
  //float rescalled_v = (value - 0.05f) / (0.8f- 0.15f);
  float3 value_interp = color_map_speed(value_vel);
  float max_col = max(value_interp.x, (max(value_interp.y, value_interp.z)));

  bool show = false;
  if(length(value) <= 0.00001) {
    show = true;
  }
  if (length(value) < 0.1 && length(value) > 0.05) {
    show = true;
  }
  if (length(value) > 0.25 && length(value) < 0.8) {
    show = true;
  }
  
  if (length(value) > 35 && length(value) < 0.7) {
    show = false;
  }

  float alpha = show ? 1.0 : 0.0;
  float density = max(value.r, max(value.g, value.b));
  
  

    // Much softer smoke opacity
  alpha = 1.0f - exp(-density * 0.25f);

  if (density < 0) {
 //     value_interp = 1.0;
 //     value = float4(0.0, 0.1, 0.0, 1.0);
//      alpha = 1.0;
  };
  // Optional cap so it never becomes completely opaque
  
  alpha = min(alpha, 0.45f);
  out_image[uint2(x, 1024 - y)] = float4(value.xyz, value.w);
  out_image_simulation_info[uint2(x, 1024 - y)] = float4(value_interp.xyz,1.0);
 // out_image[uint2(x, 1024 - y)] = float4(value_interp.xyz * value.w,1.0);
 // out_image[uint2(x, y_write)] = float4(value, alpha);
}