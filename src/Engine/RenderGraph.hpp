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
enum class RenderNodeType
{
	NONE,
	GRAPHICS,
	COMPUTE,
	CUDA
};


struct RenderNodeConfigs : SYSTEMS::ISerializable<RenderNodeConfigs>
{
	bool automaticCache = true;
	bool manualAdd      = false;

	RenderNodeConfigs(bool automaticCache, bool manualAdd = false)
	{
		this->automaticCache = automaticCache;
		this->manualAdd      = manualAdd;
	}

	nlohmann::json Serialize() override
	{
		nlohmann::json json;
		json["automaticCache"] = automaticCache;
		json["manualAdd"]      = manualAdd;

		return json;
	}
	RenderNodeConfigs *Deserialize(std::string filename) override
	{
		return this;
	}
};

class RenderGraph;

struct CUDAPipeline
{
	CodeCuda::CodeCudaContext *context = nullptr;
	Core                      *core    = nullptr;
	
	CUDAPipeline() = default;
	CUDAPipeline *BuildCUDAPipeline()
	{
		assert(context != nullptr);
		return this;
	}
	CUDAPipeline *ExportBuffer(Buffer *buffer)
	{
		assert(context);
		assert(buffer->GetBufferHandle());
		context->C_ImportExternalBuffer(buffer->GetBufferHandle(), buffer->deviceSize);
		return this;
	}
	CUDAPipeline *CreatePipeline()
	{
		return this;
	}
	~CUDAPipeline() = default;
};
struct GPUPipeline
{
	GPUPipeline()  = default;
	~GPUPipeline() = default;

	Core                        *core;
	vk::UniquePipeline           pipeline;
	vk::UniquePipelineLayout     pipelineLayout;
	vk::UniquePipelineCache      pipelineCache;
	vk::PipelineLayoutCreateInfo pipelineLayoutCI;
	vk::PushConstantRange        pushConstantRange;
	vk::PipelineBindPoint        pipelineType;

	DynamicRenderPass                dynamicRenderPass;
	std::unique_ptr<DescriptorCache> descCache;

	GraphicsPipelineConfigs                                   graphicsPipelineConfigs = {};
	VertexInput                                               vertexInput;
	std::vector<BlendConfigs>                                 colorBlendConfigs;
	std::vector<AttachmentInfo>                               colAttachments;
	AttachmentInfo                                            depthAttachment      = {};
	AttachmentInfo                                            depthAttachmentInput = {};
	size_t                                                    pushConstantSize     = DUMMY_PC_SIZE;
	DepthConfigs                                              depthConfig          = D_NONE;
	RenderNodeConfigs                                         configs              = {true, false};
	std::unordered_map<std::string, std::unique_ptr<Shader>> *shadersProxyRef;
	std::unordered_map<std::string, AttachmentInfo>           outColAttachmentsProxyRef;
	std::unordered_map<std::string, AttachmentInfo>           outDepthAttachmentProxyRef;
	std::string                                               name;

	std::map<std::string, Shader *> shaders = {
	    {"frag", nullptr},
	    {"vert", nullptr},
	    {"comp", nullptr},
	    {"tesc", nullptr},
	    {"tese", nullptr},
	    {"geom", nullptr}};

	GPUPipeline *RecreateResources()
	{
		assert(&pipelineLayoutCI != nullptr && "Pipeline layout is null");
		assert(core != nullptr && "Core is null");

		pipeline.reset();
		pipelineLayout.reset();

		ReloadShaders();

		Shader *vertShader = shaders.at("vert");
		Shader *fragShader = shaders.at("frag");
		Shader *compShader = shaders.at("comp");
		Shader *tescShader = shaders.at("tesc");
		Shader *teseShader = shaders.at("tese");
		Shader *geomShader = shaders.at("geom");

		std::map<ShaderStage, Shader *> stages;
		GetShaderStages(stages);

		if (fragShader && vertShader)
		{
			if (configs.automaticCache)
			{
				descCache->AddShaderInfo(vertShader->sParser.get());
				descCache->AddShaderInfo(fragShader->sParser.get());

				vk::ShaderStageFlags stageFlags =
				    vk::ShaderStageFlagBits::eFragment |
				    vk::ShaderStageFlagBits::eVertex;

				if (tescShader)
				{
					descCache->AddShaderInfo(tescShader->sParser.get());

					if (!teseShader)
					{
						assert(false && "Must have tese shader");
					}

					descCache->AddShaderInfo(teseShader->sParser.get());

					stageFlags |=
					    vk::ShaderStageFlagBits::eTessellationControl |
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

				auto paintingPushConstantRanges =
				    vk::PushConstantRange()
				        .setOffset(0)
				        .setStageFlags(
				            vk::ShaderStageFlagBits::eVertex |
				            vk::ShaderStageFlagBits::eFragment)
				        .setSize(pushConstantSize);

				auto paintingLayoutCreateInfo =
				    vk::PipelineLayoutCreateInfo()
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

			dynamicRenderPass.SetPipelineRenderingInfo(
			    colAttachments.size(),
			    colorFormats,
			    depthAttachment.format);

			pipelineLayout =
			    core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);

			std::unique_ptr<GraphicsPipeline> graphicsPipeline =
			    std::make_unique<GraphicsPipeline>(
			        core->logicalDevice.get(),
			        stages,
			        pipelineLayout.get(),
			        dynamicRenderPass.pipelineRenderingCreateInfo,
			        graphicsPipelineConfigs,
			        colorBlendConfigs,
			        depthConfig,
			        vertexInput,
			        pipelineCache.get());

			pipeline        = std::move(graphicsPipeline->pipelineHandle);
			pipelineType    = vk::PipelineBindPoint::eGraphics;
		}
		else if (compShader)
		{
			if (configs.automaticCache)
			{
				descCache->AddShaderInfo(compShader->sParser.get());

				descCache->BuildDescriptorsCache(
				    vk::ShaderStageFlagBits::eCompute);

				if (pushConstantSize <= 0)
				{
					SetPushConstantSize(DUMMY_PC_SIZE);
				}

				auto paintingPushConstantRanges =
				    vk::PushConstantRange()
				        .setOffset(0)
				        .setStageFlags(vk::ShaderStageFlagBits::eCompute)
				        .setSize(pushConstantSize);

				auto paintingLayoutCreateInfo =
				    vk::PipelineLayoutCreateInfo()
				        .setSetLayoutCount(1)
				        .setPushConstantRanges(paintingPushConstantRanges)
				        .setPSetLayouts(&descCache->dstLayout.get());

				SetPipelineLayoutCI(paintingLayoutCreateInfo);
			}

			pipelineLayout =
			    core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);

			std::unique_ptr<ComputePipeline> computePipeline =
			    std::make_unique<ComputePipeline>(
			        core->logicalDevice.get(),
			        compShader->sModule->shaderModuleHandle.get(),
			        pipelineLayout.get(),
			        pipelineCache.get());

			pipeline        = std::move(computePipeline->pipelineHandle);
			pipelineType    = vk::PipelineBindPoint::eCompute;
		}
		else
		{
			std::cout << "Not viable shader configuration set\n";
		}
		return this;
	}

