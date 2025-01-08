//









// Created by carlo on 2024-10-07.
//






#ifndef FORWARDRENDERER_HPP
#define FORWARDRENDERER_HPP

namespace Rendering
{
    class ForwardRenderer : BaseRenderer 
    {
    public:
        ForwardRenderer(ENGINE::Core* core, WindowProvider* windowProvider,
                        ENGINE::DescriptorAllocator* descriptorAllocator)
        {
            this->core = core;
            this->renderGraphRef = core->renderGraphRef;
            this->windowProvider = windowProvider;
            this->descriptorAllocatorRef = descriptorAllocator;
            auto logicalDevice = core->logicalDevice.get();
            auto physicalDevice = core->physicalDevice;
            descriptorCache = std::make_unique<ENGINE::DescriptorCache>(this->core);

            camera.SetLookAt(glm::vec3(0.0f));

            std::string modelPath = SYSTEMS::OS::GetInstance()->GetAssetsPath() + "\\Models\\3d_pbr_curved_sofa\\scene.gltf";

             model = RenderingResManager::GetInstance()->GetModel(modelPath);

            vertexBuffer = std::make_unique<ENGINE::Buffer>(
                physicalDevice, logicalDevice, vk::BufferUsageFlagBits::eVertexBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                sizeof(M_Vertex3D) * model->vertices.size(), model->vertices.data());
            indexBuffer = std::make_unique<ENGINE::Buffer>(
                physicalDevice, logicalDevice, vk::BufferUsageFlagBits::eIndexBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                sizeof(uint32_t) * model->indices.size(), model->indices.data());
            
            std::string resourcesPath = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath();

            imageShipper = ENGINE::ResourcesManager::GetInstance()->GetShipper(
                "ForwardShipper", resourcesPath + "\\Images\\default_texture.jpg", 1, 1,
                renderGraphRef->core->swapchainRef->GetFormat(), ENGINE::GRAPHICS_READ);

            
            defaultImageShipper = ENGINE::ResourcesManager::GetInstance()->GetShipperFromName("default_tex");
 
            ENGINE::ImageView* computeStorage = renderGraphRef->GetImageResource("storageImage");

            //sample for storage bindless
            imagesArray.push_back(computeStorage);
            imagesArray.push_back(computeStorage);
            imagesArray.push_back(computeStorage);

            std::string shadersPath = SYSTEMS::OS::GetInstance()->GetShadersPath();
            
            vertShader = std::make_unique<ENGINE::Shader>(logicalDevice, shadersPath + "\\spirv\\Examples\\fSample.vert.spv");
            fragShader = std::make_unique<ENGINE::Shader>(logicalDevice, shadersPath + "\\spirv\\Examples\\fSample.frag.spv");
           
            ENGINE::DescriptorLayoutBuilder builder;
            
            vertShader->sParser->GetLayout(builder);
            fragShader->sParser->GetLayout(builder);

            //automatic descriptor handler
            descriptorCache->AddShaderInfo(vertShader->sParser.get());
            descriptorCache->AddShaderInfo(fragShader->sParser.get());
            descriptorCache->BuildDescriptorsCache(descriptorAllocatorRef, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment );

            auto pushConstantRange = vk::PushConstantRange()
            .setOffset(0)
            .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
            .setSize(sizeof(MvpPc));

            auto layoutCreateInfo = vk::PipelineLayoutCreateInfo()
                                    .setSetLayoutCount(1).
                                    setPushConstantRanges(pushConstantRange).
                                    setPSetLayouts(&descriptorCache->dstLayout.get());
            
            
            ENGINE::VertexInput vertexInput= M_Vertex3D::GetVertexInput();

            ENGINE::AttachmentInfo colInfo = ENGINE::GetColorAttachmentInfo(
                glm::vec4(0.0f, 0.1f, 0.1f, 1.0f), core->swapchainRef->GetFormat());
            ENGINE::AttachmentInfo depthInfo = ENGINE::GetDepthAttachmentInfo();
            forwardPassName = "ForwardPass";
            auto renderNode = renderGraphRef->AddPass(forwardPassName);
            
            renderNode->SetVertShader(vertShader.get());
            renderNode->SetFragShader(fragShader.get());
            renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
            renderNode->SetPipelineLayoutCI(layoutCreateInfo);
            renderNode->SetVertexInput(vertexInput);
            renderNode->AddColorAttachmentOutput("color", colInfo);
            renderNode->SetDepthAttachmentOutput("depth", depthInfo);
            renderNode->AddColorBlendConfig(ENGINE::BlendConfigs::B_OPAQUE);
            renderNode->SetDepthConfig(ENGINE::DepthConfigs::D_ENABLE);
            renderNode->AddSamplerResource("sampler", imageShipper->imageView.get());
            renderNode->AddStorageResource("storageImage", computeStorage);
            renderNode->BuildRenderGraphNode();
            
        }
        ~ForwardRenderer() override 
        {
        }

