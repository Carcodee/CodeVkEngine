//

// Created by carlo on 2025-03-26.
//

#ifndef GS_RENDERER_HPP
#define GS_RENDERER_HPP

namespace rendering
{
using namespace engine;
class Gs_Renderer : public Base_Renderer
{
  public:
	~Gs_Renderer() override = default;

	Gs_Renderer(engine::Core *core, ::Window_Provider *window_provider)
	{
		this->core           = core;
		this->render_graph    = core->render_graph_ref;
		this->window_provider = window_provider;

		Create_Resources();
		Create_Buffers();
		Create_Pipelines();
	}

	void Create_Resources()
	{
		std::string path = systems::Os::Get_Instance()->Get_Assets_Path() +
		                   "\\PointClouds\\Woman\\pc2.ply";
		std::vector<Gaussian_Splat> gaussian_splats;
		// for (auto& file : std::filesystem::directory_iterator(path))
		// {
		// for (auto& value : std::filesystem::directory_iterator(file))
		// {
		//     rendering_res_manager::Get_Instance()->Load_Gs(value.path().string(), gaussian_splats);
		//     break;
		// }
		//
		// }
		Rendering_Res_Manager::Get_Instance()->Load_Gs(path, gaussian_splats);

		gaussians.Init(gaussian_splats, camera.camera_properties.fov, 1024.0, 1024.0);

		auto indices_sorted = gaussians.Sort_By_Depth(split_mvp.view);
		for (int i = 0; i < gaussians.pos.size(); ++i)
		{
			engine::Draw_Indirect_Indexed_Cmd indirect_cmd{};
			indirect_cmd.first_index    = 0;
			indirect_cmd.first_instance = indices_sorted[i];
			indirect_cmd.index_count    = Vertex_2d::Get_Quad_Indices().size();
			indirect_cmd.instance_count = 1;
			indirect_cmd.vertex_offset  = 0;
			indexed_cmds.emplace_back(indirect_cmd);
		}
		indirect_buffer = engine::Resources_Manager::Get_Instance()->Get_Buffer(engine::Resources_Manager::Buffer_Params{
		    "gsIndirectDraw", vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
		    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
		    sizeof(engine::Draw_Indirect_Indexed_Cmd) * indexed_cmds.size(), sizeof(engine::Draw_Indirect_Indexed_Cmd),indexed_cmds.data()});
	}
	void Re_Sort()
	{
		auto indices_sorted = gaussians.Sort_By_Depth(split_mvp.view);
		indexed_cmds.clear();
		for (int i = 0; i < gaussians.pos.size(); ++i)
		{
			engine::Draw_Indirect_Indexed_Cmd indirect_cmd{};
			indirect_cmd.first_index    = 0;
			indirect_cmd.first_instance = indices_sorted[i];
			indirect_cmd.index_count    = Vertex_2d::Get_Quad_Indices().size();
			indirect_cmd.instance_count = 1;
			indirect_cmd.vertex_offset  = 0;
			indexed_cmds.emplace_back(indirect_cmd);
		}
		indirect_buffer = engine::Resources_Manager::Get_Instance()->Set_Buffer(
		    "gsIndirectDraw", sizeof(engine::Draw_Indirect_Indexed_Cmd) * indexed_cmds.size(), sizeof(engine::Draw_Indirect_Indexed_Cmd), indexed_cmds.data());
	}

	void Create_Buffers()
	{
		gs_points_vertex_buffer = this->render_graph->resources_manager->Get_Stage_Buffer(
		                                                              "gsPointsVertexBuffer", vk::BufferUsageFlagBits::eVertexBuffer,
		                                                              sizeof(Vertex_2d) * Vertex_2d::Get_Quad_Vertices().size(),sizeof(Vertex_2d),
		                                                              Vertex_2d::Get_Quad_Vertices().data())
		                           ->device_buffer.get();
		gs_points_index_buffer = this->render_graph->resources_manager->Get_Stage_Buffer(
		                                                             "gsPointsIndexBuffer", vk::BufferUsageFlagBits::eIndexBuffer,
		                                                             sizeof(Vertex_2d) * Vertex_2d::Get_Quad_Indices().size(),sizeof(Vertex_2d),
		                                                             Vertex_2d::Get_Quad_Indices().data())
		                          ->device_buffer.get();
	}