	GPUPipeline *BuildGPUPipeline()
	{
		assert(&pipelineLayoutCI != nullptr && "Pipeline layout is null");
		assert(core != nullptr && "Core is null");

		auto pipelineCacheCreateInfo = vk::PipelineCacheCreateInfo();

		pipelineCache =
		    core->logicalDevice->createPipelineCacheUnique(
		        pipelineCacheCreateInfo);

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

				vk::ShaderStageFlags stageFlags =
				    vk::ShaderStageFlagBits::eFragment |
				    vk::ShaderStageFlagBits::eVertex;

				if (tescShader)
				{
					descCache->AddShaderInfo(tescShader->sParser.get());

					if (!teseShader)
					{
						assert(false && "Must have tese shader");
					}

					descCache->AddShaderInfo(teseShader->sParser.get());

					stageFlags |=
					    vk::ShaderStageFlagBits::eTessellationControl |
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

				auto paintingPushConstantRanges =
				    vk::PushConstantRange()
				        .setOffset(0)
				        .setStageFlags(
				            vk::ShaderStageFlagBits::eFragment |
				            vk::ShaderStageFlagBits::eVertex)
				        .setSize(pushConstantSize);

				auto paintingLayoutCreateInfo =
				    vk::PipelineLayoutCreateInfo()
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

			dynamicRenderPass.SetPipelineRenderingInfo(
			    colAttachments.size(),
			    colorFormats,
			    depthAttachment.format);

			pipelineLayout =
			    core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);

			std::map<ShaderStage, Shader *> stages;
			GetShaderStages(stages);

			std::unique_ptr<GraphicsPipeline> graphicsPipeline =
			    std::make_unique<GraphicsPipeline>(
			        core->logicalDevice.get(),
			        stages,
			        pipelineLayout.get(),
			        dynamicRenderPass.pipelineRenderingCreateInfo,
			        graphicsPipelineConfigs,
			        colorBlendConfigs,
			        depthConfig,
			        vertexInput,
			        pipelineCache.get());

			pipeline        = std::move(graphicsPipeline->pipelineHandle);
			pipelineType    = vk::PipelineBindPoint::eGraphics;

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

				auto paintingPushConstantRanges =
				    vk::PushConstantRange()
				        .setOffset(0)
				        .setStageFlags(vk::ShaderStageFlagBits::eCompute)
				        .setSize(pushConstantSize);

				auto paintingLayoutCreateInfo =
				    vk::PipelineLayoutCreateInfo()
				        .setSetLayoutCount(1)
				        .setPushConstantRanges(paintingPushConstantRanges)
				        .setPSetLayouts(&descCache->dstLayout.get());

				SetPipelineLayoutCI(paintingLayoutCreateInfo);
			}

			pipelineLayout =
			    core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);

			std::unique_ptr<ComputePipeline> computePipeline =
			    std::make_unique<ComputePipeline>(
			        core->logicalDevice.get(),
			        compShader->sModule->shaderModuleHandle.get(),
			        pipelineLayout.get(),
			        pipelineCache.get());

			pipeline        = std::move(computePipeline->pipelineHandle);
			pipelineType    = vk::PipelineBindPoint::eCompute;

			std::cout << "Compute pipeline created\n";
		}
		else
		{
			std::cout << "No compute or graphics shaders were set\n";
		}
		return this;
	}

	GPUPipeline *SetSampler(std::string name, ImageView *imageView, Sampler *sampler = nullptr)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetSampler(name, imageView, sampler);
		return this;
	}
	GPUPipeline *SetSamplerArray(std::string name, std::vector<ImageView *> &imageViews, std::vector<Sampler *> *samplers = nullptr)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetSamplerArray(name, imageViews, samplers);
		return this;
	}
	GPUPipeline *SetStorageImage(std::string name, ImageView *imageView, Sampler *sampler = nullptr)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetStorageImage(name, imageView, sampler);
		return this;
	}
	GPUPipeline *SetStorageImageArray(std::string name, std::vector<ImageView *> &imageViews,
	                                  std::vector<Sampler *> *samplers = nullptr)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetStorageImageArray(name, imageViews, samplers);
		return this;
	}
	template <typename T>
	GPUPipeline *SetBuffer(std::string name, T &bufferData)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetBuffer<T>(name, bufferData);
		return this;
	}
	template <typename T>
	GPUPipeline *SetBuffer(std::string name, std::vector<T> &bufferData)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetBuffer<T>(name, bufferData);
		return this;
	}
	GPUPipeline *SetBuffer(std::string name, Buffer *bufferData)
	{
		assert(descCache && " Is not possible to set a shader value before building the node");
		descCache->SetBuffer(name, bufferData);
		return this;
	}

	GPUPipeline *SetPipelineLayoutCI(vk::PipelineLayoutCreateInfo createInfo)
	{
		this->pipelineLayoutCI = createInfo;

		if (createInfo.pPushConstantRanges != nullptr)
		{
			this->pushConstantRange.offset =
			    createInfo.pPushConstantRanges->offset;

			this->pushConstantRange.size =
			    createInfo.pPushConstantRanges->size;

			this->pushConstantRange.stageFlags =
			    createInfo.pPushConstantRanges->stageFlags;

			this->pipelineLayoutCI.setPushConstantRanges(
			    this->pushConstantRange);
		}
		return this;
	}

	GPUPipeline *SetPushConstantSize(size_t size)
	{
		pushConstantSize = size;
		return this;
	}

	GPUPipeline *SetGraphicsPipelineConfigs(
	    GraphicsPipelineConfigs graphicsPipelineConfigs)
	{
		this->graphicsPipelineConfigs = graphicsPipelineConfigs;
		return this;
	}

	GPUPipeline *SetDepthConfig(DepthConfigs dephtConfig)
	{
		this->depthConfig = dephtConfig;
		return this;
	}

	GPUPipeline *SetConfigs(RenderNodeConfigs configs)
	{
		this->configs = configs;
		return this;
	}
	GPUPipeline *SetVertexInput(VertexInput vertexInput)
	{
		this->vertexInput = vertexInput;
		return this;
	}

	GPUPipeline *SetVertShader(Shader *shader)
	{
		shaders.at("vert") = shader;
		return this;
	}

	GPUPipeline *SetFragShader(Shader *shader)
	{
		shaders.at("frag") = shader;
		return this;
	}

	GPUPipeline *SetCompShader(Shader *shader)
	{
		shaders.at("comp") = shader;
		return this;
	}

	GPUPipeline *SetTesControlShader(Shader *shader)
	{
		shaders.at("tesc") = shader;
		return this;
	}

	GPUPipeline *SetTesEvalShader(Shader *shader)
	{
		shaders.at("tese") = shader;
		return this;
	}

	GPUPipeline *SetGeomShader(Shader *shader)
	{
		shaders.at("geom") = shader;
		return this;
	}

	bool ReloadShaders()
	{
		Shader *vertShader = shaders.at("vert");
		Shader *fragShader = shaders.at("frag");
		Shader *compShader = shaders.at("comp");
		Shader *tescShader = shaders.at("tesc");
		Shader *teseShader = shaders.at("tese");
		Shader *geomShader = shaders.at("geom");

		// todo
		bool isReloaded = false;
		if (vertShader != nullptr && fragShader != nullptr)
		{
			vertShader->Reload();
			fragShader->Reload();
		}

		if (tescShader != nullptr)
		{
			tescShader->Reload();
		}

		if (teseShader != nullptr)
		{
			teseShader->Reload();
		}

		if (geomShader != nullptr)
		{
			geomShader->Reload();
		}

		if (compShader != nullptr)
		{
			compShader->Reload();
		}
		return isReloaded;
	}

	GPUPipeline *GetShaderStages(std::map<ShaderStage, Shader *> &stages)
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
		return this;
	}

	GPUPipeline *SetVertShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("vert") = shadersProxyRef->at(path).get();
		}
		return this;
	}

	GPUPipeline *SetFragShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("frag") = shadersProxyRef->at(path).get();
		}
		return this;
	}

	GPUPipeline *SetCompShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("comp") = shadersProxyRef->at(path).get();
		}
		return this;
	}

	GPUPipeline *SetTesControlShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("tesc") = shadersProxyRef->at(path).get();
		}
		return this;
	}

	GPUPipeline *SetTesEvalShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("tese") = shadersProxyRef->at(path).get();
		}
		return this;
	}

	GPUPipeline *SetGeomShader_IMode(std::string path)
	{
		if (shadersProxyRef->contains(path))
		{
			shaders.at("geom") = shadersProxyRef->at(path).get();
		}
		return this;
	}

	GPUPipeline *AddColorAttachmentInput(std::string name)
	{
		if (!outColAttachmentsProxyRef.contains(name))
		{
			colAttachments.push_back(outColAttachmentsProxyRef.at(name));
		}
		else
		{
			std::cout << "Attachment input: " << "\"" << name << "\"" << " does not exist";
		}
		return this;
	}

	GPUPipeline *AddColorAttachmentOutput(std::string name, AttachmentInfo attachmentInfo, BlendConfigs blendConfig)
	{
		if (!outColAttachmentsProxyRef.contains(name))
		{
			outColAttachmentsProxyRef.try_emplace(name, attachmentInfo);
			colAttachments.push_back(outColAttachmentsProxyRef.at(name));
			colorBlendConfigs.push_back(blendConfig);
		}
		else
		{
			std::cout << "Attachment: " << "\"" << name << "\"" << " already exist";
		}
		return this;
	}
	GPUPipeline *SetDepthAttachmentInput(std::string name)
	{
		if (!outDepthAttachmentProxyRef.contains(name))
		{
			depthAttachment = outDepthAttachmentProxyRef.at(name);
		}
		else
		{
			std::cout << "Attachment input: " << "\"" << name << "\"" << " does not exist";
		}
		return this;
	}

	GPUPipeline *SetDepthAttachmentOutput(std::string name, AttachmentInfo depth)
	{
		depthAttachment = depth;
		return this;
	}

	GPUPipeline *Reset()
	{
		pipeline.reset();
		pipelineLayout.reset();
		pipelineCache.reset();
		descCache.reset();
		pipelineLayoutCI  = vk::PipelineLayoutCreateInfo();
		pushConstantRange = vk::PushConstantRange();
		pipelineType      = vk::PipelineBindPoint::eGraphics;

		dynamicRenderPass.Reset();
		graphicsPipelineConfigs.rasterizationConfigs = R_FILL;
		graphicsPipelineConfigs.topologyConfigs      = T_TRIANGLE;
		colorBlendConfigs.clear();
		depthConfig = D_NONE;
		vertexInput.bindingDescription.clear();
		vertexInput.inputDescription.clear();
		pushConstantSize = 4;
		colAttachments.clear();
		depthAttachment = {};
		return this;
	}
};

