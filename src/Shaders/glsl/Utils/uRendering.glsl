#ifndef R_UTIL 
#define R_UTIL 

#define ALBEDO_OFFSET 0
#define NORMAL_OFFSET 1
#define EMISSION_OFFSET 2
#define TRANSMISSION_OFFSET 3
#define ROUGHNESS_OFFSET 4
#define METALLIC_OFFSET 5
#define METALLIC_ROUGHNESS_OFFSET 6
#define AO_OFFSET 7
#define HEIGHT_OFFSET 8





vec3 U_Screen_To_World_Ndc(mat4 inv_proj, mat4 inv_view, float depth, vec2 screen_pos){
    vec4 ndc_pos = vec4(screen_pos, depth, 1.0);
    
    vec4 view_pos = inv_proj * ndc_pos;
    view_pos = view_pos / view_pos.w;
    
    vec4 world_pos = inv_view * view_pos;
    return vec3(world_pos.xyz);
}
vec3 U_Screen_To_World(mat4 inv_proj, mat4 inv_view, float depth, vec2 screen_pos){
    vec4 ndc_pos = vec4(1.0f);
    ndc_pos.x = 2 * screen_pos.x - 1.0f;
    ndc_pos.y = 2 * screen_pos.y - 1.0f;
    ndc_pos.z = depth;

    vec4 view_pos = inv_proj * ndc_pos;
    view_pos = view_pos / view_pos.w;

    vec4 world_pos = inv_view * view_pos;
    return vec3(world_pos.xyz);
}

vec3 U_Screen_To_View(mat4 inv_proj, float depth, vec2 screen_pos, vec2 screen_size){
    vec4 ndc_pos = vec4(1.0f);
    ndc_pos.x = 2 * (screen_pos.x/screen_size.x) - 1.0f;
    ndc_pos.y = 2 * (screen_pos.y/screen_size.y) - 1.0f;
    ndc_pos.z = depth;

    vec4 view_pos = inv_proj * ndc_pos;
    view_pos = view_pos / view_pos.w;

    return vec3(view_pos.xyz);
    
}

vec4 U_Screen_To_View_Ndc(mat4 inv_proj, float depth, vec2 ndc_coords){
    vec4 ndc_pos = vec4(ndc_coords, depth, 1.0);
    vec4 view_pos = inv_proj * ndc_pos;
    view_pos = view_pos / view_pos.w;
    return view_pos;
}
vec4 U_Screen_To_View(mat4 inv_proj, float depth, vec2 screen_pos){
    vec4 ndc_pos = vec4(1.0f);
    ndc_pos.x = 2 * screen_pos.x - 1.0f;
    ndc_pos.y = 2 * screen_pos.y - 1.0f;
    ndc_pos.z = depth;

    vec4 view_pos = inv_proj * ndc_pos;
    view_pos = view_pos / view_pos.w;

    return view_pos;
}

//linear depth
float U_Get_Z_Slice(float z, float near, float far, float num_slices) {
    return max(log2(z) * num_slices / log2(far / near) - num_slices * log2(near) / log2(far / near), 0.0);
}


vec2 U_Get_Sprite_Coord_In_Atlas(int frame_index, ivec2 sprite_size_px, int rows, int cols, ivec2 frag_pos, ivec2 f_size){
    ivec2 frame_index_in_atlas = ivec2(frame_index % cols, frame_index / cols);
    
    vec2 grid_size_px = vec2(f_size) / vec2(sprite_size_px);
    vec2 sprite_pixel_pos = floor(vec2(frag_pos) / grid_size_px);
    vec2 sprite_base_index_pos = frame_index_in_atlas * vec2(sprite_size_px);
    ivec2 final_pos = ivec2((sprite_base_index_pos + sprite_pixel_pos));
    vec2 sprite_size =  vec2(sprite_size_px) * vec2(cols, rows);
    
    return vec2(final_pos) / sprite_size;
}
mat3 U_Rotation_From_Quaternion(vec4 q) {
    float qx = q.y;
    float qy = q.z;
    float qz = q.w;
    float qw = q.x;

    float qx2 = qx * qx;
    float qy2 = qy * qy;
    float qz2 = qz * qz;

    mat3 rotation_matrix;
    rotation_matrix[0][0] = 1 - 2 * qy2 - 2 * qz2;
    rotation_matrix[0][1] = 2 * qx * qy - 2 * qz * qw;
    rotation_matrix[0][2] = 2 * qx * qz + 2 * qy * qw;

    rotation_matrix[1][0] = 2 * qx * qy + 2 * qz * qw;
    rotation_matrix[1][1] = 1 - 2 * qx2 - 2 * qz2;
    rotation_matrix[1][2] = 2 * qy * qz - 2 * qx * qw;

    rotation_matrix[2][0] = 2 * qx * qz - 2 * qy * qw;
    rotation_matrix[2][1] = 2 * qy * qz + 2 * qx * qw;
    rotation_matrix[2][2] = 1 - 2 * qx2 - 2 * qy2;

    return rotation_matrix;
}
mat3 U_Get_Cov_3d(vec4 rots, vec3 scales, float scale_mod){

    
    mat3 scale_matrix = mat3(
    scale_mod * scales.x, 0, 0,
    0, scale_mod * scales.y, 0,
    0, 0, scale_mod * scales.z
    );
    mat3 rot_matrix = U_Rotation_From_Quaternion(rots);

    mat3 m_matrix = scale_matrix * rot_matrix;

    mat3 sigma = transpose(m_matrix) * m_matrix;
    
    return sigma; 
}
mat2 U_Get_Cov_2d(vec4 mean_view, float focal_x, float focal_y, float tan_fovx, float tan_fovy, mat3 cov_3d, mat4 viewmatrix)
{
    vec4 t = mean_view;
    // why need this? Try remove this later
    float limx = 1.3f * tan_fovx;
    float limy = 1.3f * tan_fovy;
    float txtz = t.x / t.z;
    float tytz = t.y / t.z;
    t.x = min(limx, max(-limx, txtz)) * t.z;
    t.y = min(limy, max(-limy, tytz)) * t.z;

    mat3 projection_jacobian = mat3(
    focal_x / t.z, 0.0f, -(focal_x * t.x) / (t.z * t.z),
    0.0f, focal_y / t.z, -(focal_y * t.y) / (t.z * t.z),
    0, 0, 0
    );
    mat3 view_rotation = transpose(mat3(viewmatrix));
    mat3 covariance_transform = view_rotation * projection_jacobian;

    mat3 cov = transpose(covariance_transform) * transpose(cov_3d) * covariance_transform;
    // Apply low-pass filter: every Gaussian should be at least
    // one pixel wide/high. Discard 3rd row and column.
    cov[0][0] += 0.3f;
    cov[1][1] += 0.3f;
    return mat2(cov);
}

#endif 