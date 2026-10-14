//
// Created by carlo on 2025-03-26.
//

#ifndef TEMPLATE_RENDERER_HPP
#define TEMPLATE_RENDERER_HPP

namespace rendering
{
using namespace engine;
class template_renderer : public base_renderer
{
  public:
	~template_renderer() override = default;

	template_renderer(engine::core *core, ::window_provider *window_provider)
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
	}

	void create_pipelines()
	{
		engine::attachment_info col_info = get_color_attachment_info(blend_configs::b_opaque,
		                                                glm::vec4(0.0f), render_graph->core->swapchain_ref->get_format());
		shader        *v_shader = render_graph->resources_manager->get_shader("SomeName", shader_stage::s_vert);
		shader        *f_shader = render_graph->resources_manager->get_shader("SomeName", shader_stage::s_frag);

		auto       image_info        = engine::image::create_info2d(window_provider->get_window_size(), 1, 1, engine::g_32b_format, engine::color_image_usage);
		engine::image_view *attachment_output = render_graph->resources_manager->get_image("TemplateOutput", image_info, 0, 0);

		auto render_node = render_graph->add_pass(pass_name);
		render_node->g_set_configs({true});
		render_node->g_set_vert_shader(v_shader);
		render_node->g_set_frag_shader(f_shader);
		render_node->g_set_framebuffer_size(window_provider->get_window_size());
		render_node->g_set_vertex_input(vertex_2d::get_vertex_input());
		// change this
		render_node->g_set_push_constant_size(4);
		render_node->g_add_color_attachment_output(0, col_info);
		// only if we want to use custom attachment
		//  render_node->G_AddColorImageResource("default_attachment", attachment_output);
		render_node->build_render_graph_node();
	}

	void recreate_swap_chain_resources() override
	{
	}

	void set_render_operation() override
	{
		auto task_op = new std::function<void()>(
		    [this] {
			    auto render_node = render_graph->get_node(pass_name);
			    render_node->g_set_color_image_attachment_binding(0, render_graph->current_back_buffer);
		    	
		    });
		auto render_op = new std::function<void()>(
		    [this]() {
			    auto          &render_node = render_graph->render_nodes.at(pass_name);
			    vk::DeviceSize offset     = 0;
			    render_node->get_curr_cmd().bindVertexBuffers(
			        0, 1,
			        &render_graph->resources_manager->get_staged_buff_from_name("quad_default")->device_buffer->buffer_handle.get(),
			        &offset);
			    render_node->get_curr_cmd().bindIndexBuffer(
			        render_graph->resources_manager->get_staged_buff_from_name("quad_index_default")->device_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);
			    render_node->get_curr_cmd().drawIndexed(
			        vertex_2d::get_quad_indices().size(), 1, 0, 0, 0);
		    });
		render_graph->get_node(pass_name)->g_set_render_operation(render_op);
		render_graph->get_node(pass_name)->add_pre_rendering_task(task_op);
	}

	void reload_shaders() override
	{
	}

	std::string pass_name = "passName";

	engine::core           *core;
	engine::render_graph    *render_graph;
	::window_provider *window_provider;

	engine::image_view *col_attachment_view;
};
}        // namespace rendering

#endif        // TEMPLATE_RENDERER_HPP
