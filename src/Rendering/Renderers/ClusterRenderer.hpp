﻿//

// Created by carlo on 2024-10-25.
//

#ifndef CLUSTERRENDERER_HPP
#define CLUSTERRENDERER_HPP

namespace Rendering
{
using namespace ENGINE;
class ClusterRenderer : public BaseRenderer
{
  public:
	ClusterRenderer(Core *core, WindowProvider *windowProvider)
	{
		this->core           = core;
		this->renderGraphRef = core->renderGraphRef;
		this->windowProvider = windowProvider;
		computeDescCache     = std::make_unique<DescriptorCache>(this->core);
		gBuffDescCache       = std::make_unique<DescriptorCache>(this->core);
		cullMeshesCache      = std::make_unique<DescriptorCache>(this->core);

		CreateResources();
		CreateBuffers();
		CreatePipelines();
	}

	void RecreateSwapChainResources() override
	{
	}

	void SetRenderOperation() override
	{
		auto meshCullTask = new std::function<void()>([this]() {
			MoveCam();
			cPropsUbo.invProj = glm::inverse(currCamera->matrices.perspective);
			cPropsUbo.invView = glm::inverse(currCamera->matrices.view);
			cPropsUbo.pos     = currCamera->position;
			cPropsUbo.zNear   = currCamera->cameraProperties.zNear;
			cPropsUbo.zFar    = currCamera->cameraProperties.zFar;
			BuildFrustumPlanes();

			meshesSpheresCompact.clear();
			std::vector<Sphere> meshesSpheresCompact2;
			for (auto &model : RenderingResManager::GetInstance()->indirectModelsToDraw)
			{
				for (auto &sphere : model.second->meshesSpheres)
				{
					meshesSpheresCompact.emplace_back(sphere);
					meshesSpheresCompact2.emplace_back(sphere);
				}
			}
		});

		auto meshCullRenderOp = new std::function<void()>(
		    [this]() {
			    cullMeshesCache->SetBuffer("IndirectCmds",
			                               RenderingResManager::GetInstance()->indirectDrawBuffer);
			    cullMeshesCache->SetBuffer("MeshesSpheres", meshesSpheresCompact);
			    cullMeshesCache->SetBuffer("CamProps", cPropsUbo);
			    cullMeshesCache->SetBuffer("CullInfo", camFrustum);

			    auto &renderNode = renderGraphRef->renderNodes.at(meshCullPassName);
			    renderGraphRef->currentFrameResources->commandBuffer->bindDescriptorSets(renderNode->pipelineType,
			                                                                             renderNode->pipelineLayout.get(), 0,
			                                                                             1,
			                                                                             &cullMeshesCache->dstSet, 0, nullptr);
			    renderGraphRef->currentFrameResources->commandBuffer->bindPipeline(renderNode->pipelineType, renderNode->pipeline.get());
			    renderGraphRef->currentFrameResources->commandBuffer->dispatch(RenderingResManager::GetInstance()->indirectDrawsCmdInfos.size(), 1, 1);
		    });

		renderGraphRef->GetNode(meshCullPassName)->SetRenderOperation(meshCullRenderOp);
		renderGraphRef->GetNode(meshCullPassName)->AddTask(meshCullTask);

		auto cullTask = new std::function<void()>([this]() {
			cullDataPc.sWidth           = (int) windowProvider->GetWindowSize().x;
			cullDataPc.sHeight          = (int) windowProvider->GetWindowSize().y;
			cullDataPc.pointLightsCount = pointLights.size();
			cullDataPc.xTileCount       = static_cast<uint32_t>((core->swapchainRef->extent.width - 1) / xTileSizePx + 1);
			cullDataPc.yTileCount       = static_cast<uint32_t>((core->swapchainRef->extent.height - 1) / yTileSizePx + 1);

			lightsMap.clear();
			for (int i = 0; i < cullDataPc.xTileCount * cullDataPc.yTileCount * zSlicesSize; ++i)
			{
				lightsMap.emplace_back(ArrayIndexer{});
			}
			lightsIndices.clear();
			lightsIndices.reserve(lightsMap.size() * pointLights.size());
			for (int i = 0; i < lightsMap.size() * pointLights.size(); ++i)
			{
				lightsIndices.emplace_back(-1);
			}
			cPropsUbo.invProj = glm::inverse(camera.matrices.perspective);
			cPropsUbo.invView = glm::inverse(camera.matrices.view);
			cPropsUbo.pos     = camera.position;
			cPropsUbo.zNear   = camera.cameraProperties.zNear;
			cPropsUbo.zFar    = camera.cameraProperties.zFar;
		});

		auto cullRenderOp = new std::function<void()>(
		    [this]() {
			    computeDescCache->SetBuffer("PointLights", pointLights);
			    computeDescCache->SetBuffer("LightMap", lightsMap);
			    computeDescCache->SetBuffer("LightIndices", lightsIndices);
			    computeDescCache->SetBuffer("CameraProperties", cPropsUbo);

			    auto &renderNode = renderGraphRef->renderNodes.at(computePassName);
			    renderGraphRef->currentFrameResources->commandBuffer->bindDescriptorSets(renderNode->pipelineType,
			                                                                             renderNode->pipelineLayout.get(), 0,
			                                                                             1,
			                                                                             &computeDescCache->dstSet, 0, nullptr);
			    renderGraphRef->currentFrameResources->commandBuffer->pushConstants(renderGraphRef->GetNode(computePassName)->pipelineLayout.get(),
			                                                                        vk::ShaderStageFlagBits::eCompute,
			                                                                        0, sizeof(ScreenDataPc), &cullDataPc);
			    renderGraphRef->currentFrameResources->commandBuffer->bindPipeline(renderNode->pipelineType, renderNode->pipeline.get());
			    renderGraphRef->currentFrameResources->commandBuffer->dispatch(cullDataPc.xTileCount / localSize, cullDataPc.yTileCount / localSize,
			                                                                   zSlicesSize);
		    });

		renderGraphRef->GetNode(computePassName)->AddTask(cullTask);
		renderGraphRef->GetNode(computePassName)->SetRenderOperation(cullRenderOp);

		auto renderOp = new std::function<void()>(
		    [this]() {
			    vk::DeviceSize           offset = 0;
			    std::vector<ImageView *> textures;
			    for (auto &image : ResourcesManager::GetInstance()->imageShippers)
			    {
				    textures.emplace_back(image->imageView.get());
			    }
			    std::vector<MaterialPackedData> materials;
			    for (auto &mat : RenderingResManager::GetInstance()->materialPackedData)
			    {
				    materials.emplace_back(*mat);
			    }
			    if (materials.empty())
			    {
				    materials.emplace_back(MaterialPackedData());
			    }

			    std::vector<glm::mat4> modelMats;
			    std::vector<int>       meshMatIds;
			    for (auto &model : RenderingResManager::GetInstance()->models)
			    {
				    for (int i = 0; i < model->modelsMat.size(); ++i)
				    {
					    modelMats.push_back(model->modelsMat[i]);
				    }
				    for (int i = 0; i < model->materials.size(); ++i)
				    {
					    meshMatIds.push_back(model->materials[i]);
				    }
			    }

			    gBuffDescCache->SetSamplerArray("textures", textures);
			    gBuffDescCache->SetBuffer("MaterialsPacked", materials);
			    gBuffDescCache->SetBuffer("MeshMaterialsIds", meshMatIds);
			    gBuffDescCache->SetBuffer("MeshesModelMatrices", modelMats);

			    renderGraphRef->currentFrameResources->commandBuffer->bindDescriptorSets(renderGraphRef->GetNode(gBufferPassName)->pipelineType,
			                                                                             renderGraphRef->GetNode(gBufferPassName)->pipelineLayout.get(), 0,
			                                                                             1,
			                                                                             &gBuffDescCache->dstSet, 0, nullptr);

			    pc.projView = camera.matrices.perspective * camera.matrices.view;
			    renderGraphRef->currentFrameResources->commandBuffer->pushConstants(renderGraphRef->GetNode(gBufferPassName)->pipelineLayout.get(),
			                                                                        vk::ShaderStageFlagBits::eVertex |
			                                                                            vk::ShaderStageFlagBits::eFragment,
			                                                                        0, sizeof(MvpPc), &pc);

			    int meshOffset = 0;

			    for (auto &modelPair : RenderingResManager::GetInstance()->indirectModelsToDraw)
			    {
				    Model *modelRef = modelPair.second;
				    renderGraphRef->currentFrameResources->commandBuffer->bindVertexBuffers(0, 1, &modelRef->vertBuffer->deviceBuffer->bufferHandle.get(),
				                                                                            &offset);
				    renderGraphRef->currentFrameResources->commandBuffer->bindIndexBuffer(modelRef->indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);

				    vk::DeviceSize sizeOffset = (meshOffset) * sizeof(DrawIndirectIndexedCmd);
				    uint32_t       stride     = sizeof(DrawIndirectIndexedCmd);
				    renderGraphRef->currentFrameResources->commandBuffer->drawIndexedIndirect(
				        RenderingResManager::GetInstance()->indirectDrawBuffer->bufferHandle.get(),
				        sizeOffset,
				        modelRef->meshCount,
				        stride);
				    meshOffset += modelRef->meshCount;
			    }
		    });

		renderGraphRef->GetNode(gBufferPassName)->SetRenderOperation(renderOp);

		auto lSetViewTask = new std::function<void()>([this]() {
			lightPc.xTileCount  = cullDataPc.xTileCount;
			lightPc.yTileCount  = cullDataPc.yTileCount;
			lightPc.xTileSizePx = xTileSizePx;
			lightPc.yTileSizePx = yTileSizePx;
			lightPc.zSlices     = zSlicesSize;

			auto *currImage = renderGraphRef->currentBackBuffer;
			renderGraphRef->AddColorImageResource(lightPassName, "lColor", currImage);
			renderGraphRef->GetNode(lightPassName)->SetFramebufferSize(windowProvider->GetWindowSize());
		});
		auto lRenderOp    = new std::function<void()>(
            [this]() {
                auto renderNode = renderGraphRef->GetNode(lightPassName);
                renderGraphRef->resourcesManager->RequestStorageImageClear("specularHolderStorage");
                renderNode->descCache->SetSampler("gCol", colAttachmentView);
                renderNode->descCache->SetSampler("gNormals", normAttachmentView);
                renderNode->descCache->SetSampler("gTang", tangAttachmentView);
                renderNode->descCache->SetSampler("gDepth", depthAttachmentView);
                renderNode->descCache->SetSampler("gMetRoughness", metRoughAttachmentView);
                renderNode->descCache->SetSampler("gMeshUV", uvAttachmentView);

                renderNode->descCache->SetStorageImage("specularHolder", specularHolder);
                renderNode->descCache->SetBuffer("CameraProperties", cPropsUbo);
                renderNode->descCache->SetBuffer("PointLights", pointLights);
                renderNode->descCache->SetBuffer("LightMap", lightsMap);
                renderNode->descCache->SetBuffer("LightIndices", lightsIndices);
                vk::DeviceSize offset = 0;
                renderGraphRef->currentFrameResources->commandBuffer->bindDescriptorSets(renderGraphRef->GetNode(lightPassName)->pipelineType,
			                                                                                renderGraphRef->GetNode(lightPassName)->pipelineLayout.get(),
			                                                                                0, 1,
			                                                                                &renderNode->descCache->dstSet, 0, nullptr);
                renderGraphRef->currentFrameResources->commandBuffer->bindVertexBuffers(0, 1, &lVertexBuffer->bufferHandle.get(), &offset);
                renderGraphRef->currentFrameResources->commandBuffer->bindIndexBuffer(lIndexBuffer->bufferHandle.get(), 0, vk::IndexType::eUint32);

                renderGraphRef->currentFrameResources->commandBuffer->pushConstants(renderGraphRef->GetNode(lightPassName)->pipelineLayout.get(),
			                                                                           vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex,
			                                                                           0, sizeof(LightPc), &lightPc);
                renderGraphRef->currentFrameResources->commandBuffer->drawIndexed(quadIndices.size(), 1, 0,
			                                                                         0, 0);
            });

		renderGraphRef->GetNode(lightPassName)->AddTask(lSetViewTask);
		renderGraphRef->GetNode(lightPassName)->SetRenderOperation(lRenderOp);
	}

