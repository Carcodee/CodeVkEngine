//
// Created by carlo on 2024-10-10.
//

#ifndef IMGUIRENDERER_HPP
#define IMGUIRENDERER_HPP
#include "CodeCuda.cuh"

namespace Rendering
{
namespace ed = ax::NodeEditor;

namespace ImguiRendererUI
{
struct Fonts
{
	ImFont *body    = nullptr;
	ImFont *caption = nullptr;
	ImFont *heading = nullptr;
	ImFont *strong  = nullptr;
};

inline ImVec4 WithAlpha(const ImVec4 &color, float alpha)
{
	return ImVec4(color.x, color.y, color.z, alpha);
}

inline ImVec4 AccentColor()
{
	return ImVec4(1.00f, 0.32f, 0.08f, 1.00f);
}

inline ImVec4 SuccessColor()
{
	return ImVec4(0.78f, 0.82f, 0.74f, 1.00f);
}

inline ImVec4 SelectionColor()
{
	return ImVec4(0.94f, 0.93f, 0.91f, 1.00f);
}

inline ImVec4 InkColor()
{
	return ImVec4(0.075f, 0.070f, 0.072f, 1.00f);
}

inline ImVec4 RaisedSurfaceColor()
{
	return ImVec4(0.095f, 0.087f, 0.088f, 0.92f);
}

inline Fonts ConfigureFonts(const std::string &resourcesPath)
{
	const std::string fontsPath  = resourcesPath + "\\Fonts\\Open_Sans\\static\\";
	const std::string regularTtf = fontsPath + "OpenSans-Regular.ttf";
	const std::string boldTtf    = fontsPath + "OpenSans-Bold.ttf";

	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->Clear();

	Fonts fonts{};
	if (std::filesystem::exists(regularTtf))
	{
		fonts.body    = io.Fonts->AddFontFromFileTTF(regularTtf.c_str(), 16.0f);
		fonts.caption = io.Fonts->AddFontFromFileTTF(regularTtf.c_str(), 13.0f);
	}
	if (!fonts.body)
	{
		fonts.body = io.Fonts->AddFontDefault();
	}
	if (!fonts.caption)
	{
		fonts.caption = fonts.body;
	}

	if (std::filesystem::exists(boldTtf))
	{
		fonts.heading = io.Fonts->AddFontFromFileTTF(boldTtf.c_str(), 22.0f);
		fonts.strong  = io.Fonts->AddFontFromFileTTF(boldTtf.c_str(), 16.0f);
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

inline void ApplyModernTheme()
{
	ImGuiStyle &style = ImGui::GetStyle();
	ImGui::StyleColorsDark(&style);

	style.WindowPadding             = ImVec2(9.0f, 9.0f);
	style.FramePadding              = ImVec2(9.0f, 6.0f);
	style.CellPadding               = ImVec2(9.0f, 7.0f);
	style.ItemSpacing               = ImVec2(8.0f, 7.0f);
	style.ItemInnerSpacing          = ImVec2(7.0f, 5.0f);
	style.TouchExtraPadding         = ImVec2(0.0f, 0.0f);
	style.IndentSpacing             = 18.0f;
	style.ScrollbarSize             = 9.0f;
	style.GrabMinSize               = 9.0f;
	style.WindowBorderSize          = 0.0f;
	style.ChildBorderSize           = 0.0f;
	style.PopupBorderSize           = 1.0f;
	style.FrameBorderSize           = 0.0f;
	style.TabBorderSize             = 0.0f;
	style.WindowRounding            = 12.0f;
	style.ChildRounding             = 9.0f;
	style.FrameRounding             = 7.0f;
	style.PopupRounding             = 9.0f;
	style.ScrollbarRounding         = 10.0f;
	style.GrabRounding              = 8.0f;
	style.TabRounding               = 7.0f;
	style.SeparatorTextBorderSize   = 0.0f;
	style.SeparatorTextAlign        = ImVec2(0.0f, 0.5f);
	style.SeparatorTextPadding      = ImVec2(0.0f, 11.0f);
	style.SelectableTextAlign       = ImVec2(0.0f, 0.5f);
	style.WindowTitleAlign          = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition  = ImGuiDir_None;
	style.ColorButtonPosition       = ImGuiDir_Right;

	const ImVec4 accent       = AccentColor();
	const ImVec4 text         = ImVec4(0.90f, 0.89f, 0.88f, 1.00f);
	const ImVec4 textMuted    = ImVec4(0.50f, 0.47f, 0.47f, 1.00f);
	const ImVec4 canvas       = ImVec4(0.055f, 0.050f, 0.052f, 0.88f);
	const ImVec4 surface      = ImVec4(0.045f, 0.042f, 0.045f, 0.91f);
	const ImVec4 surfaceHigh  = RaisedSurfaceColor();
	const ImVec4 interaction  = ImVec4(0.145f, 0.132f, 0.132f, 1.00f);
	const ImVec4 border       = ImVec4(0.28f, 0.24f, 0.23f, 0.38f);
	ImVec4       *colors      = style.Colors;

	colors[ImGuiCol_Text]                 = text;
	colors[ImGuiCol_TextDisabled]         = textMuted;
	colors[ImGuiCol_WindowBg]             = canvas;
	colors[ImGuiCol_ChildBg]              = surface;
	colors[ImGuiCol_PopupBg]              = surface;
	colors[ImGuiCol_Border]               = border;
	colors[ImGuiCol_BorderShadow]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_FrameBg]              = surfaceHigh;
	colors[ImGuiCol_FrameBgHovered]       = interaction;
	colors[ImGuiCol_FrameBgActive]        = WithAlpha(accent, 0.22f);
	colors[ImGuiCol_TitleBg]              = surface;
	colors[ImGuiCol_TitleBgActive]        = surface;
	colors[ImGuiCol_TitleBgCollapsed]     = surface;
	colors[ImGuiCol_MenuBarBg]            = surface;
	colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.03f, 0.028f, 0.03f, 0.35f);
	colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.28f, 0.25f, 0.25f, 0.82f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.34f, 0.32f, 0.92f);
	colors[ImGuiCol_ScrollbarGrabActive]  = WithAlpha(accent, 0.78f);
	colors[ImGuiCol_CheckMark]            = accent;
	colors[ImGuiCol_SliderGrab]           = WithAlpha(accent, 0.78f);
	colors[ImGuiCol_SliderGrabActive]     = accent;
	colors[ImGuiCol_Button]               = surfaceHigh;
	colors[ImGuiCol_ButtonHovered]        = interaction;
	colors[ImGuiCol_ButtonActive]         = WithAlpha(accent, 0.26f);
	colors[ImGuiCol_Header]               = surfaceHigh;
	colors[ImGuiCol_HeaderHovered]        = interaction;
	colors[ImGuiCol_HeaderActive]         = WithAlpha(accent, 0.24f);
	colors[ImGuiCol_Separator]            = border;
	colors[ImGuiCol_SeparatorHovered]     = WithAlpha(accent, 0.65f);
	colors[ImGuiCol_SeparatorActive]      = accent;
	colors[ImGuiCol_ResizeGrip]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_ResizeGripHovered]    = WithAlpha(accent, 0.65f);
	colors[ImGuiCol_ResizeGripActive]     = accent;
	colors[ImGuiCol_Tab]                  = surface;
	colors[ImGuiCol_TabHovered]           = interaction;
	colors[ImGuiCol_TabSelected]          = WithAlpha(accent, 0.22f);
	colors[ImGuiCol_TabDimmed]            = canvas;
	colors[ImGuiCol_TabDimmedSelected]    = surfaceHigh;
	colors[ImGuiCol_PlotLines]            = accent;
	colors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.00f, 0.58f, 0.30f, 1.00f);
	colors[ImGuiCol_PlotHistogram]        = accent;
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.58f, 0.30f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]        = surfaceHigh;
	colors[ImGuiCol_TableBorderStrong]    = border;
	colors[ImGuiCol_TableBorderLight]     = WithAlpha(border, 0.50f);
	colors[ImGuiCol_TableRowBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_TableRowBgAlt]        = ImVec4(1.0f, 1.0f, 1.0f, 0.025f);
	colors[ImGuiCol_TextSelectedBg]       = WithAlpha(accent, 0.30f);
	colors[ImGuiCol_NavHighlight]         = accent;
	colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.01f, 0.015f, 0.025f, 0.76f);
}

inline bool NavigationItem(const char *label, bool selected)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.08f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_Text, selected ? InkColor() : ImGui::GetStyleColorVec4(ImGuiCol_Text));
	ImGui::PushStyleColor(ImGuiCol_Button, selected ? SelectionColor() : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? SelectionColor() : RaisedSurfaceColor());
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,
	                      selected ? ImVec4(0.83f, 0.81f, 0.79f, 1.0f) : WithAlpha(AccentColor(), 0.20f));
	const bool pressed = ImGui::Button(label, ImVec2(ImGui::GetContentRegionAvail().x, 34.0f));
	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar();

	if (selected)
	{
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(max.x - 14.0f, (min.y + max.y) * 0.5f),
		                                             3.0f, ImGui::GetColorU32(AccentColor()));
	}
	return pressed;
}
}        // namespace ImguiRendererUI

class ImguiRenderer
{
  public:
	struct ImguiDsetsArray
	{
		~ImguiDsetsArray() = default;
		ImguiDsetsArray(Core *core, DescriptorAllocator *descriptorAllocator)
		{
			this->core                = core;
			this->descriptorAllocator = descriptorAllocator;
		}
		void AddSet(std::string name)
		{
			if (indexes.contains(name))
			{
				return;
			}
			ENGINE::DescriptorLayoutBuilder builder;
			builder.AddBinding(0, vk::DescriptorType::eCombinedImageSampler);
			auto dstLayout = builder.BuildBindings(core->logicalDevice.get(), vk::ShaderStageFlagBits::eFragment);
			auto dset      = descriptorAllocator->Allocate(core->logicalDevice.get(), dstLayout.get());
			indexes.try_emplace(name, dsets.size());
			descriptorSetLayouts.emplace_back(std::move(dstLayout));
			dsets.emplace_back(std::move(dset));
		}
		vk::DescriptorSet GetDsetByName(std::string name)
		{
			return dsets.at(indexes.at(name)).get();
		}
		vk::DescriptorSetLayout GetLayoutByName(std::string name)
		{
			return descriptorSetLayouts.at(indexes.at(name)).get();
		}

