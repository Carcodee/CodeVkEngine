//

// Created by carlo on 2024-12-02.
//

#ifndef FLATRENDERER_HPP
#define FLATRENDERER_HPP
#include "CodeCuda.cuh"
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
		auto imageInfo                  = Image::CreateInfo2d(glm::uvec2(rcResolutionW, rcResolutionH), 1, 1,
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

		auto       storageImageInfo = ENGINE::Image::CreateInfo2d(glm::uvec2(rcResolutionW, rcResolutionH), 1, 1,
		                                                          ENGINE::g_32bFormat,
		                                                          vk::ImageUsageFlagBits::eStorage |
		                                                              vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
		ImageView *lightLayer       = ResourcesManager::GetInstance()->GetImage("PaintingLayer", storageImageInfo, 0, 0);
		ImageView *occluderLayer    = ResourcesManager::GetInstance()->GetImage(
            "OccluderLayer", storageImageInfo, 0, 0);
		ImageView *debugLayer        = ResourcesManager::GetInstance()->GetImage("DebugRaysLayer", storageImageInfo, 0, 0);
		ImageView *fluidSimLayer     = ResourcesManager::GetInstance()->GetImage("FluidSimLayer", storageImageInfo, 0, 0);
		ImageView *fluidSimLayerInfo = ResourcesManager::GetInstance()->GetImage("FluidSimInfoLayer", storageImageInfo, 0, 0);
		paintingLayers.push_back(lightLayer);
		paintingLayers.push_back(occluderLayer);
		paintingLayers.push_back(debugLayer);
		paintingLayers.push_back(fluidSimLayer);
		paintingLayers.push_back(fluidSimLayerInfo);

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
		                                                       sizeof(Vertex2D) * Vertex2D::GetQuadVertices().size(), sizeof(Vertex2D),
		                                                       Vertex2D::GetQuadVertices().data())
		                        ->deviceBuffer.get();
		quadIndexBufferRef = ResourcesManager::GetInstance()->GetStageBuffer(
		                                                        "QuadRcIndices", vk::BufferUsageFlagBits::eIndexBuffer,
		                                                        sizeof(uint32_t) * Vertex2D::GetQuadIndices().size(), sizeof(uint32_t),
		                                                        Vertex2D::GetQuadIndices().data())
		                         ->deviceBuffer.get();
	}

	void CreatePipelines()
	{
		auto        logicalDevice = core->logicalDevice.get();
		std::string shaderPath    = SYSTEMS::OS::GetInstance()->GetShadersPath();

		auto cudaBuffer = renderGraph->resourcesManager->GetBuffer(ENGINE::ResourcesManager::BufferParams{
		    "CudaBuffer", vk::BufferUsageFlagBits::eStorageBuffer, {}, sizeof(float) * 4 * rcResolutionW * rcResolutionH, sizeof(float) * 4, nullptr, ENGINE::ResourcesManager::BufferType::EXTERNAL});

		auto cudaBufferVel = renderGraph->resourcesManager->GetBuffer(ENGINE::ResourcesManager::BufferParams{
		    "CudaBufferVel", vk::BufferUsageFlagBits::eStorageBuffer, {}, sizeof(float) * rcResolutionW * rcResolutionH, sizeof(float), nullptr, ENGINE::ResourcesManager::BufferType::EXTERNAL});

		auto cudaPI = renderGraph->AddCUDAPipeline("CudaCopySmoke");
		cudaPI
		    ->C_ExportBuffer(cudaBuffer)
		    ->C_SetKernelFunction(*CodeCuda::FluidSimulation::C_GetKernelLauncherMappers(0).GetKernelFunct())
		    ->C_BuildCUDAPipeline();
		auto cudaNode = renderGraph->AddCudaPass(cudaPI, "CudaNode");

		auto cudaPI_Vel = renderGraph->AddCUDAPipeline("CudaCopyVel");
		cudaPI_Vel
		    ->C_ExportBuffer(cudaBufferVel)
		    ->C_SetKernelFunction(*CodeCuda::FluidSimulation::C_GetKernelLauncherMappers(1).GetKernelFunct())
		    ->C_BuildCUDAPipeline();

		auto cudaVel = renderGraph->AddCudaPass(cudaPI_Vel, "CudaVelNode");

		//

		paintCompShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\slang\\test\\paintingGen.slang", S_COMP);
		auto *paintingGPUPipeline = renderGraph->AddGPUPipeline("PaintingCompute");
		paintingGPUPipeline->G_SetCompShader(paintCompShader);
		paintingGPUPipeline->G_SetPushConstantSize(sizeof(PaintingPc));
		paintingGPUPipeline->G_BuildGPUPipeline();

		auto *paintingNode = renderGraph->AddPass(paintingGPUPipeline, paintingPassName, "Graphics");
		paintingNode->G_AddStorageResource(paintingLayers[0]);
		paintingNode->G_AddStorageResource(paintingLayers[1]);
		paintingNode->G_AddStorageResource(paintingLayers[2]);

		auto cudaBufferImports = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\slang\\test\\cudaBufferToImage.slang", S_COMP);

		auto *importBufferNode = renderGraph->AddPass("CudaBufferImporterNode");
		importBufferNode->G_SetCompShader(cudaBufferImports);
		importBufferNode->G_AddStorageResource(paintingLayers[3]);
		importBufferNode->G_AddStorageResource(paintingLayers[4]);
		importBufferNode->DependsOn(cudaNode);
		importBufferNode->DependsOn(cudaVel);
		importBufferNode->BuildRenderGraphNode();

		VertexInput    vertexInput = Vertex2D::GetVertexInput();
		AttachmentInfo colInfo     = GetColorAttachmentInfo(BlendConfigs::B_OPAQUE,
            glm::vec4(0.0f), vk::Format::eR32G32B32A32Sfloat);

		probesVertShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\Common\\Quad.vert.spv", S_VERT);
		probesFragShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\FlatRendering\\cascadeGen.frag.spv",
		    S_FRAG);

		auto probesGenGPUPipeline = renderGraph->AddGPUPipeline("ProbesGenShader");
		probesGenGPUPipeline->G_SetConfigs({true});
		probesGenGPUPipeline->G_SetPushConstantSize(sizeof(PaintingPc));
		probesGenGPUPipeline->G_SetVertShader(probesVertShader);
		probesGenGPUPipeline->G_SetFragShader(probesFragShader);
		probesGenGPUPipeline->G_SetVertexInput(vertexInput);
		probesGenGPUPipeline->G_AddColorAttachmentOutput(0, colInfo);
		probesGenGPUPipeline->G_BuildGPUPipeline();

		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			std::string name = "ProbesGen_" + std::to_string(i);
			probesGenPassNames.push_back(name);
			auto renderNode = renderGraph->AddPass(probesGenGPUPipeline, name, "Graphics_Test");
			renderNode->G_SetFramebufferSize(glm::uvec2(rcResolutionW, rcResolutionH));
			renderNode->G_SetColorImageAttachmentBinding(0, cascadesAttachmentsImagesViews[i]);
			renderNode->DependsOn(paintingPassName);
		}

		vertShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\Common\\Quad.vert.spv", S_VERT);
		fragShader = renderGraph->resourcesManager->GetShader(
		    shaderPath + "\\spirvGlsl\\FlatRendering\\rCascadesOutput.frag.spv",
		    S_FRAG);

		AttachmentInfo outputColInfo = GetColorAttachmentInfo(BlendConfigs::B_ALPHA_BLEND,
		    glm::vec4(0.0f), core->swapchainRef->GetFormat());

		auto cascadesGPUPipeline = renderGraph->AddGPUPipeline("CascadesGPUPipeline");
		cascadesGPUPipeline->G_SetVertShader(vertShader);
		cascadesGPUPipeline->G_SetFragShader(fragShader);
		cascadesGPUPipeline->G_SetPushConstantSize(sizeof(RcPc));
		cascadesGPUPipeline->G_SetVertexInput(vertexInput);
		cascadesGPUPipeline->G_AddColorAttachmentOutput(0, outputColInfo);
		cascadesGPUPipeline->G_BuildGPUPipeline();

		auto renderNode = renderGraph->AddPass(cascadesGPUPipeline, rCascadesPassName);
		renderNode->G_SetFramebufferSize(glm::uvec2(rcResolutionW, rcResolutionH));
		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			renderNode->DependsOn("ProbesGen_" + std::to_string(i));
		}

		AttachmentInfo mergeColInfo = GetColorAttachmentInfo( BlendConfigs::B_OPAQUE,
		    glm::vec4(0.0f), core->swapchainRef->GetFormat(), vk::AttachmentLoadOp::eLoad,
		    vk::AttachmentStoreOp::eStore);

		mergeVertShader = renderGraph->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\Common\\Quad.vert.spv", S_VERT);
		mergeFragShader = renderGraph->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\FlatRendering\\cascadesMerge.frag.spv", S_FRAG);

		auto shaderNode = renderGraph->AddGPUPipeline("MergeShader");
		shaderNode->G_SetVertShader(mergeVertShader);
		shaderNode->G_SetFragShader(mergeFragShader);
		shaderNode->G_SetPushConstantSize(sizeof(RcPc));
		shaderNode->G_SetVertexInput(vertexInput);
		shaderNode->G_AddColorAttachmentOutput(0, mergeColInfo);
		shaderNode->G_BuildGPUPipeline();

		for (int i = cascadesInfo.cascadeCount - 2; i >= 0; i--)
		{
			std::string name            = rMergePassName + "_" + std::to_string(i);
			auto        mergeRenderNode = renderGraph->AddPass(shaderNode, name, "Graphics");
			mergeRenderNode->G_SetFramebufferSize(glm::uvec2(rcResolutionW, rcResolutionH));
			mergeRenderNode->G_AddStorageResource(radiancesImages[i]);
			mergeRenderNode->G_AddStorageResource(radiancesImages[i + 1]);
			mergeRenderNode->DependsOn(rCascadesPassName);
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

		
		AttachmentInfo resultColInfo = GetColorAttachmentInfo(BlendConfigs::B_OPAQUE,
			glm::vec4(0.0f), core->swapchainRef->GetFormat());
		auto resultGPUPipeline = renderGraph->AddGPUPipeline("ResultCascadesShader");
		resultGPUPipeline->G_SetVertShader(resultVertShader);
		resultGPUPipeline->G_SetFragShader(resultFragShader);
		resultGPUPipeline->G_SetVertexInput(Vertex2D::GetVertexInput());
		resultGPUPipeline->G_SetPushConstantSize(sizeof(RcPc));
		resultGPUPipeline->G_AddColorAttachmentOutput(0, resultColInfo);

		auto resultNode = renderGraph->AddPass(resultGPUPipeline, resultPassName, "Graphics");
		resultNode->G_SetFramebufferSize(glm::uvec2(rcResolutionW, rcResolutionH));
		resultNode->DependsOn(rMergePassName + "_" + std::to_string(0));
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
			    // if (glfwGetMouseButton(windowProvider->window, GLFW_MOUSE_BUTTON_2))
			    // {
			    //  paintingPc.painting = 1;
			    // }
			    // else
			    // {
			    //  paintingPc.painting = 0;
			    // }

			    auto &renderNode = renderGraph->renderNodes.at(paintingPassName);
			    renderNode->G_SetStorageImageArray("PaintingLayers", paintingLayers);
			    renderNode->GetCurrCmd().pushConstants(renderNode->GPUPipelineRef->pipelineLayout.get(),
			                                           vk::ShaderStageFlagBits::eCompute,
			                                           0, sizeof(PaintingPc), &paintingPc);
			    renderNode->GetCurrCmd().dispatch(paintingPc.radius, paintingPc.radius, 1);
		    });

		renderGraph->GetNode(paintingPassName)->G_SetRenderOperation(paintingRenderOP);

		auto importCudaBufferNodeOp = new std::function<void()>(
		    [this]() {
			    auto &renderNode = renderGraph->renderNodes.at("CudaBufferImporterNode");
			    renderNode->G_SetStorageImage("OutImage", paintingLayers[3]);
			    renderNode->G_SetStorageImage("OutImageSimulationInfo", paintingLayers[4]);
			    renderNode->G_SetBuffer("SimulationBuffer", renderGraph->resourcesManager->GetBuffFromName("CudaBuffer"));
			    renderNode->G_SetBuffer("SimulationBufferInfo", renderGraph->resourcesManager->GetBuffFromName("CudaBufferVel"));

			    renderNode->GetCurrCmd().dispatch((rcResolutionW + rcResolutionW - 1) / 8, (rcResolutionH + rcResolutionH - 1) / 8, 1);
		    });

		renderGraph->GetNode("CudaBufferImporterNode")->G_SetRenderOperation(importCudaBufferNodeOp);

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

				    renderNode->GetCurrCmd().pushConstants(renderNode->GPUPipelineRef->pipelineLayout.get(),
				                                           vk::ShaderStageFlagBits::eVertex |
				                                               vk::ShaderStageFlagBits::eFragment,
				                                           0, sizeof(ProbesGenPc), &probesGenPc);
				    vk::DeviceSize offset = 0;
				    renderNode->GetCurrCmd().bindVertexBuffers(0, 1, &quadVertBufferRef->bufferHandle.get(), &offset);
				    renderNode->GetCurrCmd().bindIndexBuffer(quadIndexBufferRef->bufferHandle.get(), 0,
				                                             vk::IndexType::eUint32);
				    renderNode->GetCurrCmd().drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0, 0, 0);
			    });
			renderGraph->GetNode(probesGenPassNames[i])->G_SetRenderOperation(probesGenOp);
		}

		auto radianceOutputTask = new std::function<void()>([this]() {
			rcPc.cascadesCount      = cascadesInfo.cascadeCount;
			rcPc.probeSizePx        = cascadesInfo.probeSizePx;
			rcPc.intervalCount      = cascadesInfo.intervalCount;
			rcPc.baseIntervalLength = cascadesInfo.baseIntervalLength;
			rcPc.fWidth             = rcResolutionW;
			rcPc.fHeight            = rcResolutionH;

			auto *currImage = renderGraph->currentBackBuffer;
			auto &renderNode = renderGraph->renderNodes.at(rCascadesPassName);
			renderNode->G_SetColorImageAttachmentBinding(0, currImage);
			renderGraph->GetNode(rCascadesPassName)->G_SetFramebufferSize(glm::uvec2(rcResolutionW, rcResolutionH));
		});
		auto radianceOutputOp   = new std::function<void()>(
            [this]() {
                auto &renderNode = renderGraph->renderNodes.at(rCascadesPassName);
                renderNode->G_SetSamplerArray("Cascades", cascadesAttachmentsImagesViews);
                renderNode->G_SetStorageImageArray("PaintingLayers", paintingLayers);
                renderNode->G_SetStorageImageArray("Radiances", radiancesImages);
                renderNode->G_SetSampler("TestImage", testImage->imageView.get());
                renderNode->G_SetSamplerArray("SpriteAnims", testSpriteAnim->imagesFrames);
                renderNode->G_SetBuffer("SpriteInfo", testSpriteAnim->animatorInfo);
                renderNode->G_SetSamplerArray("MatTextures",
			                                    backgroundMaterials.at(materialIndexSelected)->ConvertTexturesToVec());

                vk::DeviceSize offset = 0;
                renderNode->GetCurrCmd().bindVertexBuffers(0, 1, &quadVertBufferRef->bufferHandle.get(), &offset);
                renderNode->GetCurrCmd().bindIndexBuffer(quadIndexBufferRef->bufferHandle.get(), 0, vk::IndexType::eUint32);

                renderNode->GetCurrCmd().pushConstants(renderNode->GPUPipelineRef->pipelineLayout.get(),
			                                             vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			                                             0, sizeof(RcPc), &rcPc);

                renderNode->GetCurrCmd().drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0,
			                                           0, 0);
            });
		renderGraph->GetNode(rCascadesPassName)->G_SetRenderOperation(radianceOutputOp);
		renderGraph->GetNode(rCascadesPassName)->AddPreRenderingTask(radianceOutputTask);

		for (int i = cascadesInfo.cascadeCount - 2; i >= 0; i--)
		{
			std::string mergeNameCascades = rMergePassName + "_" + std::to_string(i);
			auto        mergeTask         = new std::function<void()>([this, i]() {
                int         idx       = i;
                std::string mergeName = rMergePassName + "_" + std::to_string(idx);
                auto       *currImage = renderGraph->currentBackBuffer;
                renderGraph->GetNode(mergeName)->G_SetColorImageAttachmentBinding(0, currImage);
                renderGraph->GetNode(mergeName)->G_SetFramebufferSize(glm::uvec2(rcResolutionW, rcResolutionH));
            });
			auto        mergeRenderOp     = new std::function<void()>(
                [this, i]() {
                    int         idx        = i;
                    std::string mergeName  = rMergePassName + "_" + std::to_string(idx);
                    auto       &renderNode = renderGraph->renderNodes.at(mergeName);
                    renderNode->G_SetSamplerArray("Cascades", cascadesAttachmentsImagesViews);
                    renderNode->G_SetStorageImageArray("Radiances", radiancesImages);

                    rcPc.cascadeIndex = idx;

                    vk::DeviceSize offset = 0;
                    renderNode->GetCurrCmd().bindVertexBuffers(0, 1, &quadVertBufferRef->bufferHandle.get(), &offset);
                    renderNode->GetCurrCmd().bindIndexBuffer(quadIndexBufferRef->bufferHandle.get(), 0,
				                                                        vk::IndexType::eUint32);

                    renderNode->GetCurrCmd().pushConstants(renderNode->GPUPipelineRef->pipelineLayout.get(),
				                                                      vk::ShaderStageFlagBits::eVertex |
				                                                          vk::ShaderStageFlagBits::eFragment,
				                                                      0, sizeof(RcPc), &rcPc);
                    renderNode->GetCurrCmd().drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0,
				                                                    0, 0);
                });
			renderGraph->GetNode(mergeNameCascades)->G_SetRenderOperation(mergeRenderOp);
			renderGraph->GetNode(mergeNameCascades)->AddPreRenderingTask(mergeTask);
		}

		auto resultTask     = new std::function<void()>([this]() {
            auto *currImage = renderGraph->currentBackBuffer;
            renderGraph->GetNode(resultPassName)->G_SetColorImageAttachmentBinding(0, currImage);
            renderGraph->GetNode(resultPassName)->G_SetFramebufferSize(glm::uvec2(rcResolutionW, rcResolutionH));
        });
		auto resultRenderOp = new std::function<void()>(
		    [this]() {
			    auto &renderNode = renderGraph->renderNodes.at(resultPassName);
			    renderNode->G_SetStorageImageArray("PaintingLayers", paintingLayers);
			    renderNode->G_SetStorageImageArray("Radiances", radiancesImages);
			    renderNode->G_SetSampler("TestImage", testImage->imageView.get());
			    renderNode->G_SetSamplerArray("SpriteAnims", testSpriteAnim->imagesFrames);
			    renderNode->G_SetBuffer("SpriteInfo", testSpriteAnim->animatorInfo);
			    renderNode->G_SetSamplerArray("MatTextures",
			                                  backgroundMaterials.at(materialIndexSelected)->ConvertTexturesToVec());
			    renderNode->G_SetBuffer("LightInfo", light);
			    renderNode->G_SetBuffer("RConfigs", rConfigs);

			    vk::DeviceSize offset = 0;
			    renderNode->GetCurrCmd().bindVertexBuffers(0, 1, &quadVertBufferRef->bufferHandle.get(), &offset);
			    renderNode->GetCurrCmd().bindIndexBuffer(quadIndexBufferRef->bufferHandle.get(), 0, vk::IndexType::eUint32);

			    renderNode->GetCurrCmd().pushConstants(renderNode->GPUPipelineRef->pipelineLayout.get(),
			                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			                                           0, sizeof(RcPc), &rcPc);

			    renderNode->GetCurrCmd().drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0,
			                                         0, 0);
			    testSpriteAnim->UseFrame();
		    });
		renderGraph->GetNode(resultPassName)->G_SetRenderOperation(resultRenderOp);
		renderGraph->GetNode(resultPassName)->AddPreRenderingTask(resultTask);
	}

	void ReloadShaders() override
	{
		auto *paintingNode = renderGraph->GetNode(paintingPassName);
		paintingNode->G_RecreateResources();
		for (int i = 0; i < cascadesInfo.cascadeCount; ++i)
		{
			auto *genNode = renderGraph->GetNode(probesGenPassNames[i]);
			genNode->G_RecreateResources();
		}
		auto *outputNode = renderGraph->GetNode(rCascadesPassName);
		outputNode->G_RecreateResources();
		for (int i = cascadesInfo.cascadeCount - 2; i >= 0; i--)
		{
			std::string name      = rMergePassName + "_" + std::to_string(i);
			auto       *mergeNode = renderGraph->GetNode(name);
			mergeNode->G_RecreateResources();
		}
		auto *resultNode = renderGraph->GetNode(resultPassName);
		resultNode->G_RecreateResources();
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
	int                     rcResolutionW = 1024;
	int                     rcResolutionH = 1024;
};
}        // namespace Rendering
#endif        // FLATRENDERER_HPP
