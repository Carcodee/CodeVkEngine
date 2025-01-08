#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 textCoord;


layout(set = 0, binding = 0) uniform sampler2D testImage;

layout (set = 0, binding = 1, rgba8) uniform image2D storageImg;

layout (set = 0, binding = 2) uniform sampler2D textures[];

layout(set = 0, binding = 3, rgba32f) uniform image2D storagesImgs[];

layout(set = 0, binding = 4, std140) uniform Camera{
    mat4 model;
    mat4 projView;
}cPropsUniform;


void main() {

    ivec2 coord = ivec2(gl_FragCoord.xy);
    vec4 computeImage= imageLoad(storageImg, coord);
    vec4 text = texture(testImage, textCoord);

    vec4 textureArr = texture(textures[0], textCoord);
    vec4 storageArr = imageLoad(storagesImgs[0], coord);
    
    outColor = computeImage;
}