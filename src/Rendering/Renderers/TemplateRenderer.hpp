//
// Created by carlo on 2025-03-26.
//

#ifndef TEMPLATE_RENDERER_HPP
#define TEMPLATE_RENDERER_HPP

namespace rendering
{
using namespace engine;
class Template_Renderer : public Base_Renderer
{
  public:
	~Template_Renderer() override = default;

	Template_Renderer(engine::Core *core, ::Window_Provider *window_provider)
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
	}

	void Create_Pipelines()
	{
		engine::Attachment_Info col_info = Get_Color_Attachment_Info(Blend_Configs::B_Opaque,
		                                                glm::vec4(0.0f), render_graph->core->swapchain_ref->Get_Format());
		Shader        *v_shader = render_graph->resources_manager->Get_Shader("SomeName", Shader_Stage::S_Vert);
		Shader        *f_shader = render_graph->resources_manager->Get_Shader("SomeName", Shader_Stage::S_Frag);

		auto       image_info        = engine::Image::Create_Info2d(window_provider->Get_Window_Size(), 1, 1, engine::g_32b_format, engine::color_image_usage);
		engine::Image_View *attachment_output = render_graph->resources_manager->Get_Image("TemplateOutput", image_info, 0, 0);

		auto render_node = render_graph->Add_Pass(pass_name);
		render_node->G_Set_Configs({true});
		render_node->G_Set_Vert_Shader(v_shader);
		render_node->G_Set_Frag_Shader(f_shader);
		render_node->G_Set_Framebuffer_Size(window_provider->Get_Window_Size());
		render_node->G_Set_Vertex_Input(Vertex_2d::Get_Vertex_Input());
		// change this
		render_node->G_Set_Push_Constant_Size(4);
		render_node->G_Add_Color_Attachment_Output(0, col_info);
		// only if we want to use custom attachment
		//  render_node->G_AddColorImageResource("default_attachment", attachment_output);
		render_node->Build_Render_Graph_Node();
	}

	void Recreate_Swap_Chain_Resources() override
	{
	}

	void Set_Render_Operation() override
	{
		auto task_op = new std::function<void()>(
		    [this] {
			    auto render_node = render_graph->Get_Node(pass_name);
			    render_node->G_Set_Color_Image_Attachment_Binding(0, render_graph->current_back_buffer);
		    	
		    });
		auto render_op = new std::function<void()>(
		    [this]() {
			    auto          &render_node = render_graph->render_nodes.at(pass_name);
			    vk::DeviceSize offset     = 0;
			    render_node->Get_Curr_Cmd().bindVertexBuffers(
			        0, 1,
			        &render_graph->resources_manager->Get_Staged_Buff_From_Name("quad_default")->device_buffer->buffer_handle.get(),
			        &offset);
			    render_node->Get_Curr_Cmd().bindIndexBuffer(
			        render_graph->resources_manager->Get_Staged_Buff_From_Name("quad_index_default")->device_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);
			    render_node->Get_Curr_Cmd().drawIndexed(
			        Vertex_2d::Get_Quad_Indices().size(), 1, 0, 0, 0);
		    });
		render_graph->Get_Node(pass_name)->G_Set_Render_Operation(render_op);
		render_graph->Get_Node(pass_name)->Add_Pre_Rendering_Task(task_op);
	}

	void Reload_Shaders() override
	{
	}

	std::string pass_name = "passName";

	engine::Core           *core;
	engine::Render_Graph    *render_graph;
	::Window_Provider *window_provider;

	engine::Image_View *col_attachment_view;
};
}        // namespace rendering

#endif        // TEMPLATE_RENDERER_HPP
