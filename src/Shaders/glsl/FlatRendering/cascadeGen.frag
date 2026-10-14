#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"


layout(push_constant)uniform push_constants{
    int cascade_index;
    int interval_size;
    int probe_size_px;
}pc;

layout (location = 0) in vec2 text_coord;

layout(location = 0) out vec4 out_color;


void main() {

    vec2 text_coord_per_grid = vec2(1.0)/ pc.probe_size_px;
    float interval_size_px = floor(float(pc.probe_size_px) / float(pc.interval_size));
    
    
    ivec2 local_texel_pos_in_probe = ivec2((ivec2(gl_FragCoord.xy) % pc.probe_size_px));
    vec2 cuadrant = floor(vec2(local_texel_pos_in_probe) / interval_size_px);
    
    vec2 index_col_new = cuadrant / float(pc.interval_size);
    
//    
//    ivec2 probeId = ivec2(text_coord * pc.probe_size_px);
//    vec2 gridCol = vec2(probeId) / pc.probe_size_px;
//    
//    vec2 origin = text_coord_per_grid * vec2(probeId);
//
//    vec2 localUV = ((text_coord) - origin) / text_coord_per_grid;
//    
//    ivec2 index = ivec2(localUV * (pc.interval_size));
//    vec2 indexCol = vec2(index)/(float(pc.interval_size));
//    
//    int dir_count = pc.interval_size;
//    int dir_index = index.x + index.y * (pc.interval_size);

//    float test = float(dir_index)/float(dir_count);
   
    out_color = vec4(index_col_new, 0.0, 1.0);

}