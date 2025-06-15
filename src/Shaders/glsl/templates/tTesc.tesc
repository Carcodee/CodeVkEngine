#version 450

layout(vertices = 3) out; // triangle patch with 3 control points

void main() {
    // Pass vertex data through
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Set tessellation levels only once
    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 4.0;
        gl_TessLevelOuter[1] = 4.0;
        gl_TessLevelOuter[2] = 4.0;
        gl_TessLevelInner[0] = 4.0;
    }
}