struct Push_Constants {
    float4x4 model;
    float4x4 proj_view;
};

struct Arrays_Of_Gaussians
{
		float3x3 covariance_mats;
		float3 positions;
		float3 cols;
		float alphas;
};

struct Buffer_Wrapper<T>{
    T data[];
};

struct Matrix{
    float3x3 data[];
};
struct Pos_Data{
    float3 data[];
};


[vk::push_constant]
cbuffer Push_Constants_Buffer{
    Push_Constants pc;
};

// [[vk::binding(0, 0)]]
RWStructuredBuffer<float3x3> gs_mats;

// [[vk::binding(0, 1)]]
RWStructuredBuffer<float> gs_pos;
// 
// [[vk::binding(0, 2)]]
// GLSLShaderStorageBuffer<Buffer_Wrapper<float3>> gs_cols;
// 
// [[vk::binding(0, 3)]]
// GLSLShaderStorageBuffer<Buffer_Wrapper<float>> gs_alphas;


struct Vs_Input {
    int id : ID;    // gaussean id 
};

struct Vs_Output {
    float4 position : SV_Position; // Clip-space position (required by Vulkan)
    float point_size : SV_PointSize;
};

Vs_Output Main_Vs(Vs_Input input) {
    Vs_Output output;
    float3 pos = gs_pos[input.id];
    float3x3 mat = gs_mats[input.id];
    
    float4 world_pos = mul(pc.model, float4(pos, 1.0));
    float4 clip_pos = mul(pc.proj_view, world_pos);
    
    output.position = clip_pos;
    output.point_size = 4.0;
    return output;
}

struct Ps_Input {
};

struct Ps_Output {
    float4 color : SV_Target;      // Fragment output
};


// Descriptor set binding for Vulkan
// Binding set = 0, binding = 0 for texture, binding = 1 for sampler

Ps_Output Main_Fs(Ps_Input input) {
    Ps_Output output;
    output.color = float4(1.0); // Sample the texture
    return output;
}