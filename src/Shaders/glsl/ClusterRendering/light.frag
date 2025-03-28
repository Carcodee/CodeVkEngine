﻿#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable


#include "../Utils/uRendering.glsl"
#include "../Utils/uStructs.glsl"
#include "../Utils/uMath.glsl"
#include "../Utils/uPBR.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 textCoord;

layout(set = 0, binding = 0) uniform sampler2D gCol;
layout(set = 0, binding = 1) uniform sampler2D gNormals;
layout(set = 0, binding = 2) uniform sampler2D gTang;
layout(set = 0, binding = 3) uniform sampler2D gDepth;
layout(set = 0, binding = 4) uniform sampler2D gMetRoughness;

layout(set = 0, binding = 5, scalar) uniform CameraProperties{
    mat4 invProj;
    mat4 invView;
    vec3 pos;
    float zNear;
    float zFar;
}cProps;
layout (set = 0, binding = 6, scalar) buffer PointLights{
    u_PointLight[] pointLights;
};
layout (set = 0, binding = 7, scalar) buffer LightMap{
    u_ArrayIndexer[] lightMap;
};
layout (set = 0, binding = 8, scalar) buffer LightIndices{
    int[] lightIndices;
};

layout(set = 0, binding = 9) writeonly uniform image2D samplerMap;

layout (set = 0, binding = 10, scalar) writeonly buffer cameraPositions{
    vec3[] camPositions;
};

layout(push_constant)uniform pushConstants{
    uint tileCountX;
    uint tileCountY;
    int xTileSizePx;
    int yTileSizePx;
    int zSlicesSize;
}pc;
vec3 EvalPointLight(u_PointLight light, vec3 col, vec3 pos, vec3 normal){
    float d = u_SDF_Sphere(light.pos, pos);
    if(d < light.radius){
        vec3 lightDir = normalize(light.pos - pos);
        float diff = max(0.00, dot(lightDir, normal));
        float attenuation = 1.0 / (1.0 + (light.lAttenuation * d) + (light.qAttenuation * (d * d)));
        
        vec3 finalCol = col * diff *light.col * attenuation * light.intensity;
        return finalCol;
    }
    return vec3(0.0);
}

void main() {
    
    vec4 norm = texture(gNormals, textCoord);
    vec4 metRoughness = texture(gMetRoughness, textCoord);
    vec4 tangs = texture(gTang, textCoord);
    
    vec2 fragCoord = vec2(textCoord.x , textCoord.y);
    vec4 col = texture(gCol, textCoord);
    float depth = texture(gDepth, textCoord).r;
    if(norm == vec4(0.0)){

        discard;
    }
    
//    col =vec4(0.01);
    vec3 pos = u_ScreenToWorld(cProps.invProj, cProps.invView, depth, fragCoord);

    ivec2 tileId = ivec2(gl_FragCoord.xy/uvec2(pc.xTileSizePx, pc.yTileSizePx));
    
    
    vec4 linearDepth = cProps.invProj * vec4(0.0, 0.0, depth, 1.0);
    linearDepth.z = linearDepth.z / linearDepth.w;

    int zId =int(u_GetZSlice(abs(linearDepth.z), cProps.zNear, cProps.zFar, float(pc.zSlicesSize)));
    
    uint mapIndex = tileId.x + (tileId.y * pc.tileCountX) + (zId * pc.tileCountX * pc.tileCountY);

    int lightOffset = lightMap[mapIndex].offset;
    int lightsInTile = lightMap[mapIndex].size;
    
    vec3 lightPos = vec3(5.0, 10.0, 0.0);
    vec3 lightDir = normalize(pos - lightPos) ;
    vec3 viewDir = normalize(pos - cProps.pos) ;
    
    vec3 halfway = normalize(lightDir + viewDir);
    
    vec3 lightCol = vec3(1.0, 1.0, 1.0);
    vec3 ambientCol = vec3(0.0, 0.0, 0.0);
    vec3 finalCol = col.xyz;

    for (int i = 0; i < lightsInTile; i++) {
        int lightIndex = lightIndices[lightOffset + i];
//        finalCol += EvalPointLight(pointLights[lightIndex], finalCol, pos, norm.xyz);
    };
//    if(true){
//        float intensityId= u_InvLerp(0.0, pc.tileCountX * pc.tileCountY * float(pc.zSlicesSize), float(mapIndex));
//
//        float hue = intensityId;
//        float saturation = 0.8;
//        float lightness = 0.4;
//        vec3 tileCol = u_HSLToRGB(hue, saturation, lightness);
//
//        float intensity= u_InvLerp(0.0, 400.0 , float(lightsInTile));
//        vec3 debugCol = u_Lerp(vec3(0.0, 0.5, 0.4), vec3(1.0, 0.0, 0.0), intensity);
//         finalCol += debugCol*2 + tileCol * 0.3;
//    }
    
    //fix rough maps
    vec3 brdf = u_GetBRDF(norm.xyz, viewDir, lightDir, halfway, finalCol, vec3(0, 0.0, 0.0), metRoughness.b, metRoughness.g);
    brdf = brdf * lightCol * AbsCosThetaWs(lightDir, norm.xyz) * 2.0 ;
    
    outColor = vec4(brdf.xyz, 1.0);

}