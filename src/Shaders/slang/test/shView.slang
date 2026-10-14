struct Vs_Input {
    float2 position : POSITION;    // Input vertex position (clip-space)
    float2 uv : TEXCOORD;          // Input UV coordinates
};

struct Vs_Output {
    float4 position : SV_Position; // Clip-space position (required by Vulkan)
    float2 uv : TEXCOORD;          // UV passed to fragment shader
};

Vs_Output Main_Vs(Vs_Input input) {
    Vs_Output output;
    output.position = float4(input.position, 0.0, 1.0); // Clip-space position
    output.uv = input.uv;                               // Pass UV
    return output;
}

struct Ps_Input {
    float2 uv : TEXCOORD;          // Interpolated UV coordinates
};

struct Ps_Output {
    float4 color : SV_Target;      // Fragment output
};

// Descriptor set binding for Vulkan
// Binding set = 0, binding = 0 for texture, binding = 1 for sampler

[[vk::binding(0, 0)]]
Sampler2D my_texture; // Texture at set 0, binding 0

Ps_Output Main_Fs(Ps_Input input) {
    Ps_Output output;
    output.color = float4(input.uv, 0.0, 1.0); // Sample the texture
    return output;
}