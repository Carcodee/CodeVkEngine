#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"
#include "../Utils/uPBR.glsl"


layout (location = 0) in vec2 textCoord;

layout(location = 0) out vec4 outColor;

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
layout (set = 0, binding = 1, rgba8) uniform image2D PaintingLayers[];
layout (set = 0, binding = 2, rgba8) uniform image2D Radiances[];
layout (set = 0, binding = 3) uniform sampler2D TestImage;
layout (set = 0, binding = 4) uniform sampler2D SpriteAnims[];

layout (set = 0, binding = 5, scalar) buffer SpriteInfo{
    u_SpriteAnimationInfo spriteAnimInfo;
};

layout (set = 0, binding = 6) uniform sampler2D MatTextures[];


#define CASCADE_SIZE 5

vec4 MergeIntervals(vec4 near, vec4 far){
    vec3 radiance = near.rgb + (far.rgb * near.a);
    return vec4(radiance, near.a * far.a);
}

vec4 CastInterval(vec2 intervalStart, vec2 intervalEnd, int cascadeIndex, vec2 fSize){
    vec4 accumulatedRadiance = vec4(0.0, 0.0, 0.0, 1.0);
    float stepSize = 1.0;
    vec2 dir = normalize(intervalEnd - intervalStart) * stepSize;
    int maxSteps = 500;
    bool occluded = false;
    int sampleCount = 0;
    vec2 pos = intervalStart;

    for (int i = 0; i < maxSteps; i++){
        vec2 textCoordPos = vec2(pos)/ vec2(fSize);
        vec4 sampleCol= imageLoad(PaintingLayers[0], ivec2(pos));
        vec4 ocluddersFinded= imageLoad(Radiances[0], ivec2(pos));
        vec4 sampleColImage= texture(TestImage,textCoordPos);

        int size = spriteAnimInfo.rows * spriteAnimInfo.cols;
        vec2 spritePos = u_GetSpriteCoordInAtlas(spriteAnimInfo.currentFrame, spriteAnimInfo.spriteSizePx, spriteAnimInfo.rows, spriteAnimInfo.cols, ivec2(pos), ivec2(fSize));
        vec4 spriteCol = texture(SpriteAnims[0], vec2(spritePos));
        ///debug
        vec3 cascadeCol = u_HSLToRGB(float(spriteAnimInfo.currentFrame) / 36.0, 0.8, 0.4);
//        if(cascadeIndex == 3){
//            imageStore(PaintingLayers[2], ivec2(pos), vec4(cascadeCol, 1.0));
//        }       
        ///
        vec4 normalMap = texture(MatTextures[NORMAL_OFFSET], textCoordPos);
        float normalMapCombined =normalMap.x * normalMap.y * normalMap.z;
        float dotProd= dot(normalMap.xy, dir);
        if(normalMapCombined < 0.1){
            occluded = true;
            float powwdCom= pow(normalMapCombined, 2.0);
            accumulatedRadiance += vec4(vec3(0.1), 1.0) * normalMapCombined;
            sampleCount++;
        }
//        if(spriteCol.w > 0.3){
//            occluded = true;
//            accumulatedRadiance += spriteCol * spriteCol.w;
//            sampleCount++;
//        }       
        if(sampleCol.w > 0.01){
            occluded = true;
            accumulatedRadiance = sampleCol * sampleCol.w;
            sampleCount++;
        }

        pos += dir;
        if(occluded){
            accumulatedRadiance /= sampleCount;
            accumulatedRadiance.w = 0.0;
            break;
        }
        if (pos.x < 0 || pos.x >= pc.fWidth || pos.y < 0 || pos.y >= pc.fHeight || distance(pos, intervalEnd) < 0.1) {
            break;
        }
    }
//    accumulatedRadiance /= sampleCount;
    return accumulatedRadiance;
}

float IntervalScale(int cascadeIndex){
    if(cascadeIndex<= 0){
        return 0.0;
    }
    return float(1 << (2 * cascadeIndex));
}

vec2 IntervalRange(int cascadeIndex, float baseLenght){
    return baseLenght * vec2(IntervalScale(cascadeIndex), IntervalScale(cascadeIndex + 1));
}


void main() {
    float aspect = float(pc.fWidth)/float(pc.fHeight);
    ivec2 fSize = ivec2(pc.fWidth, pc.fHeight);
    vec2 normalizedTextCoords = vec2(textCoord.x, textCoord.y);
    ivec2 coord = ivec2(gl_FragCoord.xy);
    
    int intervalGrid= pc.intervalCount;
    int gridSize = pc.probeSizePx;
    //debug
   
    for(int i= 0; i < CASCADE_SIZE; i++ ){

        vec2 pos = vec2(gl_FragCoord.xy) / vec2(fSize);
        vec2 cascades = texture(Cascades[i], pos).xy;
        uint dirCount = intervalGrid * intervalGrid;
        ivec2 textIdx = ivec2(cascades.xy * float(intervalGrid));
        int dirIndex = textIdx.x + textIdx.y * intervalGrid;
        
        float angle = 2.0 * PI * ((float(dirIndex) + 0.5) / float(dirCount));
        vec2 texelDir = vec2(cos(angle), sin(angle));

        vec2 probeCenterGrid = floor(vec2(gl_FragCoord.xy) / float(gridSize)) + vec2(0.5);

        vec2 probePos = (probeCenterGrid * (gridSize));

        if(distance(probePos, vec2(gl_FragCoord.xy)) < 0.1){
            continue;
        }
        vec2 intervalRange = IntervalRange(i, pc.baseIntervalLength);
        vec2 intervalStart = probePos + texelDir * intervalRange.x;
        vec2 intervalEnd = probePos + texelDir * intervalRange.y;
        vec4 radiance = CastInterval(intervalStart, intervalEnd, i, vec2(fSize));
        
        ivec2 dirPos = ivec2((probeCenterGrid - vec2(0.5)) * gridSize) + ivec2(dirIndex % gridSize, dirIndex / gridSize);
        vec2 dirCol = vec2(dirPos) / vec2(fSize);

        imageStore(Radiances[i], ivec2(dirPos), radiance);

        intervalGrid *= 2;
        gridSize *= 2;
    }

    

}