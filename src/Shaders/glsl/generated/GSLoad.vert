#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : enable 

#include "../Utils/uRendering.glsl"

layout(set = 0, binding = 0, scalar) buffer GSScale {
    vec3 gsScale[];
};
layout(set = 0, binding = 1, scalar) buffer GSRot {
    vec4 gsRot[];
};
layout(set = 0, binding = 2, scalar) buffer GSPos {
    vec3 gsPos[];
};
layout(set = 0, binding = 3, scalar) buffer GSCols {
    vec3 gsCols[];
};

layout(set = 0, binding = 4, scalar) buffer GSAlphas {
    float gsAlphas[];
};

layout(set = 0, binding = 5, scalar) uniform HFov{
    vec3 hFov;
}camHFov;

layout(set = 0, binding = 6, scalar) uniform GSConfigs{
    float scaleMod;
}gsConfigs;


layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
    mat4 proj;
} pc;

// Vertex Input
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

// Vertex Output
layout(location = 0) out vec2 coordxy;
layout(location = 1) out float alpha;
layout(location = 2) out vec3 col;
layout(location = 3) out vec2 textCoord;
layout(location = 4) out vec3 conic;

void main() {

    int id = gl_InstanceIndex;
//    if(gl_InstanceIndex != 5){
//        gl_Position = vec4(-100.0, -100.0, -100.0, 1.0); // or return;
//        return;
//    }
    textCoord = uv;
    
    vec3 pCloudPos = gsPos[id];
    vec3 scaleVal = abs(gsScale[id]);
    vec4 rotVal = gsRot[id];
    float scale = gsConfigs.scaleMod;
    mat3 cov3d = u_GetCov3D(rotVal, scaleVal, scale);
    vec4 viewPos = pc.view * vec4(pCloudPos, 1.0);
    vec4 screenPos = pc.proj * viewPos;

    screenPos.xyz = screenPos.xyz / screenPos.w;
    screenPos.w = 1.f;
    
    vec2 wh = 2 * camHFov.hFov.xy * camHFov.hFov.z;

    // Cull
    if (any(greaterThan(abs(screenPos.xyz), vec3(1.3)))) {
        gl_Position = vec4(0, 0, 0, 1);
        return;
    }

    mat4 viewForCov = pc.view;
    viewForCov[0][1] *= -1.0f;
    viewForCov[1][1] *= -1.0f;
    viewForCov[2][1] *= -1.0f;
    viewForCov[3][1] *= -1.0f;
    viewForCov[0][2] *= -1.0f;
    viewForCov[1][2] *= -1.0f;
    viewForCov[2][2] *= -1.0f;
    viewForCov[3][2] *= -1.0f;

    mat2 cov2d = u_GetCov2D(viewPos, camHFov.hFov.z, camHFov.hFov.z, camHFov.hFov.x, camHFov.hFov.y, cov3d, viewForCov);
    float det = determinant(cov2d);
    
    if (det == 0.0f){
        gl_Position = vec4(-100.0, -100.0, -100.0, 1.0); // or return;
        return;
    }
    
    mat2 conicMat = inverse(cov2d);
    conic = vec3(conicMat[0][0], conicMat[0][1], conicMat[1][1]);
    
    vec3 myCov2d = vec3(cov2d[0][0], cov2d[0][1], cov2d[1][1]);

    // Project quad into screen space
    vec2 quadwh_scr = vec2(3.f * sqrt(myCov2d.x), 3.f * sqrt(myCov2d.z));  // screen space half quad height and width
    // Convert screenspace quad to NDC
    vec2 quadwh_ndc = quadwh_scr / wh * 2.0;
    // Update gaussian's position w.r.t the quad in NDC
    screenPos.xy = screenPos.xy + pos * quadwh_ndc;
    // Calculate where this quad lies in pixel coordinates 
    coordxy = pos * quadwh_scr;

    gl_Position = screenPos;
    
    alpha = gsAlphas[id];
    col = gsCols[id];
    

    
}
