#version 450

layout (location = 0) in vec3 pos;

layout(push_constant)uniform push_constants{
    mat4 proj_view;
    vec3 origin;
    uvec3 dimensions;
    float cell_size;
}pc;

mat4 calculate_model_mat(float cell_size, uvec3 dimensions, vec3 origin, uint cell_id){

    uvec3 pos_in_grid = uvec3(cell_id % dimensions.x, cell_id / dimensions.y, cell_id / (dimensions.x * dimensions.y));
    vec3 offset= vec3(pos_in_grid.x, pos_in_grid.y, pos_in_grid.z) * vec3(cell_size, cell_size, cell_size);

    mat4 model = mat4(1.0);
    model[0][0] = cell_size;
    model[1][1] = cell_size;
    model[2][2] = cell_size;

    model[3][0] = origin.x + offset.x;
    model[3][1] = origin.z + offset.y;
    model[3][2] = origin.y + offset.z;
    return model;
}
void main() {

    mat4 model = calculate_model_mat(pc.cell_size, pc.dimensions, pc.origin, gl_InstanceIndex);
    gl_Position = pc.proj_view * model * vec4(pos, 1.0f);
}