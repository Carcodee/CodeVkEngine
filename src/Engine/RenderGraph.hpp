//

// Created by carlo on 2024-10-02.
//

#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP
#define DUMMY_PC_SIZE 4

namespace ENGINE
{
struct BufferKey
{
	BufferUsageTypes srcUsage;
	BufferUsageTypes dstUsage;
	Buffer          *buffer;
	std::string      name;
};

struct RenderNodeConfigs : SYSTEMS::ISerializable<RenderNodeConfigs>
{
	bool automaticCache = true;

	RenderNodeConfigs(bool automaticCache)
	{
		this->automaticCache = automaticCache;
	}

	nlohmann::json Serialize() override
	{
		nlohmann::json json;
		json["automaticCache"] = false;

		return json;
	}
	RenderNodeConfigs *Deserialize(std::string filename) override
	{
		return this;
	}
};

class RenderGraph;

struct RenderGraphNode : SYSTEMS::ISerializable<RenderGraphNode>
{
	RenderGraphNode() = default;

	nlohmann::json Serialize() override
	{
		nlohmann::json json;
		json["passName"]                 = passName;
		json["active"]                   = active;
		json["waitForResourcesCreation"] = waitForResourcesCreation;
		json["pipelineType"]             = static_cast<int>(pipelineType);
		json["frameBufferSize"]          = {frameBufferSize.x, frameBufferSize.y};
		json["pushConstantSize"]         = pushConstantSize;
		json["configs"]                  = configs.Serialize();
		json["rasterizationConfigs"]     = static_cast<int>(graphicsPipelineConfigs.rasterizationConfigs);
		json["topologyConfigs"]          = static_cast<int>(graphicsPipelineConfigs.topologyConfigs);
		json["depthConfig"]              = static_cast<int>(depthConfig);

		// Serialize shaders
		json["shaders"] = nlohmann::json::array();
		for (const auto &[key, shader] : shaders)
		{
			if (shader != nullptr && !shader->path.empty())
			{
				json["shaders"].push_back({shader->stage, shader->path});
			}
		}
		// Serialize color attachments
		json["colAttachments"] = nlohmann::json::array();
		for (auto &attachment : colAttachments)
		{
			json["colAttachments"].push_back(attachment.Serialize());
		}
		// Serialize depth attachment
		json["depthAttachment"] = depthAttachment.Serialize();

		// Serialize dependencies
		json["dependencies"] = nlohmann::json::array();
		for (const auto &dep : dependencies)
		{
			json["dependencies"].push_back(dep);
		}

		// Serialize resources
		json["imageAttachmentsNames"] = nlohmann::json::array();
		for (const auto &pair : imageAttachmentsNames)
		{
			json["imageAttachmentsNames"].push_back({{"name", pair.first}, {"index", pair.second}});
		}
		json["imagesAttachmentOutputs"] = nlohmann::json::array();
		for (const auto &imgView : imagesAttachmentOutputs)
		{
			json["imagesAttachmentOutputs"].push_back({{"id", imgView->id}});
		}

		json["storageImages"] = nlohmann::json::array();
		for (const auto &[name, imgView] : storageImages)
		{
			json["storageImages"].push_back({{"id", imgView->id}, {"name", name}});
		}
		json["sampledImages"] = nlohmann::json::array();
		for (const auto &[name, imgView] : sampledImages)
		{
			json["sampledImages"].push_back({{"id", imgView->id}, {"name", name}});
		}
		std::string text     = json.dump(4);
		std::string filename = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\RenderNodes\\pass_" + passName + ".json";
		if (!filename.empty())
		{
			SYSTEMS::OS::GetInstance()->WriteFile(filename, text.c_str(), text.size());
		}
		return text;
	}

