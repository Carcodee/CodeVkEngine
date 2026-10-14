//
// Created by carlo on 2024-12-04.
//

#ifndef DEBUG_RENDERER_HPP
#define DEBUG_RENDERER_HPP

namespace rendering
{
    using namespace engine;

    class debug_renderer : public base_renderer
    {
    public:
        ~debug_renderer() override = default;

        debug_renderer(engine::core* core, ::window_provider* window_provider,
                      std::map<std::string, std::unique_ptr<base_renderer>>& renderers)
        {
            this->core = core;
            this->render_graph = core->render_graph_ref;
            this->window_provider = window_provider;
            if (renderers.contains("ClusterRenderer"))
            {
                this->cluster_renderer = dynamic_cast<rendering::cluster_renderer*>(renderers.at("ClusterRenderer").get());
            }

            if (renderers.contains("CFDRenderer"))
            {
                this->cfd_renderer = dynamic_cast<rendering::cfd_renderer*>(renderers.at("CFDRenderer").get());
            }

            create_resources();
            create_buffers();
            create_pipelines();
        }

        void create_resources()
        {
            auto image_info = engine::image::create_info2d(window_provider->get_window_size(), 1, 1,
                                                 vk::Format::eR32G32B32A32Sfloat,
                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eSampled);

            // image view not used right now
            //  col_attachment_view = resources_manager::get_instance()->get_image("debugColAttachment", image_info, 0, 0);
            set_view_camera();
        }

        void create_buffers()
        {
            raw_vertices_buff = resources_manager::get_instance()->get_buffer(
                resources_manager::buffer_params{
                    "debugRawVertices", vk::BufferUsageFlagBits::eVertexBuffer,
                    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                    1
                });

            auto* models_voxels = resources_manager::get_instance()->get_buffer(
                resources_manager::buffer_params{
                    "models_voxels", vk::BufferUsageFlagBits::eStorageBuffer,
                    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                    cfd_renderer->voxel_grid.pc_voxel_grid.get_size(), sizeof(glm::mat4)
                });
        }

