struct vs_input {
    float2 position : POSITION;    // Input vertex position (clip-space)
    float2 uv : TEXCOORD;          // Input UV coordinates
};

struct vs_output {
    float4 position : SV_Position; // Clip-space position (required by Vulkan)
    float2 uv : TEXCOORD;          // UV passed to fragment shader
};

vs_output main_vs(vs_input input) {
    vs_output output;
    output.position = float4(input.position, 0.0, 1.0); // Clip-space position
    output.uv = input.uv;                               // Pass UV
    return output;
}

struct ps_input {
    float2 uv : TEXCOORD;          // Interpolated UV coordinates
};

struct ps_output {
    float4 color : SV_Target;      // Fragment output
};

// Descriptor set binding for Vulkan
// Binding set = 0, binding = 0 for texture, binding = 1 for sampler

[[vk::binding(0, 0)]]
Sampler2D my_texture; // Texture at set 0, binding 0

ps_output main_fs(ps_input input) {
    ps_output output;
    output.color = float4(input.uv, 0.0, 1.0); // Sample the texture
    return output;
}