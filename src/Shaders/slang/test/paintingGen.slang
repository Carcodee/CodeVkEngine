struct PushConstants {
    float4 color;
    int layerSelected;
    int painting;
    int xMouseCoord;
    int yMouseCoord;
    int radius;
};

[vk::push_constant]
cbuffer PushConstantsBuffer{
    PushConstants pc;
}

// Declare textures for painting layers
[[vk::binding(0, 0)]]
RWTexture2D<float4> PaintingLayers[2];

// Entry point for compute shader
[numthreads(1, 1, 1)]
void mainCS(uint3 dtid : SV_DispatchThreadID) {
    // Calculate position
    int posX = int(dtid.x - float(pc.radius) / 2.0 + 1);
    int posY = int(dtid.y - float(pc.radius) / 2.0 + 1);

    int xCoord = pc.xMouseCoord - 1 + posX;
    int yCoord = pc.yMouseCoord - 1 + posY;

    // Direction and squared distance calculation
    float2 dir = float2(xCoord, yCoord) - float2(pc.xMouseCoord, pc.yMouseCoord);
    float squaredD = pow(2.0, dir.x) + pow(2.0, dir.y);

    // Check radius condition and paint
    if (length(float2(xCoord, yCoord) - float2(pc.xMouseCoord, pc.yMouseCoord)) < float(pc.radius) / 2.0) {
        int2 xy = int2(xCoord, yCoord);
        bool painting = (pc.painting == 1);
        if (painting && pc.layerSelected == 0) {
            PaintingLayers[0][xy] = pc.color;
        }
        if (painting && pc.layerSelected == 1) {
            PaintingLayers[1][xy] = float4(1.0, 1.0, 1.0, 1.0);
        }
    }
}