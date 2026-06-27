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
			RCascadesInfo();
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
		DisplayEngineInfo();
		DisplayAllTextures();
	}

	void DisplayEngineInfo()
	{
		ImGui::Begin("engine info");

		auto *queueWorkerManager = core->queueWorkerManager.get();
		const int engineQueueCount = static_cast<int>(queueWorkerManager->workersQueues.size());
		ImGui::Text("Engine queues: %d", engineQueueCount);

		ImGui::SeparatorText("Queue Members");
		if (ImGui::BeginTable("engine_queue_members", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
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
		for (int i = 0; i < renderGraph->sortedByDepNodes.size(); ++i)
		{
			auto *node = renderGraph->sortedByDepNodes[i];
			ImGui::Text("%02d. %s | queue: %s | active: %s", i, node->passName.c_str(), node->workerQueueName.c_str(), node->active ? "true" : "false");
		}

		DisplayRenderGraphDag();

		DisplayQueueExecutionTimeline();

		ImGui::End();
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

		ImGui::End();
	}

	void DisplayRenderGraphDag()
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
		std::map<std::string, int>              levelByName;
		std::map<std::string, int>              incomingCountByName;
		std::map<std::string, int>              outgoingCountByName;
		int                                     activeNodeCount        = 0;
		int                                     edgeCount              = 0;
		int                                     missingDependencyCount = 0;
		for (auto *node : graphNodes)
		{
			if (!node)
			{
				continue;
			}
			nodeByName[node->passName]        = node;
			levelByName[node->passName]       = 0;
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
				ImVec2          minPos(
                    origin.x + padding.x + level * (nodeSize.x + levelGap),
                    origin.y + padding.y + yOffset + row * (nodeSize.y + rowGap));
				ImVec2 maxPos(minPos.x + nodeSize.x, minPos.y + nodeSize.y);
				nodeMinByName[node->passName] = minPos;
				nodeMaxByName[node->passName] = maxPos;
			}
		}

		const ImU32 edgeColor      = IM_COL32(130, 140, 170, 180);
		const ImU32 edgeArrowColor = IM_COL32(170, 180, 220, 220);
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

		const ImU32 activeFill   = IM_COL32(38, 55, 65, 255);
		const ImU32 inactiveFill = IM_COL32(42, 38, 44, 255);
		const ImU32 selectedFill = IM_COL32(56, 76, 92, 255);
		const ImU32 warningFill  = IM_COL32(80, 54, 36, 255);
		const ImU32 borderColor  = IM_COL32(150, 155, 170, 220);
		const ImU32 selectedBorderColor = IM_COL32(110, 185, 230, 255);
		const ImU32 warningColor        = IM_COL32(245, 172, 95, 255);
		const ImU32 textColor    = IM_COL32(230, 230, 235, 255);
		const ImU32 mutedColor   = IM_COL32(175, 175, 185, 255);
		for (auto *node : graphNodes)
		{
			if (!node || !nodeMinByName.contains(node->passName))
			{
				continue;
			}
			ImVec2 minPos = nodeMinByName.at(node->passName);
			ImVec2 maxPos = nodeMaxByName.at(node->passName);
			bool   hasMissingDependency = false;
			for (const auto &dependency : node->dependencies)
			{
				if (!nodeByName.contains(dependency))
				{
					hasMissingDependency = true;
					break;
				}
			}
			bool  selected = selectedNodeName == node->passName;
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
				ImGui::Text("Shader node: %s", node->shaderNodeRef ? node->shaderNodeRef->name.c_str() : "null");
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
			ShaderNode      *shaderNode   = selectedNode->shaderNodeRef;

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

		std::vector<std::string> queueNames;
		std::map<std::string, int> queueLaneByName;
		std::map<std::string, int> queueBatchCount;
		std::map<std::string, int> queueNodeCount;
		int totalNodeCount   = 0;
		int activeNodeCount  = 0;
		int emptyBatchCount  = 0;
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
		    IM_COL32(72, 113, 153, 255),
		    IM_COL32(92, 133, 96, 255),
		    IM_COL32(153, 117, 72, 255),
		    IM_COL32(126, 96, 153, 255),
		    IM_COL32(153, 82, 88, 255),
		    IM_COL32(74, 138, 132, 255),
		};
		const ImU32 laneFill      = IM_COL32(24, 24, 29, 255);
		const ImU32 laneBorder    = IM_COL32(72, 74, 84, 180);
		const ImU32 selectedColor = IM_COL32(110, 185, 230, 255);
		const ImU32 nodeFill      = IM_COL32(30, 35, 42, 255);
		const ImU32 inactiveFill  = IM_COL32(50, 43, 48, 255);
		const ImU32 emptyFill     = IM_COL32(48, 44, 37, 255);
		const ImU32 textColor     = IM_COL32(232, 232, 238, 255);
		const ImU32 mutedColor    = IM_COL32(175, 178, 188, 255);

		const float labelWidth  = 150.0f;
		const float laneHeight  = 94.0f;
		const float batchGap    = 18.0f;
		const float batchHeight = 62.0f;
		const float nodeHeight  = 20.0f;
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
			float y = origin.y + padding.y + lane * laneHeight;
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
					RenderGraphNode *node = batch.sortedNodes[nodeIdx];
					float chipWidth = std::min(96.0f, std::max(48.0f, (batchWidth - 18.0f) / static_cast<float>(batch.sortedNodes.size()) - 4.0f));
					ImVec2 chipMin(nodeX, minPos.y + 41.0f);
					ImVec2 chipMax(nodeX + chipWidth, chipMin.y + nodeHeight);
					drawList->AddRectFilled(chipMin, chipMax, node && node->active ? nodeFill : inactiveFill, 4.0f);
					drawList->AddRect(chipMin, chipMax, IM_COL32(120, 124, 138, 160), 4.0f);
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
		
		PaintingInfo();
		
		AnimatorInfo();

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