struct RenderGraphNode : SYSTEMS::ISerializable<RenderGraphNode>
{
	RenderGraphNode() = default;

	nlohmann::json Serialize() override
	{
		nlohmann::json json;
		std::string    text     = json.dump(3);
		std::string    filename = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\RenderNodes\\pass_" + passName + ".json";
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
		return this;
	}
	RenderGraphNode *RecreateResources()
	{
		GPUPipelineRef->RecreateResources();
		return this;
	}

	RenderGraphNode *BuildRenderGraphNode()
	{
		if (GPUPipelineRef)
		{
			GPUPipelineRef->BuildGPUPipeline();
			if (GPUPipelineRef->pipelineType == vk::PipelineBindPoint::eGraphics)
			{
				nodeType =RenderNodeType::GRAPHICS; 
			}else
			{
				nodeType =RenderNodeType::COMPUTE; 
			}
		}else
		{
			nodeType = RenderNodeType::CUDA;
			CUDAPipeline->BuildCUDAPipeline();
		}
		return this;
	}

	RenderGraphNode *EnqueueNode()
	{
		return this;
	}
	
	vk::PipelineBindPoint GetGPUPipelineType()
	{
		assert(CUDAPipeline == nullptr &&"this call should not happen in a cuda pipeline");
		return GPUPipelineRef->pipelineType;
	}

