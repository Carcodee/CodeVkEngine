#version 450

layout (location = 0) in vec3 pos;

layout(push_constant)uniform pushConstants{
    mat4 model;
    mat4 projView;
}pc;

void main() {
    gl_Position = pc.projView * pc.model * vec4(pos, 1.0f);
}