	RenderGraphNode *Deserialize(std::string filename) override
	{
		if (!std::filesystem::exists(filename))
		{
			SYSTEMS::Logger::GetInstance()->LogMessage(
			    "Pass with name: (" + passName + ") does not have a serialized file, the pass may be null");
			return this;
		}
		nlohmann::json json = SYSTEMS::OS::GetJsonFromFile(filename);
		assert(!json.empty());
		std::vector<float> fSize;
		passName                 = json.at("passName");
		active                   = json.at("active");
		waitForResourcesCreation = json.at("waitForResourcesCreation");
		pipelineType             = json.at("pipelineType");

		fSize             = json.at("frameBufferSize").get<std::vector<float>>();
		frameBufferSize.x = fSize[0];
		frameBufferSize.y = fSize[1];

		pushConstantSize                             = json.at("pushConstantSize");
		configs.Serialize()                          = json.at("configs");
		graphicsPipelineConfigs.rasterizationConfigs = json.at("rasterizationConfigs");
		graphicsPipelineConfigs.topologyConfigs      = json.at("topologyConfigs");
		depthConfig                                  = json.at("depthConfig");

		// to do: missing images deserialize;
		//  Serialize shaders
		if (json.contains("shaders") && json["shaders"].is_array())
		{
			for (auto &shaderJson : json["shaders"])
			{
				if (shaderJson.is_array() && shaderJson.size() == 2)
				{
					ShaderStage stage  = static_cast<ShaderStage>(shaderJson[0]);
					std::string path   = static_cast<std::string>(shaderJson[1]);
					Shader     *shader = resManagerRef->GetShader(path, stage);
					switch (stage)
					{
						case S_VERT:
							SetVertShader(shader);
							break;
						case S_FRAG:
							SetFragShader(shader);
							break;
						case S_COMP:
							SetCompShader(shader);
							break;
						case S_TESS_CONTROL:
							SetTesControlShader(shader);
							break;
						case S_TESS_EVAL:
							SetTesEvalShader(shader);
							break;
						case S_GEOM:
							SetGeomShader(shader);
							break;
						case S_UNKNOWN:
							assert(false);
							break;
					}
				}
				else
				{
					assert(false);
				}
			}
		}

		if (colAttachments.size() == json["colAttachments"].size())
		{
			colAttachments.clear();
			for (auto &colAttachment : json["colAttachments"])
			{
				colAttachments.emplace_back(AttachmentInfo{});
				colAttachments.back().GetFromJson(colAttachment);
			}
		}
		else
		{
			SYSTEMS::Logger::GetInstance()->LogMessage("There is a mismatch in size with the col attachments(" + filename + ") avoiding override");
		}

		// Serialize depth attachment
		depthAttachment.GetFromJson(json.at("depthAttachment"));

		// Serialize dependencies
		for (const auto &dep : json["dependencies"])
		{
			DependsOn(dep);
		}
		waitForResourcesCreation = true;

		return this;
	}
	void RecreateResources()
	{
		assert(&pipelineLayoutCI != nullptr && "Pipeline layout is null");
		pipeline.reset();
		pipelineLayout.reset();
		ReloadShaders();
		Shader                         *vertShader = shaders.at("vert");
		Shader                         *fragShader = shaders.at("frag");
		Shader                         *compShader = shaders.at("comp");
		Shader                         *tescShader = shaders.at("tesc");
		Shader                         *teseShader = shaders.at("tese");
		Shader                         *geomShader = shaders.at("geom");
		std::map<ShaderStage, Shader *> stages;
		GetShaderStages(stages);

		if (fragShader && vertShader)
		{
			if (configs.automaticCache)
			{
				descCache.reset();
				descCache = std::make_unique<DescriptorCache>(core);
				descCache->AddShaderInfo(vertShader->sParser.get());
				descCache->AddShaderInfo(fragShader->sParser.get());
				vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eFragment |
				                                  vk::ShaderStageFlagBits::eVertex;
				if (tescShader)
				{
					descCache->AddShaderInfo(tescShader->sParser.get());
					if (!teseShader)
					{
						assert(false && "Must have tese shader");
					}
					descCache->AddShaderInfo(teseShader->sParser.get());
					stageFlags |= vk::ShaderStageFlagBits::eTessellationControl |
					              vk::ShaderStageFlagBits::eTessellationEvaluation;
				}
				if (geomShader)
				{
					descCache->AddShaderInfo(geomShader->sParser.get());
					stageFlags |= vk::ShaderStageFlagBits::eGeometry;
				}

				descCache->BuildDescriptorsCache(stageFlags);
				if (pushConstantSize <= 0)
				{
					SetPushConstantSize(DUMMY_PC_SIZE);
				}
				auto paintingPushConstantRanges = vk::PushConstantRange()
				                                      .setOffset(0)
				                                      .setStageFlags(
				                                          vk::ShaderStageFlagBits::eVertex |
				                                          vk::ShaderStageFlagBits::eFragment)
				                                      .setSize(pushConstantSize);
				auto paintingLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
				                                    .setSetLayoutCount(1)
				                                    .setPushConstantRanges(paintingPushConstantRanges)
				                                    .setPSetLayouts(&descCache->dstLayout.get());
				SetPipelineLayoutCI(paintingLayoutCreateInfo);
			}
			std::vector<vk::Format> colorFormats;
			colorFormats.reserve(colAttachments.size());
			std::vector<vk::RenderingAttachmentInfo> renderingAttachmentInfos;
			for (auto &colAttachment : colAttachments)
			{
				colorFormats.push_back(colAttachment.format);
				renderingAttachmentInfos.push_back(colAttachment.attachmentInfo);
			}
			dynamicRenderPass.SetPipelineRenderingInfo(colAttachments.size(), colorFormats, depthAttachment.format);

			pipelineLayout                                     = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
			std::unique_ptr<GraphicsPipeline> graphicsPipeline = std::make_unique<ENGINE::GraphicsPipeline>(
			    core->logicalDevice.get(), stages, pipelineLayout.get(),
			    dynamicRenderPass.pipelineRenderingCreateInfo, graphicsPipelineConfigs,
			    colorBlendConfigs, depthConfig,
			    vertexInput, pipelineCache.get());
			pipeline     = std::move(graphicsPipeline->pipelineHandle);
			pipelineType = vk::PipelineBindPoint::eGraphics;
		}
		else if (compShader)
		{
			if (configs.automaticCache)
			{
				descCache.reset();
				descCache = std::make_unique<DescriptorCache>(core);
				descCache->AddShaderInfo(compShader->sParser.get());
				descCache->BuildDescriptorsCache(
				    vk::ShaderStageFlagBits::eCompute);

				if (pushConstantSize <= 0)
				{
					SetPushConstantSize(DUMMY_PC_SIZE);
				}
				auto paintingPushConstantRanges = vk::PushConstantRange()
				                                      .setOffset(0)
				                                      .setStageFlags(
				                                          vk::ShaderStageFlagBits::eCompute)
				                                      .setSize(pushConstantSize);
				auto paintingLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
				                                    .setSetLayoutCount(1)
				                                    .setPushConstantRanges(paintingPushConstantRanges)
				                                    .setPSetLayouts(&descCache->dstLayout.get());
				SetPipelineLayoutCI(paintingLayoutCreateInfo);
			}

			pipelineLayout                                   = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
			std::unique_ptr<ComputePipeline> computePipeline = std::make_unique<ENGINE::ComputePipeline>(
			    core->logicalDevice.get(), compShader->sModule->shaderModuleHandle.get(), pipelineLayout.get(),
			    pipelineCache.get());
			pipeline     = std::move(computePipeline->pipelineHandle);
			pipelineType = vk::PipelineBindPoint::eCompute;
		}
		else
		{
			std::cout << "Not viable shader configuration set\n";
		}
	}