	RenderGraphNode *TransitionImages(vk::CommandBuffer commandBuffer)
	{
		for (auto &storageImage : storageImages)
		{
			LayoutPatterns dstPattern = EMPTY;
			switch (nodeType)
			{
				case RenderNodeType::GRAPHICS:
					dstPattern = GRAPHICS_WRITE;
					break;
				case RenderNodeType::COMPUTE:
					dstPattern = COMPUTE_WRITE;
					break;
				case RenderNodeType::CUDA:
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
			switch (nodeType)
			{
				case RenderNodeType::GRAPHICS:
					dstPattern = GRAPHICS_READ;
					break;
				case RenderNodeType::COMPUTE:
					dstPattern = COMPUTE;
					break;
				case RenderNodeType::CUDA:
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
		return this;
	}

	RenderGraphNode *SyncBuffers(vk::CommandBuffer commandBuffer)
	{
		for (auto &pair : buffers)
		{
			BufferKey           buffer     = pair.second;
			BufferAccessPattern srcPattern = GetSrcBufferAccessPattern(buffer.srcUsage);
			BufferAccessPattern dstPattern = GetSrcBufferAccessPattern(buffer.dstUsage);
			CreateMemBarrier(srcPattern, dstPattern, commandBuffer);
		}
		return this;
	}
	RenderGraphNode *SetSampler(std::string name, ImageView *imageView, Sampler *sampler = nullptr)
	{
		GPUPipelineRef->SetSampler(name, imageView, sampler);
		return this;
	}
	RenderGraphNode *SetSamplerArray(std::string name, std::vector<ImageView *> &imageViews, std::vector<Sampler *> *samplers = nullptr)
	{
		GPUPipelineRef->SetSamplerArray(name, imageViews, samplers);
		return this;
	}
	RenderGraphNode *SetStorageImage(std::string name, ImageView *imageView, Sampler *sampler = nullptr)
	{
		GPUPipelineRef->SetStorageImage(name, imageView, sampler);
		return this;
	}
	RenderGraphNode *SetStorageImageArray(std::string name, std::vector<ImageView *> &imageViews,
	                                      std::vector<Sampler *> *samplers = nullptr)
	{
		GPUPipelineRef->SetStorageImageArray(name, imageViews, samplers);
		return this;
	}
	template <typename T>
	RenderGraphNode *SetBuffer(std::string name, T &bufferData)
	{
		GPUPipelineRef->SetBuffer<T>(name, bufferData);
		return this;
	}
	template <typename T>
	RenderGraphNode *SetBuffer(std::string name, std::vector<T> &bufferData)
	{
		GPUPipelineRef->SetBuffer<T>(name, bufferData);
		return this;
	}
	RenderGraphNode *SetBuffer(std::string name, Buffer *bufferData)
	{
		GPUPipelineRef->SetBuffer(name, bufferData);
		return this;
	}
	RenderGraphNode *ReloadShaders()
	{
		GPUPipelineRef->ReloadShaders();
		return this;
	}
	RenderGraphNode *ValidateNodeType()
	{
		assert(!(GPUPipelineRef && CUDAPipeline) && "you only can have either gpu or cuda pipeline not both");
		if (nodeType != RenderNodeType::NONE)
		{
			return this;
		}
		if (GPUPipelineRef == nullptr)
		{
			nodeType = RenderNodeType::CUDA;
			return this;
		}
		if (GPUPipelineRef->pipelineType == vk::PipelineBindPoint::eCompute)
		{
			nodeType = RenderNodeType::COMPUTE;
		}else
		{
			nodeType = RenderNodeType::GRAPHICS;
		}
		return this;
		
	}
	

	RenderGraphNode *ExecutePass(vk::CommandBuffer commandBuffer)
	{
		assert(imagesAttachmentOutputs.size() == GPUPipelineRef->colAttachments.size() && "Not all color attachments were set");
		assert(!imagesAttachmentOutputs.empty() && "No color attachaments were set");
		assert(!(GPUPipelineRef->depthAttachment.format != vk::Format::eUndefined && depthImage == nullptr) && "there is no depth attachment set");

		SetFramebufferSize(imagesAttachmentOutputs[0]->imageData->GetImageSize());
		GPUPipelineRef->dynamicRenderPass.SetViewport(frameBufferSize, frameBufferSize);
		commandBuffer.setViewport(0, 1, &GPUPipelineRef->dynamicRenderPass.viewport);
		commandBuffer.setScissor(0, 1, &GPUPipelineRef->dynamicRenderPass.scissor);

		int                                      index = 0;
		std::vector<vk::RenderingAttachmentInfo> attachmentInfos;
		attachmentInfos.reserve(GPUPipelineRef->colAttachments.size());
		for (auto &imagePair : imagesAttachmentOutputs)
		{
			if (IsImageTransitionNeeded(imagePair->imageData->currentLayout, COLOR_ATTACHMENT))
			{
				TransitionImage(imagePair->imageData, COLOR_ATTACHMENT,
				                imagePair->GetSubresourceRange(), commandBuffer);
			}
			GPUPipelineRef->colAttachments[index].attachmentInfo.setImageView(imagePair->imageView.get());
			GPUPipelineRef->colAttachments[index].attachmentInfo.imageLayout = imagePair->imageData->currentPattern.layout;
			attachmentInfos.push_back(GPUPipelineRef->colAttachments[index].attachmentInfo);
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
			GPUPipelineRef->depthAttachment.attachmentInfo.imageView = depthImage->imageView.get();
		}
		GPUPipelineRef->dynamicRenderPass.SetRenderInfo(attachmentInfos, frameBufferSize, &GPUPipelineRef->depthAttachment.attachmentInfo);

		commandBuffer.bindPipeline(GPUPipelineRef->pipelineType, GPUPipelineRef->pipeline.get());
		commandBuffer.beginRendering(GPUPipelineRef->dynamicRenderPass.renderInfo);
		commandBuffer.bindDescriptorSets(GPUPipelineRef->pipelineType,
		                                 GPUPipelineRef->pipelineLayout.get(), 0,
		                                 1,
		                                 &GPUPipelineRef->descCache->dstSet, 0, nullptr);
		(*renderOperations)();
		commandBuffer.endRendering();
		return this;
	}

	RenderGraphNode *ExecuteCompute(vk::CommandBuffer commandBuffer)
	{
		TransitionImages(commandBuffer);
		SyncBuffers(commandBuffer);
		commandBuffer.bindPipeline(GPUPipelineRef->pipelineType, GPUPipelineRef->pipeline.get());
		commandBuffer.bindDescriptorSets(GPUPipelineRef->pipelineType,
		                                 GPUPipelineRef->pipelineLayout.get(), 0,
		                                 1,
		                                 &GPUPipelineRef->descCache->dstSet, 0, nullptr);
		(*renderOperations)();
		return this;
	}
	
	RenderGraphNode *ExecuteCuda(vk::CommandBuffer commandBuffer)
	{
		TransitionImages(commandBuffer);
		SyncBuffers(commandBuffer);
		assert(this->CUDAPipeline);
		return this;
	}

	RenderGraphNode *Execute(vk::CommandBuffer commandBuffer)
	{
		for (int i = 0; i < tasks.size(); ++i)
		{
			if (tasks[i] != nullptr)
			{
				(*tasks[i])();
			}
		}
		switch (nodeType)
		{
			case RenderNodeType::GRAPHICS:
				ExecutePass(commandBuffer);
				break;
			case RenderNodeType::COMPUTE:
				ExecuteCompute(commandBuffer);
				break;
			case RenderNodeType::CUDA:
				ExecuteCuda(commandBuffer);
				break;
			default:
				assert(false && "Unsuported pipeline type");
				break;
		}
		return this;
	}

	RenderGraphNode *SetVertexInput(VertexInput vertexInput)
	{
		this->GPUPipelineRef->SetVertexInput(vertexInput);
		return this;
	}

	RenderGraphNode *SetFramebufferSize(glm::uvec2 size)
	{
		this->frameBufferSize = size;
		return this;
	}

	RenderGraphNode *SetRenderOperation(std::function<void()> *renderOperations)
	{
		if (this->renderOperations)
		{
			delete (this->renderOperations);
		}
		this->renderOperations = renderOperations;
		return this;
	}

	RenderGraphNode *AddTask(std::function<void()> *task)
	{
		this->tasks.push_back(task);
		return this;
	}

	RenderGraphNode *SetPipelineLayoutCI(vk::PipelineLayoutCreateInfo createInfo)
	{
		GPUPipelineRef->SetPipelineLayoutCI(createInfo);
		return this;
	}

	RenderGraphNode *SetPushConstantSize(size_t size)
	{
		GPUPipelineRef->SetPushConstantSize(size);
		return this;
	}

	RenderGraphNode *SetGraphicsPipelineConfigs(GraphicsPipelineConfigs graphicsPipelineConfigs)
	{
		GPUPipelineRef->SetGraphicsPipelineConfigs(graphicsPipelineConfigs);
		return this;
	}

	RenderGraphNode *SetDepthConfig(DepthConfigs dephtConfig)
	{
		GPUPipelineRef->SetDepthConfig(dephtConfig);
		return this;
	}

	RenderGraphNode *SetVertShader(Shader *shader)
	{
		GPUPipelineRef->SetVertShader(shader);
		return this;
	}

	RenderGraphNode *SetFragShader(Shader *shader)
	{
		GPUPipelineRef->SetFragShader(shader);
		return this;
	}

	RenderGraphNode *SetCompShader(Shader *shader)
	{
		GPUPipelineRef->SetCompShader(shader);
		return this;
	}

	RenderGraphNode *SetTesControlShader(Shader *shader)
	{
		GPUPipelineRef->SetTesControlShader(shader);
		return this;
	}

	RenderGraphNode *SetTesEvalShader(Shader *shader)
	{
		GPUPipelineRef->SetTesEvalShader(shader);
		return this;
	}

	RenderGraphNode *SetGeomShader(Shader *shader)
	{
		GPUPipelineRef->SetGeomShader(shader);
		return this;
	}

	RenderGraphNode *SetVertShader_IMode(std::string path)
	{
		GPUPipelineRef->SetVertShader_IMode(path);
		return this;
	}

	RenderGraphNode *SetFragShader_IMode(std::string path)
	{
		GPUPipelineRef->SetFragShader_IMode(path);
		return this;
	}

	RenderGraphNode *SetCompShader_IMode(std::string path)
	{
		GPUPipelineRef->SetCompShader_IMode(path);
		return this;
	}

	RenderGraphNode *SetTesControlShader_IMode(std::string path)
	{
		GPUPipelineRef->SetTesControlShader_IMode(path);
		return this;
	}

	RenderGraphNode *SetTesEvalShader_IMode(std::string path)
	{
		GPUPipelineRef->SetTesEvalShader_IMode(path);
		return this;
	}

	RenderGraphNode *SetGeomShader_IMode(std::string path)
	{
		GPUPipelineRef->SetGeomShader_IMode(path);
		return this;
	}

	RenderGraphNode *AddColorAttachmentInput(std::string name)
	{
		GPUPipelineRef->AddColorAttachmentInput(name);
		return this;
	}

	RenderGraphNode *AddColorAttachmentOutput(std::string name, AttachmentInfo attachmentInfo, BlendConfigs blendConfig)
	{
		GPUPipelineRef->AddColorAttachmentOutput(name, attachmentInfo, blendConfig);
		return this;
	}
	RenderGraphNode *SetDepthAttachmentInput(std::string name)
	{
		GPUPipelineRef->SetDepthAttachmentInput(name);
		return this;
	}

	RenderGraphNode *SetDepthAttachmentOutput(std::string name, AttachmentInfo depth)
	{
		GPUPipelineRef->SetDepthAttachmentOutput(name, depth);
		return this;
	}
	RenderGraphNode *SetDepthImageResource(ImageView *imageView)
	{
		this->depthImage = imageView;
		return this;
	}

	RenderGraphNode *ActivateNode(bool value)
	{
		this->active = value;
		return this;
	}

	RenderGraphNode *AddColorImageResource(ImageView *imageView)
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
		return this;
	}

	RenderGraphNode *AddSamplerResource(ImageView *imageView)
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
		return this;
	}

	RenderGraphNode *AddStorageResource(ImageView *imageView)
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
		return this;
	}

