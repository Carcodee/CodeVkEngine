struct VSInput {
    float2 position : POSITION;    // Input vertex position (clip-space)
    float2 uv : TEXCOORD;          // Input UV coordinates
};

struct VSOutput {
    float4 position : SV_Position; // Clip-space position (required by Vulkan)
    float2 uv : TEXCOORD;          // UV passed to fragment shader
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = float4(input.position, 0.0, 1.0); // Clip-space position
    output.uv = input.uv;                               // Pass UV
    return output;
}

struct PSInput {
    float2 uv : TEXCOORD;          // Interpolated UV coordinates
};

struct PSOutput {
    float4 color : SV_Target;      // Fragment output
};

// Descriptor set binding for Vulkan
// Binding set = 0, binding = 0 for texture, binding = 1 for sampler
Texture2D<float4> myTexture : register(t0, space0); // Texture at set 0, binding 0
SamplerState samplerState : register(s0, space0);   // Sampler at set 0, binding 1

PSOutput main(PSInput input) {
    PSOutput output;
    output.color = myTexture.Sample(samplerState, input.uv); // Sample the texture
    return output;
}