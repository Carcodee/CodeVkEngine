#version 450

layout(location = 0) out vec4 out_color;
layout(location = 0) in vec2 text_coord;


layout(set = 0, binding = 0) uniform sampler2D test_image;

layout (set = 0, binding = 1, rgba8) uniform image2D storage_img;

layout (set = 0, binding = 2) uniform sampler2D textures[];

layout(set = 0, binding = 3, rgba32f) uniform image2D storages_imgs[];

layout(set = 0, binding = 4, std140) uniform camera{
    mat4 model;
    mat4 proj_view;
}c_props_uniform;


void main() {

    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec4 compute_image= imageLoad(storage_img, coord);
    vec4 text = texture(test_image, text_coord);

    vec4 texture_arr = texture(textures[0], text_coord);
    vec4 storage_arr = imageLoad(storages_imgs[0], coord);
    
    out_color = compute_image;
}