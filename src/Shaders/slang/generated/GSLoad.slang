struct push_constants {
    float4x4 model;
    float4x4 proj_view;
};

struct arrays_of_gaussians
{
		float3x3 covariance_mats;
		float3 positions;
		float3 cols;
		float alphas;
};

struct buffer_wrapper<t>{
    t data[];
};

struct matrix{
    float3x3 data[];
};
struct pos_data{
    float3 data[];
};


[vk::push_constant]
cbuffer push_constants_buffer{
    push_constants pc;
};

// [[vk::binding(0, 0)]]
RWStructuredBuffer<float3x3> gs_mats;

// [[vk::binding(0, 1)]]
RWStructuredBuffer<float> gs_pos;
// 
// [[vk::binding(0, 2)]]
// GLSLShaderStorageBuffer<buffer_wrapper<float3>> gs_cols;
// 
// [[vk::binding(0, 3)]]
// GLSLShaderStorageBuffer<buffer_wrapper<float>> gs_alphas;


struct vs_input {
    int id : ID;    // gaussean id 
};

struct vs_output {
    float4 position : SV_Position; // Clip-space position (required by Vulkan)
    float point_size : SV_PointSize;
};

vs_output main_vs(vs_input input) {
    vs_output output;
    float3 pos = gs_pos[input.id];
    float3x3 mat = gs_mats[input.id];
    
    float4 world_pos = mul(pc.model, float4(pos, 1.0));
    float4 clip_pos = mul(pc.proj_view, world_pos);
    
    output.position = clip_pos;
    output.point_size = 4.0;
    return output;
}

struct ps_input {
};

struct ps_output {
    float4 color : SV_Target;      // Fragment output
};


// Descriptor set binding for Vulkan
// Binding set = 0, binding = 0 for texture, binding = 1 for sampler

ps_output main_fs(ps_input input) {
    ps_output output;
    output.color = float4(1.0); // Sample the texture
    return output;
}