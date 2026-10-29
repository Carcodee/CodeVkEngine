//
// Created by carlo on 2024-12-04.
//

#ifndef DEBUG_RENDERER_HPP
#define DEBUG_RENDERER_HPP

namespace rendering
{
    using namespace engine;

    class Debug_Renderer : public Base_Renderer
    {
    public:
        ~Debug_Renderer() override = default;

        Debug_Renderer(engine::Core* core, ::Window_Provider* window_provider,
                      std::map<std::string, std::unique_ptr<Base_Renderer>>& renderers)
        {
            this->core = core;
            this->render_graph = core->render_graph_ref;
            this->window_provider = window_provider;
            if (renderers.contains("ClusterRenderer"))
            {
                this->cluster_renderer = dynamic_cast<rendering::Cluster_Renderer*>(renderers.at("ClusterRenderer").get());
            }

            if (renderers.contains("CFDRenderer"))
            {
                this->cfd_renderer = dynamic_cast<rendering::Cfd_Renderer*>(renderers.at("CFDRenderer").get());
            }

            Create_Resources();
            Create_Buffers();
            Create_Pipelines();
        }

        void Create_Resources()
        {
            auto image_info = engine::Image::Create_Info2d(window_provider->Get_Window_Size(), 1, 1,
                                                 vk::Format::eR32G32B32A32Sfloat,
                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eSampled);

            // image view not used right now
            //  col_attachment_view = resources_manager::Get_Instance()->Get_Image("debugColAttachment", image_info, 0, 0);
            Set_View_Camera();
        }

        void Create_Buffers()
        {
            raw_vertices_buff = Resources_Manager::Get_Instance()->Get_Buffer(
                Resources_Manager::Buffer_Params{
                    "debugRawVertices", vk::BufferUsageFlagBits::eVertexBuffer,
                    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                    1
                });

            auto* models_voxels = Resources_Manager::Get_Instance()->Get_Buffer(
                Resources_Manager::Buffer_Params{
                    "models_voxels", vk::BufferUsageFlagBits::eStorageBuffer,
                    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                    cfd_renderer->voxel_grid.pc_voxel_grid.Get_Size(), sizeof(glm::mat4)
                });
        }

