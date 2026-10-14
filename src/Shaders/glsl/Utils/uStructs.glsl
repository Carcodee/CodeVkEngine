#ifndef R_STRUCTS 
#define R_STRUCTS 

//Lights

struct U_Point_Light{
    vec3 pos;
    vec3 col;
    float radius;
    float intensity;
    float l_attenuation;
    float q_attenuation;
};

struct U_Dir_Light{
    vec3 pos;
    vec3 col;
    float intensity;
};

//End Lights

//Util

struct U_Array_Indexer{
    int offset;
    int size;
};

struct U_Frustum{
    vec4 planes[6];
    vec3 points[8];
};

struct U_Material_Packed{
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

struct U_Draw_Indirect_Indexed_Cmd
{
    uint index_count;
    uint instance_count;
    uint first_index;
    uint vertex_offset;
    uint first_instance;
};

struct U_Sphere{
    vec3 center;
    float radius;
};

struct U_Merge_Cascades_Info{
    ivec2 probe_centers_positions_px;
    ivec2 probe_sizes_px;
    int dir_indices;
};

struct U_Sprite_Animation_Info{
    ivec2 sprite_size_px;
    int rows;
    int cols;
    int current_frame;
    int frame_count;
    float interp_val;
    bool is_atlas;
};
struct U_Radiance_Cascades_Configs
{
    int radiance_pow;
    int normal_map_pow;
    int specular_pow;
    int roughness_pow;
};

struct U_Pbr_Context{
    vec3 col;
    vec3 normal;
    vec3 f0;
    float metallic;
    float roughness;
};
//End Util

#endif 