	void ReloadShaders() override
	{
		auto *gRenderNode     = renderGraphRef->GetNode(gBufferPassName);
		auto *renderNode      = renderGraphRef->GetNode(lightPassName);
		auto *cRenderNode     = renderGraphRef->GetNode(computePassName);
		auto *meshCRenderNode = renderGraphRef->GetNode(meshCullPassName);

		gRenderNode->RecreateResources();
		renderNode->RecreateResources();
		cRenderNode->RecreateResources();
		meshCRenderNode->RecreateResources();
	}

	void CreateResources()
	{
		auto imageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
		                                     vk::Format::eR32G32B32A32Sfloat,
		                                     vk::ImageUsageFlagBits::eColorAttachment |
		                                         vk::ImageUsageFlagBits::eSampled);

		auto storageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
		                                       vk::Format::eR32G32B32A32Sfloat,
		                                       vk::ImageUsageFlagBits::eStorage |
		                                           vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

		auto depthImageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
		                                          core->swapchainRef->depthFormat,
		                                          vk::ImageUsageFlagBits::eDepthStencilAttachment |
		                                              vk::ImageUsageFlagBits::eSampled);

		colAttachmentView      = ResourcesManager::GetInstance()->GetImage("colAttachment", imageInfo, 0, 0);
		normAttachmentView     = ResourcesManager::GetInstance()->GetImage("normAttachment", imageInfo, 0, 0);
		tangAttachmentView     = ResourcesManager::GetInstance()->GetImage("tangAttachment", imageInfo, 0, 0);
		metRoughAttachmentView = ResourcesManager::GetInstance()->GetImage("metRoughnessAttachmentView", imageInfo, 0, 0);
		uvAttachmentView       = ResourcesManager::GetInstance()->GetImage("uvAttachmentView", imageInfo, 0, 0);

