#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"

layout(location = 0) in flat int in_count;

layout(location = 0) out vec4 out_color;


void main() {

    int count = in_count;
    if(count > 0){
        out_color = vec4(1.0, 0.0, 0.0, 1.0);
    }else if(count == -1){
        out_color = vec4(0.0, 1.0, 0.0, 0.2);
    }else if(count == -2){
        out_color = vec4(0.0, 0.0, 1.0, 0.8);
    }else{
        discard;
    }
}