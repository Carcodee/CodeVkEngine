#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uStructs.glsl"

layout(location = 0) out vec4 colors;
layout(location = 1) out vec4 normals;
layout(location = 2) out vec4 tangents;
layout(location = 3) out vec4 met_roughness_attachment;
layout(location = 4) out vec4 uv_coords;

layout(location = 0) in vec2 text_coord;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 tang;
layout(location = 3) in flat int id;
layout(location = 4) in vec2 screen_uv;

layout (set = 0, binding = 0) uniform sampler2D textures[];

layout (set = 0, binding = 1, scalar) readonly  buffer materials_packed_buffer{
    u_material_packed[] materials_packed;
};
layout (set = 0, binding = 2) readonly buffer mesh_materials_ids{
    int[] mesh_mat_ids;
};

void get_texture(int offset, out vec4 value){
    value = vec4(0);
    if(offset > -1){
        value = texture(textures[offset], text_coord);
    }
}

void unpack_material(out u_material_packed mat){
    int mat_id = mesh_mat_ids[id];
    mat = materials_packed[mat_id];
}

void main() {
    u_material_packed material;
    int issd =0;
    unpack_material(material);
    
    vec4 albedo; 
    get_texture(material.albedo_offset, albedo);
    if(albedo ==vec4(0)){albedo = material.diff;}
    
    
    vec4 material_normal;
    get_texture(material.normal_offset, material_normal);
    
    if(material_normal ==vec4(0)){
//        material_normal = vec4(norm, 1.0);
    }else{
        vec3 bit_tang = normalize(cross(tang.xyz, norm.xyz));
        mat3 tbn = mat3(tang.xyz, bit_tang, norm.xyz);
        material_normal.xyz = tbn * material_normal.xyz;
    }

    vec4 met_roughness;
    get_texture(material.met_roughness_offset, met_roughness);
    if(met_roughness == vec4(0)){met_roughness = vec4(0.0,material.roughness_factor, material.metallic_factor, 1.0);}

    colors = albedo;
    normals = vec4(material_normal);
    tangents = vec4(tang, 1.0);
    met_roughness_attachment = met_roughness;
    uv_coords = vec4(screen_uv, 0.0, 1.0);
    
}