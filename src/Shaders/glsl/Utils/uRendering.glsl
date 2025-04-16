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
mat3 u_RotationFromQuaternion(vec4 q) {
    float qx = q.y;
    float qy = q.z;
    float qz = q.w;
    float qw = q.x;

    float qx2 = qx * qx;
    float qy2 = qy * qy;
    float qz2 = qz * qz;

    mat3 rotationMatrix;
    rotationMatrix[0][0] = 1 - 2 * qy2 - 2 * qz2;
    rotationMatrix[0][1] = 2 * qx * qy - 2 * qz * qw;
    rotationMatrix[0][2] = 2 * qx * qz + 2 * qy * qw;

    rotationMatrix[1][0] = 2 * qx * qy + 2 * qz * qw;
    rotationMatrix[1][1] = 1 - 2 * qx2 - 2 * qz2;
    rotationMatrix[1][2] = 2 * qy * qz - 2 * qx * qw;

    rotationMatrix[2][0] = 2 * qx * qz - 2 * qy * qw;
    rotationMatrix[2][1] = 2 * qy * qz + 2 * qx * qw;
    rotationMatrix[2][2] = 1 - 2 * qx2 - 2 * qy2;

    return rotationMatrix;
}
mat3 u_GetCov3D(vec4 rots, vec3 scales, float scaleMod){

    
    mat3 scaleMatrix = mat3(
    scaleMod * scales.x, 0, 0,
    0, scaleMod * scales.y, 0,
    0, 0, scaleMod * scales.z 
    );
    mat3 rotMatrix = u_RotationFromQuaternion(rots);

    mat3 mMatrix = scaleMatrix * rotMatrix;

    mat3 sigma = transpose(mMatrix) * mMatrix;
    
    return sigma; 
}
mat2 u_GetCov2D(vec4 mean_view, float focal_x, float focal_y, float tan_fovx, float tan_fovy, mat3 cov3D, mat4 viewmatrix)
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
    return mat2(cov);
}

#endif 