	void Create_Pipelines()
	{
		// auto histogramNode = rendering_res_manager::Get_Instance()->Get_Template_Compute_Node(
		//     "histogramPass",
		//     systems::Os::Get_Instance()->shaders_path.string() +
		//     "\\glsl\\ThirdParty\\multi_radixsort_histograms.comp");
		// histogramNode->G_Set_Push_Constant_Size(sizeof(pc_histogram));
		//
		// auto radixSortNode = rendering_res_manager::Get_Instance()->Get_Template_Compute_Node(
		//     "radixSortPass",
		//     systems::Os::Get_Instance()->shaders_path.string() +
		//     "\\glsl\\ThirdParty\\multi_radixsort.comp");
		// histogramNode->G_Set_Push_Constant_Size(sizeof(pc_radix_sort));

		engine::Attachment_Info col_info = Get_Color_Attachment_Info(Blend_Configs::B_Alpha_Blend,
		    glm::vec4(0.0f), core->swapchain_ref->Get_Format());
		auto image_info = engine::Image::Create_Info2d(window_provider->Get_Window_Size(), 1, 1, engine::g_32b_format, engine::color_image_usage);
		// Image_View* attachment_output = render_graph->resources_manager->Get_Image("shOutput", image_info, 0, 0);

		auto render_node = render_graph->Get_Template_Node_Df(pass_name, "GSLoad", C_Glsl);
		render_node->G_Set_Framebuffer_Size(window_provider->Get_Window_Size());
		render_node->G_Set_Vertex_Input(Vertex_2d::Get_Vertex_Input());
		render_node->G_Set_Push_Constant_Size(sizeof(split_mvp));
		render_node->G_Add_Color_Attachment_Output(0, col_info);
		render_node->G_Set_Graphics_Pipeline_Configs({R_Fill, T_Triangle});
		render_node->Build_Render_Graph_Node();

		render_node->G_Set_Buffer("Gs_Scale_Buffer", gaussians.scales);
		render_node->G_Set_Buffer("Gs_Rot_Buffer", gaussians.rots);
		render_node->G_Set_Buffer("Gs_Pos_Buffer", gaussians.pos);
		render_node->G_Set_Buffer("Gs_Cols_Buffer", gaussians.cols);
		render_node->G_Set_Buffer("Gs_Alphas_Buffer", gaussians.alphas);
		render_node->G_Set_Buffer("H_Fov_Uniform", gaussians.h_fov_focal);
		render_node->G_Set_Buffer("Gs_Shs_Buffer", gaussians.sh_coefs);
	}

	void Recreate_Swap_Chain_Resources() override
	{
	}