	void BuildRenderGraphNode()
	{
		assert(&pipelineLayoutCI != nullptr && "Pipeline layout is null");

		auto pipelineCacheCreateInfo = vk::PipelineCacheCreateInfo();
		pipelineCache                = core->logicalDevice->createPipelineCacheUnique(pipelineCacheCreateInfo);

		Shader *vertShader = shaders.at("vert");
		Shader *fragShader = shaders.at("frag");
		Shader *compShader = shaders.at("comp");
		Shader *tescShader = shaders.at("tesc");
		Shader *teseShader = shaders.at("tese");
		Shader *geomShader = shaders.at("geom");

		if (fragShader && vertShader)
		{
			if (configs.automaticCache)
			{
				descCache.reset();
				descCache = std::make_unique<DescriptorCache>(core);
				descCache->AddShaderInfo(vertShader->sParser.get());
				descCache->AddShaderInfo(fragShader->sParser.get());
				vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eFragment |
				                                  vk::ShaderStageFlagBits::eVertex;
				if (tescShader)
				{
					descCache->AddShaderInfo(tescShader->sParser.get());
					if (!teseShader)
					{
						assert(false && "Must have tese shader");
					}
					descCache->AddShaderInfo(teseShader->sParser.get());
					stageFlags |= vk::ShaderStageFlagBits::eTessellationControl |
					              vk::ShaderStageFlagBits::eTessellationEvaluation;
				}
				if (geomShader)
				{
					descCache->AddShaderInfo(geomShader->sParser.get());
					stageFlags |= vk::ShaderStageFlagBits::eGeometry;
				}

				descCache->BuildDescriptorsCache(stageFlags);

				if (pushConstantSize <= 0)
				{
					SetPushConstantSize(DUMMY_PC_SIZE);
				}
				auto paintingPushConstantRanges = vk::PushConstantRange()
				                                      .setOffset(0)
				                                      .setStageFlags(
				                                          vk::ShaderStageFlagBits::eFragment |
				                                          vk::ShaderStageFlagBits::eVertex)
				                                      .setSize(pushConstantSize);
				auto paintingLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
				                                    .setSetLayoutCount(1)
				                                    .setPushConstantRanges(paintingPushConstantRanges)
				                                    .setPSetLayouts(&descCache->dstLayout.get());

				SetPipelineLayoutCI(paintingLayoutCreateInfo);
			}
			std::vector<vk::Format> colorFormats;
			colorFormats.reserve(colAttachments.size());
			std::vector<vk::RenderingAttachmentInfo> renderingAttachmentInfos;
			for (auto &colAttachment : colAttachments)
			{
				colorFormats.push_back(colAttachment.format);
				renderingAttachmentInfos.push_back(colAttachment.attachmentInfo);
			}
			dynamicRenderPass.SetPipelineRenderingInfo(colAttachments.size(), colorFormats, depthAttachment.format);

			pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);

			std::map<ShaderStage, Shader *> stages;
			GetShaderStages(stages);

			std::unique_ptr<GraphicsPipeline> graphicsPipeline = std::make_unique<ENGINE::GraphicsPipeline>(
			    core->logicalDevice.get(), stages, pipelineLayout.get(),
			    dynamicRenderPass.pipelineRenderingCreateInfo, graphicsPipelineConfigs,
			    colorBlendConfigs, depthConfig,
			    vertexInput, pipelineCache.get());
			pipeline     = std::move(graphicsPipeline->pipelineHandle);
			pipelineType = vk::PipelineBindPoint::eGraphics;
			active       = true;
			std::cout << "Graphics pipeline created\n";
		}
		else if (compShader)
		{
			if (configs.automaticCache)
			{
				descCache.reset();
				descCache = std::make_unique<DescriptorCache>(core);
				descCache->AddShaderInfo(compShader->sParser.get());
				descCache->BuildDescriptorsCache(
				    vk::ShaderStageFlagBits::eCompute);
				if (pushConstantSize <= 0)
				{
					SetPushConstantSize(DUMMY_PC_SIZE);
				}
				auto paintingPushConstantRanges = vk::PushConstantRange()
				                                      .setOffset(0)
				                                      .setStageFlags(
				                                          vk::ShaderStageFlagBits::eCompute)
				                                      .setSize(pushConstantSize);
				auto paintingLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
				                                    .setSetLayoutCount(1)
				                                    .setPushConstantRanges(paintingPushConstantRanges)
				                                    .setPSetLayouts(&descCache->dstLayout.get());
				SetPipelineLayoutCI(paintingLayoutCreateInfo);
			}
			pipelineLayout                                   = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
			std::unique_ptr<ComputePipeline> computePipeline = std::make_unique<ENGINE::ComputePipeline>(
			    core->logicalDevice.get(), compShader->sModule->shaderModuleHandle.get(), pipelineLayout.get(),
			    pipelineCache.get());
			pipeline     = std::move(computePipeline->pipelineHandle);
			pipelineType = vk::PipelineBindPoint::eCompute;
			active       = true;
			std::cout << "Compute pipeline created\n";
		}
		else
		{
			std::cout << "No compute or graphics shaders were set\n";
		}
	}

	void TransitionImages(vk::CommandBuffer commandBuffer)
	{
		for (auto &storageImage : storageImages)
		{
			LayoutPatterns dstPattern = EMPTY;
			switch (pipelineType)
			{
				case vk::PipelineBindPoint::eGraphics:
					dstPattern = GRAPHICS_WRITE;
					break;
				case vk::PipelineBindPoint::eCompute:
					dstPattern = COMPUTE_WRITE;
					break;
				default:
					assert(false && "pipeline type is unknown");
			}
			if (IsImageTransitionNeeded(storageImage.second->imageData->currentLayout, dstPattern))
			{
				TransitionImage(storageImage.second->imageData, dstPattern,
				                storageImage.second->GetSubresourceRange(),
				                commandBuffer);
			}
		}
		for (auto &sampler : sampledImages)
		{
			LayoutPatterns dstPattern = EMPTY;
			switch (pipelineType)
			{
				case vk::PipelineBindPoint::eGraphics:
					dstPattern = GRAPHICS_READ;
					break;
				case vk::PipelineBindPoint::eCompute:
					dstPattern = COMPUTE;
					break;
				default:
					assert(false && "pipeline type is unknown");
			}
			// TODO: Better transition for sampler images
			//  if (IsImageTransitionNeeded(sampler.second->imageData->currentLayout, dstPattern))
			//  {
			//  }
			TransitionImage(sampler.second->imageData, dstPattern, sampler.second->GetSubresourceRange(),
			                commandBuffer);
		}
	}

