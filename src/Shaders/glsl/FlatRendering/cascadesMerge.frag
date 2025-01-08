#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"

layout(push_constant) uniform pushConstants{
    int cascadesCount;
    int probeSizePx;
    int intervalCount;
    int fWidth;
    int fHeight;
    int baseIntervalLength;
    int cascadeIndex;
}pc;

layout (set = 0, binding = 0) uniform sampler2D Cascades[];
layout (set = 0, binding = 1, rgba8) uniform image2D Radiances[];

layout (location = 0) in vec2 textCoord;

layout(location = 0) out vec4 outColor;

vec4 MergeIntervals(vec4 near, vec4 far){
    vec3 radiance = near.rgb + (far.rgb * near.a);
    return vec4(radiance, near.a * far.a);
}

vec4 BilinearWeights(vec2 ratio) {
    return vec4(
    (1.0 - ratio.x) * (1.0 - ratio.y),
    ratio.x * (1.0 - ratio.y),
    (1.0 - ratio.x) * ratio.y,
    ratio.x * ratio.y
    );
}

void BilinearSamples(vec2 destCenter, vec2 bilinearSize, out vec4 weights, out ivec2 baseIndex) {
    const vec2 baseCoord = (destCenter / bilinearSize) - vec2(0.5, 0.5);
    const vec2 ratio = fract(baseCoord);
    weights = BilinearWeights(ratio);
    baseIndex = ivec2(floor(baseCoord));
}
ivec2 bilinearOffset(int offsetIndex) {
    const ivec2 offsets[4] = {ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(1, 1)};
    return offsets[offsetIndex];
}


#define CASCADE_SIZE 5
void main() {

    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 fSize = ivec2(pc.fWidth, pc.fHeight);
    //debug
    //this is grid so the interval is the double of this val e.g 2 inteval count means 4 directions
    int intervalGrid= pc.intervalCount;
    int gridSize = pc.probeSizePx;
    vec2 probeCentersPositionsPx[CASCADE_SIZE];
    vec2 probeSizesPx[CASCADE_SIZE];
    int dirIndices[CASCADE_SIZE];
    vec2 texelDir[CASCADE_SIZE];
    vec2 dirCol;
    for(int i = 0; i < CASCADE_SIZE; i++){
        vec2 pos = vec2(gl_FragCoord.xy) / vec2(fSize);
        vec2 cascade = texture(Cascades[i], pos).xy;
        uint dirCount = intervalGrid * intervalGrid;
        ivec2 textIdx = ivec2(cascade.xy * float(intervalGrid));
        int dirIndex = textIdx.x + textIdx.y * intervalGrid;
        
        float angle = 2.0 * PI * ((float(dirIndex) + 0.5) / float(dirCount));
        texelDir[i]= vec2(cos(angle), sin(angle));

        dirIndices[i] = dirIndex;

        vec2 probeCenterGrid = floor(vec2(gl_FragCoord.xy) / float(gridSize)) + vec2(0.5);
        vec2 probeCenterPos = (probeCenterGrid * (gridSize)); 

        probeSizesPx[i] = vec2(gridSize);
        probeCentersPositionsPx[i] = vec2(probeCenterPos);
        intervalGrid *= 2;
        gridSize *= 2;
    }

    int n = pc.cascadeIndex;
    int nPlusOne = n + 1;
    vec2 destCenter = probeCentersPositionsPx[n];
    vec2 bilinearSize = probeSizesPx[nPlusOne];
    vec4 weights = vec4(0.0);
    ivec2 baseIndex = ivec2(0.0);
    BilinearSamples(destCenter, bilinearSize, weights, baseIndex);

    ivec2 destDirCoord = ivec2(dirIndices[n] % int(probeSizesPx[n].x), dirIndices[n] / probeSizesPx[n].y);
    vec2 destBaseCoord =floor(destCenter / probeSizesPx[n]) * probeSizesPx[n];
    ivec2 destTexel = ivec2(destBaseCoord + destDirCoord);

    vec4 destInterval = imageLoad(Radiances[n], destTexel);
    vec4 merged = vec4(0.0);

    if(destInterval.a < 1.0){
        merged += destInterval;
    }else{
        //per direction in n+1 
        for(int d = 0; d < 4; d++){
            vec4 radianceBilinear = vec4(0.0);
            //per probe in n+1
            for (int b = 0; b < 4; b++){
                const ivec2 baseOffset = bilinearOffset(b);
                const ivec2 bilinearIndex = baseIndex + baseOffset;
                const int baseDirIndex = dirIndices[n] * 4;
                const int bilinearDirIndex = baseDirIndex + d;
                const ivec2 bilinearDirCoord = ivec2(bilinearDirIndex % int(bilinearSize.x),
                bilinearDirIndex / bilinearSize.y);
                const ivec2 bilinearTexel = bilinearIndex * ivec2(bilinearSize) + bilinearDirCoord;
                vec4 bilinearInterval = imageLoad(Radiances[nPlusOne], bilinearTexel);
                radianceBilinear += MergeIntervals(destInterval, bilinearInterval) * weights[b];
            }
            merged += radianceBilinear;
        }
        merged/= 4.0;       
    }
    imageStore(Radiances[n], destTexel, merged);

    
//    vec2 pos = ivec2(gl_FragCoord.xy) % ivec2(probeSizesPx[3]);
//    vec4 col = vec4( pos / probeSizesPx[3], 0.0, 1.0);
//    imageStore(Radiances[3] , ivec2(probeCentersPositionsPx[3]), vec4(1.0, 0.0, 0.0, 1.0));
//    vec4 destInterval = imageLoad(Radiances[3], coords);
//    ivec2 destDirCoord = ivec2(dirIndices[0] % int(probeSizesPx[0].x), dirIndices[0] / probeSizesPx[0].y);
//    vec2 destBaseCoord =floor(probeCentersPositionsPx[0] / probeSizesPx[0]) * probeSizesPx[0];
//    ivec2 destTexel = ivec2(destBaseCoord + destDirCoord);

//    vec4 destInterval = imageLoad(Radiances[0], destTexel);

//    outColor = destInterval;
}