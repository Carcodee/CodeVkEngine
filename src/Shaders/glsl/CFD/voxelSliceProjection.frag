#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"
#include "../Utils/uPushConstants.glsl"

layout(location = 0) out vec4 out_color;
layout(location = 0) in vec2 text_coord;


layout (set = 0, binding = 0, scalar) buffer Voxels{
    uint cells[];
};


void main() {

    vec2 uv = text_coord;
    uvec3 voxel_pos;
    if(voxel_grid.plane_selected.x == 1 && voxel_grid.plane_selected.y == 1){
        voxel_pos = uvec3(int((uv.x) * voxel_grid.dimensions.x), int((1.0 - uv.y) * voxel_grid.dimensions.y), voxel_grid.selected_dimension_offset);
    }else if(voxel_grid.plane_selected.y == 1 && voxel_grid.plane_selected.z == 1){
        voxel_pos = uvec3(voxel_grid.selected_dimension_offset, int((1.0 - uv.y) * voxel_grid.dimensions.y), int((uv.x) * voxel_grid.dimensions.z));
    }else if(voxel_grid.plane_selected.x == 1 && voxel_grid.plane_selected.z == 1){
        voxel_pos = uvec3(int((uv.x) * voxel_grid.dimensions.x), voxel_grid.selected_dimension_offset, int((1.0 - uv.y) * voxel_grid.dimensions.z));
    }else{
        //falback to xy
//        voxel_pos = uvec3(int((uv.x) * voxel_grid.dimensions.x), int((1.0 - uv.y) * voxel_grid.dimensions.y), voxel_grid.selected_dimension_offset);
    }

    //this indexing is wrong to the plane selected
    uint idx = U_Grid_To_Linear_Idx(voxel_grid.dimensions, voxel_pos);
    uint cell_value = cells[idx];

    if(cell_value > 0){
        out_color = vec4(1.0, 0.0, 0.0, 1.0);
    }else{
        out_color = vec4(0.0, 0.0, 0.0, 1.0);
    }

//    out_color = vec4(float(idx)/float(voxel_grid.dimensions.x * voxel_grid.dimensions.y * voxel_grid.dimensions.z), 0.0, 0.0, 1.0);
//    out_color = vec4(vec2(voxel_pos.xy)/vec2(voxel_grid.dimensions.xy), 0.0, 1.0);

}