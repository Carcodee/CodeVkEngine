#ifndef R_STRUCTS 
#define R_STRUCTS 

//Lights

struct u_PointLight{
    vec3 pos;
    vec3 col;
    float radius;
    float intensity;
    float lAttenuation;
    float qAttenuation;
};

struct u_DirLight{
    vec3 pos;
    vec3 col;
    float intensity;
};

//End Lights

//Util

struct u_ArrayIndexer{
    int offset;
    int size;
};

struct u_Frustum{
    vec4 planes[6];
    vec3 points[8];
};

struct u_MaterialPacked{
    vec4 diff;
    float albedoFactor;
    float normalFactor;
    float roughnessFactor;
    float metallicFactor;
    float alphaCutoff;

    int albedoOffset;
    int normalOffset;
    int emissionOffset;
    int transOffset;
    int roughnessOffset;
    int metallicOffset;
    int metRoughnessOffset;
};

struct u_DrawIndirectIndexedCmd
{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    uint vertexOffset;
    uint firstInstance;
};

struct u_Sphere{
    vec3 center;
    float radius;
};

struct u_MergeCascadesInfo{
    ivec2 probeCentersPositionsPx;
    ivec2 probeSizesPx;
    int dirIndices;
};

struct u_SpriteAnimationInfo{
    ivec2 spriteSizePx;
    int rows;
    int cols;
    int currentFrame;
    int frameCount;
    float interpVal;
    bool isAtlas;
};
struct u_RadianceCascadesConfigs
{
    int radiancePow;
    int normalMapPow;
    int specularPow;
    int roughnessPow;
};

//End Util

#endif 