#version 450

layout (location = 0) in vec3 pos;

layout(push_constant)uniform Push_Constants{
    mat4 model;
    mat4 proj_view;
}pc;

void main() {
    gl_Position = pc.proj_view * pc.model * vec4(pos, 1.0f);
}