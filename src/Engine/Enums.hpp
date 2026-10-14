//
// Created by carlo on 2025-01-14.
//

#ifndef ENUMS_HPP
#define ENUMS_HPP
namespace engine{


    //Sync
    enum struct queue_family_types
    {
        graphics,
        transfer,
        compute,
        present,
        undefined
      };

    enum layout_patterns
    {
        graphics_read,
        graphics_write,
        compute,
        compute_write,
        transfer_src,
        transfer_dst,
        color_attachment,
        depth_attachment,
        present,
        empty,
    };

    enum buffer_usage_types
    {
        b_vertex_buffer,
        b_graphics_write,
        b_compute_write,
        b_transfer_dst,
        b_transfer_src,
        b_draw_indirect,
        b_empty
    };
    ///

    //Pipeline
    enum blend_configs
    {
        b_none,
        b_opaque,
        b_add,
        b_mix,
        b_alpha_blend
    };

    enum depth_configs
    {
        d_none,
        d_enable,
        d_disable
    };

    enum rasterization_configs
    {
        r_fill,
        r_line,
        r_point
    };

    enum topology_configs
    {
       t_triangle,
       t_point_list,
       t_patch_list,
    };
    ///


}

#endif //ENUMS_HPP