		depthAttachmentView = ResourcesManager::GetInstance()->GetImage("depthAttachment", depthImageInfo, 0, 0);
		specularHolder      = ResourcesManager::GetInstance()->GetImage("specularHolderStorage", storageInfo, 0, 0);

		// gbuff
		camera.SetPerspective(
		    45.0f, (float) windowProvider->GetWindowSize().x / (float) windowProvider->GetWindowSize().y,
		    0.1f, 512.0f);
		debugCam.SetPerspective(
		    45.0f, (float) windowProvider->GetWindowSize().x / (float) windowProvider->GetWindowSize().y,
		    0.1f, 512.0f);

		currCamera = &camera;

		camera.SetLookAt(glm::vec3(0.0f, 0.0f, 1.0f));
		camera.position = glm::vec3(0.0f);

		std::string path = SYSTEMS::OS::GetInstance()->GetAssetsPath();
		RenderingResManager::GetInstance()->PushModelToIndirectBatch(path + "\\Models\\floating_lighthouse\\scene.gltf");

		// compute
		std::random_device rd;
		std::mt19937       gen(rd());

		pointLights.reserve(1);
		for (int i = 0; i < 1; ++i)
		{
			std::uniform_real_distribution<> distributionPos(-10.0f, 10.0f);
			std::uniform_real_distribution<> distributionCol(0.0f, 1.0f);
			glm::vec3                        pos = glm::vec3(distributionPos(gen), distributionPos(gen), distributionPos(gen));

			glm::vec3 col = glm::vec3(distributionCol(gen), distributionCol(gen), distributionCol(gen));

			std::uniform_real_distribution<> distributionIntensity(0.5f, 2.0f);
			float                            intensity = static_cast<float>(distributionIntensity(gen));

			std::uniform_real_distribution<> distributionRadius(0.5f, 10.0f);
			float                            radius = 2.0f;

			std::uniform_real_distribution<> distributionAttenuation(0.3f, 10.0f);
			float                            lAttenuation = 0.01f;
			float                            qAttenuation = static_cast<float>(distributionAttenuation(gen));

			pointLights.emplace_back(PointLight{pos, col, radius, intensity, lAttenuation, 0.0f});
		}