		Core                                      *core                = nullptr;
		DescriptorAllocator                       *descriptorAllocator = nullptr;
		std::map<std::string, int>                 indexes;
		std::vector<vk::UniqueDescriptorSet>       dsets;
		std::vector<vk::UniqueDescriptorSetLayout> descriptorSetLayouts;
	};
	ImguiRenderer(RenderGraph *renderGraph, WindowProvider *windowProvider, std::map<std::string, std::unique_ptr<BaseRenderer>> &renderers)
	{
		this->core        = renderGraph->core;
		this->renderGraph = renderGraph;
		this->renderers   = &renderers;

		if (renderers.contains("ClusterRenderer"))
		{
			this->clusterRenderer = dynamic_cast<ClusterRenderer *>(renderers.at("ClusterRenderer").get());
		}
		if (renderers.contains("FlatRenderer"))
		{
			this->flatRenderer = dynamic_cast<FlatRenderer *>(renderers.at("FlatRenderer").get());
		}
		if (renderers.contains("GSRenderer"))
		{
			this->gsRenderer = dynamic_cast<GSRenderer *>(renderers.at("GSRenderer").get());
		}
		this->windowProvider = windowProvider;

		std::vector<ENGINE::DescriptorAllocator::PoolSizeRatio> poolSizeRatios = {
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
		descriptorAllocator.BeginPool(core->logicalDevice.get(), 1000, poolSizeRatios);

		this->dsetsArrays = std::make_unique<ImguiDsetsArray>(core, &descriptorAllocator);

		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForVulkan(windowProvider->window, true);

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance                  = core->instance.get();
		initInfo.PhysicalDevice            = core->physicalDevice;
		initInfo.Device                    = core->logicalDevice.get();
		initInfo.Queue                     = core->queueWorkerManager->GetWorkerQueue("UI")->workerQueue;
		initInfo.QueueFamily               = core->queueWorkerManager->GetWorkerQueue("UI")->familyIndex;
		initInfo.DescriptorPool            = descriptorAllocator.pool.get();
		initInfo.MinImageCount             = 3;
		initInfo.ImageCount                = 3;
		initInfo.UseDynamicRendering       = true;

		initInfo.PipelineRenderingCreateInfo                         = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
		initInfo.PipelineRenderingCreateInfo.colorAttachmentCount    = 1;
		imguiSwapchainFormat                                         = static_cast<VkFormat>(core->swapchainRef->GetFormat());
		initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &imguiSwapchainFormat;

		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		SetStyle();

		ImGui_ImplVulkan_Init(&initInfo);

		ImGui_ImplVulkan_CreateFontsTexture();

		ed::Config config;
		config.SettingsFile = "Simple.json";
		m_Context           = ed::CreateEditor(&config);

		std::string resourcesPath = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath();
		std::string fileName      = resourcesPath + "\\Images\\f3.png";
	}
	ed::EditorContext *m_Context = nullptr;
	void               StartNodeEditor()
	{
		ed::SetCurrentEditor(m_Context);
		nodeEditor.Init(renderGraph, windowProvider);
		nodeEditor.Draw();
		ed::SetCurrentEditor(nullptr);
	}

	void RenderFrame(vk::CommandBuffer commandBuffer, vk::ImageView &imageView)
	{
		currCommandBuffer = &commandBuffer;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		RenderDebuggerWindow();

		ImGui::Render();
		ENGINE::AttachmentInfo attachmentInfo = ENGINE::GetColorAttachmentInfo(glm::vec4(0.0f), core->swapchainRef->GetFormat(), vk::AttachmentLoadOp::eLoad);
		attachmentInfo.attachmentInfo.setImageView(imageView);

		std::vector<vk::RenderingAttachmentInfo> attachmentInfos = {attachmentInfo.attachmentInfo};
		vk::RenderingAttachmentInfo              depthAttachment;

		dynamicRenderPass.SetRenderInfoUnsafe(attachmentInfos, windowProvider->GetWindowSize(), &depthAttachment);

		for (int i = 0; i < imageViewsToRecover.size(); ++i)
		{
			TransitionImage(imageViewsToRecover[i]->imageData, LayoutPatterns::GRAPHICS_READ, imageViewsToRecover[i]->GetSubresourceRange(), commandBuffer);
		}

		commandBuffer.beginRendering(dynamicRenderPass.renderInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		commandBuffer.endRendering();

		for (int i = 0; i < imageViewsToRecover.size(); ++i)
		{
			TransitionImage(imageViewsToRecover[i]->imageData, layoutPatternsToRecover[i], imageViewsToRecover[i]->GetSubresourceRange(), commandBuffer);
		}

		imageViewsToRecover.clear();
		layoutPatternsToRecover.clear();
	}
	enum class DebuggerPage
	{
		RenderGraph,
		Engine,
		Textures,
		FluidSimulation,
		ClusterRenderer,
		RadianceCascades,
		GaussianSplatting,
		Profiler
	};

	struct DebuggerPageInfo
	{
		const char *title;
		const char *description;
	};

	DebuggerPageInfo GetDebuggerPageInfo() const
	{
		switch (activePage)
		{
			case DebuggerPage::RenderGraph:
				return {"Render graph", "Inspect passes, resources, and node connections."};
			case DebuggerPage::Engine:
				return {"Engine overview", "Queues, execution order, and runtime scheduling."};
			case DebuggerPage::Textures:
				return {"Texture inspector", "Browse images currently registered with the renderer."};
			case DebuggerPage::FluidSimulation:
				return {"Fluid simulation", "Control simulation inputs, tools, emitters, and solver parameters."};
			case DebuggerPage::ClusterRenderer:
				return {"Cluster renderer", "Lighting, culling, camera, and render-node controls."};
			case DebuggerPage::RadianceCascades:
				return {"Radiance cascades", "Configure cascades, materials, lighting, and painting."};
			case DebuggerPage::GaussianSplatting:
				return {"Gaussian splatting", "Inspect splat data, scale, camera, and sort state."};
			case DebuggerPage::Profiler:
				return {"Performance", "Review CPU and GPU timing for the current frame."};
		}
		return {"Engine", "Runtime tools and diagnostics."};
	}

	void RenderApplicationHeader()
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
				ImGui::TextColored(ImguiRendererUI::AccentColor(), "RENDERER");
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
				ImGui::TextColored(ImguiRendererUI::AccentColor(), "ACTIVE");
				ImGui::EndTable();
			}

		}
		ImGui::EndChild();
	}

	void RenderSidebar()
	{
		const DebuggerPageInfo activePageInfo = GetDebuggerPageInfo();
		auto navigate = [this](const char *label, DebuggerPage page) {
			const bool selected = activePage == page;
			if (ImGui::Selectable(label, selected))
			{
				activePage = page;
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
		if (ImGui::BeginCombo("##workspace_page", activePageInfo.title))
		{
			navigate("Render graph", DebuggerPage::RenderGraph);
			navigate("Engine overview", DebuggerPage::Engine);
			navigate("Texture inspector", DebuggerPage::Textures);
			navigate("Fluid simulation", DebuggerPage::FluidSimulation);
			if (clusterRenderer)
			{
				navigate("Cluster lighting", DebuggerPage::ClusterRenderer);
			}
			if (flatRenderer)
			{
				navigate("Radiance cascades", DebuggerPage::RadianceCascades);
			}
			if (gsRenderer)
			{
				navigate("Gaussian splatting", DebuggerPage::GaussianSplatting);
			}
			navigate("Performance", DebuggerPage::Profiler);
			ImGui::EndCombo();
		}
	}

	void RenderActivePage()
	{
		switch (activePage)
		{
			case DebuggerPage::RenderGraph:
				StartNodeEditor();
				break;
			case DebuggerPage::Engine:
				DisplayEngineInfo();
				break;
			case DebuggerPage::Textures:
				DisplayAllTextures();
				break;
			case DebuggerPage::FluidSimulation:
				FluidSimInfo();
				break;
			case DebuggerPage::ClusterRenderer:
				if (clusterRenderer)
				{
					ClusterRendererInfo();
				}
				break;
			case DebuggerPage::RadianceCascades:
				if (flatRenderer)
				{
					RCascadesInfo();
				}
				break;
			case DebuggerPage::GaussianSplatting:
				if (gsRenderer)
				{
					GSRendererInfo();
				}
				break;
			case DebuggerPage::Profiler:
				RenderGraphProfiler();
				break;
		}
	}

	void RenderSceneViewport()
	{
		sceneViewportValid   = false;
		sceneViewportHovered = false;

		ImageView *sceneImage = nullptr;
		if (renderGraph && renderGraph->resourcesManager)
		{
			sceneImage = renderGraph->resourcesManager->GetImageViewFromName("bf");
		}

		if (!sceneImage || !sceneImage->imageData)
		{
			const ImVec2 available = ImGui::GetContentRegionAvail();
			const char  *message   = "Viewport image 'bf' is unavailable";
			const ImVec2 textSize  = ImGui::CalcTextSize(message);
			ImGui::SetCursorPos(ImVec2(
			    std::max(0.0f, (available.x - textSize.x) * 0.5f),
			    std::max(0.0f, (available.y - textSize.y) * 0.5f)));
			ImGui::TextDisabled("%s", message);
			return;
		}

		AddImage("bf", sceneImage);

		const ImVec2 available = ImGui::GetContentRegionAvail();
		const glm::uvec2 imageExtent = sceneImage->imageData->GetImageSize();
		if (available.x <= 1.0f || available.y <= 1.0f || imageExtent.x == 0 || imageExtent.y == 0)
		{
			return;
		}

		const float scale = std::min(
		    available.x / static_cast<float>(imageExtent.x),
		    available.y / static_cast<float>(imageExtent.y));
		const ImVec2 imageSize(
		    static_cast<float>(imageExtent.x) * scale,
		    static_cast<float>(imageExtent.y) * scale);
		const ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(
		    cursor.x + (available.x - imageSize.x) * 0.5f,
		    cursor.y + (available.y - imageSize.y) * 0.5f));

		ImGui::Image(
		    (ImTextureID) dsetsArrays->GetDsetByName("bf"), imageSize,
		    ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

		sceneViewportMin     = ImGui::GetItemRectMin();
		sceneViewportMax     = ImGui::GetItemRectMax();
		sceneViewportValid   = true;
		sceneViewportHovered = ImGui::IsItemHovered();

		ImDrawList *drawList = ImGui::GetWindowDrawList();
		const ImU32 borderColor = ImGui::GetColorU32(
		    sceneViewportHovered ? ImguiRendererUI::AccentColor()
		                         : ImVec4(0.30f, 0.27f, 0.27f, 0.72f));
		drawList->AddRect(sceneViewportMin, sceneViewportMax, borderColor, 4.0f, 0,
		                  sceneViewportHovered ? 2.0f : 1.0f);

		const ImVec2 badgeMin(sceneViewportMin.x + 12.0f, sceneViewportMin.y + 12.0f);
		const ImVec2 badgeMax(badgeMin.x + 122.0f, badgeMin.y + 27.0f);
		drawList->AddRectFilled(badgeMin, badgeMax, IM_COL32(18, 17, 18, 220), 6.0f);
		drawList->AddCircleFilled(ImVec2(badgeMin.x + 13.0f, badgeMin.y + 13.5f), 3.0f,
		                          ImGui::GetColorU32(ImguiRendererUI::AccentColor()));
		drawList->AddText(ImVec2(badgeMin.x + 23.0f, badgeMin.y + 5.0f),
		                  IM_COL32(232, 229, 226, 255), "VIEWPORT  /  bf");
	}

	void RenderDebuggerWindow()
	{
		const ImGuiViewport *mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(mainViewport->WorkPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(mainViewport->WorkSize, ImGuiCond_Always);
		ImGui::SetNextWindowViewport(mainViewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.025f, 0.024f, 0.026f, 1.0f));
		const ImGuiWindowFlags rootFlags =
		    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

		if (ImGui::Begin("CodeVK Engine Workspace", nullptr, rootFlags))
		{
			const ImVec2 available = ImGui::GetContentRegionAvail();
			const float  splitterWidth = 6.0f;
			const float  minimumPanelWidth =
			    std::min(360.0f, std::max(220.0f, available.x * 0.45f));
			const float maximumPanelWidth =
			    std::max(minimumPanelWidth, available.x - 280.0f - splitterWidth);

			if (inspectorPanelWidth <= 0.0f)
			{
				inspectorPanelWidth = available.x * 0.36f;
			}
			if (inspectorPanelResizing && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				inspectorPanelWidth -= ImGui::GetIO().MouseDelta.x;
			}
			else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				inspectorPanelResizing = false;
			}
			inspectorPanelWidth =
			    std::clamp(inspectorPanelWidth, minimumPanelWidth, maximumPanelWidth);
			const float viewportWidth =
			    std::max(1.0f, available.x - inspectorPanelWidth - splitterWidth);

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.015f, 0.015f, 0.017f, 1.0f));
			if (ImGui::BeginChild("##scene_viewport", ImVec2(viewportWidth, 0.0f),
			                      ImGuiChildFlags_None,
			                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				RenderSceneViewport();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();

			ImGui::SameLine(0.0f, 0.0f);
			ImGui::InvisibleButton("##inspector_splitter", ImVec2(splitterWidth, available.y),
			                       ImGuiButtonFlags_MouseButtonLeft);
			const bool splitterHovered = ImGui::IsItemHovered();
			const bool splitterActive  = ImGui::IsItemActive();
			if (splitterHovered || splitterActive || inspectorPanelResizing)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}
			if (ImGui::IsItemActivated())
			{
				inspectorPanelResizing = true;
			}
			if (splitterHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				inspectorPanelWidth = std::clamp(available.x * 0.36f,
				                                 minimumPanelWidth, maximumPanelWidth);
			}
			const ImVec2 splitterMin = ImGui::GetItemRectMin();
			const ImVec2 splitterMax = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(
			    splitterMin, splitterMax,
			    ImGui::GetColorU32(splitterHovered || splitterActive
			                            ? ImguiRendererUI::AccentColor()
			                            : ImVec4(0.18f, 0.16f, 0.16f, 0.75f)));

			ImGui::SameLine(0.0f, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.045f, 0.042f, 0.045f, 0.98f));
			if (ImGui::BeginChild("##docked_inspector", ImVec2(inspectorPanelWidth, 0.0f),
			                      ImGuiChildFlags_None))
			{
				RenderApplicationHeader();
				ImGui::Spacing();
				RenderSidebar();
				ImGui::Spacing();

				const DebuggerPageInfo pageInfo = GetDebuggerPageInfo();
				ImGui::PushFont(fonts.heading);
				ImGui::TextUnformatted(pageInfo.title);
				ImGui::PopFont();
				ImGui::TextDisabled("%s", pageInfo.description);
				ImGui::Separator();
				ImGui::Spacing();

				if (ImGui::BeginChild("##page_content", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None,
				                      ImGuiWindowFlags_HorizontalScrollbar))
				{
					RenderActivePage();
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
	void RenderGraphProfiler()
	{
		profilersWindow.cpuGraph.LoadFrameData(Profiler::GetInstance()->cpuTasks.data(),
		                                       Profiler::GetInstance()->cpuTasks.size());
		profilersWindow.gpuGraph.LoadFrameData(Profiler::GetInstance()->gpuTasks.data(),
		                                       Profiler::GetInstance()->gpuTasks.size());
		profilersWindow.RenderContent();
	}

	void AddSmokePretty(CodeCuda::CodeCudaContext *context, const glm::vec2 mousePosition)
	{
		if (!context || CodeCuda::FluidSimulation::s_width <= 0 || CodeCuda::FluidSimulation::s_height <= 0)
		{
			return;
		}

		auto randomPrettyColor = []() {
			return glm::vec4(
			    glm::linearRand(0.02f, 0.35f),
			    glm::linearRand(0.02f, 0.35f),
			    glm::linearRand(0.05f, 0.45f),
			    glm::linearRand(0.05f, 0.45f));
		};

		const int radius = std::max(1, CodeCuda::FluidSimulation::s_height / 46);
		if (!prettySmokeStrokeActive)
		{
			prettySmokeLastMousePosition = mousePosition;
			prettySmokeLastColor         = randomPrettyColor();
			prettySmokeStrokeActive      = true;
		}

		const glm::vec2 mouseDelta = mousePosition - prettySmokeLastMousePosition;
		const float     distance   = glm::length(mouseDelta);
		const float     sampleSpacing = std::max(1.0f, static_cast<float>(radius) * 0.35f);
		const int       sampleCount = std::max(1, static_cast<int>(glm::ceil(distance / sampleSpacing)));
		const glm::vec4 newColor    = randomPrettyColor();

		for (int sample = 1; sample <= sampleCount; ++sample)
		{
			const float     t = static_cast<float>(sample) / static_cast<float>(sampleCount);
			const glm::vec2 interpolatedPosition =
			    glm::mix(prettySmokeLastMousePosition, mousePosition, t);
			const float     smoothT = t * t * (3.0f - 2.0f * t);
			const glm::vec4 interpolatedColor =
			    glm::mix(prettySmokeLastColor, newColor, smoothT);

			const float u = glm::clamp(interpolatedPosition.x / 1023.0f, 0.0f, 1.0f);
			const float v = glm::clamp(interpolatedPosition.y / 1023.0f, 0.0f, 1.0f);
			const int   x = static_cast<int>(u * static_cast<float>(CodeCuda::FluidSimulation::s_width - 1));
			const int   y = static_cast<int>((1.0f - v) * static_cast<float>(CodeCuda::FluidSimulation::s_height - 1));

			const glm::vec2 simulationDelta(
			    mouseDelta.x * static_cast<float>(CodeCuda::FluidSimulation::s_width) / 1023.0f,
			    mouseDelta.y * static_cast<float>(CodeCuda::FluidSimulation::s_height) / 1023.0f);

			CodeCuda::FluidSimulation::C_AddVelocityGPU(x, y, radius,
			                           simulationDelta.x * 0.2f,
			                           -simulationDelta.y * 0.2f, context);
			CodeCuda::FluidSimulation::C_AddSmokeGPU(x, y, radius,
			                        interpolatedColor.r, interpolatedColor.g,
			                        interpolatedColor.b, interpolatedColor.a, context);
		}

		prettySmokeLastMousePosition = mousePosition;
		prettySmokeLastColor         = newColor;
	}

	void FluidSimInfo()
	{
		CodeCuda::FluidSimulation::sim_params fluidSimParams{};
		if (CodeCuda::FluidSimulation::C_GetSimulationParams(&fluidSimParams) != CodeCuda::C_Res::OK)
		{
			ImGui::TextDisabled("Unable to read fluid simulation parameters.");
			return;
		}

		bool        paramsChanged        = false;
		static bool simulationActionFail = false;
		auto        runSimulationAction  = [](CodeCuda::C_Res result) {
			simulationActionFail = result != CodeCuda::C_Res::OK;
		};

		ImGui::TextDisabled("Grid: %d x %d", CodeCuda::FluidSimulation::s_width, CodeCuda::FluidSimulation::s_height);
		const float quickControlWidth =
		    (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		if (ImGui::Button("Restart simulation", ImVec2(quickControlWidth, 30.0f)))
		{
			runSimulationAction(CodeCuda::FluidSimulation::C_RestartSimulation());
		}
		ImGui::SameLine();
		paramsChanged |= ImGui::Checkbox("GPU simulation", &fluidSimParams.gpu_sim);

		std::string resourcesPath =
		    SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\Images";

		static std::vector<std::string> imagePaths;
		static std::string              selectedImagePath;
		static char                     searchBuffer[128] = {};
		static bool                     imagesScanned     = false;
		static bool                     imageLoadFailed   = false;

		if (!imagesScanned)
		{
			std::error_code scanError;
			if (std::filesystem::exists(resourcesPath, scanError))
			{
				for (const auto &dir : std::filesystem::directory_iterator(resourcesPath, scanError))
				{
					if (scanError)
					{
						break;
					}
					if (dir.is_regular_file())
					{
						imagePaths.emplace_back(dir.path().string());
					}
				}
			}
			std::sort(imagePaths.begin(), imagePaths.end());
			imagesScanned = true;
		}

		if (ImGui::CollapsingHeader("Image import###fluid_image_import"))
		{
		ImGui::TextDisabled("Load an image as smoke density or a solid mask.");

		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint(
		    "##ImageSearch",
		    "Search images...",
		    searchBuffer,
		    sizeof(searchBuffer));

		ImGui::Spacing();

		const float sectionHeight = 260.0f;

		if (ImGui::BeginChild(
		        "##ImagesSection",
		        ImVec2(0.0f, sectionHeight),
		        ImGuiChildFlags_Borders,
		        ImGuiWindowFlags_AlwaysVerticalScrollbar))
		{
			for (int i = 0; i < static_cast<int>(imagePaths.size()); ++i)
			{
				const std::string &path = imagePaths[i];

				const std::string filename =
				    std::filesystem::path(path).filename().string();

				if (searchBuffer[0] != '\0')
				{
					if (filename.find(searchBuffer) == std::string::npos)
						continue;
				}

				ImGui::PushID(i);

				const bool selected = selectedImagePath == path;

				const float rowHeight = 32.0f;

				if (ImGui::Selectable(
				        "##ImageRow",
				        selected,
				        ImGuiSelectableFlags_AllowOverlap,
				        ImVec2(0.0f, rowHeight)))
				{
					selectedImagePath = path;
				}

				ImGui::SameLine();

				const ImVec2 rowMin = ImGui::GetItemRectMin();

				ImGui::SetCursorScreenPos(
				    ImVec2(
				        rowMin.x + 8.0f,
				        rowMin.y + 7.0f));

				ImGui::TextUnformatted(filename.c_str());

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", path.c_str());
				}

				const float buttonWidth = 60.0f;

				ImGui::SetCursorScreenPos(
				    ImVec2(
				        ImGui::GetWindowPos().x +
				            ImGui::GetWindowContentRegionMax().x -
				            buttonWidth - 8.0f,

				        rowMin.y + 4.0f));

				if (ImGui::Button("Load", ImVec2(buttonWidth, 24.0f)))
				{
					selectedImagePath = path;

					stbi_uc *pixelsData = stbi_load(selectedImagePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
					imageLoadFailed     = pixelsData == nullptr;
					if (!pixelsData)
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
								const size_t pixelIndex = static_cast<size_t>(height - 1 - y) * width + x;
								const size_t byteIndex  = pixelIndex * 4;

								image_pixels[static_cast<size_t>(y) * width + x] = glm::vec4(
								    static_cast<float>(pixelsData[byteIndex + 0]) / 255.0f,
								    static_cast<float>(pixelsData[byteIndex + 1]) / 255.0f,
								    static_cast<float>(pixelsData[byteIndex + 2]) / 255.0f,
								    static_cast<float>(pixelsData[byteIndex + 3]) / 255.0f);

								solid_mask[static_cast<size_t>(y) * width + x] =
								    image_pixels[static_cast<size_t>(y) * width + x].w > 0.5f ? 1 : 0;
							}
						}
						stbi_image_free(pixelsData);
					}
				}

				ImGui::PopID();
			}
		}

		ImGui::EndChild();

		if (!selectedImagePath.empty())
		{
			ImGui::Spacing();
			ImGui::TextDisabled("Selected:");
			ImGui::SameLine();
			ImGui::TextUnformatted(
			    std::filesystem::path(selectedImagePath)
			        .filename()
			        .string()
			        .c_str());
		}
		if (imageLoadFailed)
		{
			ImGui::TextColored(ImVec4(0.95f, 0.38f, 0.42f, 1.0f), "The selected image could not be loaded.");
		}

		if (!image_pixels.empty())
		{
			if (ImGui::Button("Load Image as Smoke"))
			{
				runSimulationAction(CodeCuda::FluidSimulation::C_MapImageToSmoke(width, height, 4,
				                                                image_pixels.data()));
			}
		}
		if (!solid_mask.empty())
		{
			ImGui::SameLine();
			if (ImGui::Button("Use Solid Mask"))
			{
				runSimulationAction(CodeCuda::FluidSimulation::C_MapSolidMask(width, height,
				                                             solid_mask.data()));
			}
		}
		}

		static int randomVelocityScale = 10;

		ImGui::SeparatorText("Viewport Tool");
		static int   fluidTool                        = 0;
		static int   fluidBrushRadius                 = 25;
		static float fluidSmokeColor[4]               = {0.15f, 0.05f, 0.25f, 1.0f};
		static float fluidVelocityStrength            = 0.2f;
		static float fluidRadialVelocityStrength      = 1.0f;
		static float fluidEmitterVelocityDirection[2] = {1.0f, 0.0f};
		static float fluidEmitterColor[4]             = {0.15f, 0.05f, 0.25f, 1.0f};
		static bool  fluidToolEnabled                 = true;

		const char *fluidTools[] = {"Smoke", "Velocity", "Add solid", "Erase solid",
		                            "Radial velocity", "Add emitter", "Pretty smoke"};
		const char *fluidToolDescriptions[] = {
		    "Paint smoke with the selected color.",
		    "Drag to push the fluid in the cursor direction.",
		    "Paint solid obstacles into the simulation.",
		    "Remove solid obstacles from the simulation.",
		    "Push fluid outward from the cursor.",
		    "Place a persistent smoke and velocity source.",
		    "Paint a smooth trail with changing random colors."};

		const int maxSimulationDimension =
		    CodeCuda::FluidSimulation::s_width > CodeCuda::FluidSimulation::s_height ? CodeCuda::FluidSimulation::s_width : CodeCuda::FluidSimulation::s_height;
		const int maxBrushRadius = maxSimulationDimension > 0 ? maxSimulationDimension : 1;

		ImGui::TextDisabled("Tool");
		if (ImGui::BeginTable("##fluid_tool_selector", 2,
		                      ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings))
		{
			for (int toolIndex = 0; toolIndex < IM_ARRAYSIZE(fluidTools); ++toolIndex)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(toolIndex);
				const bool selected = fluidTool == toolIndex;
				if (selected)
				{
					const ImVec4 accent = ImguiRendererUI::AccentColor();
					ImGui::PushStyleColor(ImGuiCol_Button, accent);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
					                      ImVec4(accent.x, accent.y, accent.z, 0.90f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive,
					                      ImVec4(accent.x, accent.y, accent.z, 0.78f));
					ImGui::PushStyleColor(ImGuiCol_Text, ImguiRendererUI::InkColor());
				}

				if (ImGui::Button(fluidTools[toolIndex], ImVec2(-FLT_MIN, 30.0f)))
				{
					if (fluidTool != toolIndex)
					{
						prettySmokeStrokeActive = false;
					}
					fluidTool = toolIndex;
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					ImGui::SetTooltip("%s", fluidToolDescriptions[toolIndex]);
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
			auto beginProperty = [](const char *label) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(label);
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-1.0f);
			};

			beginProperty("Enabled");
			ImGui::Checkbox("##fluid_tool_enabled", &fluidToolEnabled);
			if (fluidTool != 6)
			{
				beginProperty("Brush radius");
				ImGui::DragInt("##fluid_brush_radius", &fluidBrushRadius, 1.0f, 1,
				               maxBrushRadius, "%d px", ImGuiSliderFlags_AlwaysClamp);
			}

			if (fluidTool == 0)
			{
				beginProperty("Smoke color");
				ImGui::ColorEdit4("##fluid_smoke_color", fluidSmokeColor);
			}
			else if (fluidTool == 1)
			{
				beginProperty("Velocity strength");
				ImGui::DragFloat("##fluid_velocity_strength", &fluidVelocityStrength,
				                 0.01f, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			}
			else if (fluidTool == 4)
			{
				beginProperty("Radial strength");
				ImGui::DragFloat("##fluid_radial_strength", &fluidRadialVelocityStrength,
				                 0.05f, -100.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			}
			else if (fluidTool == 5)
			{
				beginProperty("Emitter velocity");
				ImGui::DragFloat2("##fluid_emitter_velocity", fluidEmitterVelocityDirection,
				                  0.05f, -10.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
				beginProperty("Emitter color");
				ImGui::ColorEdit4("##fluid_emitter_color", fluidEmitterColor);
			}
			else if (fluidTool == 6)
			{
				beginProperty("Style");
				ImGui::TextDisabled("Interpolated random color");
			}

			beginProperty("Random force");
			const float addButtonWidth = 42.0f;
			ImGui::SetNextItemWidth(std::max(60.0f, ImGui::GetContentRegionAvail().x -
			                                          addButtonWidth - ImGui::GetStyle().ItemSpacing.x));
			ImGui::DragInt("##random_velocity_scale", &randomVelocityScale, 1.0f, 1,
			               1000, "%d", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SameLine();
			if (ImGui::Button("Add", ImVec2(addButtonWidth, 0.0f)))
			{
				runSimulationAction(CodeCuda::FluidSimulation::C_AddRandomVelocity(randomVelocityScale));
			}
			ImGui::EndTable();
		}

		ImGui::TextDisabled("Right-click and drag over the viewport to apply the selected tool.");

		const ImVec2 mousePosition = ImGui::GetMousePos();
		const float  viewportWidth  = std::max(1.0f, sceneViewportMax.x - sceneViewportMin.x);
		const float  viewportHeight = std::max(1.0f, sceneViewportMax.y - sceneViewportMin.y);
		const bool   mouseOverViewport = sceneViewportValid && sceneViewportHovered;
		const int    activeBrushRadius =
		    fluidTool == 6 ? std::max(1, CodeCuda::FluidSimulation::s_height / 46) : fluidBrushRadius;
		if (fluidToolEnabled && mouseOverViewport)
		{
			const float radiusPixels = std::max(
			    2.0f, activeBrushRadius * 0.5f *
			              (viewportWidth / std::max(1, CodeCuda::FluidSimulation::s_width) +
			               viewportHeight / std::max(1, CodeCuda::FluidSimulation::s_height)));
			ImGui::GetForegroundDrawList()->AddCircle(
			    mousePosition, radiusPixels,
			    ImGui::GetColorU32(ImguiRendererUI::AccentColor()), 48, 1.5f);
		}
		if (fluidToolEnabled && mouseOverViewport &&
		    ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			const float u = glm::clamp(
			    (mousePosition.x - sceneViewportMin.x) / viewportWidth, 0.0f, 1.0f);
			const float v = glm::clamp(
			    1.0f - (mousePosition.y - sceneViewportMin.y) / viewportHeight, 0.0f, 1.0f);
			const int   xPosition = static_cast<int>(u * static_cast<float>(CodeCuda::FluidSimulation::s_width - 1));
			const int   yPosition = static_cast<int>(v * static_cast<float>(CodeCuda::FluidSimulation::s_height - 1));

			switch (fluidTool)
			{
				case 0:
					runSimulationAction(CodeCuda::FluidSimulation::C_AddSmoke(xPosition, yPosition, fluidBrushRadius,
					                                         fluidSmokeColor[0], fluidSmokeColor[1],
					                                         fluidSmokeColor[2], fluidSmokeColor[3]));
					break;
				case 1:
				{
					const ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
					const float  velocityX  = mouseDelta.x * static_cast<float>(CodeCuda::FluidSimulation::s_width) / viewportWidth;
					const float  velocityY  = mouseDelta.y * static_cast<float>(CodeCuda::FluidSimulation::s_height) / viewportHeight;
					runSimulationAction(CodeCuda::FluidSimulation::C_AddVelocity(xPosition, yPosition, fluidBrushRadius,
					                                            velocityX * fluidVelocityStrength,
					                                            -velocityY * fluidVelocityStrength));
					break;
				}
				case 2:
					runSimulationAction(CodeCuda::FluidSimulation::C_SetSolid(xPosition, yPosition, fluidBrushRadius, true));
					break;
				case 3:
					runSimulationAction(CodeCuda::FluidSimulation::C_SetSolid(xPosition, yPosition, fluidBrushRadius, false));
					break;
				case 4:
					runSimulationAction(CodeCuda::FluidSimulation::C_AddRadialVelocity(xPosition, yPosition, fluidBrushRadius,
					                                                  fluidRadialVelocityStrength));
					break;
				case 6:
				{
					auto *cudaNode = renderGraph ? renderGraph->GetNode("CudaNode") : nullptr;
					if (!cudaNode || !cudaNode->CUDAPipeline || !cudaNode->CUDAPipeline->context)
					{
						simulationActionFail = true;
						break;
					}
					AddSmokePretty(cudaNode->CUDAPipeline->context,
					               glm::vec2(u * 1023.0f, (1.0f - v) * 1023.0f));
					simulationActionFail = false;
					break;
				}
				default:
					break;
			}
		}
		if (fluidTool != 6 || !fluidToolEnabled || !mouseOverViewport ||
		    !ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			prettySmokeStrokeActive = false;
		}
		if (fluidToolEnabled && mouseOverViewport &&
		    ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			const float u = glm::clamp(
			    (mousePosition.x - sceneViewportMin.x) / viewportWidth, 0.0f, 1.0f);
			const float v = glm::clamp(
			    1.0f - (mousePosition.y - sceneViewportMin.y) / viewportHeight, 0.0f, 1.0f);
			const int   xPosition = static_cast<int>(u * static_cast<float>(CodeCuda::FluidSimulation::s_width - 1));
			const int   yPosition = static_cast<int>(v * static_cast<float>(CodeCuda::FluidSimulation::s_height - 1));
			switch (fluidTool)
			{
				case 5:
				{
					glm::vec4 col = glm::make_vec4(fluidEmitterColor);
					glm::vec2 vel = glm::make_vec2(fluidEmitterVelocityDirection);
					emitters.emplace_back(emitter{col, vel, fluidBrushRadius, xPosition, yPosition});
					break;
				}
				default:
					break;
			}
		}
		if (simulationActionFail)
		{
			ImGui::TextDisabled("The last simulation action failed.");
		}
		if (!emitters.empty())
		{
			auto *cudaNode = renderGraph->GetNode("CudaNode");
			if (!cudaNode || !cudaNode->CUDAPipeline || !cudaNode->CUDAPipeline->context)
			{
				simulationActionFail = true;
			}
			else
			{
				for (const auto &activeEmitter : emitters)
				{
					runSimulationAction(CodeCuda::FluidSimulation::C_AddVelocityGPU(
					    activeEmitter.xPos, activeEmitter.yPos, activeEmitter.radius,
					    activeEmitter.velocity.x, activeEmitter.velocity.y,
					    cudaNode->CUDAPipeline->context));
					runSimulationAction(CodeCuda::FluidSimulation::C_AddSmokeGPU(
					    activeEmitter.xPos, activeEmitter.yPos, activeEmitter.radius,
					    activeEmitter.color.x, activeEmitter.color.y,
					    activeEmitter.color.z, activeEmitter.color.w,
					    cudaNode->CUDAPipeline->context));
				}
			}
		}

		const std::string emitterSectionLabel =
		    "Emitters (" + std::to_string(emitters.size()) + ")###fluid_emitters";
		if (ImGui::CollapsingHeader(emitterSectionLabel.c_str()))
		{
			if (emitters.empty())
			{
				ImGui::TextDisabled("Choose Add Emitter, then right-click the viewport.");
			}
			int emitterToRemove = -1;

			for (int i = 0; i < static_cast<int>(emitters.size()); ++i)
			{
				ImGui::PushID(i);
				auto &activeEmitter = emitters[i];

				if (ImGui::TreeNode("Emitter", "Emitter %d", i + 1))
				{
					ImGui::DragInt("X", &activeEmitter.xPos, 1.0f, 0, CodeCuda::FluidSimulation::s_width - 1);
					ImGui::DragInt("Y", &activeEmitter.yPos, 1.0f, 0, CodeCuda::FluidSimulation::s_height - 1);
					ImGui::DragInt("Radius", &activeEmitter.radius, 1.0f, 1,
					               std::max(CodeCuda::FluidSimulation::s_width, CodeCuda::FluidSimulation::s_height));
					ImGui::DragFloat2("Velocity", &activeEmitter.velocity.x, 0.05f,
					                  -10.0f, 10.0f);
					ImGui::ColorEdit4("Color", &activeEmitter.color.x);

					if (ImGui::Button("Remove emitter"))
					{
						emitterToRemove = i;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			if (emitterToRemove >= 0)
			{
				emitters.erase(emitters.begin() + emitterToRemove);
			}
		}

		static int  resolutionWidth         = CodeCuda::FluidSimulation::s_width;
		static int  resolutionHeight        = CodeCuda::FluidSimulation::s_height;
		static bool resolutionChangePending = false;
		static bool resolutionUpdateFailed  = false;

		if (!resolutionChangePending)
		{
			resolutionWidth  = CodeCuda::FluidSimulation::s_width;
			resolutionHeight = CodeCuda::FluidSimulation::s_height;
		}

		auto beginSettingsProperty = [](const char *label) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-1.0f);
		};

		bool widthChanged = false;
		bool widthActive = false;
		bool heightChanged = false;
		bool heightActive = false;
		if (ImGui::CollapsingHeader("Resolution###fluid_resolution"))
		{
			if (ImGui::BeginTable("##fluid_resolution_properties", 2,
			                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 140.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				beginSettingsProperty("Width");
				widthChanged = ImGui::DragInt("##fluid_resolution_width", &resolutionWidth,
				                              1.0f, 1, 4096, "%d",
				                              ImGuiSliderFlags_AlwaysClamp);
				widthActive = ImGui::IsItemActive();
				beginSettingsProperty("Height");
				heightChanged = ImGui::DragInt("##fluid_resolution_height", &resolutionHeight,
				                               1.0f, 1, 4096, "%d",
				                               ImGuiSliderFlags_AlwaysClamp);
				heightActive = ImGui::IsItemActive();
				ImGui::EndTable();
			}
		}
		resolutionChangePending |= widthChanged || heightChanged;

		if (resolutionChangePending && !widthActive && !heightActive)
		{
			resolutionUpdateFailed =
			    CodeCuda::FluidSimulation::C_SetSimulationResolution(resolutionWidth, resolutionHeight) != CodeCuda::C_Res::OK;
			resolutionChangePending = false;
		}
		if (resolutionUpdateFailed)
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
				beginSettingsProperty("Density");
				paramsChanged |= ImGui::DragFloat("##fluid_density", &fluidSimParams.density,
				                                  0.01f, 0.001f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				beginSettingsProperty("Velocity dissipation");
				paramsChanged |= ImGui::DragFloat("##fluid_velocity_dissipation",
				                                  &fluidSimParams.velocity_dissipation,
				                                  0.001f, 0.0f, 0.2f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				beginSettingsProperty("Smoke dissipation");
				paramsChanged |= ImGui::DragFloat("##fluid_smoke_dissipation",
				                                  &fluidSimParams.smoke_dissipation,
				                                  0.001f, 0.0f, 0.2f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);

				int timeStepDenominator = fluidSimParams.dt > 0.0f
				                              ? static_cast<int>((1.0f / fluidSimParams.dt) + 0.5f)
				                              : 120;
				beginSettingsProperty("Time step");
				if (ImGui::DragInt("##fluid_time_step", &timeStepDenominator, 1.0f,
				                   1, 10000, "1 / %d", ImGuiSliderFlags_AlwaysClamp))
				{
					fluidSimParams.dt = 1.0f / static_cast<float>(timeStepDenominator);
					paramsChanged = true;
				}
				beginSettingsProperty("Gravity");
				paramsChanged |= ImGui::DragFloat("##fluid_gravity", &fluidSimParams.g,
				                                  0.01f, -100.0f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				beginSettingsProperty("Wind speed");
				paramsChanged |= ImGui::DragFloat("##fluid_wind_speed", &fluidSimParams.wind_speed,
				                                  0.01f, -100.0f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				beginSettingsProperty("Viscosity");
				paramsChanged |= ImGui::DragFloat("##fluid_viscosity", &fluidSimParams.viscosity,
				                                  0.01f, 0.0f, 100.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				beginSettingsProperty("Smoke diffusion");
				paramsChanged |= ImGui::DragFloat("##fluid_smoke_diffusion",
				                                  &fluidSimParams.smoke_diffuse_coef,
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
				beginSettingsProperty("SOR weight");
				paramsChanged |= ImGui::DragFloat("##fluid_sor_weight", &fluidSimParams.weight_sor,
				                                  0.01f, 0.0f, 2.0f, "%.3f",
				                                  ImGuiSliderFlags_AlwaysClamp);
				beginSettingsProperty("GPU iterations");
				paramsChanged |= ImGui::DragInt("##fluid_gpu_iterations",
				                                &fluidSimParams.total_iter_gpu, 1.0f,
				                                1, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);
				beginSettingsProperty("CPU iterations");
				paramsChanged |= ImGui::DragInt("##fluid_cpu_iterations",
				                                &fluidSimParams.total_iter_cpu, 1.0f,
				                                1, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);
				beginSettingsProperty("Debug output");
				paramsChanged |= ImGui::Checkbox("##fluid_debug", &fluidSimParams.debug);
				ImGui::EndTable();
			}
		}

		if (paramsChanged && CodeCuda::FluidSimulation::C_SetSimulationParams(&fluidSimParams) != CodeCuda::C_Res::OK)
		{
			ImGui::TextDisabled("Unable to update fluid simulation parameters.");
		}
	}

	void SetStyle()
	{
		ImguiRendererUI::ApplyModernTheme();
		fonts = ImguiRendererUI::ConfigureFonts(SYSTEMS::OS::GetInstance()->GetEngineResourcesPath());
	}
	void DisplayGeneralEngineInfo()
	{
		DisplayEngineInfo();
		DisplayAllTextures();
	}

	void RenderMetricCard(const char *id, const char *label, const std::string &value)
	{
		ImGui::PushID(id);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImguiRendererUI::RaisedSurfaceColor());
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

	void DisplayEngineInfo()
	{
		auto     *queueWorkerManager = core->queueWorkerManager.get();
		const int engineQueueCount   = static_cast<int>(queueWorkerManager->workersQueues.size());
		const int renderNodeCount    = static_cast<int>(renderGraph->sortedByDepNodes.size());
		const int queueBatchCount    = static_cast<int>(renderGraph->sortedQueueBatches.size());
		int       activeNodeCount    = 0;
		for (const auto *node : renderGraph->sortedByDepNodes)
		{
			if (node && node->active)
			{
				activeNodeCount++;
			}
		}

		if (ImGui::BeginTable("engine_summary", 4,
		                      ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextColumn();
			RenderMetricCard("queues", "QUEUES", std::to_string(engineQueueCount));
			ImGui::TableNextColumn();
			RenderMetricCard("passes", "RENDER PASSES", std::to_string(renderNodeCount));
			ImGui::TableNextColumn();
			RenderMetricCard("active", "ACTIVE PASSES", std::to_string(activeNodeCount));
			ImGui::TableNextColumn();
			RenderMetricCard("batches", "QUEUE BATCHES", std::to_string(queueBatchCount));
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

			for (auto &queuePair : queueWorkerManager->workersQueues)
			{
				const auto &queue = queuePair.second;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", queue.name.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%d", queue.familyIndex);
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", queue.isMainThreat ? "true" : "false");
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%d", queue.cmdsPoolSize);
				ImGui::TableSetColumnIndex(4);
				ImGui::Text("%d", queue.perCmdPoolSize);
				ImGui::TableSetColumnIndex(5);
				ImGui::Text("%d", queue.activeCmdIdx);
				ImGui::TableSetColumnIndex(6);
				ImGui::Text("%d", queue.currentPoolCmdIdx);
				ImGui::TableSetColumnIndex(7);
				ImGui::Text("%p", static_cast<VkQueue>(queue.workerQueue));
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
			for (int i = 0; i < renderGraph->sortedByDepNodes.size(); ++i)
			{
				auto *node = renderGraph->sortedByDepNodes[i];
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%02d", i);
				ImGui::TableSetColumnIndex(1);
				ImGui::TextUnformatted(node ? node->passName.c_str() : "Unavailable");
				ImGui::TableSetColumnIndex(2);
				ImGui::TextUnformatted(node ? node->workerQueueName.c_str() : "-");
				ImGui::TableSetColumnIndex(3);
				const bool nodeActive = node && node->active;
				ImGui::TextColored(nodeActive ? ImguiRendererUI::SuccessColor()
				                              : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled),
				                   "%s", nodeActive ? "Active" : "Inactive");
			}
			ImGui::EndTable();
		}

		DisplayRenderGraphDag();

		DisplayQueueExecutionTimeline();
	}

	void ClusterRendererInfo()
	{
		ImGui::SeparatorText("Light Info");

		float speed = 0.01f;
		for (auto &pointLight : clusterRenderer->pointLights)
		{
			if (pointLight.pos.y >= 20.0f)
			{
				pointLight.pos.y = 0;
			}
			pointLight.pos += glm::vec3(0.0f, 1.0f, 0.0f) * speed;
			pointLight.CalculateQAttenuationFromRadius();
		}
		static float pointLightRadiuses = 1.0f;
		if (ImGui::SliderFloat("Point lights Radiuses", &pointLightRadiuses, 1.0f, 20.0f))
		{
			for (auto &pointLight : clusterRenderer->pointLights)
			{
				pointLight.radius = pointLightRadiuses;
				pointLight.CalculateQAttenuationFromRadius();
			}
		}
		static float pointQuadraticAttenuation = 1.0f;
		if (ImGui::SliderFloat("Point lights Quadratic Attenuation", &pointQuadraticAttenuation, 1.0f, 30.0f))
		{
			for (auto &pointLight : clusterRenderer->pointLights)
			{
				pointLight.qAttenuation = pointQuadraticAttenuation;
				pointLight.CalculateRadiusFromParams();
				pointLightRadiuses = pointLight.radius;
			}
		}
		static float pointLightLinearAttenuation = 1.0f;
		if (ImGui::SliderFloat("Point lights Linear Attenuation", &pointLightLinearAttenuation, 1.0f, 30.0f))
		{
			for (auto &pointLight : clusterRenderer->pointLights)
			{
				pointLight.lAttenuation = pointLightLinearAttenuation;
				pointLight.CalculateRadiusFromParams();
				pointLightRadiuses = pointLight.radius;
			}
		}
		static float pointLightIntensity = 1.0f;
		if (ImGui::SliderFloat("Point lights Intensity", &pointLightIntensity, 0.0f, 30.0f))
		{
			for (auto &pointLight : clusterRenderer->pointLights)
			{
				pointLight.intensity = pointLightIntensity;
				pointLight.CalculateRadiusFromParams();
				pointLightRadiuses = pointLight.radius;
			}
		}

		ImGui::SeparatorText("Tile/Cluster renderer");

		static int xTileSizePx = 256;
		static int yTileSizePx = 256;
		static int zSlicesSize = 24;

		if (ImGui::SliderInt("x tile size (px): ", &xTileSizePx, 32, 512))
		{
			clusterRenderer->xTileSizePx = xTileSizePx;
		}
		if (ImGui::SliderInt("y tile size (px): ", &yTileSizePx, 32, 512))
		{
			clusterRenderer->yTileSizePx = yTileSizePx;
		}
		if (ImGui::SliderInt("number of z slices: ", &zSlicesSize, 1, 28))
		{
			clusterRenderer->zSlicesSize = zSlicesSize;
		}

		ImGui::SeparatorText("Cull Info");

		std::string cullCount = "Cull Count: " + std::to_string(RenderingResManager::GetInstance()->cullCount) + " / " + std::to_string(RenderingResManager::GetInstance()->indirectDrawsCmdInfos.size());

		ImGui::Text("%s", cullCount.c_str());

		ImGui::SeparatorText("First Person Camera Info");

		std::string cameraPos = "Position: (" + std::to_string(clusterRenderer->camera.position.x) + ", " + std::to_string(clusterRenderer->camera.position.y) + ", " + std::to_string(clusterRenderer->camera.position.z) + ")";

		std::string cameraForward = "Forward: (" + std::to_string(clusterRenderer->camera.forward.x) + ", " + std::to_string(clusterRenderer->camera.forward.y) + ", " + std::to_string(clusterRenderer->camera.forward.z) + ")";
		std::string cameraRight   = "Right: (" + std::to_string(clusterRenderer->camera.right.x) + ", " + std::to_string(clusterRenderer->camera.right.y) + ", " + std::to_string(clusterRenderer->camera.right.z) + ")";
		std::string cameraUp      = "Up: (" + std::to_string(clusterRenderer->camera.up.x) + ", " + std::to_string(clusterRenderer->camera.up.y) + ", " + std::to_string(clusterRenderer->camera.up.z) + ")";

		ImGui::Text("%s", cameraPos.c_str());
		ImGui::Text("%s", cameraForward.c_str());
		ImGui::Text("%s", cameraRight.c_str());
		ImGui::Text("%s", cameraUp.c_str());

		ImGui::SeparatorText("Virtual Cam Info");
		static bool detachedCam = false;
		if (ImGui::Checkbox("Debug Cull", &detachedCam))
		{
			if (detachedCam)
			{
				clusterRenderer->debugCam.position = clusterRenderer->camera.position;
				clusterRenderer->debugCam.yaw      = clusterRenderer->camera.yaw;
				clusterRenderer->debugCam.pitch    = clusterRenderer->camera.pitch;
			}
		}
		if (detachedCam)
		{
			clusterRenderer->currCamera = &clusterRenderer->debugCam;
			clusterRenderer->debugCam.UpdateCam();
			clusterRenderer->debugCam.RotateCamera();
			static float yaw   = clusterRenderer->debugCam.yaw;
			static float pitch = clusterRenderer->debugCam.pitch;

			if (ImGui::SliderFloat("Yaw", &yaw, 0.0f, 360.0f))
			{
				clusterRenderer->debugCam.yaw = yaw;
			}
			if (ImGui::SliderFloat("Pitch", &pitch, 0.0f, 360.0f))
			{
				clusterRenderer->debugCam.pitch = pitch;
			}
			static float pos[3] = {
			    clusterRenderer->debugCam.position.x, clusterRenderer->debugCam.position.y,
			    clusterRenderer->debugCam.position.z};

			if (ImGui::Button("Snap Cam to view", {50, 50}))
			{
				clusterRenderer->debugCam.position = clusterRenderer->camera.position;
				clusterRenderer->debugCam.yaw      = clusterRenderer->camera.yaw;
				clusterRenderer->debugCam.pitch    = clusterRenderer->camera.pitch;
			};
			clusterRenderer->debugCam.UpdateCam();
			clusterRenderer->debugCam.RotateCamera();
		}
		else
		{
			clusterRenderer->currCamera = &clusterRenderer->camera;
		}

		struct NodeInfo
		{
			bool       *active;
			std::string name;
		};
		static std::vector<NodeInfo> nodeInfos;

		for (auto &node : core->renderGraphRef->sequentialRenderNodes)
		{
			nodeInfos.emplace_back(NodeInfo{&node->active, node->passName});
		}

		ImGui::SeparatorText("Render Nodes");
		for (auto &nodeInfo : nodeInfos)
		{
			std::string name = "Node: " + nodeInfo.name;
			ImGui::Checkbox(name.c_str(), nodeInfo.active);
		}
		nodeInfos.clear();
	}

	void DisplayRenderGraphDag();

	void DisplayQueueExecutionTimeline()
	{
		ImGui::SeparatorText("Queue Execution Timeline");

		if (renderGraph->sortedQueueBatches.empty())
		{
			ImGui::Text("No queue batches");
			return;
		}

		static int selectedBatchIdx = 0;
		if (selectedBatchIdx < 0 || selectedBatchIdx >= renderGraph->sortedQueueBatches.size())
		{
			selectedBatchIdx = 0;
		}

		std::vector<std::string>   queueNames;
		std::map<std::string, int> queueLaneByName;
		std::map<std::string, int> queueBatchCount;
		std::map<std::string, int> queueNodeCount;
		int                        totalNodeCount  = 0;
		int                        activeNodeCount = 0;
		int                        emptyBatchCount = 0;
		for (const auto &batch : renderGraph->sortedQueueBatches)
		{
			if (!queueLaneByName.contains(batch.queueName))
			{
				queueLaneByName[batch.queueName] = static_cast<int>(queueNames.size());
				queueNames.push_back(batch.queueName);
			}
			queueBatchCount[batch.queueName]++;
			if (batch.sortedNodes.empty())
			{
				emptyBatchCount++;
			}
			for (auto *node : batch.sortedNodes)
			{
				if (!node)
				{
					continue;
				}
				totalNodeCount++;
				queueNodeCount[batch.queueName]++;
				if (node->active)
				{
					activeNodeCount++;
				}
			}
		}

		ImGui::Text("Batches: %d | Queues: %d | Nodes: %d | Active nodes: %d | Empty batches: %d",
		            static_cast<int>(renderGraph->sortedQueueBatches.size()),
		            static_cast<int>(queueNames.size()),
		            totalNodeCount,
		            activeNodeCount,
		            emptyBatchCount);

		const ImU32 queueColors[] = {
		    IM_COL32(119, 69, 48, 255),
		    IM_COL32(93, 85, 69, 255),
		    IM_COL32(88, 70, 77, 255),
		    IM_COL32(74, 83, 82, 255),
		    IM_COL32(112, 82, 52, 255),
		    IM_COL32(96, 72, 65, 255),
		};
		const ImU32 laneFill      = IM_COL32(18, 17, 18, 245);
		const ImU32 laneBorder    = IM_COL32(84, 75, 73, 125);
		const ImU32 selectedColor = IM_COL32(255, 82, 20, 255);
		const ImU32 nodeFill      = IM_COL32(36, 32, 33, 255);
		const ImU32 inactiveFill  = IM_COL32(49, 40, 41, 255);
		const ImU32 emptyFill     = IM_COL32(47, 41, 34, 255);
		const ImU32 textColor     = IM_COL32(235, 231, 228, 255);
		const ImU32 mutedColor    = IM_COL32(151, 143, 141, 255);

		const float  labelWidth  = 150.0f;
		const float  laneHeight  = 94.0f;
		const float  batchGap    = 18.0f;
		const float  batchHeight = 62.0f;
		const float  nodeHeight  = 20.0f;
		const ImVec2 padding(18.0f, 18.0f);

		float timelineWidth = labelWidth + padding.x * 2.0f;
		for (const auto &batch : renderGraph->sortedQueueBatches)
		{
			timelineWidth += std::max(170.0f, 74.0f * std::max(1, static_cast<int>(batch.sortedNodes.size()))) + batchGap;
		}
		float timelineHeight = padding.y * 2.0f + std::max(1, static_cast<int>(queueNames.size())) * laneHeight;

		ImGui::BeginChild("queue_execution_timeline_canvas", ImVec2(0.0f, 320.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		ImVec2      origin   = ImGui::GetCursorScreenPos();

		for (int lane = 0; lane < queueNames.size(); ++lane)
		{
			float  y = origin.y + padding.y + lane * laneHeight;
			ImVec2 laneMin(origin.x + padding.x, y);
			ImVec2 laneMax(origin.x + timelineWidth - padding.x, y + laneHeight - 12.0f);
			drawList->AddRectFilled(laneMin, laneMax, laneFill, 6.0f);
			drawList->AddRect(laneMin, laneMax, laneBorder, 6.0f);
			drawList->AddText(ImVec2(laneMin.x + 10.0f, laneMin.y + 9.0f), textColor, queueNames[lane].c_str());
			std::string laneMeta = "batches " + std::to_string(queueBatchCount[queueNames[lane]]) +
			                       " | nodes " + std::to_string(queueNodeCount[queueNames[lane]]);
			drawList->AddText(ImVec2(laneMin.x + 10.0f, laneMin.y + 30.0f), mutedColor, laneMeta.c_str());
		}

		float xCursor = origin.x + padding.x + labelWidth;
		for (int batchIdx = 0; batchIdx < renderGraph->sortedQueueBatches.size(); ++batchIdx)
		{
			const auto &batch      = renderGraph->sortedQueueBatches[batchIdx];
			int         lane       = queueLaneByName.at(batch.queueName);
			float       batchWidth = std::max(170.0f, 74.0f * std::max(1, static_cast<int>(batch.sortedNodes.size())));
			float       y          = origin.y + padding.y + lane * laneHeight + 14.0f;
			ImVec2      minPos(xCursor, y);
			ImVec2      maxPos(xCursor + batchWidth, y + batchHeight);
			bool        selected = selectedBatchIdx == batchIdx;
			ImU32       fill     = batch.sortedNodes.empty() ? emptyFill : queueColors[lane % IM_ARRAYSIZE(queueColors)];

			drawList->AddRectFilled(minPos, maxPos, fill, 6.0f);
			drawList->AddRect(minPos, maxPos, selected ? selectedColor : laneBorder, 6.0f, 0, selected ? 3.0f : 1.0f);

			std::string batchTitle = "#" + std::to_string(batchIdx) + " batch " + std::to_string(batch.id);
			drawList->AddText(ImVec2(minPos.x + 9.0f, minPos.y + 7.0f), textColor, batchTitle.c_str());
			std::string batchMeta = "pool " + std::to_string(batch.poolIdUsed) + " | nodes " + std::to_string(batch.sortedNodes.size());
			drawList->AddText(ImVec2(minPos.x + 9.0f, minPos.y + 26.0f), mutedColor, batchMeta.c_str());

			if (batch.sortedNodes.empty())
			{
				drawList->AddText(ImVec2(minPos.x + 9.0f, minPos.y + 45.0f), mutedColor, "no render nodes");
			}
			else
			{
				float nodeX = minPos.x + 9.0f;
				for (int nodeIdx = 0; nodeIdx < batch.sortedNodes.size(); ++nodeIdx)
				{
					RenderGraphNode *node      = batch.sortedNodes[nodeIdx];
					float            chipWidth = std::min(96.0f, std::max(48.0f, (batchWidth - 18.0f) / static_cast<float>(batch.sortedNodes.size()) - 4.0f));
					ImVec2           chipMin(nodeX, minPos.y + 41.0f);
					ImVec2           chipMax(nodeX + chipWidth, chipMin.y + nodeHeight);
					drawList->AddRectFilled(chipMin, chipMax, node && node->active ? nodeFill : inactiveFill, 4.0f);
					drawList->AddRect(chipMin, chipMax, IM_COL32(118, 104, 101, 130), 4.0f);
					std::string nodeLabel = node ? node->passName : "null";
					if (nodeLabel.size() > 10)
					{
						nodeLabel = nodeLabel.substr(0, 9) + ".";
					}
					drawList->AddText(ImVec2(chipMin.x + 5.0f, chipMin.y + 3.0f), textColor, nodeLabel.c_str());
					nodeX += chipWidth + 4.0f;
				}
			}

			ImGui::SetCursorScreenPos(minPos);
			ImGui::PushID(batchIdx);
			if (ImGui::InvisibleButton("queue_batch_hitbox", ImVec2(batchWidth, batchHeight)))
			{
				selectedBatchIdx = batchIdx;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Batch index: %d", batchIdx);
				ImGui::Text("Batch id: %d", batch.id);
				ImGui::Text("Queue: %s", batch.queueName.c_str());
				ImGui::Text("Pool id: %d", batch.poolIdUsed);
				ImGui::Text("Nodes: %d", static_cast<int>(batch.sortedNodes.size()));
				for (int nodeIdx = 0; nodeIdx < batch.sortedNodes.size(); ++nodeIdx)
				{
					RenderGraphNode *node = batch.sortedNodes[nodeIdx];
					ImGui::BulletText("%02d. %s%s", nodeIdx, node ? node->passName.c_str() : "null", node && !node->active ? " (inactive)" : "");
				}
				ImGui::EndTooltip();
			}
			ImGui::PopID();

			xCursor += batchWidth + batchGap;
		}

		ImGui::SetCursorScreenPos(origin);
		ImGui::Dummy(ImVec2(timelineWidth, timelineHeight));
		ImGui::EndChild();

		const auto &selectedBatch = renderGraph->sortedQueueBatches[selectedBatchIdx];
		ImGui::SeparatorText("Selected Queue Batch");
		if (ImGui::BeginTable("selected_queue_batch", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Order");
			ImGui::TableSetupColumn("Pass");
			ImGui::TableSetupColumn("Queue");
			ImGui::TableSetupColumn("State");
			ImGui::TableSetupColumn("Dependencies");
			ImGui::TableHeadersRow();

			for (int nodeIdx = 0; nodeIdx < selectedBatch.sortedNodes.size(); ++nodeIdx)
			{
				RenderGraphNode *node = selectedBatch.sortedNodes[nodeIdx];
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%02d", nodeIdx);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", node ? node->passName.c_str() : "null");
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", node ? node->workerQueueName.c_str() : selectedBatch.queueName.c_str());
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

			if (selectedBatch.sortedNodes.empty())
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d", selectedBatchIdx);
				ImGui::TableSetColumnIndex(1);
				ImGui::TextUnformatted("no render nodes");
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", selectedBatch.queueName.c_str());
				ImGui::TableSetColumnIndex(3);
				ImGui::TextUnformatted("empty");
				ImGui::TableSetColumnIndex(4);
				ImGui::Text("batch id %d | pool %d", selectedBatch.id, selectedBatch.poolIdUsed);
			}

			ImGui::EndTable();
		}
	}

	void PaintingInfo()
	{
		ImGui::SliderInt("Brush Radius", &flatRenderer->paintingPc.radius, 1, 100);

		static float color[4] = {1.0f, 1.0f, 1.0f, 1.0};
		if (ImGui::ColorEdit4("Brush Radius", color))
		{
			flatRenderer->paintingPc.color = glm::make_vec4(color);
		}
		ImGui::SliderInt("Layer", &flatRenderer->paintingPc.layerSelected, 0, 1);

		if (ImGui::Button("Clear Canvas"))
		{
			ResourcesManager::GetInstance()->RequestStorageImageClear("PaintingLayer");
			ResourcesManager::GetInstance()->RequestStorageImageClear("OccluderLayer");
			ResourcesManager::GetInstance()->RequestStorageImageClear("DebugRaysLayer");
		}
		for (int i = 0; i < flatRenderer->cascadesInfo.cascadeCount; ++i)
		{
			std::string name = "radianceStorage_" + std::to_string(i);
			ResourcesManager::GetInstance()->RequestStorageImageClear(name);
		}
	}
	void AnimatorInfo()
	{
		int i = 0;
		for (const auto &animatorPair : RenderingResManager::GetInstance()->animatorsNames)
		{
			auto        animator         = RenderingResManager::GetInstance()->GetAnimatorByName(animatorPair.first);
			std::string frameSpacingName = animatorPair.first + ": Frame Spacing";
			ImGui::SliderInt(frameSpacingName.c_str(), &animator->frameSpacing, 1, 1000);

			std::string frameInfo = animatorPair.first + " Frames Info: " + std::to_string(animator->animatorInfo.currentFrame) + " / " + std::to_string(animator->animatorInfo.frameCount);
			ImGui::Text("%s", frameInfo.c_str());

			std::string frameSpacingInfo = animatorPair.first + " Frame Spacing info: " + std::to_string(animator->currentFrameSpacing) + " / " + std::to_string(animator->frameSpacing);
			ImGui::Text("%s", frameSpacingInfo.c_str());

			std::string interpInfo = animatorPair.first + " Interpolation info: " + std::to_string(animator->animatorInfo.interpVal) +
			                         " / 1.0";
			ImGui::Text("%s", interpInfo.c_str());

			std::string stopAnim = animatorPair.first + ": Stop anim";
			ImGui::Checkbox(stopAnim.c_str(), &animator->stop);
			if (animator->stop)
			{
				std::string frameIndexLabelName = animatorPair.first + ": Frame index";
				ImGui::SliderInt(frameIndexLabelName.c_str(), &animator->animatorInfo.currentFrame, 1, animator->animatorInfo.frameCount);
			}
		}
	}

	void RCascadesInfo()
	{
		// ImGui::Begin("Radiance Output Info");
		//
		// ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		//
		// std::vector<ImageView*> cascades = flatRenderer->cascadesAttachmentsImagesViews;
		//
		// for (int i = 0; i < cascades.size(); ++i)
		// {
		// 	std::string imageName = "cascade_" + std::to_string(i);
		// 	AddImage(imageName ,cascades[i], viewportSize);
		// }
		// std::vector<ImageView*> paintingLayers = flatRenderer->paintingLayers;
		//
		// ImGui::End();
		//
		ImGui::SeparatorText("Cascades Configs");
		static int probeSizePx = flatRenderer->cascadesInfo.probeSizePx;
		if (ImGui::SliderInt("Probe Size in Px", &probeSizePx, 2, 1024))
		{
			flatRenderer->cascadesInfo.probeSizePx = probeSizePx;
		}
		static int intervalCount = flatRenderer->cascadesInfo.intervalCount;
		if (ImGui::SliderInt("Interval Count", &intervalCount, 1, 16))
		{
			flatRenderer->cascadesInfo.intervalCount = intervalCount;
		}

		static int baseIntervalLength = flatRenderer->cascadesInfo.baseIntervalLength;
		if (ImGui::SliderInt("Base Interval Length", &baseIntervalLength, 1, 1000))
		{
			flatRenderer->cascadesInfo.baseIntervalLength = baseIntervalLength;
		}

		ImGui::SeparatorText("Light Configs");
		static float lightDir[3] = {0.0, 1.0, 0.0};
		if (ImGui::SliderFloat3("Light Dir", lightDir, 0.0, 1.0))
		{
			flatRenderer->light.pos = glm::make_vec3(lightDir);
		}
		static float lightCol[3] = {0.0, 0.0, 1.0};
		if (ImGui::ColorEdit3("Light Col", lightCol))
		{
			flatRenderer->light.col = glm::make_vec3(lightCol);
		}
		static float intensity = flatRenderer->light.intensity;
		if (ImGui::SliderFloat("Light Intensity", &intensity, 0.0, 1.0))
		{
			flatRenderer->light.intensity = intensity;
		}
		ImGui::SeparatorText("Texture Configs");
		static int radiancePow = flatRenderer->rConfigs.radiancePow;
		if (ImGui::SliderInt("Radiance Pow", &radiancePow, 1, 24))
		{
			flatRenderer->rConfigs.radiancePow = radiancePow;
		}
		static int normalMapPow = flatRenderer->rConfigs.normalMapPow;
		if (ImGui::SliderInt("Normal Map Pow", &normalMapPow, 1, 24))
		{
			flatRenderer->rConfigs.normalMapPow = normalMapPow;
		}
		static int specularPow = flatRenderer->rConfigs.specularPow;
		if (ImGui::SliderInt("SpecularPow Pow", &specularPow, 1, 24))
		{
			flatRenderer->rConfigs.specularPow = specularPow;
		}
		static int roughnessPow = flatRenderer->rConfigs.roughnessPow;
		if (ImGui::SliderInt("Roughness Pow", &roughnessPow, 1, 24))
		{
			flatRenderer->rConfigs.roughnessPow = roughnessPow;
		}

		ImGui::SeparatorText("Background Material");
		static int materialSelected = flatRenderer->materialIndexSelected;
		if (ImGui::SliderInt("Material Selected", &materialSelected, 0, flatRenderer->backgroundMaterials.size() - 1))
		{
			flatRenderer->materialIndexSelected = materialSelected;
		}
		DisplayMaterial(flatRenderer->backgroundMaterials.at(flatRenderer->materialIndexSelected));

		PaintingInfo();

		AnimatorInfo();
	}

	void GSRendererInfo()
	{
		if (!gsRenderer)
			return;

		ImGui::SeparatorText("Gaussian Info");
		int gaussianCount = gsRenderer->gaussians.pos.size();
		ImGui::Text("Gaussian Count: %d", gaussianCount);
		int drawCount = gsRenderer->indexedCmds.size();
		ImGui::Text("Indirect Draw Count: %d", drawCount);

		ImGui::SeparatorText("GS Configs");

		// Bind to GSConfigsPc fields
		ImGui::SliderFloat("Global Scale", &gsRenderer->gsConfigsPc.scaleMod, 0.01f, 10.0f);

		// static const char* renderModes[] = {"Default", "Debug Ellipsoids", "BBox Only"};
		// ImGui::Combo("Render Mode", &gsRenderer->gsConfigsPc.renderMode, renderModes, IM_ARRAYSIZE(renderModes));

		ImGui::SeparatorText("Camera Info");
		ImGui::SliderFloat("Camera Speed", &gsRenderer->camera.movementSpeed, 0.01f, 100.0f);

		const auto &cam = gsRenderer->camera;
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", cam.position.x, cam.position.y, cam.position.z);
		ImGui::Text("Forward: (%.2f, %.2f, %.2f)", cam.forward.x, cam.forward.y, cam.forward.z);
		ImGui::Text("Movement Speed: %.2f", cam.movementSpeed);

		if (ImGui::Button("Update Sort"))
		{
			gsRenderer->ReSort();
		}
	}

	void AddImage(std::string name, ImageView *imageView)
	{
		if (name == "default_storage" || name == "default_tex")
		{
			return;
		}
		Sampler       *sampler    = ResourcesManager::GetInstance()->shipperSampler;
		LayoutPatterns lastLayout = imageView->imageData->currentLayout;
		layoutPatternsToRecover.push_back(lastLayout);
		imageViewsToRecover.push_back(imageView);

		if (dsetsArrays->indexes.contains(name))
		{
			TransitionImage(imageView->imageData, lastLayout, imageView->GetSubresourceRange(),
			                *currCommandBuffer);
			return;
		}
		dsetsArrays->AddSet(name);
		ENGINE::DescriptorWriterBuilder writerBuilder;
		writerBuilder.AddWriteImage(0, imageView, sampler->samplerHandle.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
		writerBuilder.UpdateSet(core->logicalDevice.get(), dsetsArrays->GetDsetByName(name));
	}
	void DisplayMaterial(Material *mat)
	{
		UI::TextureViewer textureViewerBaseCol;
		textureViewerBaseCol.AddProperty(UI::DRAG);
		textureViewerBaseCol.AddProperty(UI::DROP);
		for (auto &texture : mat->texturesRef)
		{
			if (texture.second == nullptr)
			{
				continue;
			}
			std::string name = mat->texturesStrings.at(texture.first);
			AddImage(texture.second->name, texture.second);
			ImageView *imageViewRef = textureViewerBaseCol.DisplayTexture(name, texture.second, (ImTextureID) dsetsArrays->GetDsetByName(texture.second->name), {50, 50});
			if (imageViewRef->name != texture.second->name)
			{
				texture.second = imageViewRef;
			}
		}
	}
	void AddAllImages()
	{
		for (auto &image : renderGraph->resourcesManager->imageViews)
		{
			AddImage(image->name, image.get());
		}
		for (auto &image : renderGraph->resourcesManager->storageImgsViews)
		{
			AddImage(image->name, image.get());
		}
		for (auto &image : renderGraph->resourcesManager->imageShippers)
		{
			AddImage(image->imageView->name, image->imageView.get());
		}
	}
	void DisplayAllTextures()
	{
		AddAllImages();
		static char textBuff[256] = "";
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##texture_filter", "Filter textures by name...", textBuff, 256);
		std::string input(textBuff);

		std::vector<ImageView *> visibleImages;
		for (auto &image : renderGraph->resourcesManager->imageViews)
		{
			if (!input.empty() && image->name.find(input) == std::string::npos)
			{
				continue;
			}
			visibleImages.push_back(image.get());
		}
		for (auto &image : renderGraph->resourcesManager->storageImgsViews)
		{
			if (image->name == "default_storage")
			{
				continue;
			}
			if (!input.empty() && image->name.find(input) == std::string::npos)
			{
				continue;
			}
			visibleImages.push_back(image.get());
		}
		for (auto &image : renderGraph->resourcesManager->imageShippers)
		{
			if (image->imageView->name == "default_tex")
			{
				continue;
			}
			if (!input.empty() && image->imageView->name.find(input) == std::string::npos)
			{
				continue;
			}
			visibleImages.push_back(image->imageView.get());
		}

		ImGui::TextDisabled("%d texture%s visible", static_cast<int>(visibleImages.size()),
		                    visibleImages.size() == 1 ? "" : "s");
		ImGui::Spacing();

		if (visibleImages.empty())
		{
			ImGui::TextDisabled("No textures match the current filter.");
			return;
		}

		const float availableWidth = ImGui::GetContentRegionAvail().x;
		const float minimumCardWidth = 190.0f;
		const float columnSpacing = ImGui::GetStyle().ItemSpacing.x;
		const int   columnCount = std::max(1, static_cast<int>((availableWidth + columnSpacing) /
		                                                     (minimumCardWidth + columnSpacing)));
		const float cardWidth = (availableWidth - columnSpacing * static_cast<float>(columnCount - 1)) /
		                        static_cast<float>(columnCount);
		const float previewSize = std::min(220.0f, std::max(96.0f, cardWidth - 24.0f));

		if (ImGui::BeginTable("texture_card_grid", columnCount,
		                      ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings))
		{
			for (ImageView *imageView : visibleImages)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(imageView);
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImguiRendererUI::RaisedSurfaceColor());
				if (ImGui::BeginChild("##texture_card", ImVec2(0.0f, previewSize + 68.0f),
				                      ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar))
				{
					ImGui::PushFont(fonts.strong);
					ImGui::TextUnformatted(imageView->name.c_str());
					ImGui::PopFont();
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s", imageView->name.c_str());
					}

					const glm::uvec2 dimensions = imageView->imageData->GetImageSize();
					ImGui::TextDisabled("%u x %u", dimensions.x, dimensions.y);
					const float previewOffset = std::max(0.0f, (ImGui::GetContentRegionAvail().x - previewSize) * 0.5f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + previewOffset);
					ImGui::Image(
					    (ImTextureID) dsetsArrays->GetDsetByName(imageView->name),
					    ImVec2(previewSize, previewSize), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s\n%u x %u", imageView->name.c_str(), dimensions.x, dimensions.y);
					}
				}
				ImGui::EndChild();
				ImGui::PopStyleColor();
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
	void Destroy()
	{
		if (m_Context)
		{
			ed::DestroyEditor(m_Context);
			m_Context = nullptr;
		}
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	vk::CommandBuffer  *currCommandBuffer = nullptr;
	DynamicRenderPass   dynamicRenderPass;
	WindowProvider     *windowProvider;
	DescriptorAllocator descriptorAllocator;
	VkFormat            imguiSwapchainFormat = VK_FORMAT_UNDEFINED;
	Core               *core;
	RenderGraph        *renderGraph;

	std::map<std::string, std::unique_ptr<BaseRenderer>> *renderers       = nullptr;
	ClusterRenderer                                      *clusterRenderer = nullptr;
	FlatRenderer                                         *flatRenderer    = nullptr;
	GSRenderer                                           *gsRenderer      = nullptr;
	ImGuiUtils::ProfilersWindow                           profilersWindow{};
	UI::RG_NodeEditor                                     nodeEditor;
	ImguiRendererUI::Fonts                                fonts{};
	DebuggerPage                                          activePage = DebuggerPage::RenderGraph;
	bool                                                  sceneViewportValid = false;
	bool                                                  sceneViewportHovered = false;
	ImVec2                                                sceneViewportMin{};
	ImVec2                                                sceneViewportMax{};
	float                                                 inspectorPanelWidth = 0.0f;
	bool                                                  inspectorPanelResizing = false;
	glm::vec2                                             prettySmokeLastMousePosition{};
	glm::vec4                                             prettySmokeLastColor{0.15f, 0.05f, 0.25f, 1.0f};
	bool                                                  prettySmokeStrokeActive = false;

	std::unique_ptr<ImguiDsetsArray> dsetsArrays;
	std::vector<LayoutPatterns>      layoutPatternsToRecover;
	std::vector<ImageView *>         imageViewsToRecover;

	std::vector<glm::vec4> image_pixels = {};
	std::vector<int>       solid_mask   = {};
	int                    width, height, channels;
	struct emitter
	{
		glm::vec4 color;
		glm::vec2 velocity;
		int       radius;
		int       xPos;
		int       yPos;
	};
	std::vector<emitter> emitters;
};
inline void ImguiRenderer::DisplayRenderGraphDag()
{
	ImGui::SeparatorText("Render Graph DAG");

	std::vector<RenderGraphNode *> graphNodes = renderGraph->sortedByDepNodes;
	if (graphNodes.empty())
	{
		graphNodes = renderGraph->sequentialRenderNodes;
	}
	if (graphNodes.empty())
	{
		ImGui::Text("No render graph nodes");
		return;
	}

	static std::string selectedNodeName;

	std::map<std::string, RenderGraphNode *> nodeByName;
	std::map<std::string, int>               levelByName;
	std::map<std::string, int>               incomingCountByName;
	std::map<std::string, int>               outgoingCountByName;
	int                                      activeNodeCount        = 0;
	int                                      edgeCount              = 0;
	int                                      missingDependencyCount = 0;
	for (auto *node : graphNodes)
	{
		if (!node)
		{
			continue;
		}
		nodeByName[node->passName]          = node;
		levelByName[node->passName]         = 0;
		incomingCountByName[node->passName] = 0;
		outgoingCountByName[node->passName] = 0;
		if (node->active)
		{
			activeNodeCount++;
		}
	}

	for (auto *node : graphNodes)
	{
		if (!node)
		{
			continue;
		}
		incomingCountByName[node->passName] = static_cast<int>(node->dependencies.size());
		edgeCount += static_cast<int>(node->dependencies.size());
		for (const auto &dependency : node->dependencies)
		{
			if (outgoingCountByName.contains(dependency))
			{
				outgoingCountByName.at(dependency)++;
			}
			else
			{
				missingDependencyCount++;
			}
		}
	}

	if (selectedNodeName.empty() || !nodeByName.contains(selectedNodeName))
	{
		selectedNodeName = graphNodes.front() ? graphNodes.front()->passName : "";
	}

	ImGui::Text("Nodes: %d | Active: %d | Edges: %d | Queue batches: %d | Missing deps: %d",
	            static_cast<int>(nodeByName.size()),
	            activeNodeCount,
	            edgeCount,
	            static_cast<int>(renderGraph->sortedQueueBatches.size()),
	            missingDependencyCount);

	int maxLevel = 0;
	for (auto *node : graphNodes)
	{
		if (!node)
		{
			continue;
		}
		int nodeLevel = 0;
		for (const auto &dependency : node->dependencies)
		{
			if (levelByName.contains(dependency))
			{
				nodeLevel = std::max(nodeLevel, levelByName.at(dependency) + 1);
			}
		}
		levelByName[node->passName] = nodeLevel;
		maxLevel                    = std::max(maxLevel, nodeLevel);
	}

	std::vector<std::vector<RenderGraphNode *>> nodesByLevel(maxLevel + 1);
	for (auto *node : graphNodes)
	{
		if (!node)
		{
			continue;
		}
		nodesByLevel[levelByName.at(node->passName)].push_back(node);
	}

	int maxRows = 1;
	for (const auto &levelNodes : nodesByLevel)
	{
		maxRows = std::max(maxRows, static_cast<int>(levelNodes.size()));
	}

	const ImVec2 nodeSize(220.0f, 66.0f);
	const float  levelGap = 100.0f;
	const float  rowGap   = 28.0f;
	const ImVec2 padding(24.0f, 24.0f);
	const ImVec2 canvasSize(
	    padding.x * 2.0f + (maxLevel + 1) * nodeSize.x + maxLevel * levelGap,
	    padding.y * 2.0f + maxRows * nodeSize.y + std::max(0, maxRows - 1) * rowGap);

	ImGui::BeginChild("render_graph_dag_canvas", ImVec2(0.0f, 420.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
	ImDrawList *drawList = ImGui::GetWindowDrawList();
	ImVec2      origin   = ImGui::GetCursorScreenPos();

	std::map<std::string, ImVec2> nodeMinByName;
	std::map<std::string, ImVec2> nodeMaxByName;
	for (int level = 0; level < nodesByLevel.size(); ++level)
	{
		const auto &levelNodes  = nodesByLevel[level];
		float       levelHeight = levelNodes.size() * nodeSize.y + std::max(0, static_cast<int>(levelNodes.size()) - 1) * rowGap;
		float       yOffset     = std::max(0.0f, (canvasSize.y - padding.y * 2.0f - levelHeight) * 0.5f);
		for (int row = 0; row < levelNodes.size(); ++row)
		{
			RenderGraphNode *node = levelNodes[row];
			ImVec2           minPos(
                origin.x + padding.x + level * (nodeSize.x + levelGap),
                origin.y + padding.y + yOffset + row * (nodeSize.y + rowGap));
			ImVec2 maxPos(minPos.x + nodeSize.x, minPos.y + nodeSize.y);
			nodeMinByName[node->passName] = minPos;
			nodeMaxByName[node->passName] = maxPos;
		}
	}

	const ImU32 edgeColor      = IM_COL32(126, 108, 103, 160);
	const ImU32 edgeArrowColor = IM_COL32(176, 145, 135, 205);
	for (auto *node : graphNodes)
	{
		if (!node || !nodeMinByName.contains(node->passName))
		{
			continue;
		}
		for (const auto &dependency : node->dependencies)
		{
			if (!nodeMaxByName.contains(dependency))
			{
				continue;
			}
			ImVec2 src(nodeMaxByName.at(dependency).x, (nodeMinByName.at(dependency).y + nodeMaxByName.at(dependency).y) * 0.5f);
			ImVec2 dst(nodeMinByName.at(node->passName).x, (nodeMinByName.at(node->passName).y + nodeMaxByName.at(node->passName).y) * 0.5f);
			float  midX = (src.x + dst.x) * 0.5f;
			drawList->AddBezierCubic(src, ImVec2(midX, src.y), ImVec2(midX, dst.y), dst, edgeColor, 2.0f);
			drawList->AddTriangleFilled(
			    ImVec2(dst.x, dst.y),
			    ImVec2(dst.x - 8.0f, dst.y - 5.0f),
			    ImVec2(dst.x - 8.0f, dst.y + 5.0f),
			    edgeArrowColor);
		}
	}

	const ImU32 activeFill          = IM_COL32(50, 42, 40, 255);
	const ImU32 inactiveFill        = IM_COL32(38, 34, 36, 255);
	const ImU32 selectedFill        = IM_COL32(76, 45, 34, 255);
	const ImU32 warningFill         = IM_COL32(80, 54, 36, 255);
	const ImU32 borderColor         = IM_COL32(125, 111, 107, 185);
	const ImU32 selectedBorderColor = IM_COL32(255, 82, 20, 255);
	const ImU32 warningColor        = IM_COL32(245, 172, 95, 255);
	const ImU32 textColor           = IM_COL32(235, 231, 228, 255);
	const ImU32 mutedColor          = IM_COL32(151, 143, 141, 255);
	for (auto *node : graphNodes)
	{
		if (!node || !nodeMinByName.contains(node->passName))
		{
			continue;
		}
		ImVec2 minPos               = nodeMinByName.at(node->passName);
		ImVec2 maxPos               = nodeMaxByName.at(node->passName);
		bool   hasMissingDependency = false;
		for (const auto &dependency : node->dependencies)
		{
			if (!nodeByName.contains(dependency))
			{
				hasMissingDependency = true;
				break;
			}
		}
		bool  selected  = selectedNodeName == node->passName;
		ImU32 fillColor = hasMissingDependency ? warningFill : (selected ? selectedFill : (node->active ? activeFill : inactiveFill));
		drawList->AddRectFilled(minPos, maxPos, fillColor, 6.0f);
		drawList->AddRect(minPos, maxPos, selected ? selectedBorderColor : (hasMissingDependency ? warningColor : borderColor), 6.0f, 0, selected ? 3.0f : 1.0f);
		drawList->AddText(ImVec2(minPos.x + 10.0f, minPos.y + 7.0f), textColor, node->passName.c_str());
		std::string nodeMeta = node->workerQueueName + (node->active ? " | active" : " | inactive");
		drawList->AddText(ImVec2(minPos.x + 10.0f, minPos.y + 24.0f), mutedColor, nodeMeta.c_str());
		std::string edgeMeta = "in " + std::to_string(incomingCountByName[node->passName]) +
		                       " | out " + std::to_string(outgoingCountByName[node->passName]);
		drawList->AddText(ImVec2(minPos.x + 10.0f, minPos.y + 43.0f), hasMissingDependency ? warningColor : mutedColor, edgeMeta.c_str());

		ImGui::SetCursorScreenPos(minPos);
		ImGui::PushID(node->passName.c_str());
		if (ImGui::InvisibleButton("dag_node_hitbox", nodeSize))
		{
			selectedNodeName = node->passName;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%s", node->passName.c_str());
			ImGui::Text("Queue: %s", node->workerQueueName.c_str());
			ImGui::Text("Dependencies: %d", static_cast<int>(node->dependencies.size()));
			ImGui::Text("Dependents: %d", outgoingCountByName[node->passName]);
			ImGui::Text("Shader node: %s", node->GPUPipelineRef ? node->GPUPipelineRef->name.c_str() : "null");
			ImGui::EndTooltip();
		}
		ImGui::PopID();
	}

	ImGui::SetCursorScreenPos(origin);
	ImGui::Dummy(canvasSize);
	ImGui::EndChild();

	if (!selectedNodeName.empty() && nodeByName.contains(selectedNodeName))
	{
		RenderGraphNode *selectedNode = nodeByName.at(selectedNodeName);
		GPUPipeline     *shaderNode   = selectedNode->GPUPipelineRef;

		ImGui::SeparatorText("Selected Node");
		if (ImGui::BeginTable("selected_render_graph_node", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Field");
			ImGui::TableSetupColumn("Value");
			ImGui::TableHeadersRow();

			auto addRow = [](const char *name, const std::string &value) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(name);
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped("%s", value.c_str());
			};

			auto pipelineName = [](vk::PipelineBindPoint bindPoint) {
				switch (bindPoint)
				{
					case vk::PipelineBindPoint::eGraphics:
						return "Graphics";
					case vk::PipelineBindPoint::eCompute:
						return "Compute";
					default:
						return "Unknown";
				}
			};

			auto imageMapNames = [](const std::unordered_map<std::string, ImageView *> &images) {
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

			auto bufferNames = [](const std::unordered_map<std::string, BufferKey> &buffers) {
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

			addRow("Pass", selectedNode->passName);
			addRow("Queue", selectedNode->workerQueueName);
			addRow("State", selectedNode->active ? "active" : "inactive");
			addRow("Level", std::to_string(levelByName[selectedNode->passName]));
			addRow("Dependencies", std::to_string(selectedNode->dependencies.size()));
			addRow("Dependents", std::to_string(outgoingCountByName[selectedNode->passName]));
			addRow("Shader node", shaderNode ? shaderNode->name : "null");
			addRow("Pipeline", shaderNode ? pipelineName(shaderNode->pipelineType) : "n/a");
			addRow("Push constant bytes", shaderNode ? std::to_string(shaderNode->pushConstantSize) : "n/a");
			addRow("Color attachments", shaderNode ? std::to_string(shaderNode->colAttachments.size()) : "n/a");
			addRow("Blend configs", shaderNode ? std::to_string(shaderNode->colorBlendConfigs.size()) : "n/a");
			addRow("Depth attachment", shaderNode ? (shaderNode->depthAttachment.format == vk::Format::eUndefined ? "none" : "present") : "n/a");
			addRow("Framebuffer", std::to_string(selectedNode->GetFrameBufferSize().x) + " x " + std::to_string(selectedNode->GetFrameBufferSize().y));
			addRow("Attachment outputs", std::to_string(selectedNode->GetImageAttachmentOutputs().size()));
			addRow("Attachment names", std::to_string(selectedNode->GetImageAttachmentNames().size()));
			addRow("Depth image", selectedNode->GetDepthImage() ? selectedNode->GetDepthImage()->name : "none");
			addRow("Sampled images", std::to_string(selectedNode->GetSampledImages().size()));
			addRow("Sampled image names", imageMapNames(selectedNode->GetSampledImages()));
			addRow("Storage images", std::to_string(selectedNode->GetStorageImages().size()));
			addRow("Storage image names", imageMapNames(selectedNode->GetStorageImages()));
			addRow("Buffers", std::to_string(selectedNode->GetBuffers().size()));
			addRow("Buffer names", bufferNames(selectedNode->GetBuffers()));
			addRow("Descriptor cache", shaderNode ? (shaderNode->descCache ? "present" : "null") : "n/a");
			addRow("Auto descriptor cache", shaderNode ? (shaderNode->configs.automaticCache ? "true" : "false") : "n/a");
			addRow("Manual add", shaderNode ? (shaderNode->configs.manualAdd ? "true" : "false") : "n/a");

			std::string shaderStages;
			if (shaderNode)
			{
				for (const auto &shader : shaderNode->shaders)
				{
					if (shader.second)
					{
						if (!shaderStages.empty())
						{
							shaderStages += ", ";
						}
						shaderStages += shader.first;
					}
				}
			}
			addRow("Shaders", shaderStages.empty() ? "none" : shaderStages);

			std::string dependencies;
			for (const auto &dependency : selectedNode->dependencies)
			{
				if (!dependencies.empty())
				{
					dependencies += ", ";
				}
				dependencies += dependency;
				if (!nodeByName.contains(dependency))
				{
					dependencies += " (missing)";
				}
			}
			addRow("Depends on", dependencies.empty() ? "none" : dependencies);

			std::string dependents;
			for (auto *node : graphNodes)
			{
				if (node && node->dependencies.contains(selectedNode->passName))
				{
					if (!dependents.empty())
					{
						dependents += ", ";
					}
					dependents += node->passName;
				}
			}
			addRow("Used by", dependents.empty() ? "none" : dependents);

			ImGui::EndTable();
		}
	}
}
}        // namespace Rendering

#endif        // IMGUIRENDERER_HPP
