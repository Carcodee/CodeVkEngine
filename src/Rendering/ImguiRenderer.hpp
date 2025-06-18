//
// Created by carlo on 2024-10-10.
//

#ifndef IMGUIRENDERER_HPP
#define IMGUIRENDERER_HPP

namespace Rendering
{
namespace ed = ax::NodeEditor;
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
		this->core           = core;
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
		initInfo.Queue                     = core->graphicsQueue;
		initInfo.DescriptorPool            = descriptorAllocator.pool.get();
		initInfo.MinImageCount             = 3;
		initInfo.ImageCount                = 3;
		initInfo.UseDynamicRendering       = true;

		initInfo.PipelineRenderingCreateInfo                         = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
		initInfo.PipelineRenderingCreateInfo.colorAttachmentCount    = 1;
		VkFormat swapchainFormat                                     = static_cast<VkFormat>(core->swapchainRef->GetFormat());
		initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainFormat;

		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		SetStyle();

		ImGui_ImplVulkan_Init(&initInfo);

		ImGui_ImplVulkan_CreateFontsTexture();

		ed::Config config;
		config.SettingsFile = "Simple.json";
		m_Context           = ed::CreateEditor(&config);
	}
	ed::EditorContext *m_Context = nullptr;
	void               StartNodeEditor()
	{
		auto &io = ImGui::GetIO();

		ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

		ImGui::Separator();

		ed::SetCurrentEditor(m_Context);

		static bool firstFrame = true;
		nodeEditor.Init(renderGraph, windowProvider);
		nodeEditor.Draw();

		firstFrame = false;

		ed::SetCurrentEditor(nullptr);
	}

	void RenderFrame(vk::CommandBuffer commandBuffer, vk::ImageView &imageView)
	{
		currCommandBuffer = &commandBuffer;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		StartNodeEditor();

		DisplayGeneralEngineInfo();

		// ImGui::ShowDemoWindow();
		if (clusterRenderer)
		{
			ClusterRendererInfo();
		}
		if (flatRenderer)
		{
			PaintingInfo();
			RCascadesInfo();
			AnimatorInfo();
		}
		if (gsRenderer)
		{
			GSRendererInfo();
		}

		RenderGraphProfiler();

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
	void RenderGraphProfiler()
	{
		profilersWindow.cpuGraph.LoadFrameData(Profiler::GetInstance()->cpuTasks.data(),
		                                       Profiler::GetInstance()->cpuTasks.size());
		profilersWindow.gpuGraph.LoadFrameData(Profiler::GetInstance()->gpuTasks.data(),
		                                       Profiler::GetInstance()->gpuTasks.size());
		profilersWindow.Render();
	}

	void SetStyle()
	{
		std::string fontPath = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\Fonts";

		std::string lightOpenSans   = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\Fonts\\Open_Sans\\static\\OpenSans-Light.ttf";
		std::string regularOpenSans = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\Fonts\\Open_Sans\\static\\OpenSans-Regular.ttf";
		std::string boldOpenSans    = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\Fonts\\Open_Sans\\static\\OpenSans-Bold.ttf";
		ImGuiIO    &io              = ImGui::GetIO();

		io.Fonts->Clear();
		io.Fonts->AddFontFromFileTTF(lightOpenSans.c_str(), 16);
		io.Fonts->AddFontFromFileTTF(regularOpenSans.c_str(), 16);
		io.Fonts->AddFontFromFileTTF(lightOpenSans.c_str(), 32);
		io.Fonts->AddFontFromFileTTF(regularOpenSans.c_str(), 11);
		io.Fonts->AddFontFromFileTTF(boldOpenSans.c_str(), 11);
		io.Fonts->Build();

		ImGuiStyle *style = &ImGui::GetStyle();

		style->WindowPadding     = ImVec2(15, 15);
		style->WindowRounding    = 5.0f;
		style->FramePadding      = ImVec2(5, 5);
		style->FrameRounding     = 4.0f;
		style->ItemSpacing       = ImVec2(12, 8);
		style->ItemInnerSpacing  = ImVec2(8, 6);
		style->IndentSpacing     = 25.0f;
		style->ScrollbarSize     = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize       = 5.0f;
		style->GrabRounding      = 3.0f;

		style->Colors[ImGuiCol_Text]                 = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style->Colors[ImGuiCol_TextDisabled]         = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_WindowBg]             = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_PopupBg]              = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_Border]               = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		style->Colors[ImGuiCol_BorderShadow]         = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		style->Colors[ImGuiCol_FrameBg]              = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_FrameBgActive]        = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_TitleBg]              = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		style->Colors[ImGuiCol_TitleBgActive]        = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_MenuBarBg]            = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_CheckMark]            = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrab]           = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Button]               = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ButtonHovered]        = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ButtonActive]         = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_Header]               = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_HeaderHovered]        = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_HeaderActive]         = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ResizeGrip]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_PlotLines]            = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered]     = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram]        = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	}
	void DisplayGeneralEngineInfo()
	{
		DisplayAllTextures();
	}

	void ClusterRendererInfo()
	{
		ImGui::Begin("Debug Info");

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

		for (auto &node : core->renderGraphRef->renderNodesSorted)
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

		ImGui::End();
	}
	void PaintingInfo()
	{
		ImGui::Begin("Painting Info");

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

		ImGui::End();
	}
	void AnimatorInfo()
	{
		ImGui::Begin("Animator Info");

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

		ImGui::End();
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
		ImGui::Begin("Radiance Cascades Configs");

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

		ImGui::End();
	}

	void GSRendererInfo()
	{
		if (!gsRenderer)
			return;

		ImGui::Begin("GS Renderer Info");

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

		ImGui::End();
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
		ImGui::Begin("Images Loaded");
		static char textBuff[256] = "";
		ImGui::InputText("Filter: ", textBuff, 256);
		std::string input(textBuff);

		UI::TextureViewer textureViewer;
		int               size = 400;
		for (auto &image : renderGraph->resourcesManager->imageViews)
		{
			if (!input.empty() && image->name.find(input) == std::string::npos)
			{
				continue;
			}
			ImageView *imageViewRef = textureViewer.DisplayTexture(image->name, image.get(), (ImTextureID) dsetsArrays->GetDsetByName(image->name), {size, size});
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
			ImageView *imageViewRef = textureViewer.DisplayTexture(image->name, image.get(), (ImTextureID) dsetsArrays->GetDsetByName(image->name), {size, size});
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
			ImageView *imageViewRef = textureViewer.DisplayTexture(image->imageView->name, image->imageView.get(),
			                                                       (ImTextureID) dsetsArrays->GetDsetByName(
			                                                           image->imageView->name),
			                                                       {size, size});
		}
		ImGui::End();
	}
	void Destroy()
	{
		ImGui_ImplVulkan_Shutdown();
	}

	vk::CommandBuffer  *currCommandBuffer = nullptr;
	DynamicRenderPass   dynamicRenderPass;
	WindowProvider     *windowProvider;
	DescriptorAllocator descriptorAllocator;
	Core               *core;
	RenderGraph        *renderGraph;

	std::map<std::string, std::unique_ptr<BaseRenderer>> *renderers       = nullptr;
	ClusterRenderer                                      *clusterRenderer = nullptr;
	FlatRenderer                                         *flatRenderer    = nullptr;
	GSRenderer                                           *gsRenderer      = nullptr;
	ImGuiUtils::ProfilersWindow                           profilersWindow{};
	UI::RG_NodeEditor                                     nodeEditor;

	std::unique_ptr<ImguiDsetsArray> dsetsArrays;
	std::vector<LayoutPatterns>      layoutPatternsToRecover;
	std::vector<ImageView *>         imageViewsToRecover;
};
}        // namespace Rendering

#endif        // IMGUIRENDERER_HPP
