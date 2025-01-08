#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"


layout(push_constant)uniform pushConstants{
    int cascadeIndex;
    int intervalSize;
    int probeSizePx;
}pc;

layout (location = 0) in vec2 textCoord;

layout(location = 0) out vec4 outColor;


void main() {

    vec2 textCoordPerGrid = vec2(1.0)/ pc.probeSizePx;
    float intervalSizePx = floor(float(pc.probeSizePx) / float(pc.intervalSize));
    
    
    ivec2 localTexelPosInProbe = ivec2((ivec2(gl_FragCoord.xy) % pc.probeSizePx));
    vec2 cuadrant = floor(vec2(localTexelPosInProbe) / intervalSizePx);
    
    vec2 indexColNew = cuadrant / float(pc.intervalSize);
    
//    
//    ivec2 probeId = ivec2(textCoord * pc.probeSizePx);
//    vec2 gridCol = vec2(probeId) / pc.probeSizePx;
//    
//    vec2 origin = textCoordPerGrid * vec2(probeId);
//
//    vec2 localUV = ((textCoord) - origin) / textCoordPerGrid;
//    
//    ivec2 index = ivec2(localUV * (pc.intervalSize));
//    vec2 indexCol = vec2(index)/(float(pc.intervalSize));
//    
//    int dirCount = pc.intervalSize;
//    int dirIndex = index.x + index.y * (pc.intervalSize);

//    float test = float(dirIndex)/float(dirCount);
   
    outColor = vec4(indexColNew, 0.0, 1.0);

}