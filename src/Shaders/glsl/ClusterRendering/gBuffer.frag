#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable
#extension GL_EXT_debug_printf : enable

#include "../Utils/uStructs.glsl"

layout(location = 0) out vec4 colors;
layout(location = 1) out vec4 normals;
layout(location = 2) out vec4 metRoughnessAttachment;

layout(location = 0) in vec2 textCoord;
layout(location = 1) in vec3 norm;
layout(location = 2) in flat int id;

layout (set = 0, binding = 0) uniform sampler2D textures[];

layout (set = 0, binding = 1, scalar) readonly  buffer MaterialsPacked{
    u_MaterialPacked[] materialsPacked;
};
layout (set = 0, binding = 2) readonly buffer MeshMaterialsIds{
    int[] meshMatIds;
};

void GetTexture(int offset, out vec4 value){
    value = vec4(0);
    if(offset > -1){
        value = texture(textures[offset], textCoord);
    }
}

void UnpackMaterial(out u_MaterialPacked mat){
    int matId = meshMatIds[id];
    mat= materialsPacked[matId];
}

void main() {
    u_MaterialPacked material;
    UnpackMaterial(material);
    
    vec4 albedo; 
    GetTexture(material.albedoOffset, albedo);
    if(albedo ==vec4(0)){albedo = material.diff;}
    
    vec4 normal;
    GetTexture(material.normalOffset, normal);
    if(normal ==vec4(0)){normal = vec4(norm, 1.0);}

    vec4 metRoughness;
    GetTexture(material.metRoughnessOffset, metRoughness);
    if(metRoughness == vec4(0)){metRoughness = vec4(0.0,material.roughnessFactor, material.metallicFactor , 1.0);}


    colors = albedo;
    normals = vec4(norm, 1.0);
    metRoughnessAttachment = metRoughness;
    
}