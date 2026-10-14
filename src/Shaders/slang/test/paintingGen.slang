struct Push_Constants {
    float4 color;
    int layer_selected;
    int painting;
    int x_mouse_coord;
    int y_mouse_coord;
    int radius;
};

[vk::push_constant]
cbuffer Push_Constants_Buffer{
    Push_Constants pc;
}

// Declare textures for painting layers
[[vk::binding(0, 0)]]
RWTexture2D<float4> painting_layers[];
[[vk::binding(1, 0)]]
Sampler2D in_tex;

// Entry point for compute shader
[numthreads(1, 1, 1)]
void Main_Cs(uint3 dtid : SV_DispatchThreadID) {
  // Calculate position
  uint2 xy_screen = int2(dtid.x, dtid.y);
    int pos_x = int(dtid.x - float(pc.radius) / 2.0 + 1);
    int pos_y = int(dtid.y - float(pc.radius) / 2.0 + 1);

    int x_coord = pc.x_mouse_coord - 1 + pos_x;
    int y_coord = pc.y_mouse_coord - 1 + pos_y;

    // Direction and squared distance calculation
    float2 dir = float2(x_coord, y_coord) - float2(pc.x_mouse_coord, pc.y_mouse_coord);
    float squared_d = pow(2.0, dir.x) + pow(2.0, dir.y);

    // Check radius condition and paint
    if (length(float2(x_coord, y_coord) - float2(pc.x_mouse_coord, pc.y_mouse_coord)) < float(pc.radius) / 2.0) {
        int2 xy = int2(x_coord, y_coord);
        bool painting = (pc.painting == 1);
        if (painting && pc.layer_selected == 0) {
            painting_layers[0][xy] = pc.color;
        }
    }
}