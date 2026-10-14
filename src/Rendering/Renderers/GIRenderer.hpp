//

// Created by carlo on 2025-03-26.
//

#ifndef GI_RENDERER_HPP
#define GI_RENDERER_HPP

namespace rendering
{
class gi_renderer : public base_renderer
{
  public:
	gi_renderer(engine::core *core, ::window_provider *window_provider)
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
	}

	void create_buffers()
	{
		quad_vert_buffer  = resources_manager::get_instance()->get_staged_buff_from_name("quad_default")->device_buffer.get();
		quad_index_buffer = resources_manager::get_instance()->get_staged_buff_from_name("quad_index_default")->device_buffer.get();
	}

	void create_pipelines()
	{
		engine::attachment_info col_info = get_color_attachment_info(blend_configs::b_opaque,
		    glm::vec4(0.0f), render_graph->core->swapchain_ref->get_format());
		std::string shader_path = systems::os::get_instance()->get_shaders_path();

		// auto image_info = image::create_info2d(window_provider->get_window_size(), 1, 1, engine::g_32b_format,engine::color_image_usage);
		// image_view* attachment_output = render_graph->resources_manager->get_image("shOutput", image_info, 1, 1);

		auto render_node = render_graph->get_template_node_df(
		    sh_pass_name, "shView.slang", c_glsl);
		render_node->g_set_framebuffer_size(window_provider->get_window_size());
		render_node->g_add_color_attachment_output(0, col_info);

		render_node->build_render_graph_node();
	}

	void recreate_swap_chain_resources() override
	{
	}

	void set_render_operation() override
	{
		auto task_op = new std::function<void()>([this] {
			auto render_node     = render_graph->get_node(sh_pass_name);
			auto curr_back_buffer = render_graph->current_back_buffer;
			render_node->g_set_color_image_attachment_binding(0, curr_back_buffer);
		});

		auto sh_render_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph->render_nodes.at(sh_pass_name);
			    vk::DeviceSize offset = 0;
			    render_node->get_curr_cmd().bindVertexBuffers(
			        0, 1,
			        &render_graph->resources_manager->get_staged_buff_from_name("quad_default")->device_buffer->buffer_handle.get(),
			        &offset);
			    render_node->get_curr_cmd().bindIndexBuffer(
			        render_graph->resources_manager->get_staged_buff_from_name("quad_index_default")->device_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);
			    render_node->get_curr_cmd().drawIndexed(
			        vertex_2d::get_quad_indices().size(), 1, 0, 0, 0);
		    });
		render_graph->get_node(sh_pass_name)->add_pre_rendering_task(task_op);
		render_graph->get_node(sh_pass_name)->g_set_render_operation(sh_render_op);
	}

	void reload_shaders() override
	{
	}

	std::string sh_pass_name = "SHPass";

	engine::core           *core;
	engine::render_graph    *render_graph;
	::window_provider *window_provider;

	engine::image_view *col_attachment_view;
	engine::buffer    *quad_vert_buffer;
	engine::buffer    *quad_index_buffer;
};
}        // namespace rendering

#endif        // GI_RENDERER_HPP
