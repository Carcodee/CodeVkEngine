//
// Created by carlo on 2025-03-26.
//

#ifndef CFD_RENDERER_HPP
#define CFD_RENDERER_HPP
#include <complex>

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
            voxel_grid.Init("CFD", render_graph);
        }

        void Create_Pipelines()
        {
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
                    for (auto& model : Rendering_Res_Manager::Get_Instance()->models)
                    {
                        for (int i = 0; i < model->models_mat.size(); ++i)
                        {
                            model_mats.push_back(model->models_mat[i]);
                        }
                    }

                    auto& render_node = render_graph->render_nodes.at("voxelizer");

                    render_node->G_Set_Buffer("Model_Vertices", model_to_voxelize->vert_buffer->device_buffer.get());
                    render_node->G_Set_Buffer("Model_Mats", model_mats);
                    render_node->G_Set_Buffer(
                        "Voxels", render_graph->resources_manager->Get_Buff_From_Name(voxel_grid.name));
                    render_node->Get_Curr_Cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
                                                              vk::ShaderStageFlagBits::eCompute,
                                                              0, sizeof(Pc_Voxel_Grid), &voxel_grid.pc_voxel_grid);
                    render_node->Get_Curr_Cmd().dispatch(
                        static_cast<uint32_t>((model_to_voxelize->vertices.size() + 31) / 32), 1, 1);
                }));

            auto proj_grid_v_shader = render_graph->resources_manager->Get_Shader(
                shader_path + "\\spirvGlsl\\templates\\tVert.vert.spv", S_Vert);
            auto proj_grid_f_shader = render_graph->resources_manager->Get_Shader(
                shader_path + "\\spirvGlsl\\CFD\\voxelSliceProjection.frag.spv", S_Frag);


            auto image_info = engine::Image::Create_Info2d(
                render_graph->current_back_buffer->image_metadata->Get_Image_Size(), 1, 1,
                vk::Format::eR32G32B32A32Sfloat,
                vk::ImageUsageFlagBits::eColorAttachment |
                vk::ImageUsageFlagBits::eSampled);

            auto* slice_projection = render_graph->resources_manager->Get_Image("slice_projection", image_info, 0, 0);

            engine::Attachment_Info col_info = Get_Color_Attachment_Info(Blend_Configs::B_Opaque,
                                                                         glm::vec4(0.0f),
                                                                         vk::Format::eR32G32B32A32Sfloat,
                                                                         vk::AttachmentLoadOp::eLoad);

            struct Pc_Voxel_Proj
            {
                glm::uvec3 dimensions = glm::uvec3{10, 10, 10};
                glm::uvec3 plane_selected = glm::uvec3{1, 1, 0};
                uint32_t selected_dimension_offset;
                float cell_size = 0.5f;
                uint32_t padding;
            };

            auto* proj_voxel_slice_node = render_graph->Add_Pass("proj_voxel_slice_node");
            proj_voxel_slice_node->G_Set_Vert_Shader(proj_grid_v_shader);
            proj_voxel_slice_node->G_Set_Frag_Shader(proj_grid_f_shader);
            proj_voxel_slice_node->G_Set_Framebuffer_Size(
                render_graph->resources_manager->Get_Image_View_From_Name("slice_projection")->image_metadata->
                              Get_Image_Size());
            proj_voxel_slice_node->G_Set_Vertex_Input(Vertex_2d::Get_Vertex_Input());
            proj_voxel_slice_node->G_Add_Color_Attachment_Output(0, col_info);
            proj_voxel_slice_node->G_Set_Color_Image_Attachment_Binding(0, slice_projection);
            proj_voxel_slice_node->G_Set_Graphics_Pipeline_Configs({
                Rasterization_Configs::R_Fill, Topology_Configs::T_Triangle
            });
            proj_voxel_slice_node->G_Set_Depth_Config(Depth_Configs::D_None);
            proj_voxel_slice_node->Depends_On("voxelizer");
            proj_voxel_slice_node->G_Set_Push_Constant_Size(sizeof(Pc_Voxel_Proj));
            proj_voxel_slice_node->Build_Render_Graph_Node();

            proj_voxel_slice_node->G_Set_Render_Operation(new std::function<void()>(
                [this]()
                {
                    auto node = render_graph->Get_Node("proj_voxel_slice_node");

                    node->G_Set_Buffer(
                        "Voxels", render_graph->resources_manager->Get_Buff_From_Name(voxel_grid.name));

                    vk::DeviceSize offset = 0;
                    node->Get_Curr_Cmd().bindVertexBuffers(
                        0, 1, &render_graph->resources_manager->Get_Staged_Buff_From_Name("quad_default")->device_buffer
                                           ->buffer_handle.get(), &offset);

                    node->Get_Curr_Cmd().bindIndexBuffer(
                        render_graph->resources_manager->Get_Staged_Buff_From_Name("quad_index_default")->device_buffer
                                    ->buffer_handle.get(), 0,
                        vk::IndexType::eUint32);

                    Pc_Voxel_Proj pc_voxel_proj = {};
                    pc_voxel_proj.dimensions = voxel_grid.pc_voxel_grid.dimensions;
                    pc_voxel_proj.cell_size = voxel_grid.pc_voxel_grid.cell_size;


                    // 0 == x, 1 == y, 2 == z
                    pc_voxel_proj.plane_selected = glm::uvec3(1, 1, 0);
                    pc_voxel_proj.selected_dimension_offset = 0;

                    systems::Logger::Get_Instance()->Log_Vec(voxel_grid.pc_voxel_grid.origin);
                    node->Get_Curr_Cmd().pushConstants(node->gpu_pipeline_ref->pipeline_layout.get(),
                                                       vk::ShaderStageFlagBits::eVertex |
                                                       vk::ShaderStageFlagBits::eFragment,
                                                       0, sizeof(Pc_Voxel_Proj), &pc_voxel_proj);
                    node->Get_Curr_Cmd().drawIndexed(Vertex_2d::Get_Quad_Indices().size(), 1, 0, 0, 0);
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