        void Create_Pipelines()
        {
            std::string shader_path = systems::Os::Get_Instance()->Get_Shaders_Path();

            engine::Attachment_Info col_info = Get_Color_Attachment_Info(Blend_Configs::B_Opaque,
                                                            glm::vec4(0.0f), core->swapchain_ref->Get_Format(),
                                                            vk::AttachmentLoadOp::eLoad);
            if (cluster_renderer)
            {
                auto model_v_shader = render_graph->resources_manager->Get_Shader(
                    shader_path + "\\spirvGlsl\\DebugRendering\\debug.vert.spv", S_Vert);

                auto model_f_shader = render_graph->resources_manager->Get_Shader(
                    shader_path + "\\spirvGlsl\\DebugRendering\\debug.frag.spv", S_Frag);

                auto render_node = render_graph->Add_Pass(m_debugger_pass_name);
                render_node->G_Set_Vert_Shader(model_v_shader);
                render_node->G_Set_Frag_Shader(model_f_shader);
                render_node->G_Set_Framebuffer_Size(render_graph->current_back_buffer->image_metadata->Get_Image_Size());
                render_node->G_Set_Push_Constant_Size(sizeof(Mvp_Pc));
                render_node->G_Set_Vertex_Input( D_Vertex_3d::Get_Vertex_Input());
                render_node->G_Add_Color_Attachment_Output(0, col_info);
                render_node->G_Set_Color_Image_Attachment_Binding(0, render_graph->current_back_buffer);
                render_node->G_Set_Graphics_Pipeline_Configs({Rasterization_Configs::R_Line, Topology_Configs::T_Triangle});
                render_node->G_Set_Depth_Config(Depth_Configs::D_None);
                render_node->Depends_On("light");
                render_node->Build_Render_Graph_Node();

                auto debug_task = new std::function<void()>([this]()
                {
                    Set_View_Camera();
                    Create_Frustum_Vertices();
                    raw_vertices_buff = Resources_Manager::Get_Instance()->Set_Buffer(
                        "debugRawVertices", sizeof(D_Vertex_3d) * raw_3d_vertices.size(), sizeof(D_Vertex_3d),
                        raw_3d_vertices.data());
                });
                auto render_op = new std::function<void()>(
                    [this]()
                    {
                        vk::DeviceSize offset = 0;
                        auto node = render_graph->Get_Node(m_debugger_pass_name);

                        node->Get_Curr_Cmd().bindVertexBuffers(0, 1, &raw_vertices_buff->buffer_handle.get(), &offset);

                        pc.proj_view = current_view_cam->Get_View_Proj();
                        pc.model = glm::mat4(1.0);

                        node->Get_Curr_Cmd().pushConstants(node->gpu_pipeline_ref->pipeline_layout.get(),
                                                         vk::ShaderStageFlagBits::eVertex |
                                                         vk::ShaderStageFlagBits::eFragment,
                                                         0, sizeof(Mvp_Pc), &pc);
                        node->Get_Curr_Cmd().draw(raw_3d_vertices.size(), 1, 0, 0);
                    });

                render_graph->Get_Node(m_debugger_pass_name)->Add_Pre_Rendering_Task(debug_task);
                render_graph->Get_Node(m_debugger_pass_name)->G_Set_Render_Operation(render_op);
            }
            if (cfd_renderer)
            {

                auto model_v_shader = render_graph->resources_manager->Get_Shader(
                    shader_path + "\\spirvGlsl\\DebugRendering\\debugCFD.vert.spv", S_Vert);
                auto model_f_shader = render_graph->resources_manager->Get_Shader(
                    shader_path + "\\spirvGlsl\\DebugRendering\\debugCFD.frag.spv", S_Frag);
                auto render_node = render_graph->Add_Pass("voxelizer_viewer");
                render_node->G_Set_Vert_Shader(model_v_shader);
                render_node->G_Set_Frag_Shader(model_f_shader);
                render_node->G_Set_Framebuffer_Size(render_graph->current_back_buffer->image_metadata->Get_Image_Size());
                render_node->G_Set_Push_Constant_Size(sizeof(Debug_Voxels_Mvp));
                render_node->G_Set_Vertex_Input(D_Vertex_3d::Get_Vertex_Input());
                render_node->G_Add_Color_Attachment_Output(0, col_info);
                render_node->G_Set_Color_Image_Attachment_Binding(0, render_graph->current_back_buffer);
                render_node->G_Set_Graphics_Pipeline_Configs({Rasterization_Configs::R_Line, Topology_Configs::T_Triangle});
                render_node->G_Set_Depth_Config(Depth_Configs::D_None);
                if (cluster_renderer)
                {
                    render_node->Depends_On("light");
                    render_node->Depends_On("ModelDebugger");
                }
                render_node->Depends_On("voxelizer");
                render_node->Build_Render_Graph_Node();

                render_node->G_Set_Render_Operation(new std::function<void()>(
                    [this]()
                    {
                        debug_voxels_pc.proj_view = current_view_cam->Get_View_Proj();
                        debug_voxels_pc.origin = cfd_renderer->voxel_grid.pc_voxel_grid.origin;
                        debug_voxels_pc.cell_size = cfd_renderer->voxel_grid.pc_voxel_grid.cell_size;
                        debug_voxels_pc.dimensions = cfd_renderer->voxel_grid.pc_voxel_grid.dimensions;
                        debug_voxels_pc.plane_selected = cfd_renderer->voxel_grid.pc_voxel_grid.plane_selected;
                        debug_voxels_pc.selected_dimension_offset = cfd_renderer->voxel_grid.pc_voxel_grid.selected_dimension_offset;

                        vk::DeviceSize offset = 0;
                        auto node = render_graph->Get_Node("voxelizer_viewer");
                        node->G_Set_Buffer("Voxels",render_graph->resources_manager->Get_Buff_From_Name(cfd_renderer->voxel_grid.name));
                        node->Get_Curr_Cmd().pushConstants(node->gpu_pipeline_ref->pipeline_layout.get(),
                                                         vk::ShaderStageFlagBits::eVertex |
                                                         vk::ShaderStageFlagBits::eFragment,
                                                         0, sizeof(Debug_Voxels_Mvp), &debug_voxels_pc);
                        node->Get_Curr_Cmd().bindVertexBuffers(
                            0, 1, &render_graph->resources_manager->Get_Staged_Buff_From_Name("cube_verts_default")->device_buffer->buffer_handle.
                                                get(), &offset);
                        node->Get_Curr_Cmd().bindIndexBuffer(
                            render_graph->resources_manager->Get_Staged_Buff_From_Name("cube_index_default")->device_buffer->buffer_handle.get(), 0,
                            vk::IndexType::eUint32);
                        node->Get_Curr_Cmd().drawIndexed(36, cfd_renderer->voxel_grid.pc_voxel_grid.Get_Size(), 0, 0, 0);
                    }));
            }
        }

        void Recreate_Swap_Chain_Resources() override
        {
        }

        void Set_Render_Operation() override
        {
        }

        void Reload_Shaders() override
        {
            auto node = render_graph->Get_Node(m_debugger_pass_name);
            if (node)
            {
                node->G_Recreate_Resources();
            }
        }

        void Set_View_Camera()
        {
            current_view_cam = &cluster_renderer->camera;
        }


        void Create_Frustum_Vertices()
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

        std::vector<D_Vertex_3d> raw_3d_vertices;
        std::vector<rendering::Model> models;
        std::unique_ptr<Shader> model_f_shader;
        std::unique_ptr<Shader> model_v_shader;
        std::string m_debugger_pass_name = "ModelDebugger";

        engine::Core* core;
        engine::Render_Graph* render_graph;
        ::Window_Provider* window_provider;

        engine::Image_View* col_attachment_view;
        engine::Buffer* raw_vertices_buff;
        rendering::Camera* current_view_cam;

        Mvp_Pc pc{};
        Debug_Voxels_Mvp debug_voxels_pc{};
        rendering::Cluster_Renderer* cluster_renderer = nullptr;
        rendering::Cfd_Renderer* cfd_renderer = nullptr;
    };
} // namespace rendering
#endif        // DEBUG_RENDERER_HPP