	RenderGraphNode *AddBufferSync(BufferKey buffer)
	{
		if (!buffers.contains(buffer.name))
		{
			buffers.try_emplace(buffer.name, buffer);
		}
		else
		{
			buffers.at(buffer.name) = buffer;
		}
		return this;
	}
	// We change the image view if the name already exist when using resources

	RenderGraphNode *DependsOn(std::string dependency)
	{
		if (!dependencies.contains(dependency))
		{
			dependencies.insert(dependency);
			SYSTEMS::Logger::GetInstance()->LogMessage("Renderpass: (" + this->passName + ") Depends On-> (" + dependency + ")");
		}
		return this;
	}

	RenderGraphNode *ClearOperations()
	{
		delete renderOperations;
		for (auto &task : tasks)
		{
			delete task;
		}
		renderOperations = nullptr;
		tasks.clear();
		return this;
	}
	RenderGraphNode *SetCurrCmd(vk::CommandBuffer cmd)
	{
		currCmd = cmd;
		return this;
	}
	vk::CommandBuffer &GetCurrCmd()
	{
		assert(currCmd != nullptr && "Cmd must be valid");
		return currCmd;
	}
	RenderGraphNode *SetConfigs(RenderNodeConfigs configs)
	{
		GPUPipelineRef->SetConfigs(configs);
		return this;
	}
	RenderGraphNode *Reset()
	{
		GPUPipelineRef->Reset();
		active          = false;
		frameBufferSize = {0, 0};
		workerQueueName = "Graphics";
		currCmd         = nullptr;
		depthImage      = nullptr;

		imagesAttachmentOutputs.clear();
		storageImages.clear();
		sampledImages.clear();
		buffers.clear();
		ClearOperations();
		dependencies.clear();
		return this;
	}

	// unused

	GPUPipeline *GPUPipelineRef  = nullptr;
	CUDAPipeline *CUDAPipeline = nullptr;

	bool           waitForResourcesCreation = false;
	RenderNodeType nodeType                 = RenderNodeType::NONE;
	std::string    path;
	std::string    passName;
	std::string    workerQueueName = "Graphics";
	bool           active          = false;

	std::set<std::string> dependencies;

	const std::unordered_map<std::string, int> &GetImageAttachmentNames() const
	{
		return imageAttachmentsNames;
	}

	const std::vector<ImageView *> &GetImageAttachmentOutputs() const
	{
		return imagesAttachmentOutputs;
	}

	const std::unordered_map<std::string, ImageView *> &GetStorageImages() const
	{
		return storageImages;
	}

	const std::unordered_map<std::string, ImageView *> &GetSampledImages() const
	{
		return sampledImages;
	}

	const std::unordered_map<std::string, BufferKey> &GetBuffers() const
	{
		return buffers;
	}

	ImageView *GetDepthImage() const
	{
		return depthImage;
	}

	glm::uvec2 GetFrameBufferSize() const
	{
		return frameBufferSize;
	}

  private:
	friend class RenderGraph;

	Core             *core;
	ImageView        *depthImage = nullptr;
	vk::CommandBuffer currCmd;

	glm::uvec2                      frameBufferSize = {0, 0};
	std::vector<RenderGraphNode *> *nodesToExecuteRef;

	std::unordered_map<std::string, int> imageAttachmentsNames;
	std::vector<ImageView *>             imagesAttachmentOutputs;

	std::unordered_map<std::string, ImageView *> storageImages;
	std::unordered_map<std::string, ImageView *> sampledImages;
	std::unordered_map<std::string, BufferKey>   buffers;

	std::function<void()>               *renderOperations = nullptr;
	std::vector<std::function<void()> *> tasks;

	ResourcesManager *resManagerRef;
};
struct QueueNodesBatch
{
	std::vector<RenderGraphNode *> sortedNodes;
	std::string                    queueName;
	vk::CommandBuffer              commandBuffer;
	int                            id;
	int                            poolIdUsed;
	void ExecuteCUDA(Core* core)
	{
		auto queueRef = core->queueWorkerManager->GetWorkerQueue(queueName);
		for (int i = 0; i < sortedNodes.size(); ++i)
		{
			auto &node = sortedNodes[i];
			node->CUDAPipeline->context->C_WaitExternalSemaphore(queueRef->timelineValue);
			node->CUDAPipeline->context->C_Execute();
			node->CUDAPipeline->context->C_SignalExternalSemaphore(++queueRef->timelineValue);
		}
	}
};

class RenderGraph
{
  public:
	Core             *core;
	ResourcesManager *resourcesManager;

	ImageView *currentBackBufferSwapchain;
	// this is the backbuffer that will be blitted to the swapchain
	ImageView *currentBackBuffer;
	size_t     frameIndex;
	bool       debugUI = true;

