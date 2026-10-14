#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : enable 

#include "../Utils/uRendering.glsl"
#include "../Utils/uMath.glsl"


layout(set = 0, binding = 0, scalar) buffer Gs_Scale_Buffer {
    vec3 gs_scale[];
};
layout(set = 0, binding = 1, scalar) buffer Gs_Rot_Buffer {
    vec4 gs_rot[];
};
layout(set = 0, binding = 2, scalar) buffer Gs_Pos_Buffer {
    vec3 gs_pos[];
};
layout(set = 0, binding = 3, scalar) buffer Gs_Cols_Buffer {
    vec3 gs_cols[];
};

layout(set = 0, binding = 4, scalar) buffer Gs_Alphas_Buffer {
    float gs_alphas[];
};

layout(set = 0, binding = 5, scalar) uniform H_Fov_Uniform{
    vec3 h_fov;
}cam_h_fov;

layout(set = 0, binding = 6, scalar) uniform Gs_Configs_Uniform{
    float scale_mod;
}gs_configs;

layout(set = 0, binding = 7, scalar) buffer Gs_Shs_Buffer {
    vec3 gs_shs[];
};


layout(push_constant) uniform Push_Constants {
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
layout(location = 3) out vec2 text_coord;
layout(location = 4) out vec3 conic;

void main() {

    int id = gl_InstanceIndex;
//    if(gl_InstanceIndex != 5){
//        gl_Position = vec4(-100.0, -100.0, -100.0, 1.0); // or return;
//        return;
//    }
    text_coord = uv;
    
    vec3 cam_pos = vec3(inverse(pc.view)[3]);
    vec3 p_cloud_pos = gs_pos[id];
    vec3 scale_val = abs(gs_scale[id]);
    vec4 rot_val = gs_rot[id];
    float scale = gs_configs.scale_mod;
    mat3 cov3d = U_Get_Cov_3d(rot_val, scale_val, scale);
    vec4 view_pos = pc.view * vec4(p_cloud_pos, 1.0);
    vec4 screen_pos = pc.proj * view_pos;

    screen_pos.xyz = screen_pos.xyz / screen_pos.w;
    screen_pos.w = 1.f;
    
    vec2 wh = 2 * cam_h_fov.h_fov.xy * cam_h_fov.h_fov.z;

    // Cull
    if (any(greaterThan(abs(screen_pos.xyz), vec3(1.3)))) {
        gl_Position = vec4(0, 0, 0, 1);
        return;
    }

    mat4 view_for_cov = pc.view;
    view_for_cov[0][1] *= -1.0f;
    view_for_cov[1][1] *= -1.0f;
    view_for_cov[2][1] *= -1.0f;
    view_for_cov[3][1] *= -1.0f;
    view_for_cov[0][2] *= -1.0f;
    view_for_cov[1][2] *= -1.0f;
    view_for_cov[2][2] *= -1.0f;
    view_for_cov[3][2] *= -1.0f;

    mat2 cov2d = U_Get_Cov_2d(view_pos, cam_h_fov.h_fov.z, cam_h_fov.h_fov.z, cam_h_fov.h_fov.x, cam_h_fov.h_fov.y, cov3d, view_for_cov);
    float det = determinant(cov2d);
    
    if (det == 0.0f){
        gl_Position = vec4(-100.0, -100.0, -100.0, 1.0); // or return;
        return;
    }
    
    mat2 conic_mat = inverse(cov2d);
    conic = vec3(conic_mat[0][0], conic_mat[0][1], conic_mat[1][1]);
    
    vec3 my_cov2d = vec3(cov2d[0][0], cov2d[0][1], cov2d[1][1]);

    // Project quad into screen space
    vec2 quadwh_scr = vec2(3.f * sqrt(my_cov2d.x), 3.f * sqrt(my_cov2d.z));  // screen space half quad height and width
    // Convert screenspace quad to NDC
    vec2 quadwh_ndc = quadwh_scr / wh * 2.0;
    // Update gaussian's position w.r.t the quad in NDC
    screen_pos.xy = screen_pos.xy + pos * quadwh_ndc;
    // Calculate where this quad lies in pixel coordinates 
    coordxy = pos * quadwh_scr;

    gl_Position = screen_pos;
    
    
    
    vec3 curr_sh_coeffs[MAX_SH_COEFFS];
    curr_sh_coeffs[0] = gs_cols[id];
    vec3 new_col = curr_sh_coeffs[0];
    int idx = 1;
    for(int i = id; i < id + 15; i++){
        curr_sh_coeffs[idx] = gs_shs[i];
        idx++;
    }
    vec3 ray_dir = normalize(p_cloud_pos - cam_pos);
    vec3 full_col = U_Compute_Sh(ray_dir,curr_sh_coeffs, 3);
    
    alpha = gs_alphas[id];
    
    col = full_col;
    
}
