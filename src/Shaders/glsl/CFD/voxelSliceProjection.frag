#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"

layout(location = 0) out vec4 out_color;
layout(location = 0) in vec2 text_coord;

layout(push_constant)uniform Push_Constants{
    uvec3 dimensions;
    uvec3 plane_selected;
    uint selected_dimension_offset;
    float cell_size;
}pc;

layout (set = 0, binding = 0, scalar) buffer Voxels{
    uint cells[];
};


void main() {

    vec2 uv = text_coord;
    uvec3 voxel_pos = uvec3(0, uv.y * pc.dimensions.y, uv.x * pc.dimensions.z);
    //this indexing is wrong to the plane selected
    uint idx = U_Grid_To_Linear_Idx(pc.dimensions, voxel_pos);
    uint cell_value = cells[idx];
    if(cell_value > 0){
        out_color = vec4(1.0, 0.0, 0.0, 1.0);
    }else{
        out_color = vec4(0.0, 0.0, 0.0, 1.0);
    }
    out_color = vec4(float(idx)/18.0f, 0.0, 0.0, 1.0);

}