#ifndef U_PC
#define U_PC

layout(push_constant, std430) uniform Pc_Voxel_Grid {
    vec3 origin;
    uvec3 dimensions;
    float cell_size;
    uvec3 plane_selected;
    uint selected_dimension_offset;
} voxel_grid;

#endif