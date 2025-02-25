//
// Created by carlo on 2024-10-02.
//


#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP


namespace ENGINE
{
    struct BufferKey
    {
        BufferUsageTypes srcUsage;
        BufferUsageTypes dstUsage;
        Buffer* buffer;
    };

    struct RenderNodeConfigs
    {
        bool AutomaticCache = false;
    };

    class RenderGraph;

    struct RenderGraphNode
    {
        RenderGraphNode()
        {
        }

        void RecreateResources()
        {
            assert(&pipelineLayoutCI != nullptr && "Pipeline layout is null");
            pipeline.reset();
            pipelineLayout.reset();
            ReloadShaders();
            Shader* vertShader = shaders.at("vert");
            Shader* fragShader = shaders.at("frag");
            Shader* compShader = shaders.at("comp");

            if (fragShader && vertShader)
            {
                if (configs.AutomaticCache)
                {
                    descCache.reset();
                    descCache =std::make_unique<DescriptorCache>(core);
                    descCache->AddShaderInfo(vertShader->sParser.get());
                    descCache->AddShaderInfo(fragShader->sParser.get());
                    descCache->BuildDescriptorsCache(vk::ShaderStageFlagBits::eFragment |
                        vk::ShaderStageFlagBits::eVertex);
                    if (pushConstantSize != 0)
                    {
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
                }
                std::vector<vk::Format> colorFormats;
                colorFormats.reserve(colAttachments.size());
                std::vector<vk::RenderingAttachmentInfo> renderingAttachmentInfos;
                for (auto& colAttachment : colAttachments)
                {
                    colorFormats.push_back(colAttachment.format);
                    renderingAttachmentInfos.push_back(colAttachment.attachmentInfo);
                }
                dynamicRenderPass.SetPipelineRenderingInfo(colAttachments.size(), colorFormats, depthAttachment.format);

                pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
                std::unique_ptr<GraphicsPipeline> graphicsPipeline = std::make_unique<ENGINE::GraphicsPipeline>(
                    core->logicalDevice.get(), vertShader->sModule->shaderModuleHandle.get(),
                    fragShader->sModule->shaderModuleHandle.get(), pipelineLayout.get(),
                    dynamicRenderPass.pipelineRenderingCreateInfo, rasterizationConfigs,
                    colorBlendConfigs, depthConfig,
                    vertexInput, pipelineCache.get()
                );
                pipeline = std::move(graphicsPipeline->pipelineHandle);
                pipelineType = vk::PipelineBindPoint::eGraphics;
            }
            else if (compShader)
            {
                if (configs.AutomaticCache)
                {
                    descCache.reset();
                    descCache = std::make_unique<DescriptorCache>(core);
                    descCache->AddShaderInfo(compShader->sParser.get());
                    descCache->BuildDescriptorsCache(
                        vk::ShaderStageFlagBits::eCompute);
                    if (pushConstantSize != 0)
                    {
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
                }

                pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
                std::unique_ptr<ComputePipeline> computePipeline = std::make_unique<ENGINE::ComputePipeline>(
                    core->logicalDevice.get(), compShader->sModule->shaderModuleHandle.get(), pipelineLayout.get(),
                    pipelineCache.get());
                pipeline = std::move(computePipeline->pipelineHandle);
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
            pipelineCache = core->logicalDevice->createPipelineCacheUnique(pipelineCacheCreateInfo);

            Shader* vertShader = shaders.at("vert");
            Shader* fragShader = shaders.at("frag");
            Shader* compShader = shaders.at("comp");

            if (fragShader && vertShader)
            {
                if (configs.AutomaticCache)
                {
                    descCache.reset();
                    descCache = std::make_unique<DescriptorCache>(core);
                    descCache->AddShaderInfo(vertShader->sParser.get());
                    descCache->AddShaderInfo(fragShader->sParser.get());
                    descCache->BuildDescriptorsCache(
                        vk::ShaderStageFlagBits::eFragment |
                        vk::ShaderStageFlagBits::eVertex);

                    if (pushConstantSize != 0)
                    {
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
                }
                std::vector<vk::Format> colorFormats;
                colorFormats.reserve(colAttachments.size());
                std::vector<vk::RenderingAttachmentInfo> renderingAttachmentInfos;
                for (auto& colAttachment : colAttachments)
                {
                    colorFormats.push_back(colAttachment.format);
                    renderingAttachmentInfos.push_back(colAttachment.attachmentInfo);
                }
                dynamicRenderPass.SetPipelineRenderingInfo(colAttachments.size(), colorFormats, depthAttachment.format);

                pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);

                std::unique_ptr<GraphicsPipeline> graphicsPipeline = std::make_unique<ENGINE::GraphicsPipeline>(
                    core->logicalDevice.get(), vertShader->sModule->shaderModuleHandle.get(),
                    fragShader->sModule->shaderModuleHandle.get(), pipelineLayout.get(),
                    dynamicRenderPass.pipelineRenderingCreateInfo, rasterizationConfigs,
                    colorBlendConfigs, depthConfig,
                    vertexInput, pipelineCache.get()
                );
                pipeline = std::move(graphicsPipeline->pipelineHandle);
                pipelineType = vk::PipelineBindPoint::eGraphics;
                active = true;
                std::cout << "Graphics pipeline created\n";
            }
            else if (compShader)
            {
                if (configs.AutomaticCache)
                {
                    descCache.reset();
                    descCache = std::make_unique<DescriptorCache>(core);
                    descCache->AddShaderInfo(compShader->sParser.get());
                    descCache->BuildDescriptorsCache(
                        vk::ShaderStageFlagBits::eCompute);
                    if (pushConstantSize != 0)
                    {
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
                }
                pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
                std::unique_ptr<ComputePipeline> computePipeline = std::make_unique<ENGINE::ComputePipeline>(
                    core->logicalDevice.get(), compShader->sModule->shaderModuleHandle.get(), pipelineLayout.get(),
                    pipelineCache.get());
                pipeline = std::move(computePipeline->pipelineHandle);
                pipelineType = vk::PipelineBindPoint::eCompute;
                active = true;
                std::cout << "Compute pipeline created\n";
            }
            else
            {
                std::cout << "No compute or graphics shaders were set\n";
            }
        }

        void TransitionImages(vk::CommandBuffer commandBuffer)
        {
            for (auto& storageImage : storageImages)
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
            for (auto& sampler : sampledImages)
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
                //TODO: Better transition for sampler images
                // if (IsImageTransitionNeeded(sampler.second->imageData->currentLayout, dstPattern))
                // {
                // }
                TransitionImage(sampler.second->imageData, dstPattern, sampler.second->GetSubresourceRange(),
                                commandBuffer);
            }
        }

        void SyncBuffers(vk::CommandBuffer commandBuffer)
        {
            for (auto& pair : buffers)
            {
                BufferKey buffer = pair.second;
                BufferAccessPattern srcPattern = GetSrcBufferAccessPattern(buffer.srcUsage);
                BufferAccessPattern dstPattern = GetSrcBufferAccessPattern(buffer.dstUsage);
                CreateMemBarrier(srcPattern, dstPattern, commandBuffer);
            }
        }

        void ReloadShaders()
        {
            Shader* vertShader = shaders.at("vert");
            Shader* fragShader = shaders.at("frag");
            Shader* compShader = shaders.at("comp");
            if (vertShader && fragShader)
            {
                vertShader->Reload();
                fragShader->Reload();
            }
            else if (compShader)
            {
                compShader->Reload();
            }
        }

        void ExecutePass(vk::CommandBuffer commandBuffer)
        {
            dynamicRenderPass.SetViewport(frameBufferSize, frameBufferSize);
            commandBuffer.setViewport(0, 1, &dynamicRenderPass.viewport);
            commandBuffer.setScissor(0, 1, &dynamicRenderPass.scissor);

            assert(imagesAttachment.size()== colAttachments.size()&& "Not all color attachments were set");
            int index = 0;
            std::vector<vk::RenderingAttachmentInfo> attachmentInfos;
            attachmentInfos.reserve(colAttachments.size());
            for (auto& imagePair : imagesAttachment)
            {
                if (IsImageTransitionNeeded(imagePair.second->imageData->currentLayout, COLOR_ATTACHMENT))
                {
                    TransitionImage(imagePair.second->imageData, COLOR_ATTACHMENT,
                                    imagePair.second->GetSubresourceRange(), commandBuffer);
                }
                colAttachments[index].attachmentInfo.setImageView(imagePair.second->imageView.get());
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
            (*renderOperations)();
            commandBuffer.endRendering();
        }

        void ExecuteCompute(vk::CommandBuffer commandBuffer)
        {
            TransitionImages(commandBuffer);
            SyncBuffers(commandBuffer);
            commandBuffer.bindPipeline(pipelineType, pipeline.get());
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

        void SetRenderOperation(std::function<void()>* renderOperations)
        {
            this->renderOperations = renderOperations;
        }

        void AddTask(std::function<void()>* task)
        {
            this->tasks.push_back(task);
        }

        void SetPipelineLayoutCI(vk::PipelineLayoutCreateInfo createInfo)
        {
            this->pipelineLayoutCI = createInfo;
            if (createInfo.pPushConstantRanges != nullptr)
            {
                this->pushConstantRange.offset = createInfo.pPushConstantRanges->offset;
                this->pushConstantRange.size = createInfo.pPushConstantRanges->size;
                this->pushConstantRange.stageFlags = createInfo.pPushConstantRanges->stageFlags;
                this->pipelineLayoutCI.setPushConstantRanges(this->pushConstantRange);
            }
        }

        void SetPushConstantSize(size_t size)
        {
            pushConstantSize = size;
        }

        void SetRasterizationConfigs(RasterizationConfigs rasterizationConfigs)
        {
            this->rasterizationConfigs = rasterizationConfigs;
        }

        void SetDepthConfig(DepthConfigs dephtConfig)
        {
            this->depthConfig = dephtConfig;
        }

        void SetVertShader(Shader* shader)
        {
            shaders.at("vert") = shader;
        }

        void SetFragShader(Shader* shader)
        {
            shaders.at("frag") = shader;
        }

        void SetCompShader(Shader* shader)
        {
            shaders.at("comp") = shader;
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

        void SetDepthImageResource(std::string name, ImageView* imageView)
        {
            depthImage = imageView;
            AddImageToProxy(name, imageView);
        }

        void ActivateNode(bool value)
        {
            this->active = value;
        }

        //We change the image view if the name already exist when using resources
        void AddColorImageResource(std::string name, ImageView* imageView)
        {
            assert(imageView && "Name does not exist or image view is null");
            if (!imagesAttachment.contains(name))
            {
                imagesAttachment.try_emplace(name, imageView);
            }
            else
            {
                imagesAttachment.at(name) = imageView;
            }
            AddImageToProxy(name, imageView);
        }

        void AddSamplerResource(std::string name, ImageView* imageView)
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

        void AddStorageResource(std::string name, ImageView* imageView)
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
            }
            else
            {
                std::cout << "Renderpass \"" << this->passName << " Already depends on \"" << dependency << "\" \n";
            }
        }

        void ClearOperations()
        {
            delete renderOperations;
            for (auto& task : tasks)
            {
                delete task;
            }
            renderOperations = nullptr;
            tasks.clear();
        }

        void AddImageToProxy(std::string name, ImageView* imageView)
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

        void SetConfigs(RenderNodeConfigs configs)
        {
            this->configs = configs;
        }

        vk::UniquePipeline pipeline;
        vk::UniquePipelineLayout pipelineLayout;
        vk::UniquePipelineCache pipelineCache;
        vk::PipelineLayoutCreateInfo pipelineLayoutCI;
        vk::PushConstantRange pushConstantRange;
        vk::PipelineBindPoint pipelineType;
        DynamicRenderPass dynamicRenderPass;
        std::unique_ptr<DescriptorCache> descCache;
        std::string passName;
        bool active = false;

    private:
        friend class RenderGraph;

        RenderNodeConfigs configs;
        RasterizationConfigs rasterizationConfigs = R_FILL;
        std::vector<BlendConfigs> colorBlendConfigs;
        DepthConfigs depthConfig = D_NONE;
        VertexInput vertexInput;
        glm::uvec2 frameBufferSize;
        size_t pushConstantSize = 0;

        std::vector<AttachmentInfo> colAttachments;
        AttachmentInfo depthAttachment;
        ImageView* depthImage = nullptr;
        std::map<std::string, Shader*> shaders = {{"frag", nullptr}, {"vert", nullptr}, {"comp", nullptr}};

        std::unordered_map<std::string, ImageView*> imagesAttachment;
        std::unordered_map<std::string, ImageView*> storageImages;
        std::unordered_map<std::string, ImageView*> sampledImages;
        std::unordered_map<std::string, BufferKey> buffers;

        std::function<void()>* renderOperations = nullptr;
        std::vector<std::function<void()>*> tasks;

        std::set<std::string> dependencies;

        Core* core;
        std::unordered_map<std::string, ImageView*>* imagesProxyRef;
        std::unordered_map<std::string, BufferKey>* bufferProxyRef;
        std::unordered_map<std::string, AttachmentInfo>* outColAttachmentsProxyRef;
        std::unordered_map<std::string, AttachmentInfo>* outDepthAttachmentProxyRef;
        std::unordered_map<std::string, std::unique_ptr<Shader>>* shadersProxyRef;
        //unused
        std::unordered_map<std::string, std::unique_ptr<DescriptorCache>> descriptorsCachesRef;
    };


    class RenderGraph
    {
    public:
        
        Core* core;
        ResourcesManager* resourcesManager;

        ImageView* currentBackBuffer;
        FrameResources* currentFrameResources;
        size_t frameIndex;
        
        
        std::unordered_map<std::string, std::unique_ptr<RenderGraphNode>> renderNodes;
        std::vector<RenderGraphNode*> renderNodesSorted;
        std::unordered_map<std::string, ImageView*> imagesProxy;
        std::unordered_map<std::string, BufferKey> buffersProxy;
        std::unordered_map<std::string, AttachmentInfo> outColAttachmentsProxy;
        std::unordered_map<std::string, AttachmentInfo> outDepthAttachmentProxy;
        std::unordered_map<std::string, std::unique_ptr<Shader>> shadersProxy;
        std::unordered_map<std::string, std::unique_ptr<DescriptorCache>> descCachesProxy;
       

        RenderGraph(Core* core)
        {
            this->core = core;
        }

        ~RenderGraph()
        {
            
        };

        void CreateResManager()
        {
            resourcesManager = ResourcesManager::GetInstance();
        }


        RenderGraphNode* GetNode(std::string name)
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

        RenderGraphNode* AddPass(std::string name)
        {
            if (!renderNodes.contains(name))
            {
                auto renderGraphNode = std::make_unique<RenderGraphNode>();
                renderGraphNode->passName = name;
                renderGraphNode->imagesProxyRef = &imagesProxy;
                renderGraphNode->outColAttachmentsProxyRef = &outColAttachmentsProxy;
                renderGraphNode->outDepthAttachmentProxyRef = &outColAttachmentsProxy;
                renderGraphNode->bufferProxyRef = &buffersProxy;
                renderGraphNode->shadersProxyRef = &shadersProxy;
                renderGraphNode->core = core;

                renderNodes.try_emplace(name, std::move(renderGraphNode));
                renderNodesSorted.push_back(renderNodes.at(name).get());
                return renderNodes.at(name).get();
            }
            return renderNodes.at(name).get();
        }


        DescriptorCache* AddDescCache(std::string name)
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

        ImageView* AddColorImageResource(std::string passName, std::string name, ImageView* imageView)
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

        ImageView* SetDepthImageResource(std::string passName, std::string name, ImageView* imageView)
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

        ImageView* AddSamplerResource(std::string passName, std::string name, ImageView* imageView)
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

        ImageView* AddStorageResource(std::string passName, std::string name, ImageView* imageView)
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

        BufferKey& AddBufferSync(std::string passName, std::string name, BufferKey buffer)
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

        ImageView* GetImageResource(std::string name)
        {
            if (imagesProxy.contains(name))
            {
                return imagesProxy.at(name);
            }
            PrintInvalidResource("Resource", name);
            return nullptr;
        }

        BufferKey& GetBufferResource(std::string name)
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
            for (auto& renderNode : renderNodes)
            {
                renderNode.second->ClearOperations();
            }
        }

        void RecompileShaders()
        {
            for (auto& node : renderNodes)
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
                assert(false &&"reload shaders failed");
            }
        }

        void ExecuteAll()
        {
            assert(currentFrameResources && "Current frame reference is null");
            std::vector<std::string> allPassesNames;
            int idx = 0;
            for (auto& renderNode : renderNodesSorted)
            {
                if (!renderNode->active) { continue; }

                Profiler::GetInstance()->
                    AddProfilerCpuSpot(legit::Colors::getColor(idx), "Rp: " + renderNode->passName);
                RenderGraphNode* node = renderNode;
                bool dependancyNeed = false;
                std::string dependancyName = "";
                for (auto& passName : allPassesNames)
                {
                    if (node->dependencies.contains(passName))
                    {
                        dependancyNeed = true;
                        dependancyName = passName;
                    }
                }
                if (dependancyNeed)
                {
                    RenderGraphNode* dependancyNode = renderNodes.at(dependancyName).get();
                    if (!dependancyNode->active)
                    {
                    }
                    BufferUsageTypes lastNodeType = (dependancyNode->pipelineType == vk::PipelineBindPoint::eGraphics)
                                                        ? B_GRAPHICS_WRITE
                                                        : B_COMPUTE_WRITE;
                    BufferUsageTypes currNodeType = (node->pipelineType == vk::PipelineBindPoint::eGraphics)
                                                        ? B_GRAPHICS_WRITE
                                                        : B_COMPUTE_WRITE;
                    BufferAccessPattern lastNodePattern = GetSrcBufferAccessPattern(lastNodeType);
                    BufferAccessPattern currNodePattern = GetSrcBufferAccessPattern(currNodeType);
                    CreateMemBarrier(lastNodePattern, currNodePattern, currentFrameResources->commandBuffer.get());
                }
                node->Execute(currentFrameResources->commandBuffer.get());
                Profiler::GetInstance()->EndProfilerCpuSpot("Rp: " + renderNode->passName);
                allPassesNames.push_back(node->passName);
                idx++;
            }
        }
    };
}


#endif //RENDERGRAPH_HPP
