#version 450

layout (location = 0) in vec3 pos;

layout(push_constant)uniform pushConstants{
    mat4 projView;
    vec3 origin;
    uvec3 dimensions;
    float cellSize;
}pc;

void main() {

    mat4 modelPos = mat4(1.0);
    modelPos[3][0] = gl_InstanceIndex;
    modelPos[3][1] = gl_InstanceIndex;
    modelPos[3][2] = gl_InstanceIndex;
    gl_Position = pc.projView * modelPos * vec4(pos, 1.0f);
}