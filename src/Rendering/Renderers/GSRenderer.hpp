//

// Created by carlo on 2025-03-26.
//

#ifndef GS_RENDERER_HPP
#define GS_RENDERER_HPP

namespace rendering
{
using namespace engine;
class gs_renderer : public base_renderer
{
  public:
	~gs_renderer() override = default;

	gs_renderer(engine::core *core, ::window_provider *window_provider)
	{
		this->core           = core;
		this->render_graph    = core->render_graph_ref;
		this->window_provider = window_provider;

		create_resources();
		create_buffers();
		create_pipelines();
	}

	void create_resources()
	{
		std::string path = systems::os::get_instance()->get_assets_path() +
		                   "\\PointClouds\\Woman\\pc2.ply";
		std::vector<gaussian_splat> gaussian_splats;
		// for (auto& file : std::filesystem::directory_iterator(path))
		// {
		// for (auto& value : std::filesystem::directory_iterator(file))
		// {
		//     rendering_res_manager::get_instance()->load_gs(value.path().string(), gaussian_splats);
		//     break;
		// }
		//
		// }
		rendering_res_manager::get_instance()->load_gs(path, gaussian_splats);

		gaussians.init(gaussian_splats, camera.camera_properties.fov, 1024.0, 1024.0);

		auto indices_sorted = gaussians.sort_by_depth(split_mvp.view);
		for (int i = 0; i < gaussians.pos.size(); ++i)
		{
			engine::draw_indirect_indexed_cmd indirect_cmd{};
			indirect_cmd.first_index    = 0;
			indirect_cmd.first_instance = indices_sorted[i];
			indirect_cmd.index_count    = vertex_2d::get_quad_indices().size();
			indirect_cmd.instance_count = 1;
			indirect_cmd.vertex_offset  = 0;
			indexed_cmds.emplace_back(indirect_cmd);
		}
		indirect_buffer = engine::resources_manager::get_instance()->get_buffer(engine::resources_manager::buffer_params{
		    "gsIndirectDraw", vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
		    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
		    sizeof(engine::draw_indirect_indexed_cmd) * indexed_cmds.size(), sizeof(engine::draw_indirect_indexed_cmd),indexed_cmds.data()});
	}
	void re_sort()
	{
		auto indices_sorted = gaussians.sort_by_depth(split_mvp.view);
		indexed_cmds.clear();
		for (int i = 0; i < gaussians.pos.size(); ++i)
		{
			engine::draw_indirect_indexed_cmd indirect_cmd{};
			indirect_cmd.first_index    = 0;
			indirect_cmd.first_instance = indices_sorted[i];
			indirect_cmd.index_count    = vertex_2d::get_quad_indices().size();
			indirect_cmd.instance_count = 1;
			indirect_cmd.vertex_offset  = 0;
			indexed_cmds.emplace_back(indirect_cmd);
		}
		indirect_buffer = engine::resources_manager::get_instance()->set_buffer(
		    "gsIndirectDraw", sizeof(engine::draw_indirect_indexed_cmd) * indexed_cmds.size(), sizeof(engine::draw_indirect_indexed_cmd), indexed_cmds.data());
	}

	void create_buffers()
	{
		gs_points_vertex_buffer = this->render_graph->resources_manager->get_stage_buffer(
		                                                              "gsPointsVertexBuffer", vk::BufferUsageFlagBits::eVertexBuffer,
		                                                              sizeof(vertex_2d) * vertex_2d::get_quad_vertices().size(),sizeof(vertex_2d),
		                                                              vertex_2d::get_quad_vertices().data())
		                           ->device_buffer.get();
		gs_points_index_buffer = this->render_graph->resources_manager->get_stage_buffer(
		                                                             "gsPointsIndexBuffer", vk::BufferUsageFlagBits::eIndexBuffer,
		                                                             sizeof(vertex_2d) * vertex_2d::get_quad_indices().size(),sizeof(vertex_2d),
		                                                             vertex_2d::get_quad_indices().data())
		                          ->device_buffer.get();
	}

	void create_pipelines()
	{
		// auto histogramNode = rendering_res_manager::get_instance()->get_template_compute_node(
		//     "histogramPass",
		//     systems::os::get_instance()->shaders_path.string() +
		//     "\\glsl\\ThirdParty\\multi_radixsort_histograms.comp");
		// histogramNode->g_set_push_constant_size(sizeof(pc_histogram));
		//
		// auto radixSortNode = rendering_res_manager::get_instance()->get_template_compute_node(
		//     "radixSortPass",
		//     systems::os::get_instance()->shaders_path.string() +
		//     "\\glsl\\ThirdParty\\multi_radixsort.comp");
		// histogramNode->g_set_push_constant_size(sizeof(pc_radix_sort));

		engine::attachment_info col_info = get_color_attachment_info(blend_configs::b_alpha_blend,
		    glm::vec4(0.0f), core->swapchain_ref->get_format());
		auto image_info = engine::image::create_info2d(window_provider->get_window_size(), 1, 1, engine::g_32b_format, engine::color_image_usage);
		// image_view* attachment_output = render_graph->resources_manager->get_image("shOutput", image_info, 0, 0);

		auto render_node = render_graph->get_template_node_df(pass_name, "GSLoad", c_glsl);
		render_node->g_set_framebuffer_size(window_provider->get_window_size());
		render_node->g_set_vertex_input(vertex_2d::get_vertex_input());
		render_node->g_set_push_constant_size(sizeof(split_mvp));
		render_node->g_add_color_attachment_output(0, col_info);
		render_node->g_set_graphics_pipeline_configs({r_fill, t_triangle});
		render_node->build_render_graph_node();

		render_node->g_set_buffer("gs_scale_buffer", gaussians.scales);
		render_node->g_set_buffer("gs_rot_buffer", gaussians.rots);
		render_node->g_set_buffer("gs_pos_buffer", gaussians.pos);
		render_node->g_set_buffer("gs_cols_buffer", gaussians.cols);
		render_node->g_set_buffer("gs_alphas_buffer", gaussians.alphas);
		render_node->g_set_buffer("h_fov_uniform", gaussians.h_fov_focal);
		render_node->g_set_buffer("gs_shs_buffer", gaussians.sh_coefs);
	}