        void RecreateSwapChainResources() override 
        {
        }

        void SetRenderOperation(ENGINE::InFlightQueue* inflightQueue) override 
        {
            auto setViewTask = new std::function<void()>([this, inflightQueue]()
            {
                auto* currImage = inflightQueue->currentSwapchainImageView;
                auto* currDepthImage = core->swapchainRef->depthImagesFull.at(inflightQueue->frameIndex).imageView.get();
                renderGraphRef->AddColorImageResource("ForwardPass", "color", currImage);
                renderGraphRef->SetDepthImageResource("ForwardPass", "depth", currDepthImage);
                renderGraphRef->GetNode("ForwardPass")->SetFramebufferSize(windowProvider->GetWindowSize());
            });

            auto renderOp = new std::function<void(vk::CommandBuffer& command_buffer)>(
                [this](vk::CommandBuffer& commandBuffer)
                {

                    //ssbo sample
                    ssbo.clear();
                    ssbo.push_back(pc);
                    ssbo.push_back(pc);
                    ssbo.push_back(pc);
                    
                    //IMPORTANT
                    //image binding always should be done in the render operation, because it guarantees that the layout will be correct, otherwise layout errors can happen

                    descriptorCache->SetSampler("testImage", imageShipper->imageView.get(), imageShipper->sampler);
                    descriptorCache->SetStorageImageArray("storagesImgs", imagesArray);
                    descriptorCache->SetBuffer("Camera", pc);
                    descriptorCache->SetBuffer("CameraBuffer", ssbo);
                    ENGINE::ImageView* computeStorage = renderGraphRef->GetImageResource("storageImage");
                    descriptorCache->SetStorageImage("storageImg", computeStorage);
                    
                    vk::DeviceSize offset = 0;
                    commandBuffer.bindDescriptorSets(renderGraphRef->GetNode(forwardPassName)->pipelineType,
                                                     renderGraphRef->GetNode(forwardPassName)->pipelineLayout.get(), 0, 1,
                                                     &descriptorCache->dstSet.get(), 0 , nullptr);

                    commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer->bufferHandle.get(), &offset);
                    commandBuffer.bindIndexBuffer(indexBuffer->bufferHandle.get(), 0, vk::IndexType::eUint32);
                    for (int i = 0; i < model->meshCount; ++i)
                    {
                        camera.SetPerspective(
                            45.0f, (float)windowProvider->GetWindowSize().x / (float)windowProvider->GetWindowSize().y,
                            0.1f, 512.0f);
                        pc.projView = camera.matrices.perspective * camera.matrices.view;
                        pc.model = model->modelsMat[i];
                        // pc.model = glm::scale(pc.model, glm::vec3(0.01f));
                    
                        commandBuffer.pushConstants(renderGraphRef->GetNode(forwardPassName)->pipelineLayout.get(),
                                                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                                    0, sizeof(MvpPc), &pc);
                    
                        commandBuffer.drawIndexed(model->indicesCount[i], 1, model->firstIndices[i],
                                                  static_cast<int32_t>(model->firstVertices[i]), 0);
                    }
           
                });
            
            renderGraphRef->GetNode(forwardPassName)->AddTask(setViewTask);
            renderGraphRef->GetNode(forwardPassName)->SetRenderOperation(renderOp);
        }


        void ReloadShaders() override 
        {
            auto renderNode = renderGraphRef->GetNode(forwardPassName);
            renderNode->RecreateResources();
        }


        ENGINE::DescriptorAllocator* descriptorAllocatorRef;
        WindowProvider* windowProvider;
        ENGINE::Core* core;
        ENGINE::RenderGraph* renderGraphRef;

        std::unique_ptr<ENGINE::DescriptorCache> descriptorCache;
        ENGINE::DescriptorWriterBuilder writerBuilder;
        vk::UniqueDescriptorSetLayout dstLayout;
        vk::UniqueDescriptorSet dstSet;

        std::string forwardPassName;
        ENGINE::ImageShipper* imageShipper;
        ENGINE::ImageShipper* defaultImageShipper;
        std::vector<ENGINE::ImageView*> imagesArray;
        
        std::unique_ptr<ENGINE::Buffer> vertexBuffer;
        std::unique_ptr<ENGINE::Buffer> indexBuffer;
        
        std::unique_ptr<ENGINE::Shader> vertShader;
        std::unique_ptr<ENGINE::Shader> fragShader;
        
       
        Camera camera = {glm::vec3(3.0f), Camera::CameraMode::E_FIXED};
        Model* model;
        MvpPc pc{};
        std::vector<MvpPc> ssbo{};

    };
}

#endif //FORWARDRENDERER_HPP
