//
// Created by carlo on 2025-01-14.
//

#ifndef ENUMS_HPP
#define ENUMS_HPP
namespace engine{


    //Sync
    enum struct Queue_Family_Types
    {
        Graphics,
        Transfer,
        Compute,
        Present,
        Undefined
      };

    enum Layout_Patterns
    {
        Graphics_Read,
        Graphics_Write,
        Compute,
        Compute_Write,
        Transfer_Src,
        Transfer_Dst,
        Color_Attachment,
        Depth_Attachment,
        Present,
        Empty,
    };

    enum Buffer_Usage_Types
    {
        B_Vertex_Buffer,
        B_Graphics_Write,
        B_Compute_Write,
        B_Transfer_Dst,
        B_Transfer_Src,
        B_Draw_Indirect,
        B_Empty
    };
    ///

    //Pipeline
    enum Blend_Configs
    {
        B_None,
        B_Opaque,
        B_Add,
        B_Mix,
        B_Alpha_Blend
    };

    enum Depth_Configs
    {
        D_None,
        D_Enable,
        D_Disable
    };

    enum Rasterization_Configs
    {
        R_Fill,
        R_Line,
        R_Point
    };

    enum Topology_Configs
    {
       T_Triangle,
       T_Point_List,
       T_Patch_List,
    };
    ///


}

#endif //ENUMS_HPP
