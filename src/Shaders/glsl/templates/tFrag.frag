#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable

layout(location = 0) in vec2 frag_uv;          // Input UV coordinates from vertex shader
layout(location = 0) out vec4 out_color;       // Output color

void main() {
    out_color = vec4(frag_uv, 0.0, 1.0);
}