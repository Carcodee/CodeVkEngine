#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangents;
layout (location = 3) in vec2 uv;
layout (location = 4) in int mesh_id;

layout (location = 0) out vec2 text_coord;
layout (location = 1) out vec3 norm;
layout (location = 2) out vec3 tang;
layout (location = 3) out int id;
layout (location = 4) out vec2 screen_uv;

layout(push_constant)uniform Push_Constants{
    mat4 model;
    mat4 proj_view;
}pc;
layout (set = 0, binding = 3) readonly buffer Meshes_Model_Matrices{
    mat4[] model_matrices;
};


void main() {
    mat4 model = model_matrices[mesh_id];
    gl_Position = pc.proj_view * model * vec4(pos, 1.0f);
    gl_PointSize = 1.0;
    norm = normal;
    tang = tangents;
    text_coord = uv;
    id = mesh_id;
    screen_uv = gl_Position.xy / gl_Position.w * 0.5 + 0.5;
    
}