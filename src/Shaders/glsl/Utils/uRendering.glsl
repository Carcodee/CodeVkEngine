#ifndef R_UTIL 
#define R_UTIL 

#define ALBEDO_OFFSET 0
#define NORMAL_OFFSET 1
#define EMISSION_OFFSET 2
#define TRANSMISSION_OFFSET 3
#define ROUGHNESS_OFFSET 4
#define METALLIC_OFFSET 5
#define METALLIC_ROUGHNESS_OFFSET 6
#define AO_OFFSET 7
#define HEIGHT_OFFSET 8
        

vec3 u_ScreenToWorldNDC(mat4 invProj, mat4 invView, float depth, vec2 screenPos){
    vec4 ndcPos = vec4(screenPos, depth, 1.0);
    
    vec4 viewPos = invProj * ndcPos;
    viewPos = viewPos / viewPos.w;
    
    vec4 worldPos = invView * viewPos;
    return vec3(worldPos.xyz);
}
vec3 u_ScreenToWorld(mat4 invProj, mat4 invView, float depth, vec2 screenPos){
    vec4 ndcPos = vec4(1.0f);
    ndcPos.x = 2 * screenPos.x - 1.0f;
    ndcPos.y = 2 * screenPos.y - 1.0f;
    ndcPos.z = depth;

    vec4 viewPos = invProj * ndcPos;
    viewPos = viewPos / viewPos.w;

    vec4 worldPos = invView * viewPos;
    return vec3(worldPos.xyz);
}

vec3 u_ScreenToView(mat4 invProj, float depth, vec2 screenPos, vec2 screenSize){
    vec4 ndcPos = vec4(1.0f);
    ndcPos.x = 2 * (screenPos.x/screenSize.x) - 1.0f;
    ndcPos.y = 2 * (screenPos.y/screenSize.y) - 1.0f;
    ndcPos.z = depth;

    vec4 viewPos = invProj * ndcPos;
    viewPos = viewPos / viewPos.w;

    return vec3(viewPos.xyz);
}

vec4 u_ScreenToViewNDC(mat4 invProj, float depth, vec2 ndcCoords){
    vec4 ndcPos = vec4(ndcCoords, depth, 1.0);
    vec4 viewPos = invProj * ndcPos;
    viewPos = viewPos / viewPos.w;
    return viewPos;
}
vec4 u_ScreenToView(mat4 invProj, float depth, vec2 screenPos){
    vec4 ndcPos = vec4(1.0f);
    ndcPos.x = 2 * screenPos.x - 1.0f;
    ndcPos.y = 2 * screenPos.y - 1.0f;
    ndcPos.z = depth;

    vec4 viewPos = invProj * ndcPos;
    viewPos = viewPos / viewPos.w;

    return viewPos;
}

//linear depth
float u_GetZSlice(float Z, float near, float far, float numSlices) {
    return max(log2(Z) * numSlices / log2(far / near) - numSlices * log2(near) / log2(far / near), 0.0);
}


vec2 u_GetSpriteCoordInAtlas(int frameIndex, ivec2 spriteSizePx, int rows, int cols, ivec2 fragPos, ivec2 fSize){
    ivec2 frameIndexInAtlas = ivec2(frameIndex % cols, frameIndex / cols);
    
    vec2 gridSizePx = vec2(fSize) / vec2(spriteSizePx);
    vec2 spritePixelPos = floor(vec2(fragPos) / gridSizePx); 
    vec2 spriteBaseIndexPos = frameIndexInAtlas * vec2(spriteSizePx);
    ivec2 finalPos = ivec2((spriteBaseIndexPos + spritePixelPos));
    vec2 spriteSize =  vec2(spriteSizePx) * vec2(cols, rows);
    
    return vec2(finalPos) / spriteSize;
}

#endif 