		cullDataPc.sWidth           = windowProvider->GetWindowSize().x;
		cullDataPc.sHeight          = windowProvider->GetWindowSize().y;
		cullDataPc.pointLightsCount = 0;
		cullDataPc.xTileCount       = static_cast<uint32_t>((core->swapchainRef->extent.width - 1) / xTileSizePx + 1);
		cullDataPc.yTileCount       = static_cast<uint32_t>((core->swapchainRef->extent.height - 1) / yTileSizePx + 1);
		cullDataPc.xTileSizePx      = xTileSizePx;
		cullDataPc.yTileSizePx      = yTileSizePx;
		cullDataPc.pointLightsCount = (int) pointLights.size();

		lightsMap.reserve(cullDataPc.xTileCount * cullDataPc.yTileCount * zSlicesSize);

		for (int i = 0; i < lightsMap.capacity(); ++i)
		{
			lightsMap.emplace_back(ArrayIndexer{});
		}

		lightsIndices.reserve(lightsMap.size() * pointLights.size());
		for (int i = 0; i < lightsMap.size() * pointLights.size(); ++i)
		{
			lightsIndices.emplace_back(-1);
		}

		// light
		quadVert    = Vertex2D::GetQuadVertices();
		quadIndices = Vertex2D::GetQuadIndices();

