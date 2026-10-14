//
// Created by carlo on 2025-03-26.
//

#ifndef CFD_RENDERER_HPP
#define CFD_RENDERER_HPP

namespace rendering
{
    using namespace engine;

    class cfd_renderer : public base_renderer
    {
    public:
        ~cfd_renderer() override = default;

        cfd_renderer(engine::core* core, ::window_provider* window_provider)
        {
            this->core = core;
            this->render_graph = core->render_graph_ref;
            this->window_provider = window_provider;

            create_resources();
            create_buffers();
            create_pipelines();
        }

        void create_resources()
        {
        }

        void create_buffers()
        {
            auto image_info = engine::image::create_info2d(window_provider->get_window_size(), 1, 1,
                                                 vk::Format::eR32G32B32A32Sfloat,
                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eSampled);
            auto* col_attachment_view = render_graph->resources_manager->get_image("colAttachment", image_info, 0, 0);
            voxel_grid.init("CFD", render_graph);
        }

        void create_pipelines()
        {
            auto& logical_device = core->logical_device.get();
            std::string shader_path = systems::os::get_instance()->get_shaders_path();
            auto voxelizer_comp = render_graph->resources_manager->get_shader(
                shader_path + "\\spirvGlsl\\CFD\\voxelizeMesh.comp.spv", s_comp);
            auto model_to_voxelize = rendering_res_manager::get_instance()->get_model(
                systems::os::get_instance()->get_assets_path() + "\\Models\\porsche\\scene.gltf");

            auto* voxelizer_node = render_graph->add_pass("voxelizer");
            voxelizer_node->g_set_comp_shader(voxelizer_comp);
            voxelizer_node->g_set_push_constant_size(sizeof(pc_voxel_grid));
            voxelizer_node->build_render_graph_node();
            voxelizer_node->g_set_render_operation(new std::function<void()>(
                [this, model_to_voxelize]()
                {
                    auto& render_node = render_graph->render_nodes.at("voxelizer");

                    render_node->g_set_buffer("model_vertices", model_to_voxelize->vert_buffer->device_buffer.get());
                    render_node->g_set_buffer("voxels", voxel_grid.d_voxel_grid.d_cells);
                    render_node->get_curr_cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
                                                           vk::ShaderStageFlagBits::eCompute,
                                                           0, sizeof(pc_voxel_grid), &voxel_grid.pc_voxel_grid);
                    render_node->get_curr_cmd().dispatch(
                        static_cast<uint32_t>((model_to_voxelize->vertices.size() + 31) / 32), 1, 1);
                }));

        }

        void recreate_swap_chain_resources() override
        {
        }

        void set_render_operation() override
        {
        }

        void reload_shaders() override
        {
        }

        std::string pass_name = "passName";

        engine::core* core;
        engine::render_graph* render_graph;
        ::window_provider* window_provider;
        voxel_grid voxel_grid = {};
    };
} // namespace rendering

#endif        // CFD_RENDERER_HPP
