#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"

layout(location = 0) out vec4 out_color;
layout(location = 0) in vec2 text_coord;

layout(push_constant)uniform Push_Constants{
    uvec3 dimensions;
    float cell_size;
    uint dimension_selected;
    uint selected_dimension_offset;
}pc;

layout (set = 0, binding = 0, scalar) buffer Voxels{
    uint cells[];
};


void main() {

    vec2 uv = text_coord;
    out_color = vec4(uv.xy, 1.0, 1.0);
}