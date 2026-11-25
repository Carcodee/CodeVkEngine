#version 450

layout(location = 0) in vec3 in_local_pos;
layout(location = 1) flat in uint in_style;
layout(location = 2) flat in uvec2 in_slice_dimensions;

layout(location = 0) out vec4 out_color;

float line_coverage(float distance_to_line, float radius) {
    return 1.0 - smoothstep(radius - 0.5, radius + 0.5, distance_to_line);
}

void main() {
    const uint bounds_style = 0u;
    const uint slice_style = 1u;
    const uint selected_voxel_style = 3u;
    const vec3 bounds_color = vec3(0.32, 0.48, 0.63);
    const vec3 slice_color = vec3(0.08, 0.66, 0.80);
    const vec3 occupied_color = vec3(0.98, 0.40, 0.085);

    vec3 pixel_width = max(fwidth(in_local_pos), vec3(1e-6));
    vec3 edge_distance = max(0.5 - abs(in_local_pos), 0.0) / pixel_width;

    if (in_style == slice_style) {
        // The cube is flattened onto the selected layer. Only one cap contributes.
        if (in_local_pos.z < 0.0) {
            discard;
        }
        vec2 dimensions = vec2(in_slice_dimensions);
        vec2 stride = ceil(dimensions / 24.0);
        vec2 grid_pos = (in_local_pos.xy + 0.5) * dimensions / stride;
        vec2 grid_width = max(pixel_width.xy * dimensions / stride, vec2(1e-6));
        vec2 grid_distance = abs(grid_pos - round(grid_pos)) / grid_width;
        vec2 grid_alpha = (1.0 - smoothstep(0.0, 1.0, grid_distance)) *
                          (1.0 - smoothstep(0.12, 0.35, grid_width));
        float border_alpha = line_coverage(min(edge_distance.x, edge_distance.y), 1.5);
        float fill_alpha = 0.07 + 0.21 * max(grid_alpha.x, grid_alpha.y);
        out_color = vec4(slice_color, mix(fill_alpha, 0.82, border_alpha));
        return;
    }

    // One component is zero on each cube face. The next nearest side marks a real edge.
    float cube_edge = min(max(edge_distance.x, edge_distance.y),
                          min(max(edge_distance.x, edge_distance.z), max(edge_distance.y, edge_distance.z)));
    float outline = line_coverage(cube_edge, 0.95);
    if (in_style == bounds_style) {
        if (outline == 0.0) {
            discard;
        }
        out_color = vec4(bounds_color, outline * 0.44);
    } else {
        if (!gl_FrontFacing) {
            discard;
        }
        bool on_slice = in_style == selected_voxel_style;
        float fill_alpha = on_slice ? 0.22 : 0.08;
        float edge_alpha = on_slice ? 0.92 : 0.53;
        out_color = vec4(occupied_color, mix(fill_alpha, edge_alpha, outline));
    }
}
