﻿#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 textCoord;

void main() {
    gl_Position = vec4(pos, 0.0f, 1.0f);
    textCoord = uv;
}