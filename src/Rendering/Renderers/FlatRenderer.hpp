//

// Created by carlo on 2024-12-02.
//

#ifndef FLATRENDERER_HPP
#define FLATRENDERER_HPP

namespace Rendering
{
using namespace ENGINE;

class FlatRenderer : public BaseRenderer
{
  public:
	FlatRenderer(Core *core, WindowProvider *windowProvider)
	{
		this->core           = core;
		this->renderGraph    = core->renderGraphRef;
		this->windowProvider = windowProvider;
		CreateResources();
		CreateBuffers();
		CreatePipelines();
	}

	void CreateResources()
	{
		cascadesInfo.cascadeCount       = 5;
		cascadesInfo.probeSizePx        = 2;
		cascadesInfo.intervalCount      = 2;
		cascadesInfo.baseIntervalLength = 1;
		auto imageInfo                  = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
		                                                      ENGINE::g_32bFormat,
		                                                      vk::ImageUsageFlagBits::eColorAttachment |
		                                                          vk::ImageUsageFlagBits::eSampled);
		cascadesAttachmentsImagesViews.reserve(cascadesInfo.cascadeCount);
		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			std::string name      = "CascadeAttachment_" + std::to_string(i);
			ImageView  *imageView = ResourcesManager::GetInstance()->GetImage(name, imageInfo, 0, 0);
			cascadesAttachmentsImagesViews.emplace_back(imageView);
		}

		probesGenPc.cascadeIndex = 0;
		probesGenPc.intervalSize = 2;
		probesGenPc.probeSizePx  = cascadesInfo.probeSizePx;

		rcPc.probeSizePx        = cascadesInfo.probeSizePx;
		rcPc.intervalCount      = cascadesInfo.intervalCount;
		rcPc.baseIntervalLength = cascadesInfo.baseIntervalLength;

		paintingPc.radius = 20;

