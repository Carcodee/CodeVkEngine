#version 450
#extension GL_EXT_scalar_block_layout : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tang;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec2 text_coord;

layout(push_constant)uniform push_constants{
    mat4 model;
    mat4 proj_view;
}pc;

struct camera_ssbo{
    mat4 model;
    mat4 proj_view;
};

layout(set = 0, binding = 5, scalar) buffer camera_buffer{
    camera_ssbo [] cameras_ssbo;
};

void main() {
    gl_Position = pc.proj_view * pc.model * vec4(pos, 1.0f);
    text_coord = uv;
}