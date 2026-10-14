//
// Created by carlo on 2024-10-10.
//

#ifndef IMGUI_RENDERER_HPP
#define IMGUI_RENDERER_HPP
#include "CodeCuda.cuh"

namespace rendering
{
namespace ed = ax::NodeEditor;

namespace imgui_renderer_ui
{
struct fonts
{
	ImFont *body    = nullptr;
	ImFont *caption = nullptr;
	ImFont *heading = nullptr;
	ImFont *strong  = nullptr;
};

inline ImVec4 with_alpha(const ImVec4 &color, float alpha)
{
	return ImVec4(color.x, color.y, color.z, alpha);
}

inline ImVec4 accent_color()
{
	return ImVec4(1.00f, 0.32f, 0.08f, 1.00f);
}

inline ImVec4 success_color()
{
	return ImVec4(0.78f, 0.82f, 0.74f, 1.00f);
}

inline ImVec4 selection_color()
{
	return ImVec4(0.94f, 0.93f, 0.91f, 1.00f);
}

inline ImVec4 ink_color()
{
	return ImVec4(0.075f, 0.070f, 0.072f, 1.00f);
}

inline ImVec4 raised_surface_color()
{
	return ImVec4(0.095f, 0.087f, 0.088f, 0.92f);
}

inline fonts configure_fonts(const std::string &resources_path)
{
	const std::string fonts_path  = resources_path + "\\Fonts\\Open_Sans\\static\\";
	const std::string regular_ttf = fonts_path + "OpenSans-Regular.ttf";
	const std::string bold_ttf    = fonts_path + "OpenSans-Bold.ttf";

	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->Clear();

	fonts fonts{};
	if (std::filesystem::exists(regular_ttf))
	{
		fonts.body    = io.Fonts->AddFontFromFileTTF(regular_ttf.c_str(), 16.0f);
		fonts.caption = io.Fonts->AddFontFromFileTTF(regular_ttf.c_str(), 13.0f);
	}
	if (!fonts.body)
	{
		fonts.body = io.Fonts->AddFontDefault();
	}
	if (!fonts.caption)
	{
		fonts.caption = fonts.body;
	}

	if (std::filesystem::exists(bold_ttf))
	{
		fonts.heading = io.Fonts->AddFontFromFileTTF(bold_ttf.c_str(), 22.0f);
		fonts.strong  = io.Fonts->AddFontFromFileTTF(bold_ttf.c_str(), 16.0f);
	}
	if (!fonts.heading)
	{
		fonts.heading = fonts.body;
	}
	if (!fonts.strong)
	{
		fonts.strong = fonts.body;
	}

	io.FontDefault = fonts.body;
	io.Fonts->Build();
	return fonts;
}

inline void apply_modern_theme()
{
	ImGuiStyle &style = ImGui::GetStyle();
	ImGui::StyleColorsDark(&style);

	style.WindowPadding            = ImVec2(9.0f, 9.0f);
	style.FramePadding             = ImVec2(9.0f, 6.0f);
	style.CellPadding              = ImVec2(9.0f, 7.0f);
	style.ItemSpacing              = ImVec2(8.0f, 7.0f);
	style.ItemInnerSpacing         = ImVec2(7.0f, 5.0f);
	style.TouchExtraPadding        = ImVec2(0.0f, 0.0f);
	style.IndentSpacing            = 18.0f;
	style.ScrollbarSize            = 9.0f;
	style.GrabMinSize              = 9.0f;
	style.WindowBorderSize         = 0.0f;
	style.ChildBorderSize          = 0.0f;
	style.PopupBorderSize          = 1.0f;
	style.FrameBorderSize          = 0.0f;
	style.TabBorderSize            = 0.0f;
	style.WindowRounding           = 12.0f;
	style.ChildRounding            = 9.0f;
	style.FrameRounding            = 7.0f;
	style.PopupRounding            = 9.0f;
	style.ScrollbarRounding        = 10.0f;
	style.GrabRounding             = 8.0f;
	style.TabRounding              = 7.0f;
	style.SeparatorTextBorderSize  = 0.0f;
	style.SeparatorTextAlign       = ImVec2(0.0f, 0.5f);
	style.SeparatorTextPadding     = ImVec2(0.0f, 11.0f);
	style.SelectableTextAlign      = ImVec2(0.0f, 0.5f);
	style.WindowTitleAlign         = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ColorButtonPosition      = ImGuiDir_Right;

	const ImVec4 accent      = accent_color();
	const ImVec4 text        = ImVec4(0.90f, 0.89f, 0.88f, 1.00f);
	const ImVec4 text_muted   = ImVec4(0.50f, 0.47f, 0.47f, 1.00f);
	const ImVec4 canvas      = ImVec4(0.055f, 0.050f, 0.052f, 0.88f);
	const ImVec4 surface     = ImVec4(0.045f, 0.042f, 0.045f, 0.91f);
	const ImVec4 surface_high = raised_surface_color();
	const ImVec4 interaction = ImVec4(0.145f, 0.132f, 0.132f, 1.00f);
	const ImVec4 border      = ImVec4(0.28f, 0.24f, 0.23f, 0.38f);
	ImVec4      *colors      = style.Colors;

	colors[ImGuiCol_Text]                 = text;
	colors[ImGuiCol_TextDisabled]         = text_muted;
	colors[ImGuiCol_WindowBg]             = canvas;
	colors[ImGuiCol_ChildBg]              = surface;
	colors[ImGuiCol_PopupBg]              = surface;
	colors[ImGuiCol_Border]               = border;
	colors[ImGuiCol_BorderShadow]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_FrameBg]              = surface_high;
	colors[ImGuiCol_FrameBgHovered]       = interaction;
	colors[ImGuiCol_FrameBgActive]        = with_alpha(accent, 0.22f);
	colors[ImGuiCol_TitleBg]              = surface;
	colors[ImGuiCol_TitleBgActive]        = surface;
	colors[ImGuiCol_TitleBgCollapsed]     = surface;
	colors[ImGuiCol_MenuBarBg]            = surface;
	colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.03f, 0.028f, 0.03f, 0.35f);
	colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.28f, 0.25f, 0.25f, 0.82f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.34f, 0.32f, 0.92f);
	colors[ImGuiCol_ScrollbarGrabActive]  = with_alpha(accent, 0.78f);
	colors[ImGuiCol_CheckMark]            = accent;
	colors[ImGuiCol_SliderGrab]           = with_alpha(accent, 0.78f);
	colors[ImGuiCol_SliderGrabActive]     = accent;
	colors[ImGuiCol_Button]               = surface_high;
	colors[ImGuiCol_ButtonHovered]        = interaction;
	colors[ImGuiCol_ButtonActive]         = with_alpha(accent, 0.26f);
	colors[ImGuiCol_Header]               = surface_high;
	colors[ImGuiCol_HeaderHovered]        = interaction;
	colors[ImGuiCol_HeaderActive]         = with_alpha(accent, 0.24f);
	colors[ImGuiCol_Separator]            = border;
	colors[ImGuiCol_SeparatorHovered]     = with_alpha(accent, 0.65f);
	colors[ImGuiCol_SeparatorActive]      = accent;
	colors[ImGuiCol_ResizeGrip]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_ResizeGripHovered]    = with_alpha(accent, 0.65f);
	colors[ImGuiCol_ResizeGripActive]     = accent;
	colors[ImGuiCol_Tab]                  = surface;
	colors[ImGuiCol_TabHovered]           = interaction;
	colors[ImGuiCol_TabSelected]          = with_alpha(accent, 0.22f);
	colors[ImGuiCol_TabDimmed]            = canvas;
	colors[ImGuiCol_TabDimmedSelected]    = surface_high;
	colors[ImGuiCol_PlotLines]            = accent;
	colors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.00f, 0.58f, 0.30f, 1.00f);
	colors[ImGuiCol_PlotHistogram]        = accent;
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.58f, 0.30f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]        = surface_high;
	colors[ImGuiCol_TableBorderStrong]    = border;
	colors[ImGuiCol_TableBorderLight]     = with_alpha(border, 0.50f);
	colors[ImGuiCol_TableRowBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_TableRowBgAlt]        = ImVec4(1.0f, 1.0f, 1.0f, 0.025f);
	colors[ImGuiCol_TextSelectedBg]       = with_alpha(accent, 0.30f);
	colors[ImGuiCol_NavHighlight]         = accent;
	colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.01f, 0.015f, 0.025f, 0.76f);
}

inline bool navigation_item(const char *label, bool selected)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.08f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_Text, selected ? ink_color() : ImGui::GetStyleColorVec4(ImGuiCol_Text));
	ImGui::PushStyleColor(ImGuiCol_Button, selected ? selection_color() : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? selection_color() : raised_surface_color());
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,
	                      selected ? ImVec4(0.83f, 0.81f, 0.79f, 1.0f) : with_alpha(accent_color(), 0.20f));
	const bool pressed = ImGui::Button(label, ImVec2(ImGui::GetContentRegionAvail().x, 34.0f));
	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar();

	if (selected)
	{
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(max.x - 14.0f, (min.y + max.y) * 0.5f),
		                                            3.0f, ImGui::GetColorU32(accent_color()));
	}
	return pressed;
}
}        // namespace imgui_renderer_ui

class imgui_renderer
{
  public:
	struct imgui_dsets_array
	{
		~imgui_dsets_array() = default;
		imgui_dsets_array(engine::core *core, engine::descriptor_allocator *descriptor_allocator)
		{
			this->core                = core;
			this->descriptor_allocator = descriptor_allocator;
		}
		void add_set(std::string name)
		{
			if (indexes.contains(name))
			{
				return;
			}
			engine::descriptor_layout_builder builder;
			builder.add_binding(0, vk::DescriptorType::eCombinedImageSampler);
			auto dst_layout = builder.build_bindings(core->logical_device.get(), vk::ShaderStageFlagBits::eFragment);
			auto dset      = descriptor_allocator->allocate(core->logical_device.get(), dst_layout.get());
			indexes.try_emplace(name, dsets.size());
			descriptor_set_layouts.emplace_back(std::move(dst_layout));
			dsets.emplace_back(std::move(dset));
		}
		vk::DescriptorSet get_dset_by_name(std::string name)
		{
			return dsets.at(indexes.at(name)).get();
		}
		vk::DescriptorSetLayout get_layout_by_name(std::string name)
		{
			return descriptor_set_layouts.at(indexes.at(name)).get();
		}

		engine::core                                      *core                = nullptr;
		engine::descriptor_allocator                       *descriptor_allocator = nullptr;
		std::map<std::string, int>                 indexes;
		std::vector<vk::UniqueDescriptorSet>       dsets;
		std::vector<vk::UniqueDescriptorSetLayout> descriptor_set_layouts;
	};
	imgui_renderer(engine::render_graph *render_graph, ::window_provider *window_provider, std::map<std::string, std::unique_ptr<base_renderer>> &renderers)
	{
		this->core        = render_graph->core;
		this->render_graph = render_graph;
		this->renderers   = &renderers;

		for (const auto& entry : renderers)
		{
			if (auto* renderer = dynamic_cast<rendering::cfd_renderer*>(entry.second.get()))
			{
				this->cfd_renderer = renderer;
				break;
			}
		}

		if (renderers.contains("ClusterRenderer"))
		{
			this->cluster_renderer = dynamic_cast<rendering::cluster_renderer *>(renderers.at("ClusterRenderer").get());
		}
		if (renderers.contains("FlatRenderer"))
		{
			this->flat_renderer = dynamic_cast<rendering::flat_renderer *>(renderers.at("FlatRenderer").get());
		}
		if (renderers.contains("GSRenderer"))
		{
			this->gs_renderer = dynamic_cast<rendering::gs_renderer *>(renderers.at("GSRenderer").get());
		}
		this->window_provider = window_provider;

		std::vector<engine::descriptor_allocator::pool_size_ratio> pool_size_ratios = {
		    {vk::DescriptorType::eSampler, 1},
		    {vk::DescriptorType::eCombinedImageSampler, 1},
		    {vk::DescriptorType::eSampledImage, 1},
		    {vk::DescriptorType::eStorageImage, 1},
		    {vk::DescriptorType::eUniformTexelBuffer, 1},
		    {vk::DescriptorType::eStorageTexelBuffer, 1},
		    {vk::DescriptorType::eUniformBuffer, 1},
		    {vk::DescriptorType::eStorageBuffer, 1},
		    {vk::DescriptorType::eUniformBufferDynamic, 1},
		    {vk::DescriptorType::eStorageBufferDynamic, 1},
		    {vk::DescriptorType::eInputAttachment, 1}};
		descriptor_allocator.begin_pool(core->logical_device.get(), 1000, pool_size_ratios);

		this->dsets_arrays = std::make_unique<imgui_dsets_array>(core, &descriptor_allocator);

		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForVulkan(window_provider->window, true);

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance                  = core->instance.get();
		init_info.PhysicalDevice            = core->physical_device;
		init_info.Device                    = core->logical_device.get();
		init_info.Queue                     = core->queue_worker_manager->get_worker_queue("UI")->queue_handle;
		init_info.QueueFamily               = core->queue_worker_manager->get_worker_queue("UI")->family_index;
		init_info.DescriptorPool            = descriptor_allocator.pool.get();
		init_info.MinImageCount             = 3;
		init_info.ImageCount                = 3;
		init_info.UseDynamicRendering       = true;

		init_info.PipelineRenderingCreateInfo                         = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
		init_info.PipelineRenderingCreateInfo.colorAttachmentCount    = 1;
		imgui_swapchain_format                                         = static_cast<VkFormat>(core->swapchain_ref->get_format());
		init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &imgui_swapchain_format;

		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		set_style();

		ImGui_ImplVulkan_Init(&init_info);

		ImGui_ImplVulkan_CreateFontsTexture();

		ed::Config config;
		config.SettingsFile = "Simple.json";
		m_context           = ed::CreateEditor(&config);

		std::string resources_path = systems::os::get_instance()->get_engine_resources_path();
		std::string file_name      = resources_path + "\\Images\\f3.png";
	}
	ed::EditorContext *m_context = nullptr;
	void               start_node_editor()
	{
		ed::SetCurrentEditor(m_context);
		node_editor.init(render_graph, window_provider);
		node_editor.draw();
		ed::SetCurrentEditor(nullptr);
	}

