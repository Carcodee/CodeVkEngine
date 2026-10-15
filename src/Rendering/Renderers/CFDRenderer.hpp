//
// Created by carlo on 2025-03-26.
//

#ifndef CFD_RENDERER_HPP
#define CFD_RENDERER_HPP

namespace rendering
{
    using namespace engine;

    class Cfd_Renderer : public Base_Renderer
    {
    public:
        ~Cfd_Renderer() override = default;

        Cfd_Renderer(engine::Core* core, ::Window_Provider* window_provider)
        {
            this->core = core;
            this->render_graph = core->render_graph_ref;
            this->window_provider = window_provider;

            Create_Resources();
            Create_Buffers();
            Create_Pipelines();
        }

        void Create_Resources()
        {
        }

        void Create_Buffers()
        {
            auto image_info = engine::Image::Create_Info2d(window_provider->Get_Window_Size(), 1, 1,
                                                 vk::Format::eR32G32B32A32Sfloat,
                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eSampled);
            auto* col_attachment_view = render_graph->resources_manager->Get_Image("colAttachment", image_info, 0, 0);
            voxel_grid.Init("CFD", render_graph);
        }

        void Create_Pipelines()
        {
            auto& logical_device = core->logical_device.get();
            std::string shader_path = systems::Os::Get_Instance()->Get_Shaders_Path();
            auto voxelizer_comp = render_graph->resources_manager->Get_Shader(
                shader_path + "\\spirvGlsl\\CFD\\voxelizeMesh.comp.spv", S_Comp);
            auto model_to_voxelize = Rendering_Res_Manager::Get_Instance()->Get_Model(
                systems::Os::Get_Instance()->Get_Assets_Path() + "\\Models\\porsche\\scene.gltf");

            auto* voxelizer_node = render_graph->Add_Pass("voxelizer");
            voxelizer_node->G_Set_Comp_Shader(voxelizer_comp);
            voxelizer_node->G_Set_Push_Constant_Size(sizeof(Pc_Voxel_Grid));
            voxelizer_node->Build_Render_Graph_Node();

            voxelizer_node->Add_Pre_Rendering_Task(new std::function<void()>(
                [this, model_to_voxelize]()
                {
                    voxel_grid.Reset();
                }));

            voxelizer_node->G_Set_Render_Operation(new std::function<void()>(
                [this, model_to_voxelize]()
                {
                    std::vector<glm::mat4> model_mats;
                        for (auto &model : Rendering_Res_Manager::Get_Instance()->models)
                    {
                        for (int i = 0; i < model->models_mat.size(); ++i)
                        {
                            model_mats.push_back(model->models_mat[i]);
                        }
                    }

                    auto& render_node = render_graph->render_nodes.at("voxelizer");

                    render_node->G_Set_Buffer("Model_Vertices", model_to_voxelize->vert_buffer->device_buffer.get());
                    render_node->G_Set_Buffer("Model_Mats", model_mats);
                    render_node->G_Set_Buffer("Voxels", render_graph->resources_manager->Get_Buff_From_Name(voxel_grid.name));
                    render_node->Get_Curr_Cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
                                                           vk::ShaderStageFlagBits::eCompute,
                                                           0, sizeof(Pc_Voxel_Grid), &voxel_grid.pc_voxel_grid);
                    render_node->Get_Curr_Cmd().dispatch(
                        static_cast<uint32_t>((model_to_voxelize->vertices.size() + 31) / 32), 1, 1);
                }));

        }

        void Recreate_Swap_Chain_Resources() override
        {
        }

        void Set_Render_Operation() override
        {
        }

        void Reload_Shaders() override
        {
        }

        std::string pass_name = "passName";

        engine::Core* core;
        engine::Render_Graph* render_graph;
        ::Window_Provider* window_provider;
        Voxel_Grid voxel_grid = {};
    };
} // namespace rendering

#endif        // CFD_RENDERER_HPP
