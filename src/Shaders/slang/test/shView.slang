struct VSInput {
    float2 position : POSITION;    // Input vertex position (clip-space)
    float2 uv : TEXCOORD;          // Input UV coordinates
};

struct VSOutput {
    float4 position : SV_Position; // Clip-space position (required by Vulkan)
    float2 uv : TEXCOORD;          // UV passed to fragment shader
};

VSOutput mainVS(VSInput input) {
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

[[vk::binding(0, 0)]]
Sampler2D myTexture; // Texture at set 0, binding 0

PSOutput mainFS(PSInput input) {
    PSOutput output;
    output.color = myTexture.Sample(input.uv); // Sample the texture
    return output;
}