	std::unordered_map<std::string, std::unique_ptr<GPUPipeline>>     gpuPipelines;
	std::unordered_map<std::string, std::unique_ptr<CUDAPipeline>>     cudaPipelines;
	std::unordered_map<std::string, std::unique_ptr<RenderGraphNode>> renderNodes;
	// todo
	std::vector<RenderGraphNode *> nodesToExecute;
	std::vector<RenderGraphNode *> sequentialRenderNodes;
	std::vector<RenderGraphNode *> sortedByDepNodes;

	std::unordered_map<std::string, std::unique_ptr<Shader>>          shadersProxy;
	std::unordered_map<std::string, std::unique_ptr<DescriptorCache>> descCachesProxy;
	std::vector<QueueNodesBatch>                                      sortedQueueBatches;

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

	GPUPipeline *GetTemplateGPUPipeline(std::string name, std::string shaderName, ShaderCompiler compiler)
	{
		Shader *vShader     = resourcesManager->GetOrCreateDefaultShader(shaderName, ShaderStage::S_VERT, compiler);
		Shader *fShader     = resourcesManager->GetOrCreateDefaultShader(shaderName, ShaderStage::S_FRAG, compiler);
		auto    gpuPipeline = AddGPUPipeline(name);
		gpuPipeline->SetConfigs({true});
		gpuPipeline->SetVertShader(vShader);
		gpuPipeline->SetFragShader(fShader);
		gpuPipeline->SetVertexInput(Vertex2D::GetVertexInput());
		return gpuPipeline;
	}

	RenderGraphNode *GetTemplateNode_DF(std::string name, std::string shaderName, ShaderCompiler compiler)
	{
		Shader *vShader    = resourcesManager->GetOrCreateDefaultShader(shaderName, ShaderStage::S_VERT, compiler);
		Shader *fShader    = resourcesManager->GetOrCreateDefaultShader(shaderName, ShaderStage::S_FRAG, compiler);
		auto    renderNode = AddPass(name);
		renderNode->SetConfigs({true});
		renderNode->SetVertShader(vShader);
		renderNode->SetFragShader(fShader);
		renderNode->SetVertexInput(Vertex2D::GetVertexInput());
		return renderNode;
	}

	RenderGraphNode *GetTemplateComputeNode_DF(std::string name, std::string shaderName, ShaderCompiler compiler)
	{
		Shader *shader     = resourcesManager->GetOrCreateDefaultShader(shaderName, ShaderStage::S_COMP, compiler);
		auto    renderNode = AddPass(name);
		renderNode->SetConfigs({true});
		renderNode->SetCompShader(shader);
		return renderNode;
	}
	RenderGraphNode *GetTemplateNode(std::string name, std::string vPath, std::string fPath)
	{
		AttachmentInfo colInfo = GetColorAttachmentInfo(
		    glm::vec4(0.0f), g_32bFormat);
		Shader *vShader    = resourcesManager->GetShader(vPath, ShaderStage::S_VERT);
		Shader *fShader    = resourcesManager->GetShader(fPath, ShaderStage::S_FRAG);
		auto    renderNode = AddPass(name);
		renderNode->SetConfigs({true});
		renderNode->SetVertShader(vShader);
		renderNode->SetFragShader(fShader);
		renderNode->SetVertexInput(Vertex2D::GetVertexInput());
		// change this
		renderNode->SetPushConstantSize(4);
		return renderNode;
	}

