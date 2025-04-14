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
mat3 u_GetCov3D(vec4 rots, vec3 scales, float scaleMod){

    vec3 firstRow = vec3(
    1.f - 2.f * (rots.z * rots.z + rots.w * rots.w),
    2.f * (rots.y * rots.z - rots.x * rots.w),
    2.f * (rots.y * rots.w + rots.x * rots.z)
    );

    vec3 secondRow = vec3(
    2.f * (rots.y * rots.z + rots.x * rots.w),
    1.f - 2.f * (rots.y * rots.y + rots.w * rots.w),
    2.f * (rots.z * rots.w - rots.x * rots.y)
    );

    vec3 thirdRow = vec3(
    2.f * (rots.y * rots.w - rots.x * rots.z),
    2.f * (rots.z * rots.w + rots.x * rots.y),
    1.f - 2.f * (rots.y * rots.y + rots.z * rots.z)
    );


    mat3 scaleMatrix = mat3(
    scaleMod * scales.x, 0, 0,
    0, scaleMod * scales.y, 0,
    0, 0, scaleMod * scales.z
    );

    mat3 rotMatrix = mat3(
    firstRow,
    secondRow,
    thirdRow
    );

    mat3 mMatrix = scaleMatrix * rotMatrix;

    mat3 sigma = transpose(mMatrix) * mMatrix;
    return sigma; 
}
vec3 u_GetCov2D(vec4 mean_view, float focal_x, float focal_y, float tan_fovx, float tan_fovy, mat3 cov3D, mat4 viewmatrix)
{
    vec4 t = mean_view;
    // why need this? Try remove this later
    float limx = 1.3f * tan_fovx;
    float limy = 1.3f * tan_fovy;
    float txtz = t.x / t.z;
    float tytz = t.y / t.z;
    t.x = min(limx, max(-limx, txtz)) * t.z;
    t.y = min(limy, max(-limy, tytz)) * t.z;

    mat3 J = mat3(
    focal_x / t.z, 0.0f, -(focal_x * t.x) / (t.z * t.z),
    0.0f, focal_y / t.z, -(focal_y * t.y) / (t.z * t.z),
    0, 0, 0
    );
    mat3 W = transpose(mat3(viewmatrix));
    mat3 T = W * J;

    mat3 cov = transpose(T) * transpose(cov3D) * T;
    // Apply low-pass filter: every Gaussian should be at least
    // one pixel wide/high. Discard 3rd row and column.
    cov[0][0] += 0.3f;
    cov[1][1] += 0.3f;
    return vec3(cov[0][0], cov[0][1], cov[1][1]);
}

#endif 