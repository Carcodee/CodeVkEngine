#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"
#include "../Utils/uPBR.glsl"


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

layout (set = 0, binding = 0) uniform sampler2D cascades[];
layout (set = 0, binding = 1, rgba8) uniform image2D painting_layers[];
layout (set = 0, binding = 2, rgba8) uniform image2D radiances[];
layout (set = 0, binding = 3) uniform sampler2D test_image;
layout (set = 0, binding = 4) uniform sampler2D sprite_anims[];

layout (set = 0, binding = 5, scalar) buffer Sprite_Info{
    U_Sprite_Animation_Info sprite_anim_info;
};

layout (set = 0, binding = 6) uniform sampler2D mat_textures[];


#define CASCADE_SIZE 5

vec4 Merge_Intervals(vec4 near, vec4 far){
    vec3 radiance = near.rgb + (far.rgb * near.a);
    return vec4(radiance, near.a * far.a);
}

vec4 Cast_Interval(vec2 interval_start, vec2 interval_end, int cascade_index, vec2 f_size){
    vec4 accumulated_radiance = vec4(0.0, 0.0, 0.0, 1.0);
    float step_size = 1.0;
    vec2 dir = normalize(interval_end - interval_start) * step_size;
    int max_steps = 1;
    bool occluded = false;
    int sample_count = 0;
    vec2 pos = interval_start;

    for (int i = 0; i < max_steps; i++){
        vec2 text_coord_pos = vec2(pos)/ vec2(f_size);
        vec4 sample_col= imageLoad(painting_layers[0], ivec2(pos));
        vec4 simulation_col= imageLoad(painting_layers[3], ivec2(pos));
        vec4 ocludders_finded= imageLoad(radiances[0], ivec2(pos));
        vec4 sample_col_image= texture(test_image,text_coord_pos);

        int size = sprite_anim_info.rows * sprite_anim_info.cols;
        vec2 sprite_pos = U_Get_Sprite_Coord_In_Atlas(sprite_anim_info.current_frame, sprite_anim_info.sprite_size_px, sprite_anim_info.rows, sprite_anim_info.cols, ivec2(pos), ivec2(f_size));
        vec4 sprite_col = texture(sprite_anims[0], vec2(sprite_pos));
        ///debug
        vec3 cascade_col = U_Hsl_To_Rgb(float(sprite_anim_info.current_frame) / 36.0, 0.8, 0.4);
//        if(cascade_index == 3){
//            imageStore(painting_layers[2], ivec2(pos), vec4(cascade_col, 1.0));
//        }       
        ///
        vec4 normal_map = texture(mat_textures[NORMAL_OFFSET], text_coord_pos);
        float normal_map_combined =normal_map.x * normal_map.y * normal_map.z;
        float dot_prod= dot(normal_map.xy, dir);
        if(normal_map_combined < 0.1){
            occluded = true;
            float powwd_com= pow(normal_map_combined, 2.0);
            accumulated_radiance += vec4(vec3(0.1), 1.0) * normal_map_combined;
            sample_count++;
        }
//        if(sprite_col.w > 0.2){
//            occluded = true;
//            accumulated_radiance += sprite_col * sprite_col.w;
//            sample_count++;
//        }       
        if(sample_col.w > 0.3){
            occluded = true;
            accumulated_radiance = sample_col * sample_col.w;
            sample_count++;
        }

        if(simulation_col.w > 0.8){
            occluded = true;
            accumulated_radiance = simulation_col;
            sample_count++;
        }
        if(simulation_col.w > 0.1){
            accumulated_radiance += vec4(simulation_col.xyz, 1.0f) * 0.02;
            sample_count++;
        }

        pos += dir;
        if(occluded){
            accumulated_radiance /= sample_count;
            accumulated_radiance.w = 0.0;
            break;
        }
        if (pos.x < 0 || pos.x >= pc.f_width || pos.y < 0 || pos.y >= pc.f_height || distance(pos, interval_end) < 0.1) {
            break;
        }
    }
//    accumulated_radiance /= sample_count;
    return accumulated_radiance;
}

float Interval_Scale(int cascade_index){
    if(cascade_index<= 0){
        return 0.0;
    }
    return float(1 << (2 * cascade_index));
}

vec2 Interval_Range(int cascade_index, float base_lenght){
    return base_lenght * vec2(Interval_Scale(cascade_index), Interval_Scale(cascade_index + 1));
}


void main() {
    float aspect = float(pc.f_width)/float(pc.f_height);
    ivec2 f_size = ivec2(pc.f_width, pc.f_height);
    vec2 normalized_text_coords = vec2(text_coord.x, text_coord.y);
    ivec2 coord = ivec2(gl_FragCoord.xy);
    
    int interval_grid= pc.interval_count;
    int grid_size = pc.probe_size_px;
    //debug
   
    for(int i= 0; i < CASCADE_SIZE; i++ ){

        vec2 pos = vec2(gl_FragCoord.xy) / vec2(f_size);
        vec2 cascades = texture(cascades[i], pos).xy;
        uint dir_count = interval_grid * interval_grid;
        ivec2 text_idx = ivec2(cascades.xy * float(interval_grid));
        int dir_index = text_idx.x + text_idx.y * interval_grid;
        
        float angle = 2.0 * PI * ((float(dir_index) + 0.5) / float(dir_count));
        vec2 texel_dir = vec2(cos(angle), sin(angle));

        vec2 probe_center_grid = floor(vec2(gl_FragCoord.xy) / float(grid_size)) + vec2(0.5);

        vec2 probe_pos = (probe_center_grid * (grid_size));

        if(distance(probe_pos, vec2(gl_FragCoord.xy)) < 0.1){
            continue;
        }
        vec2 interval_range = Interval_Range(i, pc.base_interval_length);
        vec2 interval_start = probe_pos + texel_dir * interval_range.x;
        vec2 interval_end = probe_pos + texel_dir * interval_range.y;
        vec4 radiance = Cast_Interval(interval_start, interval_end, i, vec2(f_size));
        
        ivec2 dir_pos = ivec2((probe_center_grid - vec2(0.5)) * grid_size) + ivec2(dir_index % grid_size, dir_index / grid_size);
        vec2 dir_col = vec2(dir_pos) / vec2(f_size);

        imageStore(radiances[i], ivec2(dir_pos), radiance);

        interval_grid *= 2;
        grid_size *= 2;
    }

    

}