	RenderGraphNode *GetTemplateComputeNode(std::string name, std::string path)
	{
		Shader *shader     = resourcesManager->GetShader(path, ShaderStage::S_COMP);
		auto    renderNode = AddPass(name);
		renderNode->SetConfigs({true});
		renderNode->SetCompShader(shader);
		// change this
		renderNode->SetPushConstantSize(4);
		return renderNode;
	}
	void CreateUtilityShaders()
	{
		auto          *blitterShader = GetTemplateGPUPipeline("BlitterPipeline", "Blitter", ShaderCompiler::C_GLSL);
		AttachmentInfo colInfo       = GetColorAttachmentInfo(
            glm::vec4(0.0f), core->swapchainRef->GetFormat());
		blitterShader->AddColorAttachmentOutput("BF_SwapChain", colInfo, B_OPAQUE);
		blitterShader->BuildGPUPipeline();
	}
	GPUPipeline *GetBlitterShader()
	{
		auto *blitterShader = GetGPUPipeline("BlitterPipeline");
		return blitterShader;
	}
	RenderGraphNode *BlitToBackbuffer()
	{
		auto *blitterNode = AddPass(GetBlitterShader(), "BlitterNode");

		blitterNode->BuildRenderGraphNode();
		blitterNode->EnqueueNode();

		GetNode("BlitterNode")->SetSampler("MainTex", currentBackBuffer);
		auto blitterTask = new std::function<void()>([this]() {
			// there is one thing that I may change and is that when I set the sampler from outside of the task
			// function and I reload the shaders the ref of the sampler is gone because I reset the descriptor set, I should change that
			GetNode("BlitterNode")->AddColorImageResource(currentBackBufferSwapchain);
			GetNode("BlitterNode")->SetFramebufferSize(currentBackBufferSwapchain->imageData->GetImageSize());
		});

		auto blitterRenderOp = new std::function<void()>(
		    [this]() {
			    auto           renderNode = GetNode("BlitterNode");
			    vk::DeviceSize offset     = 0;
			    renderNode->GetCurrCmd().bindVertexBuffers(
			        0, 1,
			        &resourcesManager->GetStagedBuffFromName("quad_default")->deviceBuffer->bufferHandle.get(),
			        &offset);
			    renderNode->GetCurrCmd().bindIndexBuffer(
			        resourcesManager->GetStagedBuffFromName("quad_index_default")->deviceBuffer->bufferHandle.get(), 0, vk::IndexType::eUint32);

			    renderNode->GetCurrCmd().drawIndexed(Vertex2D::GetQuadIndices().size(), 1, 0,
			                                         0, 0);
		    });

		blitterNode->AddTask(blitterTask);
		blitterNode->SetRenderOperation(blitterRenderOp);
		return blitterNode;
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
	void EnqueueNode(RenderGraphNode *node)
	{
		nodesToExecute.push_back(node);
	}
	void UpdateAllFromMetaData()
	{
		for (auto &node : sequentialRenderNodes)
		{
			node->Deserialize(node->path);
			SYSTEMS::Logger::GetInstance()->LogMessage("Render Node: (" + node->passName + ") updated");
		}
	}

	void CreateResManager()
	{
		resourcesManager  = ResourcesManager::GetInstance();
		currentBackBuffer = resourcesManager->GetImageViewFromName("bf");


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

	GPUPipeline *GetGPUPipeline(std::string name)
	{
		if (!gpuPipelines.contains(name))
		{
			return nullptr;
		}
		auto gpuPipeline = gpuPipelines.at(name).get();
		assert(gpuPipeline != nullptr && "gpu pipeline is null");
		return gpuPipeline;
	}
	
	CUDAPipeline *GetCUDAPipeline(std::string name)
	{
		if (!cudaPipelines.contains(name))
		{
			return nullptr;
		}
		auto cudaPI = cudaPipelines.at(name).get();
		assert(cudaPI != nullptr && "cuda pipeline is null");
		return cudaPI;
	}
	GPUPipeline *AddGPUPipeline(std::string name)
	{
		if (!gpuPipelines.contains(name))
		{
			auto gpuPipeline             = std::make_unique<GPUPipeline>();
			gpuPipeline->shadersProxyRef = &shadersProxy;
			gpuPipeline->core            = core;
			gpuPipeline->name            = name;
			gpuPipelines.try_emplace(name, std::move(gpuPipeline));
			return gpuPipelines.at(name).get();
		}

		auto gpuPipeline = gpuPipelines.at(name).get();
		assert(gpuPipeline != nullptr && "Pipeline is null");
		return gpuPipeline;
	}
	
	CUDAPipeline *AddCUDAPipeline(std::string name)
	{
		if (!cudaPipelines.contains(name))
		{
			auto cudaPipeline             = std::make_unique<CUDAPipeline>();
			auto queueRef = core->queueWorkerManager->GetWorkerQueue("CUDA");
			cudaPipeline->context = new CodeCuda::CodeCudaContext();
			cudaPipeline->context->C_InitFromExternalDevice(core->deviceUUID.data(), VK_UUID_SIZE);
			cudaPipeline->context->C_ImportExternalSemaphore(queueRef->GetExportableHandle());
			cudaPipelines.try_emplace(name, std::move(cudaPipeline));
			return cudaPipelines.at(name).get();
		}

		auto cudaPipeline = cudaPipelines.at(name).get();
		assert(cudaPipeline != nullptr && "Pipeline is null");
		return cudaPipeline;
	}
	
	RenderGraphNode *AddCudaPass(CUDAPipeline *pipeline, std::string name, std::string workerQueueName = "CUDA")
	{
		if (!renderNodes.contains(name))
		{
			assert(pipeline != nullptr && "Pipeline is null");
			auto renderGraphNode               = std::make_unique<RenderGraphNode>();
			renderGraphNode->CUDAPipeline    = pipeline;
			renderGraphNode->passName          = name;
			renderGraphNode->core              = core;
			renderGraphNode->nodesToExecuteRef = &nodesToExecute;
			renderGraphNode->resManagerRef     = resourcesManager;
			renderGraphNode->workerQueueName   = workerQueueName;
			renderGraphNode->active            = true;
			renderGraphNode->path              = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\RenderNodes\\pass_" +
									name + ".json";
			renderNodes.try_emplace(name, std::move(renderGraphNode));
			sequentialRenderNodes.push_back(renderNodes.at(name).get());
			return renderNodes.at(name).get();
		}
		return renderNodes.at(name).get();
	}

	RenderGraphNode *AddPass(GPUPipeline *gpuPipeline, std::string name, std::string workerQueueName = "Graphics")
	{
		if (!renderNodes.contains(name))
		{
			assert(gpuPipeline != nullptr && "Pipeline is null");
			auto renderGraphNode               = std::make_unique<RenderGraphNode>();
			renderGraphNode->GPUPipelineRef    = gpuPipeline;
			renderGraphNode->passName          = name;
			renderGraphNode->core              = core;
			renderGraphNode->nodesToExecuteRef = &nodesToExecute;
			renderGraphNode->resManagerRef     = resourcesManager;
			renderGraphNode->workerQueueName   = workerQueueName;
			renderGraphNode->active            = true;
			renderGraphNode->path              = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			renderNodes.try_emplace(name, std::move(renderGraphNode));
			sequentialRenderNodes.push_back(renderNodes.at(name).get());
			return renderNodes.at(name).get();
		}
		return renderNodes.at(name).get();
	}

	RenderGraphNode *AddPass(std::string name, std::string workerQueueName = "Graphics")
	{
		if (!renderNodes.contains(name))
		{
			auto gpuPipeline = AddGPUPipeline("GPUPipeline_" + name);
			assert(gpuPipeline != nullptr && "GPUPipeline_ is null");
			auto renderGraphNode               = std::make_unique<RenderGraphNode>();
			renderGraphNode->GPUPipelineRef    = gpuPipeline;
			renderGraphNode->passName          = name;
			renderGraphNode->core              = core;
			renderGraphNode->nodesToExecuteRef = &nodesToExecute;
			renderGraphNode->resManagerRef     = resourcesManager;
			renderGraphNode->workerQueueName   = workerQueueName;
			renderGraphNode->active            = true;
			renderGraphNode->path              = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() + "\\RenderNodes\\pass_" +
			                        name + ".json";
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

	ImageView *AddColorImageResource(std::string passName, ImageView *imageView)
	{
		assert(imageView && "ImageView is null");
		if (renderNodes.contains(passName))
		{
			renderNodes.at(passName)->AddColorImageResource(imageView);
		}
		else
		{
			std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
		}
		return imageView;
	}

	ImageView *SetDepthImageResource(std::string passName, ImageView *imageView)
	{
		assert(imageView && "ImageView is null");
		if (renderNodes.contains(passName))
		{
			renderNodes.at(passName)->SetDepthImageResource(imageView);
		}
		else
		{
			std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
		}
		return imageView;
	}

	ImageView *AddSamplerResource(std::string passName, ImageView *imageView)
	{
		assert(imageView && "ImageView is null");
		if (renderNodes.contains(passName))
		{
			renderNodes.at(passName)->AddSamplerResource(imageView);
		}
		else
		{
			std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
		}
		return imageView;
	}

	ImageView *AddStorageResource(std::string passName, ImageView *imageView)
	{
		assert(imageView && "ImageView is null");
		if (renderNodes.contains(passName))
		{
			renderNodes.at(passName)->AddStorageResource(imageView);
		}
		else
		{
			std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
		}
		return imageView;
	}

	BufferKey &AddBufferSync(std::string passName, BufferKey buffer)
	{
		assert(buffer.buffer && "ImageView is null");
		if (renderNodes.contains(passName))
		{
			renderNodes.at(passName)->AddBufferSync(buffer);
		}
		else
		{
			std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
		}
		// std::cout << "Image with name: \"" << name << "\" has changed \n";
		return buffer;
	}

	void RecreateFrameResources()
	{
		// auto imageInfoBf = Image::CreateInfo2d(glm::uvec2(core->swapchainRef->extent.width, core->swapchainRef->extent.height), 1, 1,
		// 											 core->swapchainRef->GetFormat(),
		// 											 vk::ImageUsageFlagBits::eColorAttachment |
		// 												 vk::ImageUsageFlagBits::eSampled);
		// currentBackBuffer = resourcesManager->SetOrCreateImage("bf", imageInfoBf, 0, 0);
	}
	void RecreateNodePipelines()
	{
		for (auto &node : renderNodes)
		{
			if (node.second->GPUPipelineRef)
			{
				node.second->RecreateResources();
			}
		}
		SYSTEMS::Logger::GetInstance()->LogMessage("Graphics Pipelines Recreated");
	}

	void DebugShadersCompilation()
	{
		int result = std::system(
		    "C:\\Users\\carlo\\CLionProjects\\CodeVkEngine\\src\\Shaders\\compile.bat");
		if (result == 0)
		{
			std::cout << "Shaders compiled\n";
		}
		else
		{
			assert(false && "reload shaders failed");
		}
	}
	void CreateUtility()
	{
		CreateUtilityShaders();
		BlitToBackbuffer();
	}
	void SortNodesByDep(std::vector<RenderGraphNode *> &nodesSortedOut)
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
		nodesSortedOut.clear();
		nodesSortedOut.reserve(solvedNodesOrdered.size());
		for (int i = 0; i < solvedNodesOrdered.size(); ++i)
		{
			nodesSortedOut.emplace_back(GetNode(solvedNodesOrdered[i]));
		}
	}
	void BuildQueuePassesBatches(std::vector<RenderGraphNode *> &sortedNodes, std::vector<QueueNodesBatch> &batches)
	{
		assert(batches.empty() && "Queue batches should be empty before building a new queue batch");
		if (sortedNodes.empty())
		{
			return;
		}
		int batchIdx = 0;
		
		//if the first queue is cuda we need to add a "bridge sync point with image adquire semaphore"
		if(sortedNodes[0]->workerQueueName == "CUDA"){
			int               poolCmdId = 0;
			auto             *queueRef  = core->queueWorkerManager->GetWorkerQueue("Graphics");
			vk::CommandBuffer cmd       = queueRef->RequestQueueCmd(poolCmdId);
			batches.emplace_back(QueueNodesBatch{std::vector<RenderGraphNode *>{}, "Graphics", cmd, batchIdx});
			batches.back().poolIdUsed = poolCmdId;
			batchIdx++;
		}
		for (int i = 0; i < sortedNodes.size(); ++i)
		{
			if (batches.empty() || batches.back().queueName != sortedNodes[i]->workerQueueName)
			{
				batchIdx++;
				auto             *queueRef  = core->queueWorkerManager->GetWorkerQueue(sortedNodes[i]->workerQueueName);
				int               poolCmdId = 0;
				vk::CommandBuffer cmd       = queueRef->RequestQueueCmd(poolCmdId);
				batches.emplace_back(QueueNodesBatch{std::vector<RenderGraphNode *>{sortedNodes[i]}, queueRef->name, cmd, batchIdx});
				batches.back().poolIdUsed = poolCmdId;
			}
			else
			{
				batches.back().sortedNodes.emplace_back(sortedNodes[i]);
			}
		}
		//if the first queue is cuda we need to add a "bridge sync point with rendering finished semaphore"
		if(sortedNodes.back()->workerQueueName == "CUDA"){
			int               poolCmdId = 0;
			auto             *queueRef  = core->queueWorkerManager->GetWorkerQueue("Graphics");
			vk::CommandBuffer cmd       = queueRef->RequestQueueCmd(poolCmdId);
			batches.emplace_back(QueueNodesBatch{std::vector<RenderGraphNode *>{}, "Graphics", cmd, batchIdx});
			batches.back().poolIdUsed = poolCmdId;
			batchIdx++;
		}
		if (debugUI)
		{
			auto *queueRef  = core->queueWorkerManager->GetWorkerQueue("UI");
			int   poolCmdId = 0;
			batches.emplace_back(QueueNodesBatch{std::vector<RenderGraphNode *>{}, "UI", queueRef->RequestQueueCmd(poolCmdId), batchIdx + 1});
			batches.back().poolIdUsed = poolCmdId;
		}
	}

	void ResolveNodesDependancies(std::vector<RenderGraphNode *> &nodesToResolve)
	{
		for (int i = nodesToResolve.size() - 1; i >= 0; i--)
		{
			RenderGraphNode *node = nodesToResolve[i];
			for (int j = i - 1; j >= 0; j--)
			{
				RenderGraphNode *toCheckNode = nodesToResolve[j];

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

	void ExecuteQueueBatch(QueueNodesBatch &queueNodesBatch)
	{
		std::vector<std::string> allPassesNames;
		int                      idx = 0;

		auto profiler = Profiler::GetInstance();
		for (auto &renderNode : queueNodesBatch.sortedNodes)
		{
			if (!renderNode->active)
			{
				continue;
			}

			profiler->AddProfilerCpuSpot(legit::Colors::getColor(idx), "Pass: " + renderNode->passName);
			RenderGraphNode *node = renderNode;
			node->SetCurrCmd(queueNodesBatch.commandBuffer);
			auto       *queueRef  = core->queueWorkerManager->GetWorkerQueue(node->workerQueueName);
			bool        depenNeed = false;
			std::string depenName = "";
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
					profiler->EndProfilerCpuSpot("Pass: " + renderNode->passName);
					SYSTEMS::Logger::GetInstance()->LogMessage(
					    "Pass with name: (" + renderNode->passName + ") needs (" + depenName + ") and is not active");
					// if a dependency is not active we skip the node
					continue;
				}
				BufferUsageTypes    lastNodeType    = (depenNode->nodeType == RenderNodeType::GRAPHICS) ? B_GRAPHICS_WRITE : B_COMPUTE_WRITE;
				BufferUsageTypes    currNodeType    = (node->nodeType == RenderNodeType::GRAPHICS) ? B_GRAPHICS_WRITE : B_COMPUTE_WRITE;
				BufferAccessPattern lastNodePattern = GetSrcBufferAccessPattern(lastNodeType);
				BufferAccessPattern currNodePattern = GetSrcBufferAccessPattern(currNodeType);
				CreateMemBarrier(lastNodePattern, currNodePattern, node->GetCurrCmd());
			}
			if (queueRef->isMainThreat)
			{
				node->Execute(node->GetCurrCmd());
			}
			// else
			// {
			// 	std::string           name = node->passName;
			// 	std::function<void()> nodeTask([name, queueNodesBatch, this] {
			// 		renderNodes.at(name)->Execute(queueNodesBatch.commandBuffer);
			// 	});
			// 	queueRef->taskThreat.AddTask(nodeTask);
			// }
			allPassesNames.push_back(node->passName);
			profiler->EndProfilerCpuSpot("Pass: " + renderNode->passName);
		}
	}
	void PrepareRendering()
	{
		Profiler::GetInstance()->AddProfilerCpuSpot(legit::Colors::belizeHole, "Rendergraph prepare cpu");
		resourcesManager->UpdateBuffers();
		resourcesManager->UpdateImages();
		for (auto node :sequentialRenderNodes)
		{
			node->ValidateNodeType();
		}
		sortedByDepNodes.clear();
		sortedQueueBatches.clear();
		ResolveNodesDependancies(sequentialRenderNodes);
		if (sequentialRenderNodes.size() > 1)
		{
			GetNode("BlitterNode")->DependsOn(sequentialRenderNodes[sequentialRenderNodes.size() - 2]->passName);
		}
		SortNodesByDep(sortedByDepNodes);
		BuildQueuePassesBatches(sortedByDepNodes, sortedQueueBatches);
		Profiler::GetInstance()->EndProfilerCpuSpot("Rendergraph prepare cpu");
	}
	void ExecuteRendering()
	{
		Profiler::GetInstance()->AddProfilerCpuSpot(legit::Colors::belizeHole, "Rendergraph execute cpu");
		assert(!sortedQueueBatches.empty() && "There must queues");
		for (auto &queueBatch : sortedQueueBatches)
		{
			ExecuteQueueBatch(queueBatch);
		}

		Profiler::GetInstance()->EndProfilerCpuSpot("Rendergraph execute cpu");
	}
	void EndRendering()
	{
		core->queueWorkerManager->ResetPoolUsage();
	}

	void BuildDeserializedPasses()
	{
	}

	~RenderGraph()
	{
		for (auto& pi : cudaPipelines)
		{
			pi.second->context->C_Shutdown();
		}
	};
};

}        // namespace ENGINE

#endif        // RENDERGRAPH_HPP
