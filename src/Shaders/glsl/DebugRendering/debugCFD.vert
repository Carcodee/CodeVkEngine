#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable

#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"
#include "../Utils/uRendering.glsl"

layout (location = 0) in vec3 pos;

layout (location = 0) out uint out_count;

layout(push_constant)uniform Push_Constants{
    mat4 proj_view;
    vec3 origin;
    uvec3 dimensions;
    float cell_size;
}pc;

layout (set = 0, binding = 0, scalar) buffer Voxels{
    uint cells[];
};

void main() {
    mat4 model = U_Grid_3d_Pos_From_1d(pc.cell_size, pc.dimensions, pc.origin, gl_InstanceIndex);
    gl_Position = pc.proj_view * model * vec4(pos, 1.0f);
    out_count = cells[gl_InstanceIndex];
    
}