	void recreate_swap_chain_resources() override
	{
	}

	void set_render_operation() override
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
		//         render_node->get_curr_cmd().bindDescriptorSets(render_node->pipeline_type,
		//             render_node->pipeline_layout.get(), 0,
		//             1,
		//             &render_node->desc_cache->dst_set, 0, nullptr);
		//         render_node->get_curr_cmd().bindPipeline(
		//             render_node->pipeline_type, render_node->pipeline.get());
		//         vk::DeviceSize offset = 0;
		//
		//         render_node->get_curr_cmd().pushConstants(
		//             render_graph->get_node(pass_name)->pipeline_layout.get(),
		//             vk::ShaderStageFlagBits::eVertex |
		//             vk::ShaderStageFlagBits::eFragment,
		//             0, sizeof(PcHistogram), &pcHistogram);
		//     });
		//
		// render_graph->get_node("histogramPass")->add_pre_rendering_task(hTaskOp);
		// render_graph->get_node("histogramPass")->g_set_render_operation(hRenderOp);
		//
		// auto rRenderOp = new std::function<void()>(
		//     [this]()
		//     {
		//         auto& render_node = render_graph->render_nodes.at("radixSortPass");
		//
		//         render_node->get_curr_cmd().bindDescriptorSets(render_node->pipeline_type,
		//                                          render_node->pipeline_layout.get(), 0,
		//                                          1,
		//                                          &render_node->desc_cache->dst_set, 0, nullptr);
		//         render_node->get_curr_cmd().bindPipeline(render_node->pipeline_type, render_node->pipeline.get());
		//         vk::DeviceSize offset = 0;
		//
		//         render_node->get_curr_cmd().pushConstants(
		//             render_graph->get_node(pass_name)->pipeline_layout.get(),
		//             vk::ShaderStageFlagBits::eVertex |
		//             vk::ShaderStageFlagBits::eFragment,
		//             0, sizeof(PcRadixSort), &pcRadixSort);
		//     });
		//
		// render_graph->get_node("radixSortPass")->g_set_render_operation(rRenderOp);

		auto task_op = new std::function<void()>(
		    [this] {
			    move_cam();
			    split_mvp.model = glm::identity<glm::mat4>();

			    auto render_node = render_graph->get_node(pass_name);
			    render_node->g_set_color_image_attachment_binding(0, render_graph->current_back_buffer);
		    });
		auto render_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph->render_nodes.at(pass_name);

			    render_node->g_set_buffer("gs_configs_uniform", gs_configs_pc);

			    vk::DeviceSize offset = 0;

			    render_node->get_curr_cmd().pushConstants(
			        render_graph->get_node(pass_name)->gpu_pipeline_ref->pipeline_layout.get(),
			        vk::ShaderStageFlagBits::eVertex |
			            vk::ShaderStageFlagBits::eFragment,
			        0, sizeof(split_mvp), &split_mvp);

			    render_node->get_curr_cmd().bindVertexBuffers(
			        0, 1,
			        &gs_points_vertex_buffer->buffer_handle.get(),
			        &offset);

			    render_node->get_curr_cmd().bindIndexBuffer(
			        gs_points_index_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);
			    // render_node->get_curr_cmd().drawIndexed(
			    //     vertex_2d::get_quad_indices().size(), gaussians.pos.size(), 0, 0, 0);

			    vk::DeviceSize size_offset = 0;
			    uint32_t       stride     = sizeof(draw_indirect_indexed_cmd);
			    render_node->get_curr_cmd().drawIndexedIndirect(
			        indirect_buffer->buffer_handle.get(),
			        size_offset,
			        gaussians.pos.size(),
			        stride);
		    });
		render_graph->get_node(pass_name)->g_set_render_operation(render_op);
		render_graph->get_node(pass_name)->add_pre_rendering_task(task_op);
	}

	void move_cam()
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
			camera.rotate_camera();
			camera.move(delta_time, input);
		}
		else
		{
			camera.first_mouse = true;
		}

		camera.update_cam();
		split_mvp.view = camera.matrices.view;

		split_mvp.proj = camera.matrices.perspective;
		split_mvp.proj[0][1] *= -1.0f;
		split_mvp.proj[2][1] *= -1.0f;
		split_mvp.proj[3][1] *= -1.0f;
	}

	void reload_shaders() override
	{
	}

	std::string pass_name = "gsDisplayPass";

	engine::core             *core;
	engine::render_graph      *render_graph;
	::window_provider   *window_provider;
	engine::buffer   *gs_points_vertex_buffer = nullptr;
	engine::buffer   *gs_points_index_buffer  = nullptr;
	arrays_of_gaussians gaussians            = {};
	split_mvp          split_mvp             = {};
	rendering::camera            camera               = {glm::vec3(5.0f), rendering::camera::camera_mode::e_free};

	engine::buffer                             *indirect_buffer;
	engine::image_view                          *col_attachment_view;
	std::vector<glm::vec3>              pos_sorted;
	std::vector<draw_indirect_indexed_cmd> indexed_cmds;

	gs_configs_pc gs_configs_pc = {};
	pc_histogram pc_histogram = {};
	pc_radix_sort pc_radix_sort = {};
};
}        // namespace rendering

#endif        // GS_RENDERER_HPP