	void render_frame(vk::CommandBuffer command_buffer, vk::ImageView &image_view_handle)
	{
		curr_command_buffer                = &command_buffer;
		static bool simulation_action_fail = false;
		auto        run_simulation_action  = [](CodeCuda::C_Res result) {
            simulation_action_fail = result != CodeCuda::C_Res::OK;
		};
		if (!color_animator.stop)
		{
			if (const std::vector<glm::vec4> *frame = color_animator.use_cpu_frame())
			{
				run_simulation_action(CodeCuda::FluidSimulation::C_MapImageToSmoke(
				    static_cast<int>(color_animator.animation_info.sprite_size_px.x),
				    static_cast<int>(color_animator.animation_info.sprite_size_px.y),
				    4,
				    const_cast<glm::vec4 *>(frame->data())));
			}
		}
		if (!emitters.empty())
		{
			auto *cuda_node = render_graph->get_node("CudaNode");
			if (!cuda_node || !cuda_node->cuda_pipeline || !cuda_node->cuda_pipeline->context)
			{
				simulation_action_fail = true;
			}
			else
			{
				for (const auto &active_emitter : emitters)
				{
					run_simulation_action(CodeCuda::FluidSimulation::C_AddVelocityGPU(
					    active_emitter.x_pos, active_emitter.y_pos, active_emitter.radius,
					    active_emitter.velocity.x, active_emitter.velocity.y,
					    cuda_node->cuda_pipeline->context));
					run_simulation_action(CodeCuda::FluidSimulation::C_AddSmokeGPU(
					    active_emitter.x_pos, active_emitter.y_pos, active_emitter.radius,
					    active_emitter.color.x, active_emitter.color.y,
					    active_emitter.color.z, active_emitter.color.w,
					    cuda_node->cuda_pipeline->context));
				}
			}
		}
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		render_debugger_window();

		ImGui::Render();
		engine::attachment_info vk_attachment_info = engine::get_color_attachment_info(blend_configs::b_opaque, glm::vec4(0.0f), core->swapchain_ref->get_format(), vk::AttachmentLoadOp::eLoad);
		vk_attachment_info.vk_attachment_info.setImageView(image_view_handle);

		std::vector<vk::RenderingAttachmentInfo> attachment_infos = {vk_attachment_info.vk_attachment_info};
		vk::RenderingAttachmentInfo              depth_attachment;

		dynamic_render_pass.set_render_info_unsafe(attachment_infos, window_provider->get_window_size(), &depth_attachment);

		for (int i = 0; i < image_views_to_recover.size(); ++i)
		{
			transition_image(image_views_to_recover[i]->image_metadata, layout_patterns::graphics_read, image_views_to_recover[i]->get_subresource_range(), command_buffer);
		}

		command_buffer.beginRendering(dynamic_render_pass.render_info);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
		command_buffer.endRendering();

		for (int i = 0; i < image_views_to_recover.size(); ++i)
		{
			transition_image(image_views_to_recover[i]->image_metadata, layout_patterns_to_recover[i], image_views_to_recover[i]->get_subresource_range(), command_buffer);
		}

		image_views_to_recover.clear();
		layout_patterns_to_recover.clear();
	}
	enum class debugger_page
	{
		render_graph,
		engine,
		textures,
		fluid_simulation,
		cluster_renderer,
		radiance_cascades,
		gaussian_splatting,
		profiler
	};

	struct debugger_page_info
	{
		const char *title;
		const char *description;
	};

	debugger_page_info get_debugger_page_info() const
	{
		switch (active_page)
		{
			case debugger_page::render_graph:
				return {"Render graph", "Inspect passes, resources, and node connections."};
			case debugger_page::engine:
				return {"Engine overview", "Queues, execution order, and runtime scheduling."};
			case debugger_page::textures:
				return {"Texture inspector", "Browse images currently registered with the renderer."};
			case debugger_page::fluid_simulation:
				return {"Fluid simulation", "Control simulation inputs, tools, emitters, and solver parameters."};
			case debugger_page::cluster_renderer:
				return {"Cluster renderer", "Lighting, culling, camera, and render-node controls."};
			case debugger_page::radiance_cascades:
				return {"Radiance cascades", "Configure cascades, materials, lighting, and painting."};
			case debugger_page::gaussian_splatting:
				return {"Gaussian splatting", "Inspect splat data, scale, camera, and sort state."};
			case debugger_page::profiler:
				return {"Performance", "Review CPU and GPU timing for the current frame."};
		}
		return {"Engine", "Runtime tools and diagnostics."};
	}