        void create_pipelines()
        {
            std::string shader_path = systems::os::get_instance()->get_shaders_path();

            engine::attachment_info col_info = get_color_attachment_info(blend_configs::b_opaque,
                                                            glm::vec4(0.0f), core->swapchain_ref->get_format(),
                                                            vk::AttachmentLoadOp::eLoad);
            if (cluster_renderer)
            {
                auto model_v_shader = render_graph->resources_manager->get_shader(
                    shader_path + "\\spirvGlsl\\DebugRendering\\debug.vert.spv", s_vert);

                auto model_f_shader = render_graph->resources_manager->get_shader(
                    shader_path + "\\spirvGlsl\\DebugRendering\\debug.frag.spv", s_frag);

                auto render_node = render_graph->add_pass(m_debugger_pass_name);
                render_node->g_set_vert_shader(model_v_shader);
                render_node->g_set_frag_shader(model_f_shader);
                render_node->g_set_framebuffer_size(render_graph->current_back_buffer->image_metadata->get_image_size());
                render_node->g_set_push_constant_size(sizeof(mvp_pc));
                render_node->g_set_vertex_input( d_vertex_3d::get_vertex_input());
                render_node->g_add_color_attachment_output(0, col_info);
                render_node->g_set_color_image_attachment_binding(0, render_graph->current_back_buffer);
                render_node->g_set_graphics_pipeline_configs({rasterization_configs::r_line, topology_configs::t_triangle});
                render_node->g_set_depth_config(depth_configs::d_none);
                render_node->depends_on("light");
                render_node->build_render_graph_node();

                auto debug_task = new std::function<void()>([this]()
                {
                    set_view_camera();
                    create_frustum_vertices();
                    raw_vertices_buff = resources_manager::get_instance()->set_buffer(
                        "debugRawVertices", sizeof(d_vertex_3d) * raw_3d_vertices.size(), sizeof(d_vertex_3d),
                        raw_3d_vertices.data());
                });
                auto render_op = new std::function<void()>(
                    [this]()
                    {
                        vk::DeviceSize offset = 0;
                        auto node = render_graph->get_node(m_debugger_pass_name);

                        node->get_curr_cmd().bindVertexBuffers(0, 1, &raw_vertices_buff->buffer_handle.get(), &offset);

                        pc.proj_view = current_view_cam->get_view_proj();
                        pc.model = glm::mat4(1.0);

                        node->get_curr_cmd().pushConstants(node->gpu_pipeline_ref->pipeline_layout.get(),
                                                         vk::ShaderStageFlagBits::eVertex |
                                                         vk::ShaderStageFlagBits::eFragment,
                                                         0, sizeof(mvp_pc), &pc);
                        node->get_curr_cmd().draw(raw_3d_vertices.size(), 1, 0, 0);
                    });

                render_graph->get_node(m_debugger_pass_name)->add_pre_rendering_task(debug_task);
                render_graph->get_node(m_debugger_pass_name)->g_set_render_operation(render_op);
            }
            if (cfd_renderer)
            {

                auto model_v_shader = render_graph->resources_manager->get_shader(
                    shader_path + "\\spirvGlsl\\DebugRendering\\debugCFD.vert.spv", s_vert);
                auto model_f_shader = render_graph->resources_manager->get_shader(
                    shader_path + "\\spirvGlsl\\DebugRendering\\debugCFD.frag.spv", s_frag);
                auto render_node = render_graph->add_pass("voxelizer_viewer");
                render_node->g_set_vert_shader(model_v_shader);
                render_node->g_set_frag_shader(model_f_shader);
                render_node->g_set_framebuffer_size(render_graph->current_back_buffer->image_metadata->get_image_size());
                render_node->g_set_push_constant_size(sizeof(debug_voxels_mvp));
                render_node->g_set_vertex_input(d_vertex_3d::get_vertex_input());
                render_node->g_add_color_attachment_output(0, col_info);
                render_node->g_set_color_image_attachment_binding(0, render_graph->current_back_buffer);
                render_node->g_set_graphics_pipeline_configs({rasterization_configs::r_line, topology_configs::t_triangle});
                render_node->g_set_depth_config(depth_configs::d_none);
                if (cluster_renderer)
                {
                    render_node->depends_on("light");
                    render_node->depends_on("ModelDebugger");
                }
                render_node->depends_on("voxelizer");
                render_node->build_render_graph_node();

                render_node->g_set_render_operation(new std::function<void()>(
                    [this]()
                    {
                        debug_voxels_pc.proj_view = current_view_cam->get_view_proj();
                        debug_voxels_pc.origin = cfd_renderer->voxel_grid.pc_voxel_grid.origin;
                        debug_voxels_pc.cell_size = cfd_renderer->voxel_grid.pc_voxel_grid.cell_size;
                        debug_voxels_pc.dimensions = cfd_renderer->voxel_grid.pc_voxel_grid.dimensions;

                        vk::DeviceSize offset = 0;
                        auto node = render_graph->get_node("voxelizer_viewer");

                        node->get_curr_cmd().pushConstants(node->gpu_pipeline_ref->pipeline_layout.get(),
                                                         vk::ShaderStageFlagBits::eVertex |
                                                         vk::ShaderStageFlagBits::eFragment,
                                                         0, sizeof(debug_voxels_mvp), &debug_voxels_pc);
                        node->get_curr_cmd().bindVertexBuffers(
                            0, 1, &render_graph->resources_manager->get_staged_buff_from_name("cube_verts_default")->device_buffer->buffer_handle.
                                                get(), &offset);
                        node->get_curr_cmd().bindIndexBuffer(
                            render_graph->resources_manager->get_staged_buff_from_name("cube_index_default")->device_buffer->buffer_handle.get(), 0,
                            vk::IndexType::eUint32);
                        node->get_curr_cmd().drawIndexed(36, cfd_renderer->voxel_grid.pc_voxel_grid.get_size(), 0, 0, 0);
                    }));
            }
        }

        void recreate_swap_chain_resources() override
        {
        }

        void set_render_operation() override
        {
        }

        void reload_shaders() override
        {
            auto node = render_graph->get_node(m_debugger_pass_name);
            if (node)
            {
                node->g_recreate_resources();
            }
        }