	void SyncBuffers(vk::CommandBuffer commandBuffer)
	{
		for (auto &pair : buffers)
		{
			BufferKey           buffer     = pair.second;
			BufferAccessPattern srcPattern = GetSrcBufferAccessPattern(buffer.srcUsage);
			BufferAccessPattern dstPattern = GetSrcBufferAccessPattern(buffer.dstUsage);
			CreateMemBarrier(srcPattern, dstPattern, commandBuffer);
		}
	}
	void SetSampler(std::string name, ImageView *imageView, Sampler *sampler = nullptr)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetSampler(name, imageView, sampler);
	}
	void SetSamplerArray(std::string name, std::vector<ImageView *> &imageViews, std::vector<Sampler *> *samplers = nullptr)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetSamplerArray(name, imageViews, samplers);
	}
	void SetStorageImage(std::string name, ImageView *imageView, Sampler *sampler = nullptr)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetStorageImage(name, imageView, sampler);
	}
	void SetStorageImageArray(std::string name, std::vector<ImageView *> &imageViews,
	                          std::vector<Sampler *> *samplers = nullptr)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetStorageImageArray(name, imageViews, samplers);
	}
	template <typename T>
	void SetBuffer(std::string name, T &bufferData)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetBuffer<T>(name, bufferData);
	}
	template <typename T>
	void SetBuffer(std::string name, std::vector<T> &bufferData)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetBuffer<T>(name, bufferData);
	}
	void SetBuffer(std::string name, Buffer *bufferData)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetBuffer(name, bufferData);
	}
	void ReloadShaders()
	{
		Shader *vertShader = shaders.at("vert");
		Shader *fragShader = shaders.at("frag");
		Shader *compShader = shaders.at("comp");
		Shader *tescShader = shaders.at("tesc");
		Shader *teseShader = shaders.at("tese");
		Shader *geomShader = shaders.at("geom");
		if (vertShader && fragShader)
		{
			vertShader->Reload();
			fragShader->Reload();
		}
		if (tescShader)
		{
			tescShader->Reload();
		}
		if (teseShader)
		{
			teseShader->Reload();
		}
		if (geomShader)
		{
			geomShader->Reload();
		}
		if (compShader)
		{
			compShader->Reload();
		}
	}
	void GetShaderStages(std::map<ShaderStage, Shader *> &stages)
	{
		Shader *vertShader = shaders.at("vert");
		Shader *fragShader = shaders.at("frag");
		Shader *compShader = shaders.at("comp");
		Shader *tescShader = shaders.at("tesc");
		Shader *teseShader = shaders.at("tese");
		Shader *geomShader = shaders.at("geom");
		if (vertShader)
		{
			stages.try_emplace(S_VERT, vertShader);
		}
		if (fragShader)
		{
			stages.try_emplace(S_FRAG, fragShader);
		}
		if (compShader)
		{
			stages.try_emplace(S_COMP, compShader);
		}
		if (tescShader)
		{
			stages.try_emplace(S_TESS_CONTROL, tescShader);
		}
		if (teseShader)
		{
			stages.try_emplace(S_TESS_EVAL, teseShader);
		}
		if (geomShader)
		{
			stages.try_emplace(S_GEOM, geomShader);
		}
	}

	void ExecutePass(vk::CommandBuffer commandBuffer)
	{
		dynamicRenderPass.SetViewport(frameBufferSize, frameBufferSize);
		commandBuffer.setViewport(0, 1, &dynamicRenderPass.viewport);
		commandBuffer.setScissor(0, 1, &dynamicRenderPass.scissor);

		assert(imagesAttachmentOutputs.size() == colAttachments.size() && "Not all color attachments were set");
		int                                      index = 0;
		std::vector<vk::RenderingAttachmentInfo> attachmentInfos;
		attachmentInfos.reserve(colAttachments.size());
		for (auto &imagePair : imagesAttachmentOutputs)
		{
			if (IsImageTransitionNeeded(imagePair->imageData->currentLayout, COLOR_ATTACHMENT))
			{
				TransitionImage(imagePair->imageData, COLOR_ATTACHMENT,
				                imagePair->GetSubresourceRange(), commandBuffer);
			}
			colAttachments[index].attachmentInfo.setImageView(imagePair->imageView.get());
			colAttachments[index].attachmentInfo.imageLayout = imagePair->imageData->currentPattern.layout;
			attachmentInfos.push_back(colAttachments[index].attachmentInfo);
			index++;
		}
		if (depthImage)
		{
			if (depthImage->imageData->currentLayout != DEPTH_ATTACHMENT)
			{
				TransitionImage(depthImage->imageData, DEPTH_ATTACHMENT, depthImage->GetSubresourceRange(),
				                commandBuffer);
			}
		}

		TransitionImages(commandBuffer);
		SyncBuffers(commandBuffer);
		if (depthImage != nullptr)
		{
			depthAttachment.attachmentInfo.imageView = depthImage->imageView.get();
		}
		dynamicRenderPass.SetRenderInfo(attachmentInfos, frameBufferSize, &depthAttachment.attachmentInfo);

		commandBuffer.bindPipeline(pipelineType, pipeline.get());
		commandBuffer.beginRendering(dynamicRenderPass.renderInfo);
		commandBuffer.bindDescriptorSets(pipelineType,
		                                 pipelineLayout.get(), 0,
		                                 1,
		                                 &descCache->dstSet, 0, nullptr);
		(*renderOperations)();
		commandBuffer.endRendering();
	}

	void ExecuteCompute(vk::CommandBuffer commandBuffer)
	{
		TransitionImages(commandBuffer);
		SyncBuffers(commandBuffer);
		commandBuffer.bindPipeline(pipelineType, pipeline.get());
		commandBuffer.bindDescriptorSets(pipelineType,
		                                 pipelineLayout.get(), 0,
		                                 1,
		                                 &descCache->dstSet, 0, nullptr);
		(*renderOperations)();
	}

	void Execute(vk::CommandBuffer commandBuffer)
	{
		for (int i = 0; i < tasks.size(); ++i)
		{
			if (tasks[i] != nullptr)
			{
				(*tasks[i])();
			}
		}
		switch (pipelineType)
		{
			case vk::PipelineBindPoint::eGraphics:
				ExecutePass(commandBuffer);
				break;
			case vk::PipelineBindPoint::eCompute:
				ExecuteCompute(commandBuffer);
				break;
			default:
				assert(false && "Unsuported pipeline type");
				break;
		}
	}

	void SetVertexInput(VertexInput vertexInput)
	{
		this->vertexInput = vertexInput;
	}

	void SetFramebufferSize(glm::uvec2 size)
	{
		this->frameBufferSize = size;
	}

	void SetRenderOperation(std::function<void()> *renderOperations)
	{
		if (this->renderOperations)
		{
			delete (this->renderOperations);
		}
		this->renderOperations = renderOperations;
	}

	void AddTask(std::function<void()> *task)
	{
		this->tasks.push_back(task);
	}

	void SetPipelineLayoutCI(vk::PipelineLayoutCreateInfo createInfo)
	{
		this->pipelineLayoutCI = createInfo;
		if (createInfo.pPushConstantRanges != nullptr)
		{
			this->pushConstantRange.offset     = createInfo.pPushConstantRanges->offset;
			this->pushConstantRange.size       = createInfo.pPushConstantRanges->size;
			this->pushConstantRange.stageFlags = createInfo.pPushConstantRanges->stageFlags;
			this->pipelineLayoutCI.setPushConstantRanges(this->pushConstantRange);
		}
	}

	void SetPushConstantSize(size_t size)
	{
		pushConstantSize = size;
	}

	void SetGraphicsPipelineConfigs(GraphicsPipelineConfigs graphicsPipelineConfigs)
	{
		this->graphicsPipelineConfigs = graphicsPipelineConfigs;
	}

	void SetDepthConfig(DepthConfigs dephtConfig)
	{
		this->depthConfig = dephtConfig;
	}

	void SetVertShader(Shader *shader)
	{
		shaders.at("vert") = shader;
	}

	void SetFragShader(Shader *shader)
	{
		shaders.at("frag") = shader;
	}

	void SetCompShader(Shader *shader)
	{
		shaders.at("comp") = shader;
	}

	void SetTesControlShader(Shader *shader)
	{
		shaders.at("tesc") = shader;
	}

	void SetTesEvalShader(Shader *shader)
	{
		shaders.at("tese") = shader;
	}

	void SetGeomShader(Shader *shader)
	{
		shaders.at("geom") = shader;
	}

	void SetVertShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("vert") = shadersProxyRef->at(path).get();
		}
	}

	void SetFragShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("frag") = shadersProxyRef->at(path).get();
		}
	}

	void SetCompShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("comp") = shadersProxyRef->at(path).get();
		}
	}

	void SetTesControlShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("tesc") = shadersProxyRef->at(path).get();
		}
	}

	void SetTesEvalShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("tese") = shadersProxyRef->at(path).get();
		}
	}

	void SetGeomShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("geom") = shadersProxyRef->at(path).get();
		}
	}

	void AddColorAttachmentInput(std::string name)
	{
		if (outColAttachmentsProxyRef->contains(name))
		{
			colAttachments.push_back(outColAttachmentsProxyRef->at(name));
		}
		else
		{
			std::cout << "Attachment input: " << "\"" << name << "\"" << " does not exist";
		}
	}

	void AddColorAttachmentOutput(std::string name, AttachmentInfo attachmentInfo, BlendConfigs blendConfig)
	{
		if (!outColAttachmentsProxyRef->contains(name))
		{
			outColAttachmentsProxyRef->try_emplace(name, attachmentInfo);
			colAttachments.push_back(outColAttachmentsProxyRef->at(name));
			colorBlendConfigs.push_back(blendConfig);
		}
		else
		{
			std::cout << "Attachment: " << "\"" << name << "\"" << " already exist";
		}
	}

	void SetDepthAttachmentInput(std::string name)
	{
		if (outDepthAttachmentProxyRef->contains(name))
		{
			depthAttachment = outDepthAttachmentProxyRef->at(name);
		}
		else
		{
			std::cout << "Attachment input: " << "\"" << name << "\"" << " does not exist";
		}
	}

	void SetDepthAttachmentOutput(std::string name, AttachmentInfo depth)
	{
		if (!outDepthAttachmentProxyRef->contains(name))
		{
			outDepthAttachmentProxyRef->try_emplace(name, depth);
			depthAttachment = outColAttachmentsProxyRef->at(name);
		}
		else
		{
			std::cout << "Attachment: " << "\"" << name << "\"" << " already exist";
		}
	}

	void SetDepthImageResource(std::string name, ImageView *imageView)
	{
		depthImage = imageView;
		AddImageToProxy(name, imageView);
	}

	void ActivateNode(bool value)
	{
		this->active = value;
	}

	void AddColorImageResource(ImageView *imageView)
	{
		assert(imageView && "Name does not exist or image view is null");
		if (!imageAttachmentsNames.contains(imageView->name))
		{
			imageAttachmentsNames.try_emplace(imageView->name, imagesAttachmentOutputs.size());
			imagesAttachmentOutputs.emplace_back(imageView);
		}
		else
		{
			imagesAttachmentOutputs.at(imageAttachmentsNames.at(imageView->name)) = imageView;
		}
		AddImageToProxy(imageView->name, imageView);
	}

	void AddSamplerResource(ImageView *imageView)
	{
		assert(imageView && "Name does not exist or image view is null");
		if (!sampledImages.contains(imageView->name))
		{
			sampledImages.try_emplace(imageView->name, imageView);
		}
		else
		{
			sampledImages.at(imageView->name) = imageView;
		}
		AddImageToProxy(imageView->name, imageView);
	}

	void AddStorageResource(ImageView *imageView)
	{
		assert(imageView && "Name does not exist or image view is null");
		if (!storageImages.contains(imageView->name))
		{
			storageImages.try_emplace(imageView->name, imageView);
		}
		else
		{
			storageImages.at(imageView->name) = imageView;
		}
		AddImageToProxy(imageView->name, imageView);
	}

	void AddBufferSync(BufferKey buffer)
	{
		if (!buffers.contains(buffer.name))
		{
			buffers.try_emplace(buffer.name, buffer);
		}
		else
		{
			buffers.at(buffer.name) = buffer;
		}
		AddBufferToProxy(buffer.name, buffer);
	}
	// We change the image view if the name already exist when using resources
	void AddColorImageResource(std::string name, ImageView *imageView)
	{
		assert(imageView && "Name does not exist or image view is null");
		if (!imageAttachmentsNames.contains(name))
		{
			imageAttachmentsNames.try_emplace(name, imagesAttachmentOutputs.size());
			imagesAttachmentOutputs.emplace_back(imageView);
		}
		else
		{
			imagesAttachmentOutputs.at(imageAttachmentsNames.at(name)) = imageView;
		}
		AddImageToProxy(name, imageView);
	}

	void AddSamplerResource(std::string name, ImageView *imageView)
	{
		assert(imageView && "Name does not exist or image view is null");
		if (!sampledImages.contains(name))
		{
			sampledImages.try_emplace(name, imageView);
		}
		else
		{
			sampledImages.at(name) = imageView;
		}
		AddImageToProxy(name, imageView);
	}

	void AddStorageResource(std::string name, ImageView *imageView)
	{
		assert(imageView && "Name does not exist or image view is null");
		if (!storageImages.contains(name))
		{
			storageImages.try_emplace(name, imageView);
		}
		else
		{
			storageImages.at(name) = imageView;
		}
		AddImageToProxy(name, imageView);
	}

	void AddBufferSync(std::string name, BufferKey buffer)
	{
		if (!buffers.contains(name))
		{
			buffers.try_emplace(name, buffer);
		}
		else
		{
			buffers.at(name) = buffer;
		}
		AddBufferToProxy(name, buffer);
	}

	void DependsOn(std::string dependency)
	{
		if (!dependencies.contains(dependency))
		{
			dependencies.insert(dependency);
			SYSTEMS::Logger::GetInstance()->LogMessage("Renderpass: (" + this->passName + ") Depends On-> (" + dependency + ")");
		}
	}

	void ClearOperations()
	{
		delete renderOperations;
		for (auto &task : tasks)
		{
			delete task;
		}
		renderOperations = nullptr;
		tasks.clear();
	}

	void AddImageToProxy(std::string name, ImageView *imageView)
	{
		if (!imagesProxyRef->contains(name))
		{
			imagesProxyRef->try_emplace(name, imageView);
		}
		else
		{
			imagesProxyRef->at(name) = imageView;
		}
	}

	void AddBufferToProxy(std::string name, BufferKey buffer)
	{
		if (!bufferProxyRef->contains(name))
		{
			bufferProxyRef->try_emplace(name, buffer);
		}
		else
		{
			bufferProxyRef->at(name) = buffer;
		}
	}
	void CreateInternalTexture()
	{
	}
	void RegisterExternalTexture()
	{
	}

	void SetConfigs(RenderNodeConfigs configs)
	{
		this->configs = configs;
	}
	void Reset()
	{
		pipeline.release();
		pipelineLayout.release();
		pipelineCache.release();
		pipelineLayoutCI  = vk::PipelineLayoutCreateInfo();
		pushConstantRange = vk::PushConstantRange();
		pipelineType      = vk::PipelineBindPoint::eGraphics;
		dynamicRenderPass.Reset();
		descCache.release();
		active                                       = false;
		graphicsPipelineConfigs.rasterizationConfigs = R_FILL;
		graphicsPipelineConfigs.topologyConfigs      = T_TRIANGLE;
		colorBlendConfigs.clear();
		depthConfig = D_NONE;
		vertexInput.bindingDescription.clear();
		vertexInput.inputDescription.clear();
		frameBufferSize  = {0, 0};
		pushConstantSize = 0;
		workerQueueRef   = nullptr;

		colAttachments.clear();
		depthAttachment = {};
		depthImage      = nullptr;
		shaders         = {
            {"frag", nullptr},
            {"vert", nullptr},
            {"comp", nullptr},
            {"tesc", nullptr},
            {"tese", nullptr},
            {"geom", nullptr}};

		imagesAttachmentOutputs.clear();
		storageImages.clear();
		sampledImages.clear();
		buffers.clear();
		ClearOperations();
		dependencies.clear();
	}

	vk::UniquePipeline               pipeline;
	vk::UniquePipelineLayout         pipelineLayout;
	vk::UniquePipelineCache          pipelineCache;
	vk::PipelineLayoutCreateInfo     pipelineLayoutCI;
	vk::PushConstantRange            pushConstantRange;
	vk::PipelineBindPoint            pipelineType;
	DynamicRenderPass                dynamicRenderPass;
	std::unique_ptr<DescriptorCache> descCache;
	std::string                      passName;
	WorkerQueue                     *workerQueueRef = nullptr;
	bool                             active         = false;
	// unused
	bool        waitForResourcesCreation = false;
	std::string path;

  private:
	friend class RenderGraph;
	std::unique_ptr<ResourcesManager> localResManager;
	RenderNodeConfigs                 configs                 = {true};
	GraphicsPipelineConfigs           graphicsPipelineConfigs = {};
	std::vector<BlendConfigs>         colorBlendConfigs;
	DepthConfigs                      depthConfig = D_NONE;
	VertexInput                       vertexInput;
	glm::uvec2                        frameBufferSize  = {0, 0};
	size_t                            pushConstantSize = 4;

	std::vector<AttachmentInfo>     colAttachments;
	AttachmentInfo                  depthAttachment = {};
	ImageView                      *depthImage      = nullptr;
	std::map<std::string, Shader *> shaders         = {
        {"frag", nullptr},
        {"vert", nullptr},
        {"comp", nullptr},
        {"tesc", nullptr},
        {"tese", nullptr},
        {"geom", nullptr}};

	std::unordered_map<std::string, int> imageAttachmentsNames;
	std::vector<ImageView *>             imagesAttachmentOutputs;

	std::unordered_map<std::string, ImageView *> storageImages;
	std::unordered_map<std::string, ImageView *> sampledImages;
	std::unordered_map<std::string, BufferKey>   buffers;

	std::function<void()>               *renderOperations = nullptr;
	std::vector<std::function<void()> *> tasks;

	std::set<std::string> dependencies;

	Core                                                     *core;
	ResourcesManager                                         *resManagerRef;
	std::unordered_map<std::string, ImageView *>             *imagesProxyRef;
	std::unordered_map<std::string, BufferKey>               *bufferProxyRef;
	std::unordered_map<std::string, AttachmentInfo>          *outColAttachmentsProxyRef;
	std::unordered_map<std::string, AttachmentInfo>          *outDepthAttachmentProxyRef;
	std::unordered_map<std::string, std::unique_ptr<Shader>> *shadersProxyRef;
	// unused
	std::unordered_map<std::string, std::unique_ptr<DescriptorCache>> descriptorsCachesRef;
};