	void render_application_header()
	{
		const ImGuiIO &io = ImGui::GetIO();
		if (ImGui::BeginChild("##application_header", ImVec2(0.0f, 54.0f), ImGuiChildFlags_None,
		                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (ImGui::BeginTable("##application_header_layout", 4,
			                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings |
			                          ImGuiTableFlags_NoPadOuterX))
			{
				ImGui::TableSetupColumn("Workspace", ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGui::TableSetupColumn("Frame rate", ImGuiTableColumnFlags_WidthFixed, 112.0f);
				ImGui::TableSetupColumn("Frame time", ImGuiTableColumnFlags_WidthFixed, 112.0f);
				ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 76.0f);
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::PushFont(fonts.strong);
				ImGui::TextUnformatted("CODEVK");
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::TextColored(imgui_renderer_ui::accent_color(), "RENDERER");
				ImGui::PushFont(fonts.caption);
				ImGui::TextDisabled("ENGINE TOOLS");
				ImGui::PopFont();

				ImGui::TableSetColumnIndex(1);
				ImGui::PushFont(fonts.caption);
				ImGui::TextDisabled("FRAME RATE");
				ImGui::PopFont();
				ImGui::PushFont(fonts.strong);
				ImGui::Text("%.0f FPS", io.Framerate);
				ImGui::PopFont();

				ImGui::TableSetColumnIndex(2);
				ImGui::PushFont(fonts.caption);
				ImGui::TextDisabled("FRAME TIME");
				ImGui::PopFont();
				ImGui::PushFont(fonts.strong);
				ImGui::Text("%.2f ms", io.Framerate > 0.0f ? 1000.0f / io.Framerate : 0.0f);
				ImGui::PopFont();

				ImGui::TableSetColumnIndex(3);
				ImGui::PushFont(fonts.caption);
				ImGui::TextDisabled("STATUS");
				ImGui::PopFont();
				ImGui::TextColored(imgui_renderer_ui::accent_color(), "ACTIVE");
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}

	void render_sidebar()
	{
		const debugger_page_info active_page_info = get_debugger_page_info();
		auto                   navigate       = [this](const char *label, debugger_page page) {
            const bool selected = active_page == page;
            if (ImGui::Selectable(label, selected))
            {
                active_page = page;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
		};

		ImGui::PushFont(fonts.caption);
		ImGui::TextDisabled("WORKSPACE");
		ImGui::PopFont();
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginCombo("##workspace_page", active_page_info.title))
		{
			navigate("Render graph", debugger_page::render_graph);
			navigate("Engine overview", debugger_page::engine);
			navigate("Texture inspector", debugger_page::textures);
			navigate("Fluid simulation", debugger_page::fluid_simulation);
			if (cluster_renderer)
			{
				navigate("Cluster lighting", debugger_page::cluster_renderer);
			}
			if (flat_renderer)
			{
				navigate("Radiance cascades", debugger_page::radiance_cascades);
			}
			if (gs_renderer)
			{
				navigate("Gaussian splatting", debugger_page::gaussian_splatting);
			}
			navigate("Performance", debugger_page::profiler);
			ImGui::EndCombo();
		}
	}

	void render_active_page()
	{
		switch (active_page)
		{
			case debugger_page::render_graph:
				start_node_editor();
				break;
			case debugger_page::engine:
				display_engine_info();
				break;
			case debugger_page::textures:
				display_all_textures();
				break;
			case debugger_page::fluid_simulation:
				fluid_sim_info();
				break;
			case debugger_page::cluster_renderer:
				if (cluster_renderer)
				{
					cluster_renderer_info();
				}
				break;
			case debugger_page::radiance_cascades:
				if (flat_renderer)
				{
					r_cascades_info();
				}
				break;
			case debugger_page::gaussian_splatting:
				if (gs_renderer)
				{
					gs_renderer_info();
				}
				break;
			case debugger_page::profiler:
				render_graph_profiler();
				break;
		}
	}

	void render_scene_viewport()
	{
		scene_viewport_valid   = false;
		scene_viewport_hovered = false;

		engine::image_view *scene_image = nullptr;
		if (render_graph && render_graph->resources_manager)
		{
			scene_image = render_graph->resources_manager->get_image_view_from_name("bf");
		}

		if (!scene_image || !scene_image->image_metadata)
		{
			const ImVec2 available = ImGui::GetContentRegionAvail();
			const char  *message   = "Viewport image 'bf' is unavailable";
			const ImVec2 text_size  = ImGui::CalcTextSize(message);
			ImGui::SetCursorPos(ImVec2(
			    std::max(0.0f, (available.x - text_size.x) * 0.5f),
			    std::max(0.0f, (available.y - text_size.y) * 0.5f)));
			ImGui::TextDisabled("%s", message);
			return;
		}

		add_image("bf", scene_image);

		const ImVec2     available   = ImGui::GetContentRegionAvail();
		const glm::uvec2 image_extent = scene_image->image_metadata->get_image_size();
		if (available.x <= 1.0f || available.y <= 1.0f || image_extent.x == 0 || image_extent.y == 0)
		{
			return;
		}

		const float scale = std::min(
		    available.x / static_cast<float>(image_extent.x),
		    available.y / static_cast<float>(image_extent.y));
		const ImVec2 image_size(
		    static_cast<float>(image_extent.x) * scale,
		    static_cast<float>(image_extent.y) * scale);
		const ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(
		    cursor.x + (available.x - image_size.x) * 0.5f,
		    cursor.y + (available.y - image_size.y) * 0.5f));

		ImGui::Image(
		    (ImTextureID) dsets_arrays->get_dset_by_name("bf"), image_size,
		    ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

		scene_viewport_min     = ImGui::GetItemRectMin();
		scene_viewport_max     = ImGui::GetItemRectMax();
		scene_viewport_valid   = true;
		scene_viewport_hovered = ImGui::IsItemHovered();

		ImDrawList *draw_list    = ImGui::GetWindowDrawList();
		const ImU32 border_color = ImGui::GetColorU32(
		    scene_viewport_hovered ? imgui_renderer_ui::accent_color() : ImVec4(0.30f, 0.27f, 0.27f, 0.72f));
		draw_list->AddRect(scene_viewport_min, scene_viewport_max, border_color, 4.0f, 0,
		                  scene_viewport_hovered ? 2.0f : 1.0f);

		const ImVec2 badge_min(scene_viewport_min.x + 12.0f, scene_viewport_min.y + 12.0f);
		const ImVec2 badge_max(badge_min.x + 122.0f, badge_min.y + 27.0f);
		draw_list->AddRectFilled(badge_min, badge_max, IM_COL32(18, 17, 18, 220), 6.0f);
		draw_list->AddCircleFilled(ImVec2(badge_min.x + 13.0f, badge_min.y + 13.5f), 3.0f,
		                          ImGui::GetColorU32(imgui_renderer_ui::accent_color()));
		draw_list->AddText(ImVec2(badge_min.x + 23.0f, badge_min.y + 5.0f),
		                  IM_COL32(232, 229, 226, 255), "VIEWPORT  /  bf");
	}

	void render_debugger_window()
	{
		const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(main_viewport->WorkPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(main_viewport->WorkSize, ImGuiCond_Always);
		ImGui::SetNextWindowViewport(main_viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.025f, 0.024f, 0.026f, 1.0f));
		const ImGuiWindowFlags root_flags =
		    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

		if (ImGui::Begin("CodeVK Engine Workspace", nullptr, root_flags))
		{
			const ImVec2 available     = ImGui::GetContentRegionAvail();
			const float  splitter_width = 6.0f;
			const float  minimum_panel_width =
			    std::min(360.0f, std::max(220.0f, available.x * 0.45f));
			const float maximum_panel_width =
			    std::max(minimum_panel_width, available.x - 280.0f - splitter_width);

			if (inspector_panel_width <= 0.0f)
			{
				inspector_panel_width = available.x * 0.36f;
			}
			if (inspector_panel_resizing && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				inspector_panel_width -= ImGui::GetIO().MouseDelta.x;
			}
			else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				inspector_panel_resizing = false;
			}
			inspector_panel_width =
			    std::clamp(inspector_panel_width, minimum_panel_width, maximum_panel_width);
			const float viewport_width =
			    std::max(1.0f, available.x - inspector_panel_width - splitter_width);

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.015f, 0.015f, 0.017f, 1.0f));
			if (ImGui::BeginChild("##scene_viewport", ImVec2(viewport_width, 0.0f),
			                      ImGuiChildFlags_None,
			                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				render_scene_viewport();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();

			ImGui::SameLine(0.0f, 0.0f);
			ImGui::InvisibleButton("##inspector_splitter", ImVec2(splitter_width, available.y),
			                       ImGuiButtonFlags_MouseButtonLeft);
			const bool splitter_hovered = ImGui::IsItemHovered();
			const bool splitter_active  = ImGui::IsItemActive();
			if (splitter_hovered || splitter_active || inspector_panel_resizing)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}
			if (ImGui::IsItemActivated())
			{
				inspector_panel_resizing = true;
			}
			if (splitter_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				inspector_panel_width = std::clamp(available.x * 0.36f,
				                                 minimum_panel_width, maximum_panel_width);
			}
			const ImVec2 splitter_min = ImGui::GetItemRectMin();
			const ImVec2 splitter_max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(
			    splitter_min, splitter_max,
			    ImGui::GetColorU32(splitter_hovered || splitter_active ? imgui_renderer_ui::accent_color() : ImVec4(0.18f, 0.16f, 0.16f, 0.75f)));

			ImGui::SameLine(0.0f, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.045f, 0.042f, 0.045f, 0.98f));
			if (ImGui::BeginChild("##docked_inspector", ImVec2(inspector_panel_width, 0.0f),
			                      ImGuiChildFlags_None))
			{
				render_application_header();
				ImGui::Spacing();
				render_sidebar();
				ImGui::Spacing();

				const debugger_page_info page_info = get_debugger_page_info();
				ImGui::PushFont(fonts.heading);
				ImGui::TextUnformatted(page_info.title);
				ImGui::PopFont();
				ImGui::TextDisabled("%s", page_info.description);
				ImGui::Separator();
				ImGui::Spacing();

				if (ImGui::BeginChild("##page_content", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None,
				                      ImGuiWindowFlags_HorizontalScrollbar))
				{
					render_active_page();
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);
	}
	void render_graph_profiler()
	{
		profilers_window.cpuGraph.LoadFrameData(profiler::get_instance()->cpu_tasks.data(),
		                                       profiler::get_instance()->cpu_tasks.size());
		profilers_window.gpuGraph.LoadFrameData(profiler::get_instance()->gpu_tasks.data(),
		                                       profiler::get_instance()->gpu_tasks.size());
		profilers_window.RenderContent();
	}

	void add_smoke_pretty(CodeCuda::CodeCudaContext *context, const glm::vec2 mouse_position)
	{
		if (!context || CodeCuda::FluidSimulation::s_width <= 0 || CodeCuda::FluidSimulation::s_height <= 0)
		{
			return;
		}

		auto random_pretty_color = [this]() {
			float base_hue        = 0.0f;
			float base_saturation = 0.0f;
			float base_value      = 0.0f;
			ImGui::ColorConvertRGBtoHSV(pretty_smoke_base_color[0], pretty_smoke_base_color[1],
			                            pretty_smoke_base_color[2], base_hue, base_saturation, base_value);

			float hue = base_hue + glm::linearRand(-pretty_smoke_color_spread, pretty_smoke_color_spread);
			hue -= glm::floor(hue);
			const float saturation = glm::clamp(base_saturation * glm::linearRand(0.82f, 1.12f), 0.0f, 1.0f);
			const float value      = glm::clamp(base_value * glm::linearRand(0.82f, 1.12f), 0.0f, 1.0f);

			glm::vec4 color(0.0f, 0.0f, 0.0f,
			                glm::clamp(pretty_smoke_opacity * glm::linearRand(0.82f, 1.08f), 0.0f, 1.0f));
			ImGui::ColorConvertHSVtoRGB(hue, saturation, value, color.r, color.g, color.b);
			return color;
		};

		const int max_simulation_dimension =
		    std::max(CodeCuda::FluidSimulation::s_width, CodeCuda::FluidSimulation::s_height);
		const int radius = std::clamp(pretty_smoke_brush_radius, 1, max_simulation_dimension);
		if (!pretty_smoke_stroke_active)
		{
			pretty_smoke_last_mouse_position = mouse_position;
			pretty_smoke_last_color         = random_pretty_color();
			pretty_smoke_stroke_active      = true;
		}

		const glm::vec2 mouse_delta      = mouse_position - pretty_smoke_last_mouse_position;
		const float     distance        = glm::length(mouse_delta);
		const float     spacing_scale    = glm::mix(1.0f, 0.12f, pretty_smoke_trail_smoothness);
		const float     sample_spacing   = std::max(1.0f, static_cast<float>(radius) * spacing_scale);
		const int       sample_count     = std::max(1, static_cast<int>(glm::ceil(distance / sample_spacing)));
		const glm::vec4 new_color        = random_pretty_color();
		const glm::vec2 stroke_direction = distance > 0.001f ? mouse_delta / distance : glm::vec2(0.0f);
		const glm::vec2 stroke_normal(-stroke_direction.y, stroke_direction.x);
		const float     coordinate_scale =
		    1023.0f / static_cast<float>(std::max(1, max_simulation_dimension));

		for (int sample = 1; sample <= sample_count; ++sample)
		{
			const float t      = static_cast<float>(sample) / static_cast<float>(sample_count);
			const float jitter = glm::linearRand(-1.0f, 1.0f) * pretty_smoke_turbulence *
			                     static_cast<float>(radius) * coordinate_scale;
			const glm::vec2 interpolated_position =
			    glm::mix(pretty_smoke_last_mouse_position, mouse_position, t) + stroke_normal * jitter;
			const float     smooth_t = t * t * (3.0f - 2.0f * t);
			const glm::vec4 interpolated_color =
			    glm::mix(pretty_smoke_last_color, new_color, smooth_t);

			const float u = glm::clamp(interpolated_position.x / 1023.0f, 0.0f, 1.0f);
			const float v = glm::clamp(interpolated_position.y / 1023.0f, 0.0f, 1.0f);
			const int   x = static_cast<int>(u * static_cast<float>(CodeCuda::FluidSimulation::s_width - 1));
			const int   y = static_cast<int>((1.0f - v) * static_cast<float>(CodeCuda::FluidSimulation::s_height - 1));

			const glm::vec2 simulation_delta(
			    mouse_delta.x * static_cast<float>(CodeCuda::FluidSimulation::s_width) /
			        (1023.0f * static_cast<float>(sample_count)),
			    mouse_delta.y * static_cast<float>(CodeCuda::FluidSimulation::s_height) /
			        (1023.0f * static_cast<float>(sample_count)));
			const glm::vec2 curl_velocity(-simulation_delta.y, simulation_delta.x);
			const glm::vec2 velocity =
			    simulation_delta * pretty_smoke_flow_strength +
			    curl_velocity * (glm::linearRand(-1.0f, 1.0f) * pretty_smoke_turbulence *
			                    pretty_smoke_flow_strength);

			CodeCuda::FluidSimulation::C_AddVelocityGPU(x, y, radius,
			                                            velocity.x, -velocity.y, context);
			CodeCuda::FluidSimulation::C_AddSmokeGPU(x, y, radius,
			                                         interpolated_color.r, interpolated_color.g,
			                                         interpolated_color.b, interpolated_color.a, context);
		}

		pretty_smoke_last_mouse_position = mouse_position;
		pretty_smoke_last_color         = new_color;
	}

	void lattice_info()
	{
		if (!ImGui::CollapsingHeader("Lattice###fluid_lattice", ImGuiTreeNodeFlags_DefaultOpen))
			return;
		if (!cfd_renderer)
		{
			ImGui::TextDisabled("CFD renderer is unavailable.");
			return;
		}

		if (!lattice_config_initialized)
		{
			lattice_config = cfd_renderer->voxel_grid.pc_voxel_grid;
			lattice_config_initialized = true;
		}

		if (ImGui::BeginTable("##fluid_lattice_properties", 2,
		                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			auto property = [](const char* label) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(label);
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-1.0f);
			};
			property("Origin (X, Y, Z)");
			ImGui::DragFloat3("##lattice_origin", &lattice_config.origin.x, 0.1f);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Minimum corner in the mesh coordinate space.");

			int dimensions[3] = {int(lattice_config.dimensions.x), int(lattice_config.dimensions.y),
			                     int(lattice_config.dimensions.z)};
			property("Cells (X, Y, Z)");
			if (ImGui::DragInt3("##lattice_dimensions", dimensions, 1.0f, 1, 4096, "%d",
			                    ImGuiSliderFlags_AlwaysClamp))
			{
				lattice_config.dimensions = glm::uvec3(dimensions[0], dimensions[1], dimensions[2]);
			}
			property("Cell size");
			ImGui::DragFloat("##lattice_cell_size", &lattice_config.cell_size,
			                                        0.01f, 0.0001f, 10000.0f, "%.4f",
			                                        ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Edge length of each cubic cell, in mesh units.");
			ImGui::EndTable();
		}

		const glm::vec3 extent = glm::vec3(lattice_config.dimensions) * lattice_config.cell_size;
		const glm::vec3 maximum = lattice_config.origin + extent;
		const uint64_t cells = lattice_config.get_size();
		ImGui::TextDisabled("Extent: %.3f x %.3f x %.3f", extent.x, extent.y, extent.z);
		ImGui::TextDisabled("Maximum corner: %.3f, %.3f, %.3f", maximum.x, maximum.y, maximum.z);
		ImGui::TextDisabled("%llu cells", static_cast<unsigned long long>(cells));
		if (!lattice_config.is_valid())
		{
			ImGui::TextWrapped("Enter finite coordinates, positive cell size, and dimensions from 1 to 4096.");
			return;
		}

		ImGui::TextDisabled("Lattice preview");
		const ImVec2 canvas_min = ImGui::GetCursorScreenPos();
		const ImVec2 canvas_size(std::max(1.0f, ImGui::GetContentRegionAvail().x), 220.0f);
		ImGui::InvisibleButton("##lattice_preview", canvas_size);
		auto* draw = ImGui::GetWindowDrawList();
		const ImVec2 canvas_max(canvas_min.x + canvas_size.x, canvas_min.y + canvas_size.y);
		draw->AddRectFilled(canvas_min, canvas_max, ImGui::GetColorU32(ImGuiCol_FrameBg), 7.0f);
		draw->PushClipRect(canvas_min, canvas_max, true);
		const float projected_width = 0.866f * (extent.x + extent.z);
		const float projected_height = 0.5f * (extent.x + extent.z) + extent.y;
		const float scale = std::min(std::max(1.0f, canvas_size.x - 32.0f) / projected_width,
		                             (canvas_size.y - 32.0f) / projected_height);
		auto project = [&](glm::vec3 point) {
			point -= extent * 0.5f;
			return ImVec2(canvas_min.x + canvas_size.x * 0.5f + (point.x - point.z) * 0.866f * scale,
			              canvas_min.y + canvas_size.y * 0.5f + ((point.x + point.z) * 0.5f - point.y) * scale);
		};
		const ImU32 grid_color = ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.45f);
		const ImU32 edge_color = ImGui::GetColorU32(imgui_renderer_ui::accent_color());
		for (int axis = 0; axis < 3; ++axis)
		{
			const int next = (axis + 1) % 3;
			const int last = (axis + 2) % 3;
			const uint32_t step = std::max(1u, (lattice_config.dimensions[axis] + 23u) / 24u);
			for (uint32_t cell = step; cell < lattice_config.dimensions[axis]; cell += step)
			{
				glm::vec3 start(0.0f), end(0.0f);
				start[axis] = end[axis] = float(cell) * lattice_config.cell_size;
				end[next] = extent[next];
				draw->AddLine(project(start), project(end), grid_color);
				end[next] = 0.0f;
				end[last] = extent[last];
				draw->AddLine(project(start), project(end), grid_color);
			}
			for (int corner = 0; corner < 4; ++corner)
			{
				glm::vec3 start(0.0f);
				start[next] = (corner & 1) ? extent[next] : 0.0f;
				start[last] = (corner & 2) ? extent[last] : 0.0f;
				glm::vec3 end = start;
				end[axis] = extent[axis];
				draw->AddLine(project(start), project(end), edge_color, 1.5f);
			}
		}
		draw->PopClipRect();
		ImGui::TextWrapped("Preview settings only. Grid lines are sampled for readability.");
		ImGui::Spacing();
	}

	void fluid_sim_info()
	{
		lattice_info();
		CodeCuda::FluidSimulation::sim_params fluid_sim_params{};
		if (CodeCuda::FluidSimulation::C_GetSimulationParams(&fluid_sim_params) != CodeCuda::C_Res::OK)
		{
			ImGui::TextDisabled("Unable to read fluid simulation parameters.");
			return;
		}

		bool        params_changed        = false;
		static bool simulation_action_fail = false;
		auto        run_simulation_action  = [](CodeCuda::C_Res result) {
            simulation_action_fail = result != CodeCuda::C_Res::OK;
		};

		ImGui::TextDisabled("Grid: %d x %d", CodeCuda::FluidSimulation::s_width, CodeCuda::FluidSimulation::s_height);
		const float quick_control_width =
		    (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		if (ImGui::Button("Restart simulation", ImVec2(quick_control_width, 30.0f)))
		{
			run_simulation_action(CodeCuda::FluidSimulation::C_RestartSimulation());
		}
		ImGui::SameLine();
		params_changed |= ImGui::Checkbox("GPU simulation", &fluid_sim_params.gpu_sim);

		std::string resources_path =
		    systems::os::get_instance()->get_engine_resources_path() + "\\Images";

		static std::vector<std::string> image_paths;
		static std::string              selected_image_path;
		static char                     search_buffer[128] = {};
		static bool                     images_scanned     = false;
		static bool                     image_load_failed   = false;
		static std::vector<std::string> sequence_folder_paths;
		static std::string              selected_sequence_folder_path;
		static int                      sequence_frame_spacing = 1;
		static bool                     sequence_folders_scanned = false;
		static bool                     sequence_load_failed     = false;

		auto scan_sequence_folders = [&]() {
			sequence_folder_paths.clear();
			const std::filesystem::path engine_resources_path =
			    systems::os::get_instance()->get_assets_path() + "\\Animations";
			std::error_code scan_error;
			for (std::filesystem::recursive_directory_iterator iterator(engine_resources_path, scan_error), end;
			     iterator != end && !scan_error;
			     iterator.increment(scan_error))
			{
				if (!iterator->is_directory())
				{
					continue;
				}

				bool contains_images = false;
				std::error_code folder_error;
				for (const auto &entry : std::filesystem::directory_iterator(iterator->path(), folder_error))
				{
					if (folder_error)
					{
						break;
					}
					if (!entry.is_regular_file())
					{
						continue;
					}
					std::string extension = entry.path().extension().string();
					std::transform(extension.begin(), extension.end(), extension.begin(),
					               [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
					contains_images = extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
					                 extension == ".bmp" || extension == ".tga";
					if (contains_images)
					{
						break;
					}
				}
				if (contains_images)
				{
					sequence_folder_paths.emplace_back(iterator->path().string());
				}
			}
			std::sort(sequence_folder_paths.begin(), sequence_folder_paths.end());
			sequence_folders_scanned = true;
		};

		if (!images_scanned)
		{
			std::error_code scan_error;
			if (std::filesystem::exists(resources_path, scan_error))
			{
				for (const auto &dir : std::filesystem::directory_iterator(resources_path, scan_error))
				{
					if (scan_error)
					{
						break;
					}
					if (dir.is_regular_file())
					{
						image_paths.emplace_back(dir.path().string());
					}
				}
			}
			std::sort(image_paths.begin(), image_paths.end());
			images_scanned = true;
		}
		if (!sequence_folders_scanned)
		{
			scan_sequence_folders();
		}

		if (ImGui::CollapsingHeader("Image import###fluid_image_import"))
		{
			ImGui::TextDisabled("Load an image as smoke density or a solid mask.");

			ImGui::SetNextItemWidth(-1.0f);
			ImGui::InputTextWithHint(
			    "##ImageSearch",
			    "Search images...",
			    search_buffer,
			    sizeof(search_buffer));

			ImGui::Spacing();

			const float section_height = 260.0f;

			if (ImGui::BeginChild(
			        "##ImagesSection",
			        ImVec2(0.0f, section_height),
			        ImGuiChildFlags_Borders,
			        ImGuiWindowFlags_AlwaysVerticalScrollbar))
			{
				for (int i = 0; i < static_cast<int>(image_paths.size()); ++i)
				{
					const std::string &path = image_paths[i];

					const std::string filename =
					    std::filesystem::path(path).filename().string();

					if (search_buffer[0] != '\0')
					{
						if (filename.find(search_buffer) == std::string::npos)
							continue;
					}

					ImGui::PushID(i);

					const bool selected = selected_image_path == path;

					const float row_height = 32.0f;

					if (ImGui::Selectable(
					        "##ImageRow",
					        selected,
					        ImGuiSelectableFlags_AllowOverlap,
					        ImVec2(0.0f, row_height)))
					{
						selected_image_path = path;
					}

					ImGui::SameLine();

					const ImVec2 row_min = ImGui::GetItemRectMin();

					ImGui::SetCursorScreenPos(
					    ImVec2(
					        row_min.x + 8.0f,
					        row_min.y + 7.0f));

					ImGui::TextUnformatted(filename.c_str());

					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s", path.c_str());
					}

					const float button_width = 60.0f;

					ImGui::SetCursorScreenPos(
					    ImVec2(
					        ImGui::GetWindowPos().x +
					            ImGui::GetWindowContentRegionMax().x -
					            button_width - 8.0f,

					        row_min.y + 4.0f));

					if (ImGui::Button("Load", ImVec2(button_width, 24.0f)))
					{
						selected_image_path = path;

						stbi_uc *pixels_data = stbi_load(selected_image_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
						image_load_failed     = pixels_data == nullptr;
						if (!pixels_data)
						{
							image_pixels.clear();
							solid_mask.clear();
						}
						else
						{
							image_pixels.resize(static_cast<size_t>(width) * height);
							solid_mask.resize(static_cast<size_t>(width) * height);
							for (int y = 0; y < height; ++y)
							{
								for (int x = 0; x < width; ++x)
								{
									const size_t pixel_index = static_cast<size_t>(height - 1 - y) * width + x;
									const size_t byte_index  = pixel_index * 4;

									image_pixels[static_cast<size_t>(y) * width + x] = glm::vec4(
									    static_cast<float>(pixels_data[byte_index + 0]) / 255.0f,
									    static_cast<float>(pixels_data[byte_index + 1]) / 255.0f,
									    static_cast<float>(pixels_data[byte_index + 2]) / 255.0f,
									    static_cast<float>(pixels_data[byte_index + 3]) / 255.0f);

									solid_mask[static_cast<size_t>(y) * width + x] =
									    image_pixels[static_cast<size_t>(y) * width + x].w > 0.5f ? 1 : 0;
								}
							}
							stbi_image_free(pixels_data);
						}
					}

					ImGui::PopID();
				}
			}

			ImGui::EndChild();

			if (!selected_image_path.empty())
			{
				ImGui::Spacing();
				ImGui::TextDisabled("Selected:");
				ImGui::SameLine();
				ImGui::TextUnformatted(
				    std::filesystem::path(selected_image_path)
				        .filename()
				        .string()
				        .c_str());
			}
			if (image_load_failed)
			{
				ImGui::TextColored(ImVec4(0.95f, 0.38f, 0.42f, 1.0f), "The selected image could not be loaded.");
			}

			if (!image_pixels.empty())
			{
				if (ImGui::Button("Load Image as Smoke"))
				{
					run_simulation_action(CodeCuda::FluidSimulation::C_MapImageToSmoke(width, height, 4,
					                                                                 image_pixels.data()));
				}
			}
			if (!solid_mask.empty())
			{
				ImGui::SameLine();
				if (ImGui::Button("Use Solid Mask"))
				{
					run_simulation_action(CodeCuda::FluidSimulation::C_MapSolidMask(width, height,
					                                                              solid_mask.data()));
				}
			}

			ImGui::SeparatorText("Color frame sequence");
			ImGui::TextDisabled("Load a folder of color frames from the animation assets path.");
			if (ImGui::Button("Refresh folders"))
			{
				scan_sequence_folders();
			}

			if (ImGui::BeginChild(
			        "##ColorSequenceFolders",
			        ImVec2(0.0f, 180.0f),
			        ImGuiChildFlags_Borders,
			        ImGuiWindowFlags_AlwaysVerticalScrollbar))
			{
				const std::filesystem::path animations_path =
				    systems::os::get_instance()->get_assets_path() + "\\Animations";
				for (int i = 0; i < static_cast<int>(sequence_folder_paths.size()); ++i)
				{
					const std::string &folder_path = sequence_folder_paths[i];
					std::error_code relative_path_error;
					const std::string folder_label =
					    std::filesystem::relative(folder_path, animations_path, relative_path_error).string();

					ImGui::PushID(i);
					const bool selected = selected_sequence_folder_path == folder_path;
					if (ImGui::Selectable(folder_label.c_str(), selected, ImGuiSelectableFlags_AllowOverlap,
					                      ImVec2(0.0f, 32.0f)))
					{
						selected_sequence_folder_path = folder_path;
					}

					const ImVec2 row_min      = ImGui::GetItemRectMin();
					const float  button_width = 60.0f;
					ImGui::SetCursorScreenPos(ImVec2(
					    ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x - button_width - 8.0f,
					    row_min.y + 4.0f));
					if (ImGui::Button("Load", ImVec2(button_width, 24.0f)))
					{
						selected_sequence_folder_path = folder_path;
						color_animator.load_frames_cpu(selected_sequence_folder_path, true);
						color_animator.frame_spacing = sequence_frame_spacing;
						color_animator.stop         = true;
						sequence_load_failed = color_animator.images_frames_cpu.empty();
					}
					ImGui::PopID();
				}
			}
			ImGui::EndChild();

			if (sequence_load_failed)
			{
				ImGui::TextColored(ImVec4(0.95f, 0.38f, 0.42f, 1.0f),
				                   "No loadable, consistently sized frames were found in that folder.");
			}

			const bool sequence_loaded = !color_animator.images_frames_cpu.empty();
			if (sequence_loaded)
			{
				ImGui::Text("Frames: %d | Size: %u x %u | Current: %d",
				            color_animator.animation_info.frame_count,
				            color_animator.animation_info.sprite_size_px.x,
				            color_animator.animation_info.sprite_size_px.y,
				            color_animator.animation_info.current_frame + 1);
			}
			ImGui::SliderInt("Frame spacing##ColorSequence", &sequence_frame_spacing, 1, 120);
			color_animator.frame_spacing = sequence_frame_spacing;

			if (!sequence_loaded)
			{
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Play##ColorSequence"))
			{
				color_animator.stop = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop##ColorSequence"))
			{
				color_animator.stop = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear##ColorSequence"))
			{
				color_animator.clear_cpu();
				sequence_load_failed = false;
			}
			if (!sequence_loaded)
			{
				ImGui::EndDisabled();
			}

		}

		static int random_velocity_scale = 10;

		ImGui::SeparatorText("Viewport Tool");
		static int   fluid_tool                        = 0;
		static int   fluid_brush_radius                 = 25;
		static float fluid_smoke_color[4]               = {0.15f, 0.05f, 0.25f, 1.0f};
		static float fluid_velocity_strength            = 0.2f;
		static float fluid_radial_velocity_strength      = 1.0f;
		static float fluid_pressure_strength            = 1.0f;
		static float fluid_emitter_velocity_direction[2] = {1.0f, 0.0f};
		static float fluid_emitter_color[4]             = {0.15f, 0.05f, 0.25f, 1.0f};
		static bool  fluid_tool_enabled                 = true;

		const char *fluid_tools[]            = {"Smoke", "Velocity", "Add solid", "Erase solid",
		                                       "Radial velocity", "Add emitter", "Pretty smoke", "Pressure"};
		const char *fluid_tool_descriptions[] = {
		    "Paint smoke with the selected color.",
		    "Drag to push the fluid in the cursor direction.",
		    "Paint solid obstacles into the simulation.",
		    "Remove solid obstacles from the simulation.",
		    "Push fluid outward from the cursor.",
		    "Place a persistent smoke and velocity source.",
		    "Paint a smooth, colorful trail that responds to your stroke.",
		    "Add signed pressure inside the brush area."};

		const int max_simulation_dimension =
		    CodeCuda::FluidSimulation::s_width > CodeCuda::FluidSimulation::s_height ? CodeCuda::FluidSimulation::s_width : CodeCuda::FluidSimulation::s_height;
		const int max_brush_radius = max_simulation_dimension > 0 ? max_simulation_dimension : 1;

		ImGui::TextDisabled("Tool");
		if (ImGui::BeginTable("##fluid_tool_selector", 2,
		                      ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings))
		{
			for (int tool_index = 0; tool_index < IM_ARRAYSIZE(fluid_tools); ++tool_index)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(tool_index);
				const bool selected = fluid_tool == tool_index;
				if (selected)
				{
					const ImVec4 accent = imgui_renderer_ui::accent_color();
					ImGui::PushStyleColor(ImGuiCol_Button, accent);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					                      ImVec4(accent.x, accent.y, accent.z, 0.90f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive,
					                      ImVec4(accent.x, accent.y, accent.z, 0.78f));
					ImGui::PushStyleColor(ImGuiCol_Text, imgui_renderer_ui::ink_color());
				}

				if (ImGui::Button(fluid_tools[tool_index], ImVec2(-FLT_MIN, 30.0f)))
				{
					if (fluid_tool != tool_index)
					{
						pretty_smoke_stroke_active = false;
					}
					fluid_tool = tool_index;
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					ImGui::SetTooltip("%s", fluid_tool_descriptions[tool_index]);
				}
				if (selected)
				{
					ImGui::PopStyleColor(4);
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		if (ImGui::BeginTable("##fluid_tool_properties", 2,
		                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			auto begin_property = [](const char *label) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(label);
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-1.0f);
			};

			begin_property("Enabled");
			ImGui::Checkbox("##fluid_tool_enabled", &fluid_tool_enabled);
			if (fluid_tool != 6)
			{
				begin_property("Brush radius");
				ImGui::DragInt("##fluid_brush_radius", &fluid_brush_radius, 1.0f, 1,
				               max_brush_radius, "%d px", ImGuiSliderFlags_AlwaysClamp);
			}

			if (fluid_tool == 0)
			{
				begin_property("Smoke color");
				ImGui::ColorEdit4("##fluid_smoke_color", fluid_smoke_color);
			}
			else if (fluid_tool == 1)
			{
				begin_property("Velocity strength");
				ImGui::DragFloat("##fluid_velocity_strength", &fluid_velocity_strength,
				                 0.01f, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			}
			else if (fluid_tool == 4)
			{
				begin_property("Radial strength");
				ImGui::DragFloat("##fluid_radial_strength", &fluid_radial_velocity_strength,
				                 0.05f, -100.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			}
			else if (fluid_tool == 5)
			{
				begin_property("Emitter velocity");
				ImGui::DragFloat2("##fluid_emitter_velocity", fluid_emitter_velocity_direction,
				                  0.05f, -10.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
				begin_property("Emitter color");
				ImGui::ColorEdit4("##fluid_emitter_color", fluid_emitter_color);
			}
			else if (fluid_tool == 6)
			{
				if (!fluid_tool_enabled)
				{
					begin_property("Pretty smoke");
					ImGui::TextDisabled("Enable the tool to tune its trail.");
				}
				else
				{
					const char *pretty_smoke_presets[] = {"Nebula", "Aurora", "Ember", "Rainbow", "Custom"};
					begin_property("Palette");
					if (ImGui::Combo("##pretty_smoke_palette", &pretty_smoke_preset,
					                 pretty_smoke_presets, IM_ARRAYSIZE(pretty_smoke_presets)))
					{
						switch (pretty_smoke_preset)
						{
							case 0:
								pretty_smoke_base_color[0] = 0.34f;
								pretty_smoke_base_color[1] = 0.08f;
								pretty_smoke_base_color[2] = 0.90f;
								pretty_smoke_opacity      = 0.30f;
								pretty_smoke_color_spread  = 0.10f;
								pretty_smoke_turbulence   = 0.18f;
								break;
							case 1:
								pretty_smoke_base_color[0] = 0.04f;
								pretty_smoke_base_color[1] = 0.86f;
								pretty_smoke_base_color[2] = 0.56f;
								pretty_smoke_opacity      = 0.28f;
								pretty_smoke_color_spread  = 0.18f;
								pretty_smoke_turbulence   = 0.24f;
								break;
							case 2:
								pretty_smoke_base_color[0] = 1.00f;
								pretty_smoke_base_color[1] = 0.14f;
								pretty_smoke_base_color[2] = 0.02f;
								pretty_smoke_opacity      = 0.36f;
								pretty_smoke_color_spread  = 0.045f;
								pretty_smoke_turbulence   = 0.12f;
								break;
							case 3:
								pretty_smoke_base_color[0] = 1.00f;
								pretty_smoke_base_color[1] = 0.12f;
								pretty_smoke_base_color[2] = 0.30f;
								pretty_smoke_opacity      = 0.30f;
								pretty_smoke_color_spread  = 0.50f;
								pretty_smoke_turbulence   = 0.22f;
								break;
							default:
								break;
						}
					}

					begin_property("Base color");
					if (ImGui::ColorEdit3("##pretty_smoke_base_color", pretty_smoke_base_color))
					{
						pretty_smoke_preset = 4;
					}
					begin_property("Opacity");
					if (ImGui::DragFloat("##pretty_smoke_opacity", &pretty_smoke_opacity,
					                     0.01f, 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
					{
						pretty_smoke_preset = 4;
					}
					begin_property("Brush radius");
					ImGui::DragInt("##pretty_smoke_brush_radius", &pretty_smoke_brush_radius,
					               1.0f, 1, max_brush_radius, "%d px", ImGuiSliderFlags_AlwaysClamp);
					begin_property("Flow strength");
					ImGui::DragFloat("##pretty_smoke_flow_strength", &pretty_smoke_flow_strength,
					                 0.01f, 0.0f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
					begin_property("Color spread");
					if (ImGui::DragFloat("##pretty_smoke_color_spread", &pretty_smoke_color_spread,
					                     0.005f, 0.0f, 0.5f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						pretty_smoke_preset = 4;
					}
					begin_property("Trail smoothness");
					ImGui::SliderFloat("##pretty_smoke_trail_smoothness", &pretty_smoke_trail_smoothness,
					                   0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
					begin_property("Turbulence");
					if (ImGui::SliderFloat("##pretty_smoke_turbulence", &pretty_smoke_turbulence,
					                       0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
					{
						pretty_smoke_preset = 4;
					}
				}
			}
			else if (fluid_tool == 7)
			{
				begin_property("Pressure strength");
				ImGui::DragFloat("##fluid_pressure_strength", &fluid_pressure_strength,
				                 0.05f, -100.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			}

			begin_property("Random force");
			const float add_button_width = 42.0f;
			ImGui::SetNextItemWidth(std::max(60.0f, ImGui::GetContentRegionAvail().x -
			                                            add_button_width - ImGui::GetStyle().ItemSpacing.x));
			ImGui::DragInt("##random_velocity_scale", &random_velocity_scale, 1.0f, 1,
			               1000, "%d", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SameLine();
			if (ImGui::Button("Add", ImVec2(add_button_width, 0.0f)))
			{
				run_simulation_action(CodeCuda::FluidSimulation::C_AddRandomVelocity(random_velocity_scale));
			}
			ImGui::EndTable();
		}

		ImGui::TextDisabled("Right-click and drag over the viewport to apply the selected tool.");

		const ImVec2 mouse_position     = ImGui::GetMousePos();
		const float  viewport_width     = std::max(1.0f, scene_viewport_max.x - scene_viewport_min.x);
		const float  viewport_height    = std::max(1.0f, scene_viewport_max.y - scene_viewport_min.y);
		const bool   mouse_over_viewport = scene_viewport_valid && scene_viewport_hovered;
		const int    active_brush_radius = fluid_tool == 6 ? pretty_smoke_brush_radius : fluid_brush_radius;
		if (fluid_tool_enabled && mouse_over_viewport)
		{
			const float radius_pixels = std::max(
			    2.0f, active_brush_radius * 0.5f *
			              (viewport_width / std::max(1, CodeCuda::FluidSimulation::s_width) +
			               viewport_height / std::max(1, CodeCuda::FluidSimulation::s_height)));
			ImGui::GetForegroundDrawList()->AddCircle(
			    mouse_position, radius_pixels,
			    ImGui::GetColorU32(imgui_renderer_ui::accent_color()), 48, 1.5f);
		}
		if (fluid_tool_enabled && mouse_over_viewport &&
		    ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			const float u = glm::clamp(
			    (mouse_position.x - scene_viewport_min.x) / viewport_width, 0.0f, 1.0f);
			const float v = glm::clamp(
			    1.0f - (mouse_position.y - scene_viewport_min.y) / viewport_height, 0.0f, 1.0f);
			const int x_position = static_cast<int>(u * static_cast<float>(CodeCuda::FluidSimulation::s_width - 1));
			const int y_position = static_cast<int>(v * static_cast<float>(CodeCuda::FluidSimulation::s_height - 1));

			switch (fluid_tool)
			{
				case 0:
					run_simulation_action(CodeCuda::FluidSimulation::C_AddSmoke(x_position, y_position, fluid_brush_radius,
					                                                          fluid_smoke_color[0], fluid_smoke_color[1],
					                                                          fluid_smoke_color[2], fluid_smoke_color[3]));
					break;
				case 1:
				{
					const ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
					const float  velocity_x  = mouse_delta.x * static_cast<float>(CodeCuda::FluidSimulation::s_width) / viewport_width;
					const float  velocity_y  = mouse_delta.y * static_cast<float>(CodeCuda::FluidSimulation::s_height) / viewport_height;
					run_simulation_action(CodeCuda::FluidSimulation::C_AddVelocity(x_position, y_position, fluid_brush_radius,
					                                                             velocity_x * fluid_velocity_strength,
					                                                             -velocity_y * fluid_velocity_strength));
					break;
				}
				case 2:
					run_simulation_action(CodeCuda::FluidSimulation::C_SetSolid(x_position, y_position, fluid_brush_radius, true));
					break;
				case 3:
					run_simulation_action(CodeCuda::FluidSimulation::C_SetSolid(x_position, y_position, fluid_brush_radius, false));
					break;
				case 4:
					run_simulation_action(CodeCuda::FluidSimulation::C_AddRadialVelocity(x_position, y_position, fluid_brush_radius,
					                                                                   fluid_radial_velocity_strength));
					break;
				case 6:
				{
					auto *cuda_node = render_graph ? render_graph->get_node("CudaNode") : nullptr;
					if (!cuda_node || !cuda_node->cuda_pipeline || !cuda_node->cuda_pipeline->context)
					{
						simulation_action_fail = true;
						break;
					}
					add_smoke_pretty(cuda_node->cuda_pipeline->context,
					               glm::vec2(u * 1023.0f, (1.0f - v) * 1023.0f));
					simulation_action_fail = false;
					break;
				}
				case 7:
					run_simulation_action(CodeCuda::FluidSimulation::C_AddPressure(
					    x_position, y_position, fluid_brush_radius, fluid_pressure_strength));
					break;
				default:
					break;
			}
		}
		if (fluid_tool != 6 || !fluid_tool_enabled || !mouse_over_viewport ||
		    !ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			pretty_smoke_stroke_active = false;
		}
		if (fluid_tool_enabled && mouse_over_viewport &&
		    ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			const float u = glm::clamp(
			    (mouse_position.x - scene_viewport_min.x) / viewport_width, 0.0f, 1.0f);
			const float v = glm::clamp(
			    1.0f - (mouse_position.y - scene_viewport_min.y) / viewport_height, 0.0f, 1.0f);
			const int x_position = static_cast<int>(u * static_cast<float>(CodeCuda::FluidSimulation::s_width - 1));
			const int y_position = static_cast<int>(v * static_cast<float>(CodeCuda::FluidSimulation::s_height - 1));
			switch (fluid_tool)
			{
				case 5:
				{
					glm::vec4 col = glm::make_vec4(fluid_emitter_color);
					glm::vec2 vel = glm::make_vec2(fluid_emitter_velocity_direction);
					emitters.emplace_back(emitter{col, vel, fluid_brush_radius, x_position, y_position});
					break;
				}
				default:
					break;
			}
		}
		if (simulation_action_fail)
		{
			ImGui::TextDisabled("The last simulation action failed.");
		}

		const std::string emitter_section_label =
		    "Emitters (" + std::to_string(emitters.size()) + ")###fluid_emitters";
		if (ImGui::CollapsingHeader(emitter_section_label.c_str()))
		{
			if (emitters.empty())
			{
				ImGui::TextDisabled("Choose Add Emitter, then right-click the viewport.");
			}
			int emitter_to_remove = -1;

			for (int i = 0; i < static_cast<int>(emitters.size()); ++i)
			{
				ImGui::PushID(i);
				auto &active_emitter = emitters[i];

				if (ImGui::TreeNode("Emitter", "Emitter %d", i + 1))
				{
					ImGui::DragInt("X", &active_emitter.x_pos, 1.0f, 0, CodeCuda::FluidSimulation::s_width - 1);
					ImGui::DragInt("Y", &active_emitter.y_pos, 1.0f, 0, CodeCuda::FluidSimulation::s_height - 1);
					ImGui::DragInt("Radius", &active_emitter.radius, 1.0f, 1,
					               std::max(CodeCuda::FluidSimulation::s_width, CodeCuda::FluidSimulation::s_height));
					ImGui::DragFloat2("Velocity", &active_emitter.velocity.x, 0.05f,
					                  -10.0f, 10.0f);
					ImGui::ColorEdit4("Color", &active_emitter.color.x);

					if (ImGui::Button("Remove emitter"))
					{
						emitter_to_remove = i;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			if (emitter_to_remove >= 0)
			{
				emitters.erase(emitters.begin() + emitter_to_remove);
			}
		}

		static int  resolution_width         = CodeCuda::FluidSimulation::s_width;
		static int  resolution_height        = CodeCuda::FluidSimulation::s_height;
		static bool resolution_change_pending = false;
		static bool resolution_update_failed  = false;

		if (!resolution_change_pending)
		{
			resolution_width  = CodeCuda::FluidSimulation::s_width;
			resolution_height = CodeCuda::FluidSimulation::s_height;
		}

		auto begin_settings_property = [](const char *label) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-1.0f);
		};
		auto snap_dragged_resolution = [](int resolution) {
			constexpr int resolution_step = 32;
			return std::clamp(
			    ((resolution + resolution_step / 2) / resolution_step) * resolution_step,
			    resolution_step,
			    4096);
		};

		bool width_changed  = false;
		bool width_active   = false;
		bool height_changed = false;
		bool height_active  = false;
		if (ImGui::CollapsingHeader("Resolution###fluid_resolution"))
		{
			if (ImGui::BeginTable("##fluid_resolution_properties", 2,
			                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				begin_settings_property("Width");
				width_changed = ImGui::DragInt("##fluid_resolution_width", &resolution_width,
				                              32.0f, 1, 4096, "%d",
				                              ImGuiSliderFlags_AlwaysClamp);
				width_active  = ImGui::IsItemActive();
				if (width_changed && width_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					resolution_width = snap_dragged_resolution(resolution_width);
				}
				begin_settings_property("Height");
				height_changed = ImGui::DragInt("##fluid_resolution_height", &resolution_height,
				                               32.0f, 1, 4096, "%d",
				                               ImGuiSliderFlags_AlwaysClamp);
				height_active  = ImGui::IsItemActive();
				if (height_changed && height_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					resolution_height = snap_dragged_resolution(resolution_height);
				}
				ImGui::EndTable();
			}
		}
		resolution_change_pending |= width_changed || height_changed;

		if (resolution_change_pending && !width_active && !height_active)
		{
			resolution_update_failed =
			    CodeCuda::FluidSimulation::C_SetSimulationResolution(resolution_width, resolution_height) != CodeCuda::C_Res::OK;
			resolution_change_pending = false;
		}
		if (resolution_update_failed)
		{
			ImGui::TextColored(ImVec4(0.95f, 0.38f, 0.42f, 1.0f),
			                   "Unable to update fluid simulation resolution.");
		}

		if (ImGui::CollapsingHeader("Simulation###fluid_simulation",
		                            ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("##fluid_simulation_properties", 2,
			                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				begin_settings_property("Density");
				params_changed |= ImGui::DragFloat("##fluid_density", &fluid_sim_params.density,
				                                  0.01f, 0.001f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				begin_settings_property("Velocity dissipation");
				params_changed |= ImGui::DragFloat("##fluid_velocity_dissipation",
				                                  &fluid_sim_params.velocity_dissipation,
				                                  0.001f, 0.0f, 0.2f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				begin_settings_property("Smoke dissipation");
				params_changed |= ImGui::DragFloat("##fluid_smoke_dissipation",
				                                  &fluid_sim_params.smoke_dissipation,
				                                  0.001f, 0.0f, 0.2f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);

				int time_step_denominator = fluid_sim_params.dt > 0.0f ? static_cast<int>((1.0f / fluid_sim_params.dt) + 0.5f) : 120;
				begin_settings_property("Time step");
				if (ImGui::DragInt("##fluid_time_step", &time_step_denominator, 1.0f,
				                   1, 10000, "1 / %d", ImGuiSliderFlags_AlwaysClamp))
				{
					fluid_sim_params.dt = 1.0f / static_cast<float>(time_step_denominator);
					params_changed     = true;
				}
				begin_settings_property("Gravity");
				params_changed |= ImGui::DragFloat("##fluid_gravity", &fluid_sim_params.g,
				                                  0.01f, -100.0f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				begin_settings_property("Wind speed");
				params_changed |= ImGui::DragFloat("##fluid_wind_speed", &fluid_sim_params.wind_speed,
				                                  0.01f, -100.0f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				begin_settings_property("Viscosity");
				params_changed |= ImGui::DragFloat("##fluid_viscosity", &fluid_sim_params.viscosity,
				                                  0.01f, 0.0f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				begin_settings_property("Smoke diffusion");
				params_changed |= ImGui::DragFloat("##fluid_smoke_diffusion",
				                                  &fluid_sim_params.smoke_diffuse_coef,
				                                  0.01f, 0.0f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Solver###fluid_solver"))
		{
			if (ImGui::BeginTable("##fluid_solver_properties", 2,
			                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				begin_settings_property("SOR weight");
				params_changed |= ImGui::DragFloat("##fluid_sor_weight", &fluid_sim_params.weight_sor,
				                                  0.01f, 0.0f, 2.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				begin_settings_property("GPU iterations");
				params_changed |= ImGui::DragInt("##fluid_gpu_iterations",
				                                &fluid_sim_params.total_iter_gpu, 1.0f,
				                                1, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);
				begin_settings_property("CPU iterations");
				params_changed |= ImGui::DragInt("##fluid_cpu_iterations",
				                                &fluid_sim_params.total_iter_cpu, 1.0f,
				                                1, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);
				begin_settings_property("Debug output");
				params_changed |= ImGui::Checkbox("##fluid_debug", &fluid_sim_params.debug);
				ImGui::EndTable();
			}
		}

		if (params_changed && CodeCuda::FluidSimulation::C_SetSimulationParams(&fluid_sim_params) != CodeCuda::C_Res::OK)
		{
			ImGui::TextDisabled("Unable to update fluid simulation parameters.");
		}
	}

	void set_style()
	{
		imgui_renderer_ui::apply_modern_theme();
		fonts = imgui_renderer_ui::configure_fonts(systems::os::get_instance()->get_engine_resources_path());
	}
	void display_general_engine_info()
	{
		display_engine_info();
		display_all_textures();
	}

	void render_metric_card(const char *id, const char *label, const std::string &value)
	{
		ImGui::PushID(id);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, imgui_renderer_ui::raised_surface_color());
		if (ImGui::BeginChild("##metric", ImVec2(0.0f, 62.0f), ImGuiChildFlags_None,
		                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImGui::PushFont(fonts.caption);
			ImGui::TextDisabled("%s", label);
			ImGui::PopFont();
			ImGui::PushFont(fonts.strong);
			ImGui::TextUnformatted(value.c_str());
			ImGui::PopFont();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopID();
	}

	void display_engine_info()
	{
		auto     *queue_worker_manager = core->queue_worker_manager.get();
		const int engine_queue_count   = static_cast<int>(queue_worker_manager->workers_queues.size());
		const int render_node_count    = static_cast<int>(render_graph->sorted_by_dep_nodes.size());
		const int queue_batch_count    = static_cast<int>(render_graph->sorted_queue_batches.size());
		int       active_node_count    = 0;
		for (const auto *node : render_graph->sorted_by_dep_nodes)
		{
			if (node && node->active)
			{
				active_node_count++;
			}
		}

		if (ImGui::BeginTable("engine_summary", 4,
		                      ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextColumn();
			render_metric_card("queues", "QUEUES", std::to_string(engine_queue_count));
			ImGui::TableNextColumn();
			render_metric_card("passes", "RENDER PASSES", std::to_string(render_node_count));
			ImGui::TableNextColumn();
			render_metric_card("active", "ACTIVE PASSES", std::to_string(active_node_count));
			ImGui::TableNextColumn();
			render_metric_card("batches", "QUEUE BATCHES", std::to_string(queue_batch_count));
			ImGui::EndTable();
		}

		ImGui::SeparatorText("Queue Members");
		if (ImGui::BeginTable("engine_queue_members", 8,
		                      ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH |
		                          ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
		                          ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Family");
			ImGui::TableSetupColumn("Main Thread");
			ImGui::TableSetupColumn("Cmd Pools");
			ImGui::TableSetupColumn("Cmds/Pool");
			ImGui::TableSetupColumn("Active Cmd");
			ImGui::TableSetupColumn("Current Pool");
			ImGui::TableSetupColumn("Queue Handle");
			ImGui::TableHeadersRow();

			for (auto &queue_pair : queue_worker_manager->workers_queues)
			{
				const auto &queue = queue_pair.second;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", queue.name.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%d", queue.family_index);
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", queue.is_main_threat ? "true" : "false");
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%d", queue.cmds_pool_size);
				ImGui::TableSetColumnIndex(4);
				ImGui::Text("%d", queue.per_cmd_pool_size);
				ImGui::TableSetColumnIndex(5);
				ImGui::Text("%d", queue.active_cmd_idx);
				ImGui::TableSetColumnIndex(6);
				ImGui::Text("%d", queue.current_pool_cmd_idx);
				ImGui::TableSetColumnIndex(7);
				ImGui::Text("%p", static_cast<VkQueue>(queue.queue_handle));
			}
			ImGui::EndTable();
		}

		ImGui::SeparatorText("Sorted Node Order");
		if (ImGui::BeginTable("sorted_node_order", 4,
		                      ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH |
		                          ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Order", ImGuiTableColumnFlags_WidthFixed, 64.0f);
			ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Queue", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 84.0f);
			ImGui::TableHeadersRow();
			for (int i = 0; i < render_graph->sorted_by_dep_nodes.size(); ++i)
			{
				auto *node = render_graph->sorted_by_dep_nodes[i];
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%02d", i);
				ImGui::TableSetColumnIndex(1);
				ImGui::TextUnformatted(node ? node->pass_name.c_str() : "Unavailable");
				ImGui::TableSetColumnIndex(2);
				ImGui::TextUnformatted(node ? node->worker_queue_name.c_str() : "-");
				ImGui::TableSetColumnIndex(3);
				const bool node_active = node && node->active;
				ImGui::TextColored(node_active ? imgui_renderer_ui::success_color() : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled),
				                   "%s", node_active ? "Active" : "Inactive");
			}
			ImGui::EndTable();
		}

		display_render_graph_dag();

		display_queue_execution_timeline();
	}

	void cluster_renderer_info()
	{
		ImGui::SeparatorText("Light Info");

		float speed = 0.01f;
		for (auto &point_light : cluster_renderer->point_lights)
		{
			if (point_light.pos.y >= 20.0f)
			{
				point_light.pos.y = 0;
			}
			point_light.pos += glm::vec3(0.0f, 1.0f, 0.0f) * speed;
			point_light.calculate_q_attenuation_from_radius();
		}
		static float point_light_radiuses = 1.0f;
		if (ImGui::SliderFloat("Point lights Radiuses", &point_light_radiuses, 1.0f, 20.0f))
		{
			for (auto &point_light : cluster_renderer->point_lights)
			{
				point_light.radius = point_light_radiuses;
				point_light.calculate_q_attenuation_from_radius();
			}
		}
		static float point_quadratic_attenuation = 1.0f;
		if (ImGui::SliderFloat("Point lights Quadratic Attenuation", &point_quadratic_attenuation, 1.0f, 30.0f))
		{
			for (auto &point_light : cluster_renderer->point_lights)
			{
				point_light.q_attenuation = point_quadratic_attenuation;
				point_light.calculate_radius_from_params();
				point_light_radiuses = point_light.radius;
			}
		}
		static float point_light_linear_attenuation = 1.0f;
		if (ImGui::SliderFloat("Point lights Linear Attenuation", &point_light_linear_attenuation, 1.0f, 30.0f))
		{
			for (auto &point_light : cluster_renderer->point_lights)
			{
				point_light.l_attenuation = point_light_linear_attenuation;
				point_light.calculate_radius_from_params();
				point_light_radiuses = point_light.radius;
			}
		}
		static float point_light_intensity = 1.0f;
		if (ImGui::SliderFloat("Point lights Intensity", &point_light_intensity, 0.0f, 30.0f))
		{
			for (auto &point_light : cluster_renderer->point_lights)
			{
				point_light.intensity = point_light_intensity;
				point_light.calculate_radius_from_params();
				point_light_radiuses = point_light.radius;
			}
		}

		ImGui::SeparatorText("Tile/Cluster renderer");

		static int x_tile_size_px = 256;
		static int y_tile_size_px = 256;
		static int z_slices_size = 24;

		if (ImGui::SliderInt("x tile size (px): ", &x_tile_size_px, 32, 512))
		{
			cluster_renderer->x_tile_size_px = x_tile_size_px;
		}
		if (ImGui::SliderInt("y tile size (px): ", &y_tile_size_px, 32, 512))
		{
			cluster_renderer->y_tile_size_px = y_tile_size_px;
		}
		if (ImGui::SliderInt("number of z slices: ", &z_slices_size, 1, 28))
		{
			cluster_renderer->z_slices_size = z_slices_size;
		}

		ImGui::SeparatorText("Cull Info");

		std::string cull_count = "Cull Count: " + std::to_string(rendering_res_manager::get_instance()->cull_count) + " / " + std::to_string(rendering_res_manager::get_instance()->indirect_draws_cmd_infos.size());

		ImGui::Text("%s", cull_count.c_str());

		ImGui::SeparatorText("First Person Camera Info");

		std::string camera_pos = "Position: (" + std::to_string(cluster_renderer->camera.position.x) + ", " + std::to_string(cluster_renderer->camera.position.y) + ", " + std::to_string(cluster_renderer->camera.position.z) + ")";

		std::string camera_forward = "Forward: (" + std::to_string(cluster_renderer->camera.forward.x) + ", " + std::to_string(cluster_renderer->camera.forward.y) + ", " + std::to_string(cluster_renderer->camera.forward.z) + ")";
		std::string camera_right   = "Right: (" + std::to_string(cluster_renderer->camera.right.x) + ", " + std::to_string(cluster_renderer->camera.right.y) + ", " + std::to_string(cluster_renderer->camera.right.z) + ")";
		std::string camera_up      = "Up: (" + std::to_string(cluster_renderer->camera.up.x) + ", " + std::to_string(cluster_renderer->camera.up.y) + ", " + std::to_string(cluster_renderer->camera.up.z) + ")";

		ImGui::Text("%s", camera_pos.c_str());
		ImGui::Text("%s", camera_forward.c_str());
		ImGui::Text("%s", camera_right.c_str());
		ImGui::Text("%s", camera_up.c_str());

		ImGui::SeparatorText("Virtual Cam Info");
		static bool detached_cam = false;
		if (ImGui::Checkbox("Debug Cull", &detached_cam))
		{
			if (detached_cam)
			{
				cluster_renderer->debug_cam.position = cluster_renderer->camera.position;
				cluster_renderer->debug_cam.yaw      = cluster_renderer->camera.yaw;
				cluster_renderer->debug_cam.pitch    = cluster_renderer->camera.pitch;
			}
		}
		if (detached_cam)
		{
			cluster_renderer->curr_camera = &cluster_renderer->debug_cam;
			cluster_renderer->debug_cam.update_cam();
			cluster_renderer->debug_cam.rotate_camera();
			static float yaw   = cluster_renderer->debug_cam.yaw;
			static float pitch = cluster_renderer->debug_cam.pitch;

			if (ImGui::SliderFloat("Yaw", &yaw, 0.0f, 360.0f))
			{
				cluster_renderer->debug_cam.yaw = yaw;
			}
			if (ImGui::SliderFloat("Pitch", &pitch, 0.0f, 360.0f))
			{
				cluster_renderer->debug_cam.pitch = pitch;
			}
			static float pos[3] = {
			    cluster_renderer->debug_cam.position.x, cluster_renderer->debug_cam.position.y,
			    cluster_renderer->debug_cam.position.z};

			if (ImGui::Button("Snap Cam to view", {50, 50}))
			{
				cluster_renderer->debug_cam.position = cluster_renderer->camera.position;
				cluster_renderer->debug_cam.yaw      = cluster_renderer->camera.yaw;
				cluster_renderer->debug_cam.pitch    = cluster_renderer->camera.pitch;
			};
			cluster_renderer->debug_cam.update_cam();
			cluster_renderer->debug_cam.rotate_camera();
		}
		else
		{
			cluster_renderer->curr_camera = &cluster_renderer->camera;
		}

		struct node_info
		{
			bool       *active;
			std::string name;
		};
		static std::vector<node_info> node_infos;

		for (auto &node : core->render_graph_ref->sequential_render_nodes)
		{
			node_infos.emplace_back(node_info{&node->active, node->pass_name});
		}

		ImGui::SeparatorText("Render Nodes");
		for (auto &node_info : node_infos)
		{
			if (node_info.name == "BlitterNode")
			{
				continue;
			}
			std::string name = "Node: " + node_info.name;
			ImGui::Checkbox(name.c_str(), node_info.active);
		}
		node_infos.clear();
	}

	void display_render_graph_dag();

	void display_queue_execution_timeline()
	{
		ImGui::SeparatorText("Queue Execution Timeline");

		if (render_graph->sorted_queue_batches.empty())
		{
			ImGui::Text("No queue batches");
			return;
		}

		static int selected_batch_idx = 0;
		if (selected_batch_idx < 0 || selected_batch_idx >= render_graph->sorted_queue_batches.size())
		{
			selected_batch_idx = 0;
		}

		std::vector<std::string>   queue_names;
		std::map<std::string, int> queue_lane_by_name;
		std::map<std::string, int> queue_batch_count;
		std::map<std::string, int> queue_node_count;
		int                        total_node_count  = 0;
		int                        active_node_count = 0;
		int                        empty_batch_count = 0;
		for (const auto &batch : render_graph->sorted_queue_batches)
		{
			if (!queue_lane_by_name.contains(batch.queue_name))
			{
				queue_lane_by_name[batch.queue_name] = static_cast<int>(queue_names.size());
				queue_names.push_back(batch.queue_name);
			}
			queue_batch_count[batch.queue_name]++;
			if (batch.sorted_nodes.empty())
			{
				empty_batch_count++;
			}
			for (auto *node : batch.sorted_nodes)
			{
				if (!node)
				{
					continue;
				}
				total_node_count++;
				queue_node_count[batch.queue_name]++;
				if (node->active)
				{
					active_node_count++;
				}
			}
		}

		ImGui::Text("Batches: %d | Queues: %d | Nodes: %d | Active nodes: %d | Empty batches: %d",
		            static_cast<int>(render_graph->sorted_queue_batches.size()),
		            static_cast<int>(queue_names.size()),
		            total_node_count,
		            active_node_count,
		            empty_batch_count);

		const ImU32 queue_colors[] = {
		    IM_COL32(119, 69, 48, 255),
		    IM_COL32(93, 85, 69, 255),
		    IM_COL32(88, 70, 77, 255),
		    IM_COL32(74, 83, 82, 255),
		    IM_COL32(112, 82, 52, 255),
		    IM_COL32(96, 72, 65, 255),
		};
		const ImU32 lane_fill      = IM_COL32(18, 17, 18, 245);
		const ImU32 lane_border    = IM_COL32(84, 75, 73, 125);
		const ImU32 selected_color = IM_COL32(255, 82, 20, 255);
		const ImU32 node_fill      = IM_COL32(36, 32, 33, 255);
		const ImU32 inactive_fill  = IM_COL32(49, 40, 41, 255);
		const ImU32 empty_fill     = IM_COL32(47, 41, 34, 255);
		const ImU32 text_color     = IM_COL32(235, 231, 228, 255);
		const ImU32 muted_color    = IM_COL32(151, 143, 141, 255);

		const float  label_width  = 150.0f;
		const float  lane_height  = 94.0f;
		const float  batch_gap    = 18.0f;
		const float  batch_height = 62.0f;
		const float  node_height  = 20.0f;
		const ImVec2 padding(18.0f, 18.0f);

		float timeline_width = label_width + padding.x * 2.0f;
		for (const auto &batch : render_graph->sorted_queue_batches)
		{
			timeline_width += std::max(170.0f, 74.0f * std::max(1, static_cast<int>(batch.sorted_nodes.size()))) + batch_gap;
		}
		float timeline_height = padding.y * 2.0f + std::max(1, static_cast<int>(queue_names.size())) * lane_height;

		ImGui::BeginChild("queue_execution_timeline_canvas", ImVec2(0.0f, 320.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		ImVec2      origin   = ImGui::GetCursorScreenPos();

		for (int lane = 0; lane < queue_names.size(); ++lane)
		{
			float  y = origin.y + padding.y + lane * lane_height;
			ImVec2 lane_min(origin.x + padding.x, y);
			ImVec2 lane_max(origin.x + timeline_width - padding.x, y + lane_height - 12.0f);
			draw_list->AddRectFilled(lane_min, lane_max, lane_fill, 6.0f);
			draw_list->AddRect(lane_min, lane_max, lane_border, 6.0f);
			draw_list->AddText(ImVec2(lane_min.x + 10.0f, lane_min.y + 9.0f), text_color, queue_names[lane].c_str());
			std::string lane_meta = "batches " + std::to_string(queue_batch_count[queue_names[lane]]) +
			                       " | nodes " + std::to_string(queue_node_count[queue_names[lane]]);
			draw_list->AddText(ImVec2(lane_min.x + 10.0f, lane_min.y + 30.0f), muted_color, lane_meta.c_str());
		}

		float x_cursor = origin.x + padding.x + label_width;
		for (int batch_idx = 0; batch_idx < render_graph->sorted_queue_batches.size(); ++batch_idx)
		{
			const auto &batch      = render_graph->sorted_queue_batches[batch_idx];
			int         lane       = queue_lane_by_name.at(batch.queue_name);
			float       batch_width = std::max(170.0f, 74.0f * std::max(1, static_cast<int>(batch.sorted_nodes.size())));
			float       y          = origin.y + padding.y + lane * lane_height + 14.0f;
			ImVec2      min_pos(x_cursor, y);
			ImVec2      max_pos(x_cursor + batch_width, y + batch_height);
			bool        selected = selected_batch_idx == batch_idx;
			ImU32       fill     = batch.sorted_nodes.empty() ? empty_fill : queue_colors[lane % IM_ARRAYSIZE(queue_colors)];

			draw_list->AddRectFilled(min_pos, max_pos, fill, 6.0f);
			draw_list->AddRect(min_pos, max_pos, selected ? selected_color : lane_border, 6.0f, 0, selected ? 3.0f : 1.0f);

			std::string batch_title = "#" + std::to_string(batch_idx) + " batch " + std::to_string(batch.id);
			draw_list->AddText(ImVec2(min_pos.x + 9.0f, min_pos.y + 7.0f), text_color, batch_title.c_str());
			std::string batch_meta = "pool " + std::to_string(batch.pool_id_used) + " | nodes " + std::to_string(batch.sorted_nodes.size());
			draw_list->AddText(ImVec2(min_pos.x + 9.0f, min_pos.y + 26.0f), muted_color, batch_meta.c_str());

			if (batch.sorted_nodes.empty())
			{
				draw_list->AddText(ImVec2(min_pos.x + 9.0f, min_pos.y + 45.0f), muted_color, "no render nodes");
			}
			else
			{
				float node_x = min_pos.x + 9.0f;
				for (int node_idx = 0; node_idx < batch.sorted_nodes.size(); ++node_idx)
				{
					engine::render_graph_node *node      = batch.sorted_nodes[node_idx];
					float            chip_width = std::min(96.0f, std::max(48.0f, (batch_width - 18.0f) / static_cast<float>(batch.sorted_nodes.size()) - 4.0f));
					ImVec2           chip_min(node_x, min_pos.y + 41.0f);
					ImVec2           chip_max(node_x + chip_width, chip_min.y + node_height);
					draw_list->AddRectFilled(chip_min, chip_max, node && node->active ? node_fill : inactive_fill, 4.0f);
					draw_list->AddRect(chip_min, chip_max, IM_COL32(118, 104, 101, 130), 4.0f);
					std::string node_label = node ? node->pass_name : "null";
					if (node_label.size() > 10)
					{
						node_label = node_label.substr(0, 9) + ".";
					}
					draw_list->AddText(ImVec2(chip_min.x + 5.0f, chip_min.y + 3.0f), text_color, node_label.c_str());
					node_x += chip_width + 4.0f;
				}
			}

			ImGui::SetCursorScreenPos(min_pos);
			ImGui::PushID(batch_idx);
			if (ImGui::InvisibleButton("queue_batch_hitbox", ImVec2(batch_width, batch_height)))
			{
				selected_batch_idx = batch_idx;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Batch index: %d", batch_idx);
				ImGui::Text("Batch id: %d", batch.id);
				ImGui::Text("Queue: %s", batch.queue_name.c_str());
				ImGui::Text("Pool id: %d", batch.pool_id_used);
				ImGui::Text("Nodes: %d", static_cast<int>(batch.sorted_nodes.size()));
				for (int node_idx = 0; node_idx < batch.sorted_nodes.size(); ++node_idx)
				{
					engine::render_graph_node *node = batch.sorted_nodes[node_idx];
					ImGui::BulletText("%02d. %s%s", node_idx, node ? node->pass_name.c_str() : "null", node && !node->active ? " (inactive)" : "");
				}
				ImGui::EndTooltip();
			}
			ImGui::PopID();

			x_cursor += batch_width + batch_gap;
		}

		ImGui::SetCursorScreenPos(origin);
		ImGui::Dummy(ImVec2(timeline_width, timeline_height));
		ImGui::EndChild();

		const auto &selected_batch = render_graph->sorted_queue_batches[selected_batch_idx];
		ImGui::SeparatorText("Selected Queue Batch");
		if (ImGui::BeginTable("selected_queue_batch", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Order");
			ImGui::TableSetupColumn("Pass");
			ImGui::TableSetupColumn("Queue");
			ImGui::TableSetupColumn("State");
			ImGui::TableSetupColumn("Dependencies");
			ImGui::TableHeadersRow();

			for (int node_idx = 0; node_idx < selected_batch.sorted_nodes.size(); ++node_idx)
			{
				engine::render_graph_node *node = selected_batch.sorted_nodes[node_idx];
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%02d", node_idx);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", node ? node->pass_name.c_str() : "null");
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", node ? node->worker_queue_name.c_str() : selected_batch.queue_name.c_str());
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%s", node && node->active ? "active" : "inactive");
				ImGui::TableSetColumnIndex(4);
				if (node && !node->dependencies.empty())
				{
					std::string dependencies;
					for (const auto &dependency : node->dependencies)
					{
						if (!dependencies.empty())
						{
							dependencies += ", ";
						}
						dependencies += dependency;
					}
					ImGui::TextWrapped("%s", dependencies.c_str());
				}
				else
				{
					ImGui::TextUnformatted("none");
				}
			}

			if (selected_batch.sorted_nodes.empty())
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d", selected_batch_idx);
				ImGui::TableSetColumnIndex(1);
				ImGui::TextUnformatted("no render nodes");
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", selected_batch.queue_name.c_str());
				ImGui::TableSetColumnIndex(3);
				ImGui::TextUnformatted("empty");
				ImGui::TableSetColumnIndex(4);
				ImGui::Text("batch id %d | pool %d", selected_batch.id, selected_batch.pool_id_used);
			}

			ImGui::EndTable();
		}
	}

	void painting_info()
	{
		ImGui::SliderInt("Brush Radius", &flat_renderer->painting_pc.radius, 1, 100);

		static float color[4] = {1.0f, 1.0f, 1.0f, 1.0};
		if (ImGui::ColorEdit4("Brush Radius", color))
		{
			flat_renderer->painting_pc.color = glm::make_vec4(color);
		}
		ImGui::SliderInt("Layer", &flat_renderer->painting_pc.layer_selected, 0, 1);

		if (ImGui::Button("Clear Canvas"))
		{
			resources_manager::get_instance()->request_storage_image_clear("PaintingLayer");
			resources_manager::get_instance()->request_storage_image_clear("OccluderLayer");
			resources_manager::get_instance()->request_storage_image_clear("DebugRaysLayer");
		}
		for (int i = 0; i < flat_renderer->cascades_info.cascade_count; ++i)
		{
			std::string name = "radianceStorage_" + std::to_string(i);
			resources_manager::get_instance()->request_storage_image_clear(name);
		}
	}
	void animator_info()
	{
		int i = 0;
		for (const auto &animator_pair : rendering_res_manager::get_instance()->animators_names)
		{
			auto        animator         = rendering_res_manager::get_instance()->get_animator_by_name(animator_pair.first);
			std::string frame_spacing_name = animator_pair.first + ": Frame Spacing";
			ImGui::SliderInt(frame_spacing_name.c_str(), &animator->frame_spacing, 1, 1000);

			std::string frame_info = animator_pair.first + " Frames Info: " + std::to_string(animator->animation_info.current_frame) + " / " + std::to_string(animator->animation_info.frame_count);
			ImGui::Text("%s", frame_info.c_str());

			std::string frame_spacing_info = animator_pair.first + " Frame Spacing info: " + std::to_string(animator->current_frame_spacing) + " / " + std::to_string(animator->frame_spacing);
			ImGui::Text("%s", frame_spacing_info.c_str());

			std::string interp_info = animator_pair.first + " Interpolation info: " + std::to_string(animator->animation_info.interp_val) +
			                         " / 1.0";
			ImGui::Text("%s", interp_info.c_str());

			std::string stop_anim = animator_pair.first + ": Stop anim";
			ImGui::Checkbox(stop_anim.c_str(), &animator->stop);
			if (animator->stop)
			{
				std::string frame_index_label_name = animator_pair.first + ": Frame index";
				ImGui::SliderInt(frame_index_label_name.c_str(), &animator->animation_info.current_frame, 1, animator->animation_info.frame_count);
			}
		}
	}

	void r_cascades_info()
	{
		// ImGui::Begin("Radiance Output Info");
		//
		// ImVec2 viewport_size = ImGui::GetContentRegionAvail();
		//
		// std::vector<image_view*> cascades = flat_renderer->cascades_attachments_images_views;
		//
		// for (int i = 0; i < cascades.size(); ++i)
		// {
		// 	std::string image_name = "cascade_" + std::to_string(i);
		// 	AddImage(imageName ,cascades[i], viewportSize);
		// }
		// std::vector<image_view*> painting_layers = flat_renderer->painting_layers;
		//
		// ImGui::End();
		//
		ImGui::SeparatorText("Cascades Configs");
		static int probe_size_px = flat_renderer->cascades_info.probe_size_px;
		if (ImGui::SliderInt("Probe Size in Px", &probe_size_px, 2, 1024))
		{
			flat_renderer->cascades_info.probe_size_px = probe_size_px;
		}
		static int interval_count = flat_renderer->cascades_info.interval_count;
		if (ImGui::SliderInt("Interval Count", &interval_count, 1, 16))
		{
			flat_renderer->cascades_info.interval_count = interval_count;
		}

		static int base_interval_length = flat_renderer->cascades_info.base_interval_length;
		if (ImGui::SliderInt("Base Interval Length", &base_interval_length, 1, 1000))
		{
			flat_renderer->cascades_info.base_interval_length = base_interval_length;
		}

		ImGui::SeparatorText("Light Configs");
		static float light_dir[3] = {0.0, 1.0, 0.0};
		if (ImGui::SliderFloat3("Light Dir", light_dir, 0.0, 1.0))
		{
			flat_renderer->light.pos = glm::make_vec3(light_dir);
		}
		static float light_col[3] = {0.0, 0.0, 1.0};
		if (ImGui::ColorEdit3("Light Col", light_col))
		{
			flat_renderer->light.col = glm::make_vec3(light_col);
		}
		static float intensity = flat_renderer->light.intensity;
		if (ImGui::SliderFloat("Light Intensity", &intensity, 0.0, 1.0))
		{
			flat_renderer->light.intensity = intensity;
		}
		ImGui::SeparatorText("Texture Configs");
		static int radiance_pow = flat_renderer->r_configs.radiance_pow;
		if (ImGui::SliderInt("Radiance Pow", &radiance_pow, 1, 24))
		{
			flat_renderer->r_configs.radiance_pow = radiance_pow;
		}
		static int normal_map_pow = flat_renderer->r_configs.normal_map_pow;
		if (ImGui::SliderInt("Normal Map Pow", &normal_map_pow, 1, 24))
		{
			flat_renderer->r_configs.normal_map_pow = normal_map_pow;
		}
		static int specular_pow = flat_renderer->r_configs.specular_pow;
		if (ImGui::SliderInt("SpecularPow Pow", &specular_pow, 1, 24))
		{
			flat_renderer->r_configs.specular_pow = specular_pow;
		}
		static int roughness_pow = flat_renderer->r_configs.roughness_pow;
		if (ImGui::SliderInt("Roughness Pow", &roughness_pow, 1, 24))
		{
			flat_renderer->r_configs.roughness_pow = roughness_pow;
		}

		ImGui::SeparatorText("Background Material");
		static int material_selected = flat_renderer->material_index_selected;
		if (ImGui::SliderInt("Material Selected", &material_selected, 0, flat_renderer->background_materials.size() - 1))
		{
			flat_renderer->material_index_selected = material_selected;
		}
		display_material(flat_renderer->background_materials.at(flat_renderer->material_index_selected));

		painting_info();

		animator_info();
	}

	void gs_renderer_info()
	{
		if (!gs_renderer)
			return;

		ImGui::SeparatorText("Gaussian Info");
		int gaussian_count = gs_renderer->gaussians.pos.size();
		ImGui::Text("Gaussian Count: %d", gaussian_count);
		int draw_count = gs_renderer->indexed_cmds.size();
		ImGui::Text("Indirect Draw Count: %d", draw_count);

		ImGui::SeparatorText("GS Configs");

		// Bind to GSConfigsPc fields
		ImGui::SliderFloat("Global Scale", &gs_renderer->gs_configs_pc.scale_mod, 0.01f, 10.0f);

		// static const char* renderModes[] = {"Default", "Debug Ellipsoids", "BBox Only"};
		// ImGui::Combo("Render Mode", &gs_renderer->gs_configs_pc.renderMode, renderModes, IM_ARRAYSIZE(renderModes));

		ImGui::SeparatorText("Camera Info");
		ImGui::SliderFloat("Camera Speed", &gs_renderer->camera.movement_speed, 0.01f, 100.0f);

		const auto &cam = gs_renderer->camera;
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", cam.position.x, cam.position.y, cam.position.z);
		ImGui::Text("Forward: (%.2f, %.2f, %.2f)", cam.forward.x, cam.forward.y, cam.forward.z);
		ImGui::Text("Movement Speed: %.2f", cam.movement_speed);

		if (ImGui::Button("Update Sort"))
		{
			gs_renderer->re_sort();
		}
	}

	void add_image(std::string name, engine::image_view *image_view_handle)
	{
		if (name == "default_storage" || name == "default_tex")
		{
			return;
		}
		engine::sampler       *sampler    = resources_manager::get_instance()->shipper_sampler;
		layout_patterns last_layout = image_view_handle->image_metadata->current_layout;
		layout_patterns_to_recover.push_back(last_layout);
		image_views_to_recover.push_back(image_view_handle);

		if (dsets_arrays->indexes.contains(name))
		{
			transition_image(image_view_handle->image_metadata, last_layout, image_view_handle->get_subresource_range(),
			                *curr_command_buffer);
			return;
		}
		dsets_arrays->add_set(name);
		engine::descriptor_writer_builder writer_builder;
		writer_builder.add_write_image(0, image_view_handle, sampler->sampler_handle.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
		writer_builder.update_set(core->logical_device.get(), dsets_arrays->get_dset_by_name(name));
	}
	void display_material(rendering::material *mat)
	{
		ui::texture_viewer texture_viewer_base_col;
		texture_viewer_base_col.add_property(ui::drag);
		texture_viewer_base_col.add_property(ui::drop);
		for (auto &texture : mat->textures_ref)
		{
			if (texture.second == nullptr)
			{
				continue;
			}
			std::string name = mat->textures_strings.at(texture.first);
			add_image(texture.second->name, texture.second);
			engine::image_view *image_view_ref = texture_viewer_base_col.display_texture(name, texture.second, (ImTextureID) dsets_arrays->get_dset_by_name(texture.second->name), {50, 50});
			if (image_view_ref->name != texture.second->name)
			{
				texture.second = image_view_ref;
			}
		}
	}
	void add_all_images()
	{
		for (auto &image : render_graph->resources_manager->image_views)
		{
			add_image(image->name, image.get());
		}
		for (auto &image : render_graph->resources_manager->storage_imgs_views)
		{
			add_image(image->name, image.get());
		}
		for (auto &image : render_graph->resources_manager->image_shippers)
		{
			add_image(image->image_view_handle->name, image->image_view_handle.get());
		}
	}
	void display_all_textures()
	{
		add_all_images();
		static char text_buff[256] = "";
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##texture_filter", "Filter textures by name...", text_buff, 256);
		std::string input(text_buff);

		std::vector<engine::image_view *> visible_images;
		for (auto &image : render_graph->resources_manager->image_views)
		{
			if (!input.empty() && image->name.find(input) == std::string::npos)
			{
				continue;
			}
			visible_images.push_back(image.get());
		}
		for (auto &image : render_graph->resources_manager->storage_imgs_views)
		{
			if (image->name == "default_storage")
			{
				continue;
			}
			if (!input.empty() && image->name.find(input) == std::string::npos)
			{
				continue;
			}
			visible_images.push_back(image.get());
		}
		for (auto &image : render_graph->resources_manager->image_shippers)
		{
			if (image->image_view_handle->name == "default_tex")
			{
				continue;
			}
			if (!input.empty() && image->image_view_handle->name.find(input) == std::string::npos)
			{
				continue;
			}
			visible_images.push_back(image->image_view_handle.get());
		}

		ImGui::TextDisabled("%d texture%s visible", static_cast<int>(visible_images.size()),
		                    visible_images.size() == 1 ? "" : "s");
		ImGui::Spacing();

		if (visible_images.empty())
		{
			ImGui::TextDisabled("No textures match the current filter.");
			return;
		}

		const float available_width   = ImGui::GetContentRegionAvail().x;
		const float minimum_card_width = 190.0f;
		const float column_spacing    = ImGui::GetStyle().ItemSpacing.x;
		const int   column_count      = std::max(1, static_cast<int>((available_width + column_spacing) /
		                                                            (minimum_card_width + column_spacing)));
		const float card_width        = (available_width - column_spacing * static_cast<float>(column_count - 1)) /
		                        static_cast<float>(column_count);
		const float preview_size = std::min(220.0f, std::max(96.0f, card_width - 24.0f));

		if (ImGui::BeginTable("texture_card_grid", column_count,
		                      ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings))
		{
			for (engine::image_view *image_view_handle : visible_images)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(image_view_handle);
				ImGui::PushStyleColor(ImGuiCol_ChildBg, imgui_renderer_ui::raised_surface_color());
				if (ImGui::BeginChild("##texture_card", ImVec2(0.0f, preview_size + 68.0f),
				                      ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar))
				{
					ImGui::PushFont(fonts.strong);
					ImGui::TextUnformatted(image_view_handle->name.c_str());
					ImGui::PopFont();
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s", image_view_handle->name.c_str());
					}

					const glm::uvec2 dimensions = image_view_handle->image_metadata->get_image_size();
					ImGui::TextDisabled("%u x %u", dimensions.x, dimensions.y);
					const float preview_offset = std::max(0.0f, (ImGui::GetContentRegionAvail().x - preview_size) * 0.5f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + preview_offset);
					ImGui::Image(
					    (ImTextureID) dsets_arrays->get_dset_by_name(image_view_handle->name),
					    ImVec2(preview_size, preview_size), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s\n%u x %u", image_view_handle->name.c_str(), dimensions.x, dimensions.y);
					}
				}
				ImGui::EndChild();
				ImGui::PopStyleColor();
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
	void destroy()
	{
		if (m_context)
		{
			ed::DestroyEditor(m_context);
			m_context = nullptr;
		}
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	vk::CommandBuffer  *curr_command_buffer = nullptr;
	dynamic_render_pass   dynamic_render_pass;
	::window_provider     *window_provider;
	engine::descriptor_allocator descriptor_allocator;
	VkFormat            imgui_swapchain_format = VK_FORMAT_UNDEFINED;
	engine::core               *core;
	engine::render_graph        *render_graph;

	std::map<std::string, std::unique_ptr<base_renderer>> *renderers       = nullptr;
	rendering::cluster_renderer                                      *cluster_renderer = nullptr;
	rendering::flat_renderer                                         *flat_renderer    = nullptr;
	rendering::gs_renderer                                           *gs_renderer      = nullptr;
	rendering::cfd_renderer                                          *cfd_renderer     = nullptr;
	pc_voxel_grid                                          lattice_config{};
	bool                                                  lattice_config_initialized = false;
	ImGuiUtils::ProfilersWindow                           profilers_window{};
	ui::rg_node_editor                                     node_editor;
	imgui_renderer_ui::fonts                                fonts{};
	debugger_page                                          active_page           = debugger_page::render_graph;
	bool                                                  scene_viewport_valid   = false;
	bool                                                  scene_viewport_hovered = false;
	ImVec2                                                scene_viewport_min{};
	ImVec2                                                scene_viewport_max{};
	float                                                 inspector_panel_width    = 0.0f;
	bool                                                  inspector_panel_resizing = false;
	glm::vec2                                             pretty_smoke_last_mouse_position{};
	glm::vec4                                             pretty_smoke_last_color{0.15f, 0.05f, 0.25f, 1.0f};
	bool                                                  pretty_smoke_stroke_active    = false;
	float                                                 pretty_smoke_base_color[3]    = {0.34f, 0.08f, 0.90f};
	float                                                 pretty_smoke_opacity         = 0.30f;
	float                                                 pretty_smoke_color_spread     = 0.10f;
	float                                                 pretty_smoke_flow_strength    = 0.20f;
	float                                                 pretty_smoke_trail_smoothness = 0.74f;
	float                                                 pretty_smoke_turbulence      = 0.18f;
	int                                                   pretty_smoke_brush_radius     = 18;
	int                                                   pretty_smoke_preset          = 0;

	std::unique_ptr<imgui_dsets_array> dsets_arrays;
	std::vector<layout_patterns>      layout_patterns_to_recover;
	std::vector<engine::image_view *>         image_views_to_recover;

	std::vector<glm::vec4> image_pixels = {};
	std::vector<int>       solid_mask   = {};
	int                    width, height, channels;
	struct emitter
	{
		glm::vec4 color;
		glm::vec2 velocity;
		int       radius;
		int       x_pos;
		int       y_pos;
	};
	animator_2d color_animator;
	std::vector<emitter> emitters;
};
inline void imgui_renderer::display_render_graph_dag()
{
	ImGui::SeparatorText("Render Graph DAG");

	std::vector<engine::render_graph_node *> graph_nodes = render_graph->sorted_by_dep_nodes;
	if (graph_nodes.empty())
	{
		graph_nodes = render_graph->sequential_render_nodes;
	}
	if (graph_nodes.empty())
	{
		ImGui::Text("No render graph nodes");
		return;
	}

	static std::string selected_node_name;

	std::map<std::string, engine::render_graph_node *> node_by_name;
	std::map<std::string, int>               level_by_name;
	std::map<std::string, int>               incoming_count_by_name;
	std::map<std::string, int>               outgoing_count_by_name;
	int                                      active_node_count        = 0;
	int                                      edge_count              = 0;
	int                                      missing_dependency_count = 0;
	for (auto *node : graph_nodes)
	{
		if (!node)
		{
			continue;
		}
		node_by_name[node->pass_name]          = node;
		level_by_name[node->pass_name]         = 0;
		incoming_count_by_name[node->pass_name] = 0;
		outgoing_count_by_name[node->pass_name] = 0;
		if (node->active)
		{
			active_node_count++;
		}
	}

	for (auto *node : graph_nodes)
	{
		if (!node)
		{
			continue;
		}
		incoming_count_by_name[node->pass_name] = static_cast<int>(node->dependencies.size());
		edge_count += static_cast<int>(node->dependencies.size());
		for (const auto &dependency : node->dependencies)
		{
			if (outgoing_count_by_name.contains(dependency))
			{
				outgoing_count_by_name.at(dependency)++;
			}
			else
			{
				missing_dependency_count++;
			}
		}
	}

	if (selected_node_name.empty() || !node_by_name.contains(selected_node_name))
	{
		selected_node_name = graph_nodes.front() ? graph_nodes.front()->pass_name : "";
	}

	ImGui::Text("Nodes: %d | Active: %d | Edges: %d | Queue batches: %d | Missing deps: %d",
	            static_cast<int>(node_by_name.size()),
	            active_node_count,
	            edge_count,
	            static_cast<int>(render_graph->sorted_queue_batches.size()),
	            missing_dependency_count);

	int max_level = 0;
	for (auto *node : graph_nodes)
	{
		if (!node)
		{
			continue;
		}
		int node_level = 0;
		for (const auto &dependency : node->dependencies)
		{
			if (level_by_name.contains(dependency))
			{
				node_level = std::max(node_level, level_by_name.at(dependency) + 1);
			}
		}
		level_by_name[node->pass_name] = node_level;
		max_level                    = std::max(max_level, node_level);
	}

	std::vector<std::vector<engine::render_graph_node *>> nodes_by_level(max_level + 1);
	for (auto *node : graph_nodes)
	{
		if (!node)
		{
			continue;
		}
		nodes_by_level[level_by_name.at(node->pass_name)].push_back(node);
	}

	int max_rows = 1;
	for (const auto &level_nodes : nodes_by_level)
	{
		max_rows = std::max(max_rows, static_cast<int>(level_nodes.size()));
	}

	const ImVec2 node_size(220.0f, 66.0f);
	const float  level_gap = 100.0f;
	const float  row_gap   = 28.0f;
	const ImVec2 padding(24.0f, 24.0f);
	const ImVec2 canvas_size(
	    padding.x * 2.0f + (max_level + 1) * node_size.x + max_level * level_gap,
	    padding.y * 2.0f + max_rows * node_size.y + std::max(0, max_rows - 1) * row_gap);

	ImGui::BeginChild("render_graph_dag_canvas", ImVec2(0.0f, 420.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2      origin   = ImGui::GetCursorScreenPos();

	std::map<std::string, ImVec2> node_min_by_name;
	std::map<std::string, ImVec2> node_max_by_name;
	for (int level = 0; level < nodes_by_level.size(); ++level)
	{
		const auto &level_nodes  = nodes_by_level[level];
		float       level_height = level_nodes.size() * node_size.y + std::max(0, static_cast<int>(level_nodes.size()) - 1) * row_gap;
		float       y_offset     = std::max(0.0f, (canvas_size.y - padding.y * 2.0f - level_height) * 0.5f);
		for (int row = 0; row < level_nodes.size(); ++row)
		{
			engine::render_graph_node *node = level_nodes[row];
			ImVec2           min_pos(
                origin.x + padding.x + level * (node_size.x + level_gap),
                origin.y + padding.y + y_offset + row * (node_size.y + row_gap));
			ImVec2 max_pos(min_pos.x + node_size.x, min_pos.y + node_size.y);
			node_min_by_name[node->pass_name] = min_pos;
			node_max_by_name[node->pass_name] = max_pos;
		}
	}

	const ImU32 edge_color      = IM_COL32(126, 108, 103, 160);
	const ImU32 edge_arrow_color = IM_COL32(176, 145, 135, 205);
	for (auto *node : graph_nodes)
	{
		if (!node || !node_min_by_name.contains(node->pass_name))
		{
			continue;
		}
		for (const auto &dependency : node->dependencies)
		{
			if (!node_max_by_name.contains(dependency))
			{
				continue;
			}
			ImVec2 src(node_max_by_name.at(dependency).x, (node_min_by_name.at(dependency).y + node_max_by_name.at(dependency).y) * 0.5f);
			ImVec2 dst(node_min_by_name.at(node->pass_name).x, (node_min_by_name.at(node->pass_name).y + node_max_by_name.at(node->pass_name).y) * 0.5f);
			float  mid_x = (src.x + dst.x) * 0.5f;
			draw_list->AddBezierCubic(src, ImVec2(mid_x, src.y), ImVec2(mid_x, dst.y), dst, edge_color, 2.0f);
			draw_list->AddTriangleFilled(
			    ImVec2(dst.x, dst.y),
			    ImVec2(dst.x - 8.0f, dst.y - 5.0f),
			    ImVec2(dst.x - 8.0f, dst.y + 5.0f),
			    edge_arrow_color);
		}
	}

	const ImU32 active_fill          = IM_COL32(50, 42, 40, 255);
	const ImU32 inactive_fill        = IM_COL32(38, 34, 36, 255);
	const ImU32 selected_fill        = IM_COL32(76, 45, 34, 255);
	const ImU32 warning_fill         = IM_COL32(80, 54, 36, 255);
	const ImU32 border_color         = IM_COL32(125, 111, 107, 185);
	const ImU32 selected_border_color = IM_COL32(255, 82, 20, 255);
	const ImU32 warning_color        = IM_COL32(245, 172, 95, 255);
	const ImU32 text_color           = IM_COL32(235, 231, 228, 255);
	const ImU32 muted_color          = IM_COL32(151, 143, 141, 255);
	for (auto *node : graph_nodes)
	{
		if (!node || !node_min_by_name.contains(node->pass_name))
		{
			continue;
		}
		ImVec2 min_pos               = node_min_by_name.at(node->pass_name);
		ImVec2 max_pos               = node_max_by_name.at(node->pass_name);
		bool   has_missing_dependency = false;
		for (const auto &dependency : node->dependencies)
		{
			if (!node_by_name.contains(dependency))
			{
				has_missing_dependency = true;
				break;
			}
		}
		bool  selected  = selected_node_name == node->pass_name;
		ImU32 fill_color = has_missing_dependency ? warning_fill : (selected ? selected_fill : (node->active ? active_fill : inactive_fill));
		draw_list->AddRectFilled(min_pos, max_pos, fill_color, 6.0f);
		draw_list->AddRect(min_pos, max_pos, selected ? selected_border_color : (has_missing_dependency ? warning_color : border_color), 6.0f, 0, selected ? 3.0f : 1.0f);
		draw_list->AddText(ImVec2(min_pos.x + 10.0f, min_pos.y + 7.0f), text_color, node->pass_name.c_str());
		std::string node_meta = node->worker_queue_name + (node->active ? " | active" : " | inactive");
		draw_list->AddText(ImVec2(min_pos.x + 10.0f, min_pos.y + 24.0f), muted_color, node_meta.c_str());
		std::string edge_meta = "in " + std::to_string(incoming_count_by_name[node->pass_name]) +
		                       " | out " + std::to_string(outgoing_count_by_name[node->pass_name]);
		draw_list->AddText(ImVec2(min_pos.x + 10.0f, min_pos.y + 43.0f), has_missing_dependency ? warning_color : muted_color, edge_meta.c_str());

		ImGui::SetCursorScreenPos(min_pos);
		ImGui::PushID(node->pass_name.c_str());
		if (ImGui::InvisibleButton("dag_node_hitbox", node_size))
		{
			selected_node_name = node->pass_name;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%s", node->pass_name.c_str());
			ImGui::Text("Queue: %s", node->worker_queue_name.c_str());
			ImGui::Text("Dependencies: %d", static_cast<int>(node->dependencies.size()));
			ImGui::Text("Dependents: %d", outgoing_count_by_name[node->pass_name]);
			ImGui::Text("Shader node: %s", node->gpu_pipeline_ref ? node->gpu_pipeline_ref->name.c_str() : "null");
			ImGui::EndTooltip();
		}
		ImGui::PopID();
	}

	ImGui::SetCursorScreenPos(origin);
	ImGui::Dummy(canvas_size);
	ImGui::EndChild();

	if (!selected_node_name.empty() && node_by_name.contains(selected_node_name))
	{
		engine::render_graph_node *selected_node = node_by_name.at(selected_node_name);
		engine::gpu_pipeline     *shader_node   = selected_node->gpu_pipeline_ref;

		ImGui::SeparatorText("Selected Node");
		if (ImGui::BeginTable("selected_render_graph_node", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Field");
			ImGui::TableSetupColumn("Value");
			ImGui::TableHeadersRow();

			auto add_row = [](const char *name, const std::string &value) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(name);
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped("%s", value.c_str());
			};

			auto pipeline_name = [](vk::PipelineBindPoint bind_point) {
				switch (bind_point)
				{
					case vk::PipelineBindPoint::eGraphics:
						return "Graphics";
					case vk::PipelineBindPoint::eCompute:
						return "Compute";
					default:
						return "Unknown";
				}
			};

			auto image_map_names = [](const std::unordered_map<std::string, engine::image_view *> &images) {
				std::string names;
				for (const auto &image : images)
				{
					if (!names.empty())
					{
						names += ", ";
					}
					names += image.first;
					if (image.second)
					{
						names += " -> ";
						names += image.second->name;
					}
					else
					{
						names += " -> null";
					}
				}
				return names.empty() ? std::string("none") : names;
			};

			auto buffer_names = [](const std::unordered_map<std::string, buffer_key> &buffers) {
				std::string names;
				for (const auto &buffer : buffers)
				{
					if (!names.empty())
					{
						names += ", ";
					}
					names += buffer.first;
					if (!buffer.second.name.empty() && buffer.second.name != buffer.first)
					{
						names += " -> ";
						names += buffer.second.name;
					}
				}
				return names.empty() ? std::string("none") : names;
			};

			add_row("Pass", selected_node->pass_name);
			add_row("Queue", selected_node->worker_queue_name);
			add_row("State", selected_node->active ? "active" : "inactive");
			add_row("Level", std::to_string(level_by_name[selected_node->pass_name]));
			add_row("Dependencies", std::to_string(selected_node->dependencies.size()));
			add_row("Dependents", std::to_string(outgoing_count_by_name[selected_node->pass_name]));
			add_row("Shader node", shader_node ? shader_node->name : "null");
			add_row("Pipeline", shader_node ? pipeline_name(shader_node->pipeline_type) : "n/a");
			add_row("Push constant bytes", shader_node ? std::to_string(shader_node->push_constant_size) : "n/a");
			add_row("Color attachments", shader_node ? std::to_string(shader_node->out_col_attachments_bindings.size()) : "n/a");
			add_row("Depth attachment", shader_node ? (shader_node->depth_attachment.format == vk::Format::eUndefined ? "none" : "present") : "n/a");
			add_row("Framebuffer", std::to_string(selected_node->get_frame_buffer_size().x) + " x " + std::to_string(selected_node->get_frame_buffer_size().y));
			add_row("Attachment outputs", std::to_string(selected_node->get_image_attachment_outputs().size()));
			add_row("Attachment names", std::to_string(selected_node->get_image_attachment_names().size()));
			add_row("Depth image", selected_node->get_depth_image() ? selected_node->get_depth_image()->name : "none");
			add_row("Sampled images", std::to_string(selected_node->get_sampled_images().size()));
			add_row("Sampled image names", image_map_names(selected_node->get_sampled_images()));
			add_row("Storage images", std::to_string(selected_node->get_storage_images().size()));
			add_row("Storage image names", image_map_names(selected_node->get_storage_images()));
			add_row("Buffers", std::to_string(selected_node->get_buffers().size()));
			add_row("Buffer names", buffer_names(selected_node->get_buffers()));
			add_row("Descriptor cache", shader_node ? (shader_node->desc_cache ? "present" : "null") : "n/a");
			add_row("Auto descriptor cache", shader_node ? (shader_node->configs.automatic_cache ? "true" : "false") : "n/a");
			add_row("Manual add", shader_node ? (shader_node->configs.manual_add ? "true" : "false") : "n/a");

			std::string shader_stages;
			if (shader_node)
			{
				for (const auto &shader : shader_node->shaders)
				{
					if (shader.second)
					{
						if (!shader_stages.empty())
						{
							shader_stages += ", ";
						}
						shader_stages += shader.first;
					}
				}
			}
			add_row("Shaders", shader_stages.empty() ? "none" : shader_stages);

			std::string dependencies;
			for (const auto &dependency : selected_node->dependencies)
			{
				if (!dependencies.empty())
				{
					dependencies += ", ";
				}
				dependencies += dependency;
				if (!node_by_name.contains(dependency))
				{
					dependencies += " (missing)";
				}
			}
			add_row("Depends on", dependencies.empty() ? "none" : dependencies);

			std::string dependents;
			for (auto *node : graph_nodes)
			{
				if (node && node->dependencies.contains(selected_node->pass_name))
				{
					if (!dependents.empty())
					{
						dependents += ", ";
					}
					dependents += node->pass_name;
				}
			}
			add_row("Used by", dependents.empty() ? "none" : dependents);

			ImGui::EndTable();
		}
	}
}
}        // namespace rendering

#endif        // IMGUI_RENDERER_HPP
