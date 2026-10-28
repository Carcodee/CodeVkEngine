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
    uvec3 voxel_pos = uvec3(int((uv.x) * pc.dimensions.x), int((1.0 - uv.y) * pc.dimensions.y), 5);
    //this indexing is wrong to the plane selected
    uint idx = U_Grid_To_Linear_Idx(pc.dimensions, voxel_pos);
    uint cell_value = cells[idx];

    if(cell_value > 0){
        out_color = vec4(cell_value, 0.0, 0.0, 1.0);
    }else{
        out_color = vec4(cell_value, 0.0, 0.0, 1.0) + float(idx)/88.0f;
    }
//    out_color = vec4(float(idx)/float(pc.dimensions.x * pc.dimensions.y * pc.dimensions.z), 0.0, 0.0, 1.0);
//    out_color = vec4(vec2(voxel_pos.xy)/vec2(pc.dimensions.xy), 0.0, 1.0);

}