class RenderGraph
{
  public:
	Core             *core;
	ResourcesManager *resourcesManager;

	ImageView      *currentBackBuffer;
	FrameResources *currentFrameResources;
	size_t          frameIndex;

	std::unordered_map<std::string, std::unique_ptr<RenderGraphNode>> renderNodes;
	std::vector<RenderGraphNode *>                                    sequentialRenderNodes;
	std::vector<RenderGraphNode *>                                    sortedByDepNodes;

	std::unordered_map<std::string, ImageView *>                      imagesProxy;
	std::unordered_map<std::string, BufferKey>                        buffersProxy;
	std::unordered_map<std::string, AttachmentInfo>                   outColAttachmentsProxy;
	std::unordered_map<std::string, AttachmentInfo>                   outDepthAttachmentProxy;
	std::unordered_map<std::string, std::unique_ptr<Shader>>          shadersProxy;
	std::unordered_map<std::string, std::unique_ptr<DescriptorCache>> descCachesProxy;
	std::unordered_map<std::string, int>                              queueOrder;
	std::vector<std::string>                                          OrderedQueuesNames;

	RenderGraph(Core *core)
	{
		this->core = core;
	}

	void SerializeAll()
	{
		for (auto node : sequentialRenderNodes)
		{
			node->Serialize();
		}
	}
	void LoadExternalPasses()
	{
		std::string              nodesPath = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\RenderNodes";
		std::vector<std::string> paths;
		assert(std::filesystem::exists(nodesPath));
		for (auto path : std::filesystem::directory_iterator(nodesPath))
		{
			size_t      pos      = path.path().string().find('_');
			std::string passName = path.path().string().substr(pos + 1, path.path().string().length());
			size_t      extPos   = passName.find_last_of('.');
			passName             = passName.substr(0, extPos);

			RenderGraphNode *node = AddPass(passName);
			if (!renderNodes.contains(node->passName))
			{
				node->Deserialize(path.path().string());
			}
			else
			{
				SYSTEMS::Logger::GetInstance()->LogMessage("Render Node already is in ram");
				node->Deserialize(path.path().string());
			}
		}
		RecreateNodePipelines();
	}
	void UpdateAllFromMetaData()
	{
		for (auto &node : sequentialRenderNodes)
		{
			node->Deserialize(node->path);
			SYSTEMS::Logger::GetInstance()->LogMessage("Render Node: (" + node->passName + ") updated");
		}
	}

