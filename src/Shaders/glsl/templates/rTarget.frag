#version 450

layout(location = 0) in vec2 fragUV;          // Input UV coordinates from vertex shader
layout(location = 0) out vec4 outColor;       // Output color

void main() {
    outColor = vec4(fragUV, 0.0, 1.0);
}