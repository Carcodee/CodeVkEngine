//
// Created by carlo on 2025-01-14.
//

#ifndef ENUMS_HPP
#define ENUMS_HPP
namespace ENGINE{


    //Sync
    enum struct QueueFamilyTypes
    {
        GRAPHICS,
        TRANSFER,
        COMPUTE,
        PRESENT,
        UNDEFINED
      };

    enum LayoutPatterns
    {
        GRAPHICS_READ,
        GRAPHICS_WRITE,
        COMPUTE,
        COMPUTE_WRITE,
        TRANSFER_SRC,
        TRANSFER_DST,
        COLOR_ATTACHMENT,
        DEPTH_ATTACHMENT,
        PRESENT,
        EMPTY,
    };

    enum BufferUsageTypes
    {
        B_VERTEX_BUFFER,
        B_GRAPHICS_WRITE,
        B_COMPUTE_WRITE,
        B_TRANSFER_DST,
        B_TRANSFER_SRC,
        B_DRAW_INDIRECT,
        B_EMPTY
    };
    ///

    //Pipeline
    enum BlendConfigs
    {
        B_NONE,
        B_OPAQUE,
        B_ADD,
        B_MIX,
        B_ALPHA_BLEND
    };

    enum DepthConfigs
    {
        D_NONE,
        D_ENABLE,
        D_DISABLE
    };

    enum RasterizationConfigs
    {
        R_FILL,
        R_LINE,
        R_POINT
    };
    ///


}

#endif //ENUMS_HPP