	void Set_Render_Operation() override
	{
		// auto hTaskOp = new std::function<void()>(
		//     [this]
		//     {
		//         MoveCam();
		//     });
		// auto hRenderOp = new std::function<void()>(
		//     [this]()
		//     {
		//         auto& render_node = render_graph->render_nodes.at("histogramPass");
		//
		//         render_node->Get_Curr_Cmd().bindDescriptorSets(render_node->pipeline_type,
		//             render_node->pipeline_layout.get(), 0,
		//             1,
		//             &render_node->desc_cache->dst_set, 0, nullptr);
		//         render_node->Get_Curr_Cmd().bindPipeline(
		//             render_node->pipeline_type, render_node->pipeline.get());
		//         vk::DeviceSize offset = 0;
		//
		//         render_node->Get_Curr_Cmd().pushConstants(
		//             render_graph->Get_Node(pass_name)->pipeline_layout.get(),
		//             vk::ShaderStageFlagBits::eVertex |
		//             vk::ShaderStageFlagBits::eFragment,
		//             0, sizeof(PcHistogram), &pcHistogram);
		//     });
		//
		// render_graph->Get_Node("histogramPass")->Add_Pre_Rendering_Task(hTaskOp);
		// render_graph->Get_Node("histogramPass")->G_Set_Render_Operation(hRenderOp);
		//
		// auto rRenderOp = new std::function<void()>(
		//     [this]()
		//     {
		//         auto& render_node = render_graph->render_nodes.at("radixSortPass");
		//
		//         render_node->Get_Curr_Cmd().bindDescriptorSets(render_node->pipeline_type,
		//                                          render_node->pipeline_layout.get(), 0,
		//                                          1,
		//                                          &render_node->desc_cache->dst_set, 0, nullptr);
		//         render_node->Get_Curr_Cmd().bindPipeline(render_node->pipeline_type, render_node->pipeline.get());
		//         vk::DeviceSize offset = 0;
		//
		//         render_node->Get_Curr_Cmd().pushConstants(
		//             render_graph->Get_Node(pass_name)->pipeline_layout.get(),
		//             vk::ShaderStageFlagBits::eVertex |
		//             vk::ShaderStageFlagBits::eFragment,
		//             0, sizeof(PcRadixSort), &pcRadixSort);
		//     });
		//
		// render_graph->Get_Node("radixSortPass")->G_Set_Render_Operation(rRenderOp);

		auto task_op = new std::function<void()>(
		    [this] {
			    Move_Cam();
			    split_mvp.model = glm::identity<glm::mat4>();

			    auto render_node = render_graph->Get_Node(pass_name);
			    render_node->G_Set_Color_Image_Attachment_Binding(0, render_graph->current_back_buffer);
		    });
		auto render_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph->render_nodes.at(pass_name);

			    render_node->G_Set_Buffer("Gs_Configs_Uniform", gs_configs_pc);

			    vk::DeviceSize offset = 0;

			    render_node->Get_Curr_Cmd().pushConstants(
			        render_graph->Get_Node(pass_name)->gpu_pipeline_ref->pipeline_layout.get(),
			        vk::ShaderStageFlagBits::eVertex |
			            vk::ShaderStageFlagBits::eFragment,
			        0, sizeof(split_mvp), &split_mvp);

			    render_node->Get_Curr_Cmd().bindVertexBuffers(
			        0, 1,
			        &gs_points_vertex_buffer->buffer_handle.get(),
			        &offset);

			    render_node->Get_Curr_Cmd().bindIndexBuffer(
			        gs_points_index_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);
			    // render_node->Get_Curr_Cmd().drawIndexed(
			    //     vertex_2d::Get_Quad_Indices().size(), gaussians.pos.size(), 0, 0, 0);

			    vk::DeviceSize size_offset = 0;
			    uint32_t       stride     = sizeof(Draw_Indirect_Indexed_Cmd);
			    render_node->Get_Curr_Cmd().drawIndexedIndirect(
			        indirect_buffer->buffer_handle.get(),
			        size_offset,
			        gaussians.pos.size(),
			        stride);
		    });
		render_graph->Get_Node(pass_name)->G_Set_Render_Operation(render_op);
		render_graph->Get_Node(pass_name)->Add_Pre_Rendering_Task(task_op);
	}

	void Move_Cam()
	{
		glm::vec2 input = glm::vec2(0.0f);
		if (glfwGetKey(window_provider->window, GLFW_KEY_W))
		{
			input += glm::vec2(0.0f, 1.0f);
		}
		if (glfwGetKey(window_provider->window, GLFW_KEY_S))
		{
			input += glm::vec2(0.0f, -1.0f);
		}
		if (glfwGetKey(window_provider->window, GLFW_KEY_D))
		{
			input += glm::vec2(1.0f, 0.0f);
		}
		if (glfwGetKey(window_provider->window, GLFW_KEY_A))
		{
			input += glm::vec2(-1.0f, 0.0f);
		}
		if (glfwGetKey(window_provider->window, GLFW_KEY_LEFT_SHIFT))
		{
			// camera.movementSpeed = 40;
		}
		else
		{
			// camera.movementSpeed = 5;
		}
		input                = glm::clamp(input, glm::vec2(-1.0, -1.0), glm::vec2(1.0, 1.0));
		glm::vec2 mouse_input = glm::vec2(-ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		camera.mouse_input    = mouse_input;
		if (glfwGetMouseButton(window_provider->window, GLFW_MOUSE_BUTTON_2))
		{
			camera.Rotate_Camera();
			camera.Move(delta_time, input);
		}
		else
		{
			camera.first_mouse = true;
		}

		camera.Update_Cam();
		split_mvp.view = camera.matrices.view;

		split_mvp.proj = camera.matrices.perspective;
		split_mvp.proj[0][1] *= -1.0f;
		split_mvp.proj[2][1] *= -1.0f;
		split_mvp.proj[3][1] *= -1.0f;
	}

	void Reload_Shaders() override
	{
	}

	std::string pass_name = "gsDisplayPass";

	engine::Core             *core;
	engine::Render_Graph      *render_graph;
	::Window_Provider   *window_provider;
	engine::Buffer   *gs_points_vertex_buffer = nullptr;
	engine::Buffer   *gs_points_index_buffer  = nullptr;
	Arrays_Of_Gaussians gaussians            = {};
	Split_Mvp          split_mvp             = {};
	rendering::Camera            camera               = {glm::vec3(5.0f), rendering::Camera::Camera_Mode::E_Free};

	engine::Buffer                             *indirect_buffer;
	engine::Image_View                          *col_attachment_view;
	std::vector<glm::vec3>              pos_sorted;
	std::vector<Draw_Indirect_Indexed_Cmd> indexed_cmds;

	Gs_Configs_Pc gs_configs_pc = {};
	Pc_Histogram pc_histogram = {};
	Pc_Radix_Sort pc_radix_sort = {};
};
}        // namespace rendering

#endif        // GS_RENDERER_HPP
