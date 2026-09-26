#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable

layout(location = 0) in vec2 fragUV;          // Input UV coordinates from vertex shader
layout(location = 0) out vec4 outColor;       // Output color

layout(set = 0, binding = 0) uniform sampler2D MainTex;

void main() {
	 outColor = texture(MainTex, fragUV);
}
