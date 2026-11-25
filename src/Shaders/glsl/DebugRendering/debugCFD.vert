#version 450
#extension GL_GOOGLE_include_directive : require

#include "../Utils/uMath.glsl"

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 out_local_pos;
layout(location = 1) flat out uint out_style;
layout(location = 2) flat out uvec2 out_slice_dimensions;

layout(push_constant) uniform Push_Constants {
    mat4 proj_view;
    vec3 origin;
    uvec3 dimensions;
    float cell_size;
    uvec3 plane_selected;
    uint selected_dimension_offset;
} pc;

layout(set = 0, binding = 0, std430) readonly buffer Voxels {
    uint cells[];
};

void main() {
    const uint bounds_style = 0u;
    const uint slice_style = 1u;
    const uint occupied_style = 2u;
    const uint selected_voxel_style = 3u;

    uint slice_axis = pc.plane_selected == uvec3(0, 1, 1) ? 0u :
                      pc.plane_selected == uvec3(1, 0, 1) ? 1u : 2u;
    uint slice_index = min(pc.selected_dimension_offset, pc.dimensions[slice_axis] - 1u);
    uint instance_index = uint(gl_InstanceIndex);

    vec3 grid_center = (vec3(pc.dimensions) - 1.0) * 0.5;
    vec3 grid_pos;
    out_local_pos = pos;
    out_slice_dimensions = uvec2(0);

    // The first two instances draw the domain bounds and the selected slice.
    if (instance_index == 0u) {
        out_style = bounds_style;
        grid_pos = grid_center + pos * vec3(pc.dimensions);
    } else if (instance_index == 1u) {
        out_style = slice_style;
        uint u_axis = slice_axis == 0u ? 2u : 0u;
        uint v_axis = slice_axis == 1u ? 2u : 1u;
        out_local_pos = vec3(pos[u_axis], pos[v_axis], pos[slice_axis]);
        out_slice_dimensions = uvec2(pc.dimensions[u_axis], pc.dimensions[v_axis]);
        grid_pos = grid_center + pos * vec3(pc.dimensions);
        grid_pos[slice_axis] = float(slice_index);
    } else {
        uint voxel_index = instance_index - 2u;
        if (cells[voxel_index] == 0u) {
            // Clip empty instances before rasterization.
            gl_Position = vec4(2.0, 0.0, 0.0, 1.0);
            out_style = occupied_style;
            return;
        }
        uvec3 voxel_pos = U_Linear_To_Grid_Idx(pc.dimensions, voxel_index);
        out_style = voxel_pos[slice_axis] == slice_index ? selected_voxel_style : occupied_style;
        grid_pos = vec3(voxel_pos) + pos * 0.76;
    }

    gl_Position = pc.proj_view * vec4(pc.origin + grid_pos * pc.cell_size, 1.0);
}