	~RenderGraph() {

	};

	void CreateResManager()
	{
		resourcesManager = ResourcesManager::GetInstance();
	}

	RenderGraphNode *GetNode(std::string name)
	{
		if (renderNodes.contains(name))
		{
			return renderNodes.at(name).get();
		}
		else
		{
			PrintInvalidResource("Renderpass", name);
			return nullptr;
		}
	}

	RenderGraphNode *AddPass(std::string name, WorkerQueue *workerQueue = nullptr)
	{
		if (!renderNodes.contains(name))
		{
			auto renderGraphNode                        = std::make_unique<RenderGraphNode>();
			renderGraphNode->passName                   = name;
			renderGraphNode->imagesProxyRef             = &imagesProxy;
			renderGraphNode->outColAttachmentsProxyRef  = &outColAttachmentsProxy;
			renderGraphNode->outDepthAttachmentProxyRef = &outColAttachmentsProxy;
			renderGraphNode->bufferProxyRef             = &buffersProxy;
			renderGraphNode->shadersProxyRef            = &shadersProxy;
			renderGraphNode->core                       = core;
			renderGraphNode->resManagerRef              = resourcesManager;
			renderGraphNode->workerQueueRef             = workerQueue;
			renderGraphNode->path                       = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			renderGraphNode->localResManager = std::make_unique<ResourcesManager>(core);
			renderNodes.try_emplace(name, std::move(renderGraphNode));
			sequentialRenderNodes.push_back(renderNodes.at(name).get());
			return renderNodes.at(name).get();
		}
		return renderNodes.at(name).get();
	}

