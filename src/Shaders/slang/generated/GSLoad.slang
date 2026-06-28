struct PushConstants {
    float4x4 model;
    float4x4 projView;
};

struct ArraysOfGaussians
{
		float3x3 covarianceMats;
		float3 positions;
		float3 cols;
		float alphas;
};

struct BufferWrapper<T>{
    T data[];
};

struct Matrix{
    float3x3 data[];
};
struct Pos{
    float3 data[];
};


[vk::push_constant]
cbuffer PushConstantsBuffer{
    PushConstants pc;
};

// [[vk::binding(0, 0)]]
RWStructuredBuffer<float3x3> gsMats;

// [[vk::binding(0, 1)]]
RWStructuredBuffer<float> gsPos;
// 
// [[vk::binding(0, 2)]]
// GLSLShaderStorageBuffer<BufferWrapper<float3>> gsCols;
// 
// [[vk::binding(0, 3)]]
// GLSLShaderStorageBuffer<BufferWrapper<float>> gsAlphas;


struct VSInput {
    int id : ID;    // gaussean id 
};

struct VSOutput {
    float4 position : SV_Position; // Clip-space position (required by Vulkan)
    float pointSize : SV_PointSize;
};

VSOutput mainVS(VSInput input) {
    VSOutput output;
    float3 pos = gsPos[input.id];
    float3x3 mat = gsMats[input.id];
    
    float4 worldPos = mul(pc.model, float4(pos, 1.0));
    float4 clipPos = mul(pc.projView, worldPos);
    
    output.position = clipPos;
    output.pointSize = 4.0;
    return output;
}

struct PSInput {
};

struct PSOutput {
    float4 color : SV_Target;      // Fragment output
};


// Descriptor set binding for Vulkan
// Binding set = 0, binding = 0 for texture, binding = 1 for sampler

PSOutput mainFS(PSInput input) {
    PSOutput output;
    output.color = float4(1.0); // Sample the texture
    return output;
}