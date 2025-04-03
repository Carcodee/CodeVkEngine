#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangents;
layout (location = 3) in vec2 uv;
layout (location = 4) in int meshId;

layout (location = 0) out vec2 textCoord;
layout (location = 1) out vec3 norm;
layout (location = 2) out vec3 tang;
layout (location = 3) out int id;
layout (location = 4) out vec2 screenUv;

layout(push_constant)uniform pushConstants{
    mat4 model;
    mat4 projView;
}pc;
layout (set = 0, binding = 3) readonly buffer MeshesModelMatrices{
    mat4[] modelMatrices;
};


void main() {
    mat4 model = modelMatrices[meshId];
    gl_Position = pc.projView * model * vec4(pos, 1.0f);
    gl_PointSize = 1.0;
    norm = normal;
    tang = tangents;
    textCoord = uv;
    id = meshId;
    screenUv = gl_Position.xy / gl_Position.w * 0.5 + 0.5;
    
}