#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable

#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"
#include "../Utils/uRendering.glsl"

layout (location = 0) in vec3 pos;

layout (location = 0) out int out_count;

layout(push_constant)uniform Push_Constants{
    mat4 proj_view;
    vec3 origin;
    uvec3 dimensions;
    float cell_size;
    uvec3 plane_selected;
    uint selected_dimension_offset;
}pc;

layout (set = 0, binding = 0, scalar) buffer Voxels{
    uint cells[];
};

void main() {
    uvec3 pos_in_grid = U_Linear_To_Grid_Idx(pc.dimensions, gl_InstanceIndex);
    mat4 model = U_Grid_3d_World_Pos(pc.cell_size, pc.dimensions, pc.origin, pos_in_grid);

    gl_Position = pc.proj_view * model * vec4(pos, 1.0f);
    out_count = int(cells[gl_InstanceIndex]);
    if(out_count > 0){
        return;
    }
    if((pos_in_grid.x == 0 || pos_in_grid.x == pc.dimensions.x - 1) && (pos_in_grid.y == 0 || pos_in_grid.y == pc.dimensions.y - 1)){
        out_count = -1;
    }
    if((pos_in_grid.y == 0 || pos_in_grid.y == pc.dimensions.y - 1) && (pos_in_grid.z == 0 || pos_in_grid.z == pc.dimensions.z - 1)){
        out_count = -1;
    }
    if((pos_in_grid.z == 0 || pos_in_grid.z == pc.dimensions.z - 1) && (pos_in_grid.x == 0 || pos_in_grid.x == pc.dimensions.x - 1)){
        out_count = -1;
    }
    if((pc.plane_selected.x == 1 && pc.plane_selected.y == 1) && pc.selected_dimension_offset == pos_in_grid.z){
        out_count = -2;
    }
    if((pc.plane_selected.y == 1 && pc.plane_selected.z == 1) && pc.selected_dimension_offset == pos_in_grid.x){
        out_count = -2;
    }
    if((pc.plane_selected.z == 1 && pc.plane_selected.x == 1) && pc.selected_dimension_offset == pos_in_grid.y){
        out_count = -2;
    }
}