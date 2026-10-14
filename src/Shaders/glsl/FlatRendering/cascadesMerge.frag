#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"

layout(push_constant) uniform push_constants{
    int cascades_count;
    int probe_size_px;
    int interval_count;
    int f_width;
    int f_height;
    int base_interval_length;
    int cascade_index;
}pc;

layout (set = 0, binding = 0) uniform sampler2D cascades[];
layout (set = 0, binding = 1, rgba8) uniform image2D radiances[];

layout (location = 0) in vec2 text_coord;

layout(location = 0) out vec4 out_color;

vec4 merge_intervals(vec4 near, vec4 far){
    vec3 radiance = near.rgb + (far.rgb * near.a);
    return vec4(radiance, near.a * far.a);
}

vec4 bilinear_weights(vec2 ratio) {
    return vec4(
    (1.0 - ratio.x) * (1.0 - ratio.y),
    ratio.x * (1.0 - ratio.y),
    (1.0 - ratio.x) * ratio.y,
    ratio.x * ratio.y
    );
}

void bilinear_samples(vec2 dest_center, vec2 bilinear_size, out vec4 weights, out ivec2 base_index) {
    const vec2 base_coord = (dest_center / bilinear_size) - vec2(0.5, 0.5);
    const vec2 ratio = fract(base_coord);
    weights = bilinear_weights(ratio);
    base_index = ivec2(floor(base_coord));
}
ivec2 bilinear_offset(int offset_index) {
    const ivec2 offsets[4] = {ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(1, 1)};
    return offsets[offset_index];
}


#define CASCADE_SIZE 5
void main() {

    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 f_size = ivec2(pc.f_width, pc.f_height);
    //debug
    //this is grid so the interval is the double of this val e.g 2 inteval count means 4 directions
    int interval_grid= pc.interval_count;
    int grid_size = pc.probe_size_px;
    vec2 probe_centers_positions_px[CASCADE_SIZE];
    vec2 probe_sizes_px[CASCADE_SIZE];
    int dir_indices[CASCADE_SIZE];
    vec2 texel_dir[CASCADE_SIZE];
    vec2 dir_col;
    for(int i = 0; i < CASCADE_SIZE; i++){
        vec2 pos = vec2(gl_FragCoord.xy) / vec2(f_size);
        vec2 cascade = texture(cascades[i], pos).xy;
        uint dir_count = interval_grid * interval_grid;
        ivec2 text_idx = ivec2(cascade.xy * float(interval_grid));
        int dir_index = text_idx.x + text_idx.y * interval_grid;
        
        float angle = 2.0 * PI * ((float(dir_index) + 0.5) / float(dir_count));
        texel_dir[i]= vec2(cos(angle), sin(angle));

        dir_indices[i] = dir_index;

        vec2 probe_center_grid = floor(vec2(gl_FragCoord.xy) / float(grid_size)) + vec2(0.5);
        vec2 probe_center_pos = (probe_center_grid * (grid_size));

        probe_sizes_px[i] = vec2(grid_size);
        probe_centers_positions_px[i] = vec2(probe_center_pos);
        interval_grid *= 2;
        grid_size *= 2;
    }

    int n = pc.cascade_index;
    int n_plus_one = n + 1;
    vec2 dest_center = probe_centers_positions_px[n];
    vec2 bilinear_size = probe_sizes_px[n_plus_one];
    vec4 weights = vec4(0.0);
    ivec2 base_index = ivec2(0.0);
    bilinear_samples(dest_center, bilinear_size, weights, base_index);

    ivec2 dest_dir_coord = ivec2(dir_indices[n] % int(probe_sizes_px[n].x), dir_indices[n] / probe_sizes_px[n].y);
    vec2 dest_base_coord =floor(dest_center / probe_sizes_px[n]) * probe_sizes_px[n];
    ivec2 dest_texel = ivec2(dest_base_coord + dest_dir_coord);

    vec4 dest_interval = imageLoad(radiances[n], dest_texel);
    vec4 merged = vec4(0.0);

    if(dest_interval.a < 1.0){
        merged += dest_interval;
    }else{
        //per direction in n+1 
        for(int d = 0; d < 4; d++){
            vec4 radiance_bilinear = vec4(0.0);
            //per probe in n+1
            for (int b = 0; b < 4; b++){
                const ivec2 base_offset = bilinear_offset(b);
                const ivec2 bilinear_index = base_index + base_offset;
                const int base_dir_index = dir_indices[n] * 4;
                const int bilinear_dir_index = base_dir_index + d;
                const ivec2 bilinear_dir_coord = ivec2(bilinear_dir_index % int(bilinear_size.x),
                bilinear_dir_index / bilinear_size.y);
                const ivec2 bilinear_texel = bilinear_index * ivec2(bilinear_size) + bilinear_dir_coord;
                vec4 bilinear_interval = imageLoad(radiances[n_plus_one], bilinear_texel);
                radiance_bilinear += merge_intervals(dest_interval, bilinear_interval) * weights[b];
            }
            merged += radiance_bilinear;
        }
        merged/= 4.0;       
    }
    imageStore(radiances[n], dest_texel, merged);

    
//    vec2 pos = ivec2(gl_FragCoord.xy) % ivec2(probe_sizes_px[3]);
//    vec4 col = vec4( pos / probe_sizes_px[3], 0.0, 1.0);
//    imageStore(radiances[3] , ivec2(probe_centers_positions_px[3]), vec4(1.0, 0.0, 0.0, 1.0));
//    vec4 dest_interval = imageLoad(radiances[3], coords);
//    ivec2 dest_dir_coord = ivec2(dir_indices[0] % int(probe_sizes_px[0].x), dir_indices[0] / probe_sizes_px[0].y);
//    vec2 dest_base_coord =floor(probe_centers_positions_px[0] / probe_sizes_px[0]) * probe_sizes_px[0];
//    ivec2 dest_texel = ivec2(dest_base_coord + dest_dir_coord);

//    vec4 dest_interval = imageLoad(radiances[0], dest_texel);

//    out_color = dest_interval;
}