		cPropsUbo.invProj = glm::inverse(camera.matrices.perspective);
		cPropsUbo.invView = glm::inverse(camera.matrices.view);
		cPropsUbo.pos     = camera.position;
		cPropsUbo.zNear   = camera.cameraProperties.zNear;
		cPropsUbo.zFar    = camera.cameraProperties.zFar;

		lightPc.xTileCount  = cullDataPc.xTileCount;
		lightPc.yTileCount  = cullDataPc.yTileCount;
		lightPc.xTileSizePx = xTileSizePx;
		lightPc.yTileSizePx = yTileSizePx;
		lightPc.zSlices     = zSlicesSize;
	}

	void CreateBuffers()
	{
		RenderingResManager::GetInstance()->BuildIndirectBuffers();

		lVertexBuffer = ResourcesManager::GetInstance()->GetBuffer("lVertexBuffer", vk::BufferUsageFlagBits::eVertexBuffer,
		                                                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		                                                           sizeof(Vertex2D) * quadVert.size(), quadVert.data());

		lIndexBuffer = ResourcesManager::GetInstance()->GetBuffer("lIndexBuffer", vk::BufferUsageFlagBits::eIndexBuffer,
		                                                          vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		                                                          sizeof(uint32_t) * quadIndices.size(), quadIndices.data());
	}

	void CreatePipelines()
	{
		auto &logicalDevice = core->logicalDevice.get();

		// Cull meshes
		std::string shaderPath = SYSTEMS::OS::GetInstance()->GetShadersPath();

		cullMeshesCompShader = renderGraphRef->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\Compute\\meshCull.comp.spv", S_COMP);

		cullMeshesCache->AddShaderInfo(cullMeshesCompShader->sParser.get());
		cullMeshesCache->BuildDescriptorsCache(vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eFragment);

		auto meshCullLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		                                    .setSetLayoutCount(1)
		                                    .setPSetLayouts(&cullMeshesCache->dstLayout.get());

		auto *meshCullRenderNode = renderGraphRef->AddPass(meshCullPassName);
		meshCullRenderNode->SetCompShader(cullMeshesCompShader);
		meshCullRenderNode->SetPipelineLayoutCI(meshCullLayoutCreateInfo);
		meshCullRenderNode->BuildRenderGraphNode();

		// Cull pass//

		cullCompShader = renderGraphRef->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\Compute\\lightCulling.comp.spv", S_COMP);

		computeDescCache->AddShaderInfo(cullCompShader->sParser.get());
		computeDescCache->BuildDescriptorsCache(vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eFragment);

		auto cullPushConstantRange = vk::PushConstantRange()
		                                 .setOffset(0)
		                                 .setStageFlags(vk::ShaderStageFlagBits::eCompute)
		                                 .setSize(sizeof(ScreenDataPc));

		auto cullLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		                                .setSetLayoutCount(1)
		                                .setPushConstantRanges(cullPushConstantRange)
		                                .setPSetLayouts(&computeDescCache->dstLayout.get());

		auto *cullRenderNode = renderGraphRef->AddPass(computePassName);
		cullRenderNode->SetCompShader(cullCompShader);
		cullRenderNode->SetPipelineLayoutCI(cullLayoutCreateInfo);
		cullRenderNode->BuildRenderGraphNode();

		// gbuffer

		gVertShader = renderGraphRef->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\ClusterRendering\\gBuffer.vert.spv", S_VERT);
		gFragShader = renderGraphRef->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\ClusterRendering\\gBuffer.frag.spv", S_FRAG);
		gBuffDescCache->AddShaderInfo(gVertShader->sParser.get());
		gBuffDescCache->AddShaderInfo(gFragShader->sParser.get());
		gBuffDescCache->BuildDescriptorsCache(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

		auto pushConstantRange = vk::PushConstantRange()
		                             .setOffset(0)
		                             .setStageFlags(
		                                 vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
		                             .setSize(sizeof(MvpPc));

		auto layoutCreateInfo = vk::PipelineLayoutCreateInfo()
		                            .setSetLayoutCount(1)
		                            .setPushConstantRanges(pushConstantRange)
		                            .setPSetLayouts(&gBuffDescCache->dstLayout.get());

		VertexInput    vertexInput = M_Vertex3D::GetVertexInput();
		AttachmentInfo colInfo     = GetColorAttachmentInfo(
            glm::vec4(0.0f), vk::Format::eR32G32B32A32Sfloat);
		AttachmentInfo depthInfo  = GetDepthAttachmentInfo();
		auto           renderNode = renderGraphRef->AddPass(gBufferPassName);

		renderNode->SetVertShader(gVertShader);
		renderNode->SetFragShader(gFragShader);
		renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
		renderNode->SetPipelineLayoutCI(layoutCreateInfo);
		renderNode->SetVertexInput(vertexInput);
		renderNode->AddColorAttachmentOutput("gColor", colInfo, BlendConfigs::B_OPAQUE);
		renderNode->AddColorAttachmentOutput("gNorm", colInfo, BlendConfigs::B_OPAQUE);
		renderNode->AddColorAttachmentOutput("gTang", colInfo, BlendConfigs::B_OPAQUE);
		renderNode->AddColorAttachmentOutput("gMetRoughness", colInfo, BlendConfigs::B_OPAQUE);
		renderNode->AddColorAttachmentOutput("gUVs", colInfo, BlendConfigs::B_OPAQUE);
		renderNode->SetDepthAttachmentOutput("gDepth", depthInfo);
		renderNode->SetDepthConfig(DepthConfigs::D_ENABLE);
		renderNode->AddColorImageResource("gColor", colAttachmentView);
		renderNode->AddColorImageResource("gNorm", normAttachmentView);
		renderNode->AddColorImageResource("gTang", tangAttachmentView);
		renderNode->AddColorImageResource("gMetRoughness", metRoughAttachmentView);
		renderNode->AddColorImageResource("gUVs", uvAttachmentView);
		renderNode->SetDepthImageResource("gDepth", depthAttachmentView);
		renderNode->SetGraphicsPipelineConfigs({R_POINT, T_POINT_LIST});
		renderNode->AddBufferSync("indirectBuffer", {B_COMPUTE_WRITE, B_DRAW_INDIRECT});
		renderNode->DependsOn(meshCullPassName);
		renderNode->BuildRenderGraphNode();

		// light pass//

		lVertShader = renderGraphRef->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\Common\\Quad.vert.spv", S_VERT);
		lFragShader = renderGraphRef->resourcesManager->GetShader(shaderPath + "\\spirvGlsl\\ClusterRendering\\light.frag.spv", S_FRAG);

		AttachmentInfo lColInfo = GetColorAttachmentInfo(
		    glm::vec4(0.0f), core->swapchainRef->GetFormat());

		VertexInput lVertexInput = Vertex2D::GetVertexInput();

		auto lRenderNode = renderGraphRef->AddPass(lightPassName);
		lRenderNode->SetVertShader(lVertShader);
		lRenderNode->SetFragShader(lFragShader);
		lRenderNode->SetConfigs(true);
		lRenderNode->SetPushConstantSize(sizeof(LightPc));
		lRenderNode->SetFramebufferSize(windowProvider->GetWindowSize());
		lRenderNode->SetVertexInput(lVertexInput);
		lRenderNode->AddColorAttachmentOutput("lColor", lColInfo, BlendConfigs::B_OPAQUE);
		lRenderNode->AddSamplerResource("colGSampler", colAttachmentView);
		lRenderNode->AddSamplerResource("normGSampler", normAttachmentView);
		lRenderNode->AddSamplerResource("tangGSampler", tangAttachmentView);
		lRenderNode->AddSamplerResource("metRoughnessGSampler", metRoughAttachmentView);
		lRenderNode->AddSamplerResource("uvGSampler", uvAttachmentView);
		lRenderNode->AddSamplerResource("depthGSampler", depthAttachmentView);
		lRenderNode->AddStorageResource("specularHolder", specularHolder);
		lRenderNode->DependsOn(computePassName);
		lRenderNode->BuildRenderGraphNode();
	}
	glm::vec4 u_GetPlane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		glm::vec3 v1 = p1 - p0;
		glm::vec3 v2 = p2 - p0;

		glm::vec3 norm;
		norm = normalize(cross(v1, v2));

		float d = dot(norm, p0);

		glm::vec4 plane = glm::vec4(norm, d);
		return plane;
	}

	glm::vec4 ScreenToViewNDC(glm::mat4 invProj, float depth, glm::vec2 ndcCoords)
	{
		glm::vec4 ndcPos  = glm::vec4(ndcCoords, depth, 1.0);
		glm::vec4 viewPos = invProj * ndcPos;
		viewPos           = viewPos / viewPos.w;
		return viewPos;
	}
	void BuildFrustumPlanes()
	{
		glm::vec3 nearTopL    = ScreenToViewNDC(cPropsUbo.invProj, 0.0, glm::vec2(-1.0, 1.0));
		glm::vec3 nearTopR    = ScreenToViewNDC(cPropsUbo.invProj, 0.0, glm::vec2(1.0, 1.0));
		glm::vec3 nearBottomL = ScreenToViewNDC(cPropsUbo.invProj, 0.0, glm::vec2(-1.0, -1.0));
		glm::vec3 nearBottomR = ScreenToViewNDC(cPropsUbo.invProj, 0.0, glm::vec2(1.0, -1.0));

		glm::vec3 farTopL    = ScreenToViewNDC(cPropsUbo.invProj, 1.0, glm::vec2(-1.0, 1.0));
		glm::vec3 farTopR    = ScreenToViewNDC(cPropsUbo.invProj, 1.0, glm::vec2(1.0, 1.0));
		glm::vec3 farBottomL = ScreenToViewNDC(cPropsUbo.invProj, 1.0, glm::vec2(-1.0, -1.0));
		glm::vec3 farBottomR = ScreenToViewNDC(cPropsUbo.invProj, 1.0, glm::vec2(1.0, -1.0));

		camFrustum.points[0] = nearTopL;
		camFrustum.points[1] = nearTopR;
		camFrustum.points[2] = nearBottomL;
		camFrustum.points[3] = nearBottomR;

		camFrustum.points[4] = farTopL;
		camFrustum.points[5] = farTopR;
		camFrustum.points[6] = farBottomL;
		camFrustum.points[7] = farBottomR;

		// left
		camFrustum.planes[0] = u_GetPlane(nearBottomL, farBottomL, nearTopL);

		// Right Plane
		camFrustum.planes[1] = u_GetPlane(nearTopR, farTopR, nearBottomR);

		// Top Plane
		camFrustum.planes[2] = u_GetPlane(nearTopL, farTopL, nearTopR);

		// Bottom Plane
		camFrustum.planes[3] = u_GetPlane(nearBottomR, farBottomR, nearBottomL);

		// Near Plane
		camFrustum.planes[4] = u_GetPlane(nearTopL, nearBottomR, nearBottomL);

		// Far Plane
		camFrustum.planes[5] = u_GetPlane(farTopL, farBottomL, farBottomR);
	}
	void MoveCam()
	{
		glm::vec2 input = glm::vec2(0.0f);
		if (glfwGetKey(windowProvider->window, GLFW_KEY_W))
		{
			input += glm::vec2(0.0f, 1.0f);
		}
		if (glfwGetKey(windowProvider->window, GLFW_KEY_S))
		{
			input += glm::vec2(0.0f, -1.0f);
		}
		if (glfwGetKey(windowProvider->window, GLFW_KEY_D))
		{
			input += glm::vec2(1.0f, 0.0f);
		}
		if (glfwGetKey(windowProvider->window, GLFW_KEY_A))
		{
			input += glm::vec2(-1.0f, 0.0f);
		}
		if (glfwGetKey(windowProvider->window, GLFW_KEY_LEFT_SHIFT))
		{
			camera.movementSpeed = 40;
		}
		else
		{
			camera.movementSpeed = 5;
		}
		input                = glm::clamp(input, glm::vec2(-1.0, -1.0), glm::vec2(1.0, 1.0));
		glm::vec2 mouseInput = glm::vec2(-ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		camera.mouseInput    = mouseInput;
		if (glfwGetMouseButton(windowProvider->window, GLFW_MOUSE_BUTTON_2))
		{
			camera.RotateCamera();
			camera.Move(deltaTime, input);
		}
		else
		{
			camera.firstMouse = true;
		}
		camera.UpdateCam();
	};

	WindowProvider *windowProvider;
	Core           *core;
	RenderGraph    *renderGraphRef;

	Shader *gVertShader;
	Shader *gFragShader;

	Shader *lVertShader;
	Shader *lFragShader;

	Shader *cullCompShader;
	Shader *cullMeshesCompShader;

	ImageView *colAttachmentView;
	ImageView *normAttachmentView;
	ImageView *tangAttachmentView;
	ImageView *metRoughAttachmentView;
	ImageView *depthAttachmentView;
	ImageView *uvAttachmentView;
	ImageView *specularHolder;

	StagedBuffer *vertexBuffer;
	StagedBuffer *indexBuffer;

	Buffer *lVertexBuffer;
	Buffer *lIndexBuffer;

	std::unique_ptr<DescriptorCache> computeDescCache;
	std::unique_ptr<DescriptorCache> gBuffDescCache;
	std::unique_ptr<DescriptorCache> cullMeshesCache;

	std::string gBufferPassName  = "gBuffer";
	std::string computePassName  = "cullLight";
	std::string lightPassName    = "light";
	std::string meshCullPassName = "cullMesh";

	// gbuff
	Camera  camera     = {glm::vec3(5.0f), Camera::CameraMode::E_FREE};
	Camera  debugCam   = {glm::vec3(10.0f), Camera::CameraMode::E_FREE};
	Camera *currCamera = nullptr;
	MvpPc   pc{};

	// culling
	std::vector<PointLight>   pointLights;
	std::vector<ArrayIndexer> lightsMap;
	std::vector<int32_t>      lightsIndices;
	std::vector<Sphere>       meshesSpheresCompact;
	ScreenDataPc              cullDataPc{};
	uint32_t                  xTileSizePx = 512;
	uint32_t                  yTileSizePx = 512;
	uint32_t                  zSlicesSize = 4;
	uint32_t                  localSize   = 1;

	// light
	std::vector<Vertex2D> quadVert;
	std::vector<uint32_t> quadIndices;
	CPropsUbo             cPropsUbo;
	LightPc               lightPc{};
	Frustum               camFrustum;
};
}        // namespace Rendering

#endif        // CLUSTERRENDERER_HPP
