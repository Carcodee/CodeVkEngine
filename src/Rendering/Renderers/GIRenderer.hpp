//

// Created by carlo on 2025-03-26.
//

#ifndef GI_RENDERER_HPP
#define GI_RENDERER_HPP

namespace rendering
{
class Gi_Renderer : public Base_Renderer
{
  public:
	Gi_Renderer(engine::Core *core, ::Window_Provider *window_provider)
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
	}

	void Create_Buffers()
	{
		quad_vert_buffer  = Resources_Manager::Get_Instance()->Get_Staged_Buff_From_Name("quad_default")->device_buffer.get();
		quad_index_buffer = Resources_Manager::Get_Instance()->Get_Staged_Buff_From_Name("quad_index_default")->device_buffer.get();
	}

	void Create_Pipelines()
	{
		engine::Attachment_Info col_info = Get_Color_Attachment_Info(Blend_Configs::B_Opaque,
		    glm::vec4(0.0f), render_graph->core->swapchain_ref->Get_Format());
		std::string shader_path = systems::Os::Get_Instance()->Get_Shaders_Path();

		// auto image_info = image::Create_Info2d(window_provider->Get_Window_Size(), 1, 1, engine::g_32b_format,engine::color_image_usage);
		// Image_View* attachment_output = render_graph->resources_manager->Get_Image("shOutput", image_info, 1, 1);

		auto render_node = render_graph->Get_Template_Node_Df(
		    sh_pass_name, "shView.slang", C_Glsl);
		render_node->G_Set_Framebuffer_Size(window_provider->Get_Window_Size());
		render_node->G_Add_Color_Attachment_Output(0, col_info);

		render_node->Build_Render_Graph_Node();
	}

	void Recreate_Swap_Chain_Resources() override
	{
	}

	void Set_Render_Operation() override
	{
		auto task_op = new std::function<void()>([this] {
			auto render_node     = render_graph->Get_Node(sh_pass_name);
			auto curr_back_buffer = render_graph->current_back_buffer;
			render_node->G_Set_Color_Image_Attachment_Binding(0, curr_back_buffer);
		});

		auto sh_render_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph->render_nodes.at(sh_pass_name);
			    vk::DeviceSize offset = 0;
			    render_node->Get_Curr_Cmd().bindVertexBuffers(
			        0, 1,
			        &render_graph->resources_manager->Get_Staged_Buff_From_Name("quad_default")->device_buffer->buffer_handle.get(),
			        &offset);
			    render_node->Get_Curr_Cmd().bindIndexBuffer(
			        render_graph->resources_manager->Get_Staged_Buff_From_Name("quad_index_default")->device_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);
			    render_node->Get_Curr_Cmd().drawIndexed(
			        Vertex_2d::Get_Quad_Indices().size(), 1, 0, 0, 0);
		    });
		render_graph->Get_Node(sh_pass_name)->Add_Pre_Rendering_Task(task_op);
		render_graph->Get_Node(sh_pass_name)->G_Set_Render_Operation(sh_render_op);
	}

	void Reload_Shaders() override
	{
	}

	std::string sh_pass_name = "SHPass";

	engine::Core           *core;
	engine::Render_Graph    *render_graph;
	::Window_Provider *window_provider;

	engine::Image_View *col_attachment_view;
	engine::Buffer    *quad_vert_buffer;
	engine::Buffer    *quad_index_buffer;
};
}        // namespace rendering

#endif        // GI_RENDERER_HPP
