#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout :enable

// Vertex Output
//layout(location = 0) in vec2 ellipseAxisX;
//layout(location = 1) in vec2 ellipseAxisY;
//layout(location = 2) in float alpha;
//layout(location = 3) in vec3 col;
//layout(location = 4) in vec2 textCoord;

layout(location = 0) in vec2 coordxy;
layout(location = 1) in float alpha;
layout(location = 2) in vec3 col;
layout(location = 3) in vec2 textCoord;
layout(location = 4) in vec3 conic;


layout(location = 0) out vec4 color;

void main()
{
        float power = -0.5f * (conic.x * coordxy.x * coordxy.x + conic.z * coordxy.y * coordxy.y) - conic.y * coordxy.x * coordxy.y;
        float T = 1.0f;
        if(power > 0.0f) {
//                color = vec4(1.0);
//                return;
                discard;
        };
        float myAlpha = min(0.99f, alpha * exp(power));
        if(myAlpha < 1.f / 255.f){
//                color = vec4(1.0);
//                return;
                discard;
        };
        float testT = T * (1 - myAlpha);
        if(testT < 0.0001f){
//                color = vec4(1.0);
//                return;
                discard;
        }
        color = vec4(col ,myAlpha);

}