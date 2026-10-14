#ifndef R_STRUCTS 
#define R_STRUCTS 

//Lights

struct u_point_light{
    vec3 pos;
    vec3 col;
    float radius;
    float intensity;
    float l_attenuation;
    float q_attenuation;
};

struct u_dir_light{
    vec3 pos;
    vec3 col;
    float intensity;
};

//End Lights

//Util

struct u_array_indexer{
    int offset;
    int size;
};

struct u_frustum{
    vec4 planes[6];
    vec3 points[8];
};

struct u_material_packed{
    vec4 diff;
    float albedo_factor;
    float normal_factor;
    float roughness_factor;
    float metallic_factor;
    float alpha_cutoff;

    int albedo_offset;
    int normal_offset;
    int emission_offset;
    int trans_offset;
    int roughness_offset;
    int metallic_offset;
    int met_roughness_offset;
};

struct u_draw_indirect_indexed_cmd
{
    uint index_count;
    uint instance_count;
    uint first_index;
    uint vertex_offset;
    uint first_instance;
};

struct u_sphere{
    vec3 center;
    float radius;
};

struct u_merge_cascades_info{
    ivec2 probe_centers_positions_px;
    ivec2 probe_sizes_px;
    int dir_indices;
};

struct u_sprite_animation_info{
    ivec2 sprite_size_px;
    int rows;
    int cols;
    int current_frame;
    int frame_count;
    float interp_val;
    bool is_atlas;
};
struct u_radiance_cascades_configs
{
    int radiance_pow;
    int normal_map_pow;
    int specular_pow;
    int roughness_pow;
};

struct u_pbr_context{
    vec3 col;
    vec3 normal;
    vec3 f0;
    float metallic;
    float roughness;
};
//End Util

#endif 