        void set_view_camera()
        {
            current_view_cam = &cluster_renderer->camera;
        }


        void create_frustum_vertices()
        {
            glm::vec4 near_top_l = glm::inverse(cluster_renderer->curr_camera->matrices.view) * glm::vec4(
                cluster_renderer->cam_frustum.points[0], 1.0);
            glm::vec4 near_top_r = glm::inverse(cluster_renderer->curr_camera->matrices.view) * glm::vec4(
                cluster_renderer->cam_frustum.points[1], 1.0);
            glm::vec4 near_bottom_l = glm::inverse(cluster_renderer->curr_camera->matrices.view) * glm::vec4(
                cluster_renderer->cam_frustum.points[2], 1.0);
            glm::vec4 near_bottom_r = glm::inverse(cluster_renderer->curr_camera->matrices.view) * glm::vec4(
                cluster_renderer->cam_frustum.points[3], 1.0);
            glm::vec4 far_top_l = glm::inverse(cluster_renderer->curr_camera->matrices.view) * glm::vec4(
                cluster_renderer->cam_frustum.points[4], 1.0);
            glm::vec4 far_top_r = glm::inverse(cluster_renderer->curr_camera->matrices.view) * glm::vec4(
                cluster_renderer->cam_frustum.points[5], 1.0);
            glm::vec4 far_bottom_l = glm::inverse(cluster_renderer->curr_camera->matrices.view) * glm::vec4(
                cluster_renderer->cam_frustum.points[6], 1.0);
            glm::vec4 far_bottom_r = glm::inverse(cluster_renderer->curr_camera->matrices.view) * glm::vec4(
                cluster_renderer->cam_frustum.points[7], 1.0);

            raw_3d_vertices.clear();

            // Left face: 0,2,6 and 0,6,4
            raw_3d_vertices.push_back({near_top_l});
            raw_3d_vertices.push_back({near_bottom_l});
            raw_3d_vertices.push_back({far_bottom_l});
            raw_3d_vertices.push_back({near_top_l});
            raw_3d_vertices.push_back({far_bottom_l});
            raw_3d_vertices.push_back({far_top_l});

            // Right face: 1,3,7 and 1,7,5
            raw_3d_vertices.push_back({near_top_r});
            raw_3d_vertices.push_back({near_bottom_r});
            raw_3d_vertices.push_back({far_bottom_r});
            raw_3d_vertices.push_back({near_top_r});
            raw_3d_vertices.push_back({far_bottom_r});
            raw_3d_vertices.push_back({far_top_r});

            // Top face: 0,1,5 and 0,5,4
            raw_3d_vertices.push_back({near_top_l});
            raw_3d_vertices.push_back({near_top_r});
            raw_3d_vertices.push_back({far_top_r});
            raw_3d_vertices.push_back({near_top_l});
            raw_3d_vertices.push_back({far_top_r});
            raw_3d_vertices.push_back({far_top_l});

            // Bottom face: 2,3,7 and 2,7,6
            raw_3d_vertices.push_back({near_bottom_l});
            raw_3d_vertices.push_back({near_bottom_r});
            raw_3d_vertices.push_back({far_bottom_r});
            raw_3d_vertices.push_back({near_bottom_l});
            raw_3d_vertices.push_back({far_bottom_r});
            raw_3d_vertices.push_back({far_bottom_l});
        }

        std::vector<d_vertex_3d> raw_3d_vertices;
        std::vector<rendering::model> models;
        std::unique_ptr<shader> model_f_shader;
        std::unique_ptr<shader> model_v_shader;
        std::string m_debugger_pass_name = "ModelDebugger";

        engine::core* core;
        engine::render_graph* render_graph;
        ::window_provider* window_provider;

        engine::image_view* col_attachment_view;
        engine::buffer* raw_vertices_buff;
        rendering::camera* current_view_cam;

        mvp_pc pc{};
        debug_voxels_mvp debug_voxels_pc{};
        rendering::cluster_renderer* cluster_renderer = nullptr;
        rendering::cfd_renderer* cfd_renderer = nullptr;
    };
} // namespace rendering
#endif        // DEBUG_RENDERER_HPP