	DescriptorCache *AddDescCache(std::string name)
	{
		if (descCachesProxy.contains(name))
		{
			return descCachesProxy.at(name).get();
		}
		else
		{
			descCachesProxy.try_emplace(name, std::make_unique<DescriptorCache>(core));
			return descCachesProxy.at(name).get();
		}
	}

	ImageView *AddColorImageResource(std::string passName, std::string name, ImageView *imageView)
	{
		assert(imageView && "ImageView is null");
		if (!imagesProxy.contains(name))
		{
			imagesProxy.try_emplace(name, imageView);
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->AddColorImageResource(name, imageView);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
		}
		else
		{
			imagesProxy.at(name) = imageView;
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->AddColorImageResource(name, imageView);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
			// std::cout << "Image with name: \"" << name << "\" has changed \n";
		}
		return imageView;
	}

	ImageView *SetDepthImageResource(std::string passName, std::string name, ImageView *imageView)
	{
		assert(imageView && "ImageView is null");
		if (!imagesProxy.contains(name))
		{
			imagesProxy.try_emplace(name, imageView);
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->SetDepthImageResource(name, imageView);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
		}
		else
		{
			imagesProxy.at(name) = imageView;
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->SetDepthImageResource(name, imageView);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
			// std::cout << "Image with name: \"" << name << "\" has changed \n";
		}
		return imageView;
	}

	ImageView *AddSamplerResource(std::string passName, std::string name, ImageView *imageView)
	{
		assert(imageView && "ImageView is null");
		if (!imagesProxy.contains(name))
		{
			imagesProxy.try_emplace(name, imageView);
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->AddSamplerResource(name, imageView);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
		}
		else
		{
			imagesProxy.at(name) = imageView;
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->AddSamplerResource(name, imageView);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
			// std::cout << "Image with name: \"" << name << "\" has changed \n";
		}
		return imageView;
	}

	ImageView *AddStorageResource(std::string passName, std::string name, ImageView *imageView)
	{
		assert(imageView && "ImageView is null");
		if (!imagesProxy.contains(name))
		{
			imagesProxy.try_emplace(name, imageView);
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->AddStorageResource(name, imageView);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
		}
		else
		{
			imagesProxy.at(name) = imageView;
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->AddStorageResource(name, imageView);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
			// std::cout << "Image with name: \"" << name << "\" has changed \n";
		}
		return imageView;
	}

	BufferKey &AddBufferSync(std::string passName, std::string name, BufferKey buffer)
	{
		assert(buffer.buffer && "ImageView is null");
		if (!buffersProxy.contains(name))
		{
			buffersProxy.try_emplace(name, buffer);
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->AddBufferSync(name, buffer);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
		}
		else
		{
			buffersProxy.at(name) = buffer;
			if (renderNodes.contains(passName))
			{
				renderNodes.at(passName)->AddBufferSync(name, buffer);
			}
			else
			{
				std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
			}
			// std::cout << "Image with name: \"" << name << "\" has changed \n";
		}
		return buffer;
	}

	ImageView *GetImageResource(std::string name)
	{
		if (imagesProxy.contains(name))
		{
			return imagesProxy.at(name);
		}
		PrintInvalidResource("Resource", name);
		return nullptr;
	}

	BufferKey &GetBufferResource(std::string name)
	{
		if (buffersProxy.contains(name))
		{
			return buffersProxy.at(name);
		}
		PrintInvalidResource("Resource", name);
		assert(false && "Invalid name requested");
	}

	void RecreateFrameResources()
	{
		for (auto &renderNode : renderNodes)
		{
			renderNode.second->ClearOperations();
		}
	}
	void RecreateNodePipelines()
	{
		for (auto &node : renderNodes)
		{
			node.second->RecreateResources();
		}
		SYSTEMS::Logger::GetInstance()->LogMessage("Graphics Pipelines Recreated");
	}

