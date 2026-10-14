#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable


#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"
#include "../Utils/uPBR.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 text_coord;

layout(set = 0, binding = 0) uniform sampler2D g_col;
layout(set = 0, binding = 1) uniform sampler2D g_normals;
layout(set = 0, binding = 2) uniform sampler2D g_tang;
layout(set = 0, binding = 3) uniform sampler2D g_depth;
layout(set = 0, binding = 4) uniform sampler2D g_met_roughness;
layout(set = 0, binding = 5) uniform sampler2D g_mesh_uv;

layout(set = 0, binding = 6, scalar) uniform Camera_Properties{
    mat4 inv_proj;
    mat4 inv_view;
    vec3 pos;
    float z_near;
    float z_far;
}c_props;
layout (set = 0, binding = 7, scalar) buffer Point_Lights_Buffer{
    U_Point_Light[] point_lights;
};
layout (set = 0, binding = 8, scalar) buffer Light_Map_Buffer{
    U_Array_Indexer[] light_map;
};
layout (set = 0, binding = 9, scalar) buffer Light_Indices_Buffer{
    int[] light_indices;
};

layout (set = 0, binding = 10, rgba8) uniform image2D specular_holder;

layout (set = 0, binding = 11, scalar) writeonly buffer Camera_Positions{
    vec3[] cam_positions;
};

layout(push_constant)uniform Push_Constants{
    uint tile_count_x;
    uint tile_count_y;
    int x_tile_size_px;
    int y_tile_size_px;
    int z_slices_size;
}pc;


void main() {
    
    vec4 norm = texture(g_normals, text_coord);
    vec4 met_roughness = texture(g_met_roughness, text_coord);
    vec4 tangs = texture(g_tang, text_coord);
    vec4 screen_uvs = texture(g_mesh_uv, text_coord);
    
    vec2 frag_coord = vec2(text_coord.x , text_coord.y);
    vec4 col = texture(g_col, text_coord);
    float depth = texture(g_depth, text_coord).r;
    if(norm == vec4(0.0)){

        discard;
    }
    
//    col =vec4(0.01);
    vec3 pos = U_Screen_To_World(c_props.inv_proj, c_props.inv_view, depth, frag_coord);

    ivec2 tile_id = ivec2(gl_FragCoord.xy/uvec2(pc.x_tile_size_px, pc.y_tile_size_px));
    
    
    vec4 linear_depth = c_props.inv_proj * vec4(0.0, 0.0, depth, 1.0);
    linear_depth.z = linear_depth.z / linear_depth.w;

    int z_id =int(U_Get_Z_Slice(abs(linear_depth.z), c_props.z_near, c_props.z_far, float(pc.z_slices_size)));
    
    uint map_index = tile_id.x + (tile_id.y * pc.tile_count_x) + (z_id * pc.tile_count_x * pc.tile_count_y);

    int light_offset = light_map[map_index].offset;
    int lights_in_tile = light_map[map_index].size;
    
    vec3 light_pos = vec3(10.0, 25.0, 0.0);
    vec3 light_dir = normalize(light_pos - pos);
    vec3 view_dir = normalize(c_props.pos -  pos);
    
    U_Pbr_Context pbr_context;
    pbr_context.col = col.xyz;
    pbr_context.normal = norm.xyz;
    pbr_context.roughness = met_roughness.g;
    pbr_context.metallic = met_roughness.b;
    pbr_context.f0 = vec3(0.0, 0.0, 0.0);
    
    
    vec3 halfway = normalize(light_dir + view_dir);
    
    vec3 light_col = vec3(1.0, 1.0, 1.0);
    vec3 ambient_col = vec3(0.0, 0.0, 0.0);
    vec3 final_col = col.xyz;

    for (int i = 0; i < lights_in_tile; i++) {
        int light_index = light_indices[light_offset + i];
        final_col += U_Eval_Point_Light(point_lights[light_index], pos, view_dir, pbr_context);
    };
//    if(true){
//        float intensityId= U_Inv_Lerp(0.0, pc.tile_count_x * pc.tile_count_y * float(pc.z_slices_size), float(map_index));
//
//        float hue = intensityId;
//        float saturation = 0.8;
//        float lightness = 0.4;
//        vec3 tileCol = U_Hsl_To_Rgb(hue, saturation, lightness);
//
//        float intensity= U_Inv_Lerp(0.0, 400.0 , float(lights_in_tile));
//        vec3 debugCol = U_Lerp(vec3(0.0, 0.5, 0.4), vec3(1.0, 0.0, 0.0), intensity);
//         final_col += debugCol*2 + tileCol * 0.3;
//    }
    
    //fix rough maps
    final_col += U_Get_Brdf(view_dir, light_dir, halfway, pbr_context);
    final_col = final_col * light_col * Abs_Cos_Theta_Ws(light_dir, norm.xyz) * 1;
    
    out_color = vec4(final_col.xyz, 1.0);
    
    ivec2 uv_pos = ivec2(screen_uvs.xy * 1024.0);
    imageStore(specular_holder, uv_pos, vec4(final_col, 1.0));
    //    out_color = vec4(pbr_context.col, 1.0);

}