		auto       storageImageInfo = ENGINE::Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
		                                                          ENGINE::g_32bFormat,
		                                                          vk::ImageUsageFlagBits::eStorage |
		                                                              vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
		ImageView *lightLayer       = ResourcesManager::GetInstance()->GetImage("PaintingLayer", storageImageInfo, 0, 0);
		ImageView *occluderLayer    = ResourcesManager::GetInstance()->GetImage(
            "OccluderLayer", storageImageInfo, 0, 0);
		ImageView *debugLayer = ResourcesManager::GetInstance()->GetImage("DebugRaysLayer", storageImageInfo, 0, 0);
		paintingLayers.push_back(lightLayer);
		paintingLayers.push_back(occluderLayer);
		paintingLayers.push_back(debugLayer);

		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			std::string name      = "radianceStorage_" + std::to_string(i);
			ImageView  *imageView = ResourcesManager::GetInstance()->GetImage(name, storageImageInfo, 0, 0);
			radiancesImages.emplace_back(imageView);
		}

		std::string resourcesPath = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath();
		std::string assetPath     = SYSTEMS::OS::GetInstance()->GetAssetsPath();

		testImage = ResourcesManager::GetInstance()->GetShipper("TestImage",
		                                                        resourcesPath + "\\Images\\VulkanLogo.png", 1, 1,
		                                                        ENGINE::g_ShipperFormat,
		                                                        LayoutPatterns::GRAPHICS_READ);

		std::filesystem::path dirEntry(assetPath + "\\Textures\\RCascadesTextures");

		int i = 0;
		for (auto &pathView : std::filesystem::directory_iterator(dirEntry))
		{
			Material *backgroundMaterial = RenderingResManager::GetInstance()->PushMaterial(
			    "RCascadesMat_" + std::to_string(i++));
			std::string path   = pathView.path().string();
			ImageView  *colRef = ResourcesManager::GetInstance()->GetShipper(
                                                                   path + "\\Albedo.png", path + "\\Albedo.png", 1, 1, ENGINE::g_ShipperFormat,
                                                                   LayoutPatterns::GRAPHICS_READ)
			                        ->imageView.get();
			backgroundMaterial->SetTexture(TextureType::ALBEDO, colRef);

			ImageView *normRef = ResourcesManager::GetInstance()->GetShipper(
			                                                        path + "\\Normal.png", path + "\\Normal.png", 1, 1,
			                                                        ENGINE::g_ShipperFormat, LayoutPatterns::GRAPHICS_READ)
			                         ->imageView.get();
			backgroundMaterial->SetTexture(TextureType::NORMAL, normRef);

			ImageView *roughnessRef = ResourcesManager::GetInstance()->GetShipper(
			                                                             path + "\\Roughness.png", path + "\\Roughness.png", 1, 1,
			                                                             ENGINE::g_ShipperFormat, LayoutPatterns::GRAPHICS_READ)
			                              ->imageView.get();
			backgroundMaterial->SetTexture(TextureType::ROUGHNESS, roughnessRef);

			ImageView *aoRef = ResourcesManager::GetInstance()->GetShipper(
			                                                      path + "\\Ao.png", path + "\\Ao.png", 1, 1,
			                                                      ENGINE::g_ShipperFormat, LayoutPatterns::GRAPHICS_READ)
			                       ->imageView.get();
			backgroundMaterial->SetTexture(TextureType::AO, aoRef);

			ImageView *heightRef = ResourcesManager::GetInstance()->GetShipper(
			                                                          path + "\\Height.png", path + "\\Height.png", 1, 1,
			                                                          ENGINE::g_ShipperFormat, LayoutPatterns::GRAPHICS_READ)
			                           ->imageView.get();
			backgroundMaterial->SetTexture(TextureType::HEIGHT, heightRef);
			backgroundMaterials.emplace_back(backgroundMaterial);
		}
		materialIndexSelected = 0;

		// testSpriteAnim.LoadAtlas(
		// assetPath + "\\Animations\\SmokeFreePack_v2\\Compressed\\512\\Smoke_4_512-sheet.png",
		// glm::uvec2(512, 512), 7, 7, 10);

		AnimatorInfo animatorInfo = {glm::uvec2(512, 512), 7, 7, -1, -1, -1, true};
		testSpriteAnim            = RenderingResManager::GetInstance()->GetAnimator("TestAnim",
		                                                                            assetPath +
		                                                                                "\\Animations\\SmokeFreePack_v2\\Compressed\\512\\Smoke_4_512-sheet.png",
		                                                                            30, animatorInfo);
	}

	void CreateBuffers()
	{
		quadVertBufferRef = ResourcesManager::GetInstance()->GetStageBuffer(
		                                                       "QuadRcVertices", vk::BufferUsageFlagBits::eVertexBuffer,
		                                                       sizeof(Vertex2D) * Vertex2D::GetQuadVertices().size(),
		                                                       Vertex2D::GetQuadVertices().data())
		                        ->deviceBuffer.get();
		quadIndexBufferRef = ResourcesManager::GetInstance()->GetStageBuffer(
		                                                        "QuadRcIndices", vk::BufferUsageFlagBits::eIndexBuffer,
		                                                        sizeof(uint32_t) * Vertex2D::GetQuadIndices().size(),
		                                                        Vertex2D::GetQuadIndices().data())
		                         ->deviceBuffer.get();
	}

	void CreatePipelines()
	{
		auto        logicalDevice = core->logicalDevice.get();
		std::string shaderPath    = SYSTEMS::OS::GetInstance()->GetShadersPath();

		paintCompShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\slang\\test\\paintingGen.slang", S_COMP);
		// auto *paintingNode = renderGraph->AddPass(paintingPassName, renderGraph->core->queueWorkerManager.get()->GetOrCreateWorkerQueue("Compute"));
		auto *paintingNode = renderGraph->AddPass(paintingPassName);
		paintingNode->SetCompShader(paintCompShader);
		// paintingNode->SetPipelineLayoutCI(paintingLayoutCreateInfo);
		paintingNode->SetPushConstantSize(sizeof(PaintingPc));
		paintingNode->AddStorageResource("PaintingStorage", paintingLayers[0]);
		paintingNode->AddStorageResource("OcluddersStorage", paintingLayers[1]);
		paintingNode->AddStorageResource("DebugLayer", paintingLayers[2]);
		paintingNode->BuildRenderGraphNode();

		VertexInput    vertexInput = Vertex2D::GetVertexInput();
		AttachmentInfo colInfo     = GetColorAttachmentInfo(
            glm::vec4(0.0f), vk::Format::eR32G32B32A32Sfloat);

		probesVertShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\Common\\Quad.vert.spv", S_VERT);
		probesFragShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\FlatRendering\\cascadeGen.frag.spv",
		    S_FRAG);

		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			std::string name = "ProbesGen_" + std::to_string(i);
			probesGenPassNames.push_back(name);
			auto renderNode = renderGraph->AddPass(name);
			renderNode->SetConfigs({true});
			renderNode->SetPushConstantSize(sizeof(PaintingPc));
			renderNode->SetVertShader(probesVertShader);
			renderNode->SetFragShader(probesFragShader);
			renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
			// renderNode->SetPipelineLayoutCI(genLayoutCreateInfo);
			renderNode->SetVertexInput(vertexInput);
			renderNode->AddColorAttachmentOutput("CascadeAttachment_" + std::to_string(i), colInfo, BlendConfigs::B_OPAQUE);
			renderNode->AddColorImageResource("CascadeAttachment_" + std::to_string(i), cascadesAttachmentsImagesViews[i]);
			renderNode->DependsOn(paintingPassName);
			renderNode->BuildRenderGraphNode();
		}

		vertShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\Common\\Quad.vert.spv", S_VERT);
		fragShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\FlatRendering\\rCascadesOutput.frag.spv",
		    S_FRAG);

		AttachmentInfo outputColInfo = GetColorAttachmentInfo(
		    glm::vec4(0.0f), core->swapchainRef->GetFormat());

		auto renderNode = renderGraph->AddPass(rCascadesPassName);
		renderNode->SetVertShader(vertShader);
		renderNode->SetFragShader(fragShader);
		renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
		renderNode->SetPushConstantSize(sizeof(RcPc));
		renderNode->SetVertexInput(vertexInput);
		renderNode->AddColorAttachmentOutput("rColor", outputColInfo, BlendConfigs::B_ALPHA_BLEND);
		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			renderNode->DependsOn("ProbesGen_" + std::to_string(i));
		}
		renderNode->BuildRenderGraphNode();

		AttachmentInfo mergeColInfo = GetColorAttachmentInfo(
		    glm::vec4(0.0f), core->swapchainRef->GetFormat(), vk::AttachmentLoadOp::eLoad,
		    vk::AttachmentStoreOp::eStore);

		mergeVertShader = renderGraph->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\Common\\Quad.vert.spv", S_VERT);
		mergeFragShader = renderGraph->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\FlatRendering\\cascadesMerge.frag.spv", S_FRAG);

		for (int i = cascadesInfo.cascadeCount - 2; i >= 0; i--)
		{
			std::string name            = rMergePassName + "_" + std::to_string(i);
			auto        mergeRenderNode = renderGraph->AddPass(name);
			mergeRenderNode->SetVertShader(mergeVertShader);
			mergeRenderNode->SetFragShader(mergeFragShader);
			mergeRenderNode->SetPushConstantSize(sizeof(RcPc));
			mergeRenderNode->SetFramebufferSize(windowProvider->GetWindowSize());
			mergeRenderNode->SetVertexInput(vertexInput);
			mergeRenderNode->AddColorAttachmentOutput("mergeColor_" + std::to_string(i), mergeColInfo, BlendConfigs::B_OPAQUE);
			std::string name1 = "radianceStorage_" + std::to_string(i);
			mergeRenderNode->AddStorageResource(name1, radiancesImages[i]);
			std::string name2 = "radianceStorage_" + std::to_string(i + 1);
			mergeRenderNode->AddStorageResource(name2, radiancesImages[i + 1]);
			mergeRenderNode->DependsOn(rCascadesPassName);
			mergeRenderNode->BuildRenderGraphNode();
			if (i < cascadesInfo.cascadeCount - 2)
			{
				std::string dependancyName = rMergePassName + "_" + std::to_string(i + 1);
			}
		}

		resultVertShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\Common\\Quad.vert.spv", S_VERT);
		resultFragShader = renderGraph->resourcesManager->GetShader(
		    shaderPath +
		        "\\spirvGlsl\\FlatRendering\\cascadesResult.frag.spv",
		    S_FRAG);
		auto resultNode = renderGraph->AddPass(resultPassName);
		resultNode->SetVertShader(resultVertShader);
		resultNode->SetFragShader(resultFragShader);
		resultNode->SetFramebufferSize(windowProvider->GetWindowSize());
		resultNode->SetPushConstantSize(sizeof(RcPc));
		resultNode->SetVertexInput(vertexInput);
		resultNode->DependsOn(rMergePassName + "_" + std::to_string(0));
		resultNode->AddColorAttachmentOutput("resultColor", outputColInfo, BlendConfigs::B_OPAQUE);
		resultNode->BuildRenderGraphNode();
	}

	void RecreateSwapChainResources() override
	{
	}

	void SetRenderOperation() override
	{
		auto paintingRenderOP = new std::function<void()>(
		    [this]() {
			    glm::vec2 mouseInput = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
			    paintingPc.xMousePos = mouseInput.x;
			    paintingPc.yMousePos = mouseInput.y;
			    if (glfwGetMouseButton(windowProvider->window, GLFW_MOUSE_BUTTON_2))
			    {
				    paintingPc.painting = 1;
			    }
			    else
			    {
				    paintingPc.painting = 0;
			    }

			    auto &renderNode = renderGraph->renderNodes.at(paintingPassName);
			    renderNode->descCache->SetStorageImageArray("PaintingLayers", paintingLayers);
			    renderGraph->currentFrameResources->commandBuffer->pushConstants(renderNode->pipelineLayout.get(),
			                                                                     vk::ShaderStageFlagBits::eCompute,
			                                                                     0, sizeof(PaintingPc), &paintingPc);
			    renderGraph->currentFrameResources->commandBuffer->dispatch(paintingPc.radius, paintingPc.radius, 1);
		    });
		renderGraph->GetNode(paintingPassName)->SetRenderOperation(paintingRenderOP);

		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			auto probesGenOp = new std::function<void()>(
			    [this, i]() {
				    int idx            = i;
				    int intervalSizePc = cascadesInfo.intervalCount;
				    int gridSizePc     = cascadesInfo.probeSizePx;
				    for (int j = 0; j < idx; ++j)
				    {
					    intervalSizePc *= 2;
					    gridSizePc *= 2;
				    }
				    probesGenPc.cascadeIndex = idx;
				    probesGenPc.intervalSize = intervalSizePc;
				    probesGenPc.probeSizePx  = gridSizePc;
				    auto &renderNode         = renderGraph->renderNodes.at(probesGenPassNames[idx]);

				    renderGraph->currentFrameResources->commandBuffer->pushConstants(renderNode->pipelineLayout.get(),
				                                                                     vk::ShaderStageFlagBits::eVertex |
				                                                                         vk::ShaderStageFlagBits::eFragment,
				                                                                     0, sizeof(ProbesGenPc), &probesGenPc);
				    vk::DeviceSize offset = 0;
				    renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(0, 1, &quadVertBufferRef->bufferHandle.get(), &offset);
				    renderGraph->currentFrameResources->commandBuffer->bindIndexBuffer(quadIndexBufferRef->bufferHandle.get(), 0,
				                                                                       vk::IndexType::eUint32);
				    renderGraph->currentFrameResources->commandBuffer->drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0, 0, 0);
			    });
			renderGraph->GetNode(probesGenPassNames[i])->SetRenderOperation(probesGenOp);
		}

		auto radianceOutputTask = new std::function<void()>([this]() {
			rcPc.cascadesCount      = cascadesInfo.cascadeCount;
			rcPc.probeSizePx        = cascadesInfo.probeSizePx;
			rcPc.intervalCount      = cascadesInfo.intervalCount;
			rcPc.baseIntervalLength = cascadesInfo.baseIntervalLength;
			rcPc.fWidth             = windowProvider->GetWindowSize().x;
			rcPc.fHeight            = windowProvider->GetWindowSize().y;

			auto *currImage = renderGraph->currentBackBuffer;
			renderGraph->AddColorImageResource(rCascadesPassName, "rColor", currImage);
			renderGraph->GetNode(rCascadesPassName)->SetFramebufferSize(windowProvider->GetWindowSize());
		});
		auto radianceOutputOp   = new std::function<void()>(
            [this]() {
                auto &renderNode = renderGraph->renderNodes.at(rCascadesPassName);
                renderNode->SetSamplerArray("Cascades", cascadesAttachmentsImagesViews);
                renderNode->SetStorageImageArray("PaintingLayers", paintingLayers);
                renderNode->SetStorageImageArray("Radiances", radiancesImages);
                renderNode->SetSampler("TestImage", testImage->imageView.get());
                renderNode->SetSamplerArray("SpriteAnims", testSpriteAnim->imagesFrames);
                renderNode->SetBuffer("SpriteInfo", testSpriteAnim->animatorInfo);
                renderNode->SetSamplerArray("MatTextures",
			                                  backgroundMaterials.at(materialIndexSelected)->ConvertTexturesToVec());

                vk::DeviceSize offset = 0;
                renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(0, 1, &quadVertBufferRef->bufferHandle.get(), &offset);
                renderGraph->currentFrameResources->commandBuffer->bindIndexBuffer(quadIndexBufferRef->bufferHandle.get(), 0, vk::IndexType::eUint32);

                renderGraph->currentFrameResources->commandBuffer->pushConstants(renderNode->pipelineLayout.get(),
			                                                                       vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			                                                                       0, sizeof(RcPc), &rcPc);

                renderGraph->currentFrameResources->commandBuffer->drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0,
			                                                                     0, 0);
            });
		renderGraph->GetNode(rCascadesPassName)->SetRenderOperation(radianceOutputOp);
		renderGraph->GetNode(rCascadesPassName)->AddTask(radianceOutputTask);

		for (int i = cascadesInfo.cascadeCount - 2; i >= 0; i--)
		{
			std::string mergeNameCascades = rMergePassName + "_" + std::to_string(i);
			auto        mergeTask         = new std::function<void()>([this, i]() {
                int         idx       = i;
                std::string mergeName = rMergePassName + "_" + std::to_string(idx);
                auto       *currImage = renderGraph->currentBackBuffer;
                renderGraph->GetNode(mergeName)->AddColorImageResource(
                    "mergeColor_" + std::to_string(idx), currImage);
                renderGraph->GetNode(mergeName)->SetFramebufferSize(windowProvider->GetWindowSize());
            });
			auto        mergeRenderOp     = new std::function<void()>(
                [this, i]() {
                    int         idx        = i;
                    std::string mergeName  = rMergePassName + "_" + std::to_string(idx);
                    auto       &renderNode = renderGraph->renderNodes.at(mergeName);
                    renderNode->SetSamplerArray("Cascades", cascadesAttachmentsImagesViews);
                    renderNode->SetStorageImageArray("Radiances", radiancesImages);

                    rcPc.cascadeIndex = idx;

                    vk::DeviceSize offset = 0;
                    renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(0, 1, &quadVertBufferRef->bufferHandle.get(), &offset);
                    renderGraph->currentFrameResources->commandBuffer->bindIndexBuffer(quadIndexBufferRef->bufferHandle.get(), 0,
				                                                                                  vk::IndexType::eUint32);

                    renderGraph->currentFrameResources->commandBuffer->pushConstants(renderNode->pipelineLayout.get(),
				                                                                                vk::ShaderStageFlagBits::eVertex |
				                                                                                    vk::ShaderStageFlagBits::eFragment,
				                                                                                0, sizeof(RcPc), &rcPc);

                    renderGraph->currentFrameResources->commandBuffer->drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0,
				                                                                              0, 0);
                });
			renderGraph->GetNode(mergeNameCascades)->SetRenderOperation(mergeRenderOp);
			renderGraph->GetNode(mergeNameCascades)->AddTask(mergeTask);
		}

		auto resultTask     = new std::function<void()>([this]() {
            auto *currImage = renderGraph->currentBackBuffer;
            renderGraph->GetNode(resultPassName)->AddColorImageResource("resultColor", currImage);
            renderGraph->GetNode(resultPassName)->SetFramebufferSize(windowProvider->GetWindowSize());
        });
		auto resultRenderOp = new std::function<void()>(
		    [this]() {
			    auto &renderNode = renderGraph->renderNodes.at(resultPassName);
			    renderNode->SetStorageImageArray("PaintingLayers", paintingLayers);
			    renderNode->SetStorageImageArray("Radiances", radiancesImages);
			    renderNode->SetSampler("TestImage", testImage->imageView.get());
			    renderNode->SetSamplerArray("SpriteAnims", testSpriteAnim->imagesFrames);
			    renderNode->SetBuffer("SpriteInfo", testSpriteAnim->animatorInfo);
			    renderNode->SetSamplerArray("MatTextures",
			                                backgroundMaterials.at(materialIndexSelected)->ConvertTexturesToVec());
			    renderNode->SetBuffer("LightInfo", light);
			    renderNode->SetBuffer("RConfigs", rConfigs);

			    vk::DeviceSize offset = 0;
			    renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(0, 1, &quadVertBufferRef->bufferHandle.get(), &offset);
			    renderGraph->currentFrameResources->commandBuffer->bindIndexBuffer(quadIndexBufferRef->bufferHandle.get(), 0, vk::IndexType::eUint32);

			    renderGraph->currentFrameResources->commandBuffer->pushConstants(renderNode->pipelineLayout.get(),
			                                                                     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			                                                                     0, sizeof(RcPc), &rcPc);

			    renderGraph->currentFrameResources->commandBuffer->drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0,
			                                                                   0, 0);
			    testSpriteAnim->UseFrame();
		    });
		renderGraph->GetNode(resultPassName)->SetRenderOperation(resultRenderOp);
		renderGraph->GetNode(resultPassName)->AddTask(resultTask);
	}

	void ReloadShaders() override
	{
		auto *paintingNode = renderGraph->GetNode(paintingPassName);
		paintingNode->RecreateResources();
		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			auto *genNode = renderGraph->GetNode(probesGenPassNames[i]);
			genNode->RecreateResources();
		}
		auto *outputNode = renderGraph->GetNode(rCascadesPassName);
		outputNode->RecreateResources();
		for (int i = cascadesInfo.cascadeCount - 2; i >= 0; i--)
		{
			std::string name      = rMergePassName + "_" + std::to_string(i);
			auto       *mergeNode = renderGraph->GetNode(name);
			mergeNode->RecreateResources();
		}
		auto *resultNode = renderGraph->GetNode(resultPassName);
		resultNode->RecreateResources();
	}

	Core           *core;
	RenderGraph    *renderGraph;
	WindowProvider *windowProvider;

	std::string paintingPassName  = "PaintingPass";
	std::string rCascadesPassName = "rCascadesPass";
	std::string rMergePassName    = "rMergePass";
	std::string resultPassName    = "resultPass";

	Shader *resultVertShader;
	Shader *resultFragShader;

	Shader                  *mergeVertShader;
	Shader                  *mergeFragShader;
	std::vector<ImageView *> radiancesImages;

	Shader *vertShader;
	Shader *fragShader;

	std::vector<std::string> probesGenPassNames;
	Shader                  *probesVertShader;
	Shader                  *probesFragShader;
	std::vector<ImageView *> cascadesAttachmentsImagesViews;

	Shader                  *paintCompShader;
	std::vector<ImageView *> paintingLayers;
	std::vector<Material *>  backgroundMaterials;
	int                      materialIndexSelected = 0;
	ImageShipper            *testImage;

	Buffer *quadVertBufferRef;
	Buffer *quadIndexBufferRef;

	Animator2D *testSpriteAnim;

	DirectionalLight        light{glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0, 0.0, 1.0), 0.01f};
	PaintingPc              paintingPc;
	RcPc                    rcPc;
	RadianceCascadesConfigs rConfigs{};
	ProbesGenPc             probesGenPc;
	CascadesInfo            cascadesInfo;
};
}        // namespace Rendering
#endif        // FLATRENDERER_HPP