	void DebugShadersCompilation()
	{
		int result = std::system(
		    "C:\\Users\\carlo\\CLionProjects\\Vulkan_Engine_Template\\src\\shaders\\compile.bat");
		if (result == 0)
		{
			std::cout << "Shaders compiled\n";
		}
		else
		{
			assert(false && "reload shaders failed");
		}
	}
	void SortNodesByDep()
	{
		std::vector<std::string> solvedNodesOrdered;
		std::set<std::string>    solvedNodesNames;
		solvedNodesOrdered.reserve(sequentialRenderNodes.size());
		for (auto &node : sequentialRenderNodes)
		{
			if (node->dependencies.empty())
			{
				solvedNodesOrdered.emplace_back(node->passName);
				if (!solvedNodesNames.contains(node->passName))
				{
					solvedNodesNames.insert(node->passName);
				}
			}
		}

		int idx        = 0;
		int currSearch = 0;
		int maxSearch  = 10000;
		while (solvedNodesOrdered.size() < sequentialRenderNodes.size() && currSearch <= maxSearch)
		{
			auto &node = sequentialRenderNodes[idx];
			if (solvedNodesNames.contains(node->passName))
			{
				currSearch++;
				idx = (idx + 1) % sequentialRenderNodes.size();
				continue;
			}
			int toSolveDep = node->dependencies.size();
			int solvedDep  = 0;
			for (auto &depName : node->dependencies)
			{
				if (solvedNodesNames.contains(depName))
				{
					solvedDep++;
				}
			}
			if (toSolveDep == solvedDep)
			{
				if (!solvedNodesNames.contains(node->passName))
				{
					solvedNodesOrdered.emplace_back(node->passName);
					solvedNodesNames.insert(node->passName);
				}
			}
			currSearch++;
			idx = (idx + 1) % sequentialRenderNodes.size();
		}
		bool reorderDone = solvedNodesOrdered.size() == sequentialRenderNodes.size();
		sortedByDepNodes.clear();
		sortedByDepNodes.reserve(solvedNodesOrdered.size());
		for (int i = 0; i < solvedNodesOrdered.size(); ++i)
		{
			sortedByDepNodes.emplace_back(GetNode(solvedNodesOrdered[i]));
		}
	}
	void SortQueueSubmition(std::vector<RenderGraphNode> &renderGraphNodes)
	{
		for (int i = 0; i < sequentialRenderNodes.size(); ++i)
		{
			// return a queue order based on the dependancies
		}
	}

	void ResolveNodesDependancies()
	{
		for (int i = sequentialRenderNodes.size() - 1; i >= 0; i--)
		{
			RenderGraphNode *node = sequentialRenderNodes[i];
			for (int j = i - 1; j >= 0; j--)
			{
				RenderGraphNode *toCheckNode = sequentialRenderNodes[j];

				for (auto &image : toCheckNode->imageAttachmentsNames)
				{
					if (node->imageAttachmentsNames.contains(image.first))
					{
						node->DependsOn(toCheckNode->passName);
						break;
					}
					if (node->sampledImages.contains(image.first))
					{
						node->DependsOn(toCheckNode->passName);
						break;
					}
				}
				for (auto &image : toCheckNode->sampledImages)
				{
					if (node->sampledImages.contains(image.first))
					{
						node->DependsOn(toCheckNode->passName);
						break;
					}
				}
				for (auto &image : toCheckNode->storageImages)
				{
					if (node->storageImages.contains(image.first))
					{
						node->DependsOn(toCheckNode->passName);
						break;
					}
				}
				for (auto &buffer : toCheckNode->buffers)
				{
					if (node->buffers.contains(buffer.first))
					{
						node->DependsOn(toCheckNode->passName);
						break;
					}
				}
			}
		}
	}

	void ExecuteRendering()
	{
		assert(currentFrameResources && "Current frame reference is null");
		ResolveNodesDependancies();
		SortNodesByDep();

		std::vector<std::string> allPassesNames;
		int                      idx = 0;
		for (auto &renderNode : sortedByDepNodes)
		{
			// Profiler::GetInstance()->
			// AddProfilerCpuSpot(legit::Colors::getColor(idx), "Rp: " + renderNode->passName);
			if (!renderNode->active)
			{
				// Profiler::GetInstance()->EndProfilerCpuSpot("Rp: " + renderNode->passName);
				continue;
			}

			RenderGraphNode *node      = renderNode;
			bool             depenNeed = false;
			std::string      depenName = "";
			for (auto &passName : allPassesNames)
			{
				if (node->dependencies.contains(passName))
				{
					depenNeed = true;
					depenName = passName;
				}
			}
			if (depenNeed)
			{
				RenderGraphNode *depenNode = renderNodes.at(depenName).get();
				if (!depenNode->active)
				{
					// if a dependency is not active we skip the node
					continue;
				}
				BufferUsageTypes    lastNodeType    = (depenNode->pipelineType == vk::PipelineBindPoint::eGraphics) ? B_GRAPHICS_WRITE : B_COMPUTE_WRITE;
				BufferUsageTypes    currNodeType    = (node->pipelineType == vk::PipelineBindPoint::eGraphics) ? B_GRAPHICS_WRITE : B_COMPUTE_WRITE;
				BufferAccessPattern lastNodePattern = GetSrcBufferAccessPattern(lastNodeType);
				BufferAccessPattern currNodePattern = GetSrcBufferAccessPattern(currNodeType);
				CreateMemBarrier(lastNodePattern, currNodePattern, currentFrameResources->commandBuffer.get());
			}
			if (node->workerQueueRef == nullptr)
			{
				node->Execute(currentFrameResources->commandBuffer.get());
			}
			else
			{
				std::string           name = node->passName;
				std::function<void()> nodeTask([name, this] {
					renderNodes.at(name)->Execute(renderNodes.at(name)->workerQueueRef->commandBuffer.get());
				});
				node->workerQueueRef->taskThreat.AddTask(nodeTask);
			}
			// Profiler::GetInstance()->EndProfilerCpuSpot("Rp: " + renderNode->passName);
			allPassesNames.push_back(node->passName);
			idx = (idx + 1) % 16;
		}
	}

	void BuildDeserializedPasses()
	{
	}
};

}        // namespace ENGINE

#endif        // RENDERGRAPH_HPP
