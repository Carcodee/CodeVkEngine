#version 450
#extension GL_EXT_scalar_block_layout : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tang;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec2 textCoord;

layout(push_constant)uniform pushConstants{
    mat4 model;
    mat4 projView;
}pc;

struct CameraSSBO{
    mat4 model;
    mat4 projView;
};

layout(set = 0, binding = 5, scalar) buffer CameraBuffer{
    CameraSSBO [] camerasSSBO;
};

void main() {
    gl_Position = pc.projView * pc.model * vec4(pos, 1.0f); 
    textCoord = uv;
}