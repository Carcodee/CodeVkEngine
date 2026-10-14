#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"

layout (location = 0) in vec2 text_coord;

layout(location = 0) out vec4 out_color;


layout(push_constant) uniform Push_Constants{
    int cascades_count;
    int probe_size_px;
    int interval_count;
    int f_width;
    int f_height;
    int base_interval_length;
    int cascade_index;
}pc;

layout (set = 0, binding = 0)uniform sampler2D merged_cascades;
layout (set = 0, binding = 1, rgba8) uniform image2D painting_layers[];
layout (set = 0, binding = 2, rgba8) uniform image2D radiances[];
layout (set = 0, binding = 3) uniform sampler2D test_image;

layout (set = 0, binding = 4) uniform sampler2D sprite_anims[];
layout (set = 0, binding = 5) uniform sampler2D mat_textures[];

layout (set = 0, binding = 6, scalar) buffer Sprite_Info{
    U_Sprite_Animation_Info sprite_anim_info;
};


layout (set = 0, binding = 7, scalar) uniform Light_Info{
    vec3 pos;
    vec3 col;
    float intensity;
}ubo;
layout (set = 0, binding = 8, scalar) uniform R_Configs_Uniform{
    U_Radiance_Cascades_Configs configs;
}r_configs;


void main() {
    vec3 light_dir = ubo.pos ;
   
    ivec2 coord = ivec2(gl_FragCoord.xy);
    ivec2 f_size = ivec2(pc.f_width, pc.f_height);
    
    int interval_count = pc.interval_count;
    int probe_size_px = pc.probe_size_px;
    
    vec2 probe_count = floor(vec2(f_size) / float(probe_size_px));
    vec2 base_index = ((vec2(gl_FragCoord.xy)) / probe_size_px);
    vec2 cell_frac = fract(base_index);

    base_index = floor(base_index);

    vec2 tl_probe_pos= base_index;
    vec2 tr_probe_pos= base_index + vec2(1.0, 0.0);
    vec2 bl_probe_pos= base_index + vec2(0.0, 1.0);
    vec2 br_probe_pos= base_index + vec2(1.0, 1.0);

    vec2 tl_frag_pos = tl_probe_pos * probe_size_px;
    vec2 tr_frag_pos = tr_probe_pos * probe_size_px;
    vec2 bl_frag_pos = bl_probe_pos * probe_size_px;
    vec2 br_frag_pos = br_probe_pos * probe_size_px;
    
    vec4 radiance = vec4(0.0);

    ivec2 curr_dir = ivec2(0.0);
    for (int i = 0; i < interval_count * interval_count; i++){

        curr_dir = ivec2(tl_frag_pos) + ivec2(i % probe_size_px, i / probe_size_px);
        radiance+= imageLoad(radiances[0], curr_dir);
    }
    radiance /= (interval_count * interval_count);
//    
//    imageStore(radiances[0], ivec2(gl_FragCoord.xy), radiance);
//
//    vec4 tlCol = imageLoad(radiances[0], ivec2(tl_frag_pos));
//    vec4 trCol = imageLoad(radiances[0], ivec2(tr_frag_pos));
//    vec4 blCol = imageLoad(radiances[0], ivec2(bl_frag_pos));
//    vec4 brCol = imageLoad(radiances[0], ivec2(br_frag_pos));
//
//    vec4 interpolated = vec4(0.0);
//    interpolated += mix(mix(tlCol, trCol, 0.0), mix(blCol, brCol, cell_frac.x), cell_frac.y);

//    vec4 base_col = imageLoad(radiances[0], ivec2(gl_FragCoord.xy));
    vec4 painting_image = imageLoad(painting_layers[0], coord);
    vec4 simulation_image = imageLoad(painting_layers[3], coord);
    vec4 simulation_image_info = imageLoad(painting_layers[4], coord);
    vec4 test_img = texture(test_image, text_coord);
    vec4 black_oc= imageLoad(painting_layers[1], coord);
    vec4 debug= imageLoad(painting_layers[2], coord);

    int size = sprite_anim_info.rows * sprite_anim_info.cols;
    vec2 sprite_pos = U_Get_Sprite_Coord_In_Atlas(sprite_anim_info.current_frame, sprite_anim_info.sprite_size_px, sprite_anim_info.rows, sprite_anim_info.cols, ivec2(gl_FragCoord.xy), f_size);
    vec2 sprite_pos1 = U_Get_Sprite_Coord_In_Atlas((sprite_anim_info.current_frame + 1) % size, sprite_anim_info.sprite_size_px, sprite_anim_info.rows, sprite_anim_info.cols, ivec2(gl_FragCoord.xy), f_size);
    vec4 sprite_col = texture(sprite_anims[0], vec2(sprite_pos));
    vec4 sprite_col1 = texture(sprite_anims[0], vec2(sprite_pos1));
    
    vec4 final_val = mix(sprite_col, sprite_col1, sprite_anim_info.interp_val);

    vec4 base_col = texture(mat_textures[ALBEDO_OFFSET], text_coord);
    vec4 normal_map = texture(mat_textures[NORMAL_OFFSET], text_coord);
    vec4 roughness = texture(mat_textures[ROUGHNESS_OFFSET], text_coord);
    vec4 ao = texture(mat_textures[AO_OFFSET], text_coord) * 2.0;
    vec4 height = texture(mat_textures[HEIGHT_OFFSET], text_coord);

    U_Radiance_Cascades_Configs configs = r_configs.configs;
//    out_color = (base_col * ao.x * height.x) * vec4(diff, 1.0);
    float normal_map_combined =pow(float(configs.normal_map_pow), (normal_map.x * normal_map.y * normal_map.z));
    float specular = pow(configs.specular_pow ,max(0.0, normal_map_combined));
    
    vec4 final_radiance = vec4(1.0);
    for(int i = 0; i< configs.radiance_pow; i++){
        final_radiance *= radiance;
    }
    
    vec4 test = base_col * final_radiance * pow(8.0 ,height.x);
    vec4 final_col = base_col * final_radiance * normal_map_combined * specular * pow(roughness.x, configs.roughness_pow) * ao.x;
    
    vec3 dir_light_col = abs(dot(normal_map.xyz, light_dir)) * ubo.col * ubo.intensity;
    out_color = final_col + vec4(dir_light_col, 1.0);
//    if(sprite_col.w > 0.8){
//        out_color = sprite_col;
//    }
    if(painting_image.w > 0.1){
        out_color = painting_image * painting_image.w;
    }

    if(simulation_image.w > 0.9){
    }
    out_color = vec4(simulation_image.xyz, simulation_image.w);
//    out_color = simulation_image_info;
    

}