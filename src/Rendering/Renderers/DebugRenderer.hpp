//
// Created by carlo on 2024-12-04.
//






#ifndef DEBUGRENDERER_HPP
#define DEBUGRENDERER_HPP
namespace Rendering{
    using namespace ENGINE;
    class DebugRenderer : public BaseRenderer {
    public:
        
        ~DebugRenderer() override = default;
        
        DebugRenderer(Core* core, WindowProvider* windowProvider,
                        std::map<std::string, std::unique_ptr<BaseRenderer>>& renderers)
        {
            this->core = core;
            this->renderGraph = core->renderGraphRef;
            this->windowProvider = windowProvider;
            this->mDebuggerCache = std::make_unique<DescriptorCache>(core);
            if (renderers.contains("ClusterRenderer"))
            {
                this->clusterRenderer = dynamic_cast<ClusterRenderer*>(renderers.at("ClusterRenderer").get());
            }

            CreateResources();
            CreateBuffers();
            CreatePipelines();
        }
        void CreateResources()
        {
            auto imageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
                                                         vk::Format::eR32G32B32A32Sfloat,
                                                         vk::ImageUsageFlagBits::eColorAttachment |
                                                         vk::ImageUsageFlagBits::eSampled);

            //image view not used right now
            colAttachmentView = ResourcesManager::GetInstance()->GetImage("debugColAttachment", imageInfo, 0, 0);
            SetViewCamera();
        }
        
        void CreateBuffers()
        {
            rawVerticesBuff = ResourcesManager::GetInstance()->GetBuffer(
                "debugRawVertices", vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                1); 
        }
        void CreatePipelines()
        {
            if (clusterRenderer)
            {
                auto logicalDevice = core->logicalDevice.get();
                std::string shaderPath = SYSTEMS::OS::GetInstance()->GetShadersPath();
                modelVShader = std::make_unique<Shader>(logicalDevice,
                                                        shaderPath + "\\spirvGlsl\\DebugRendering\\debug.vert.spv", S_VERT);
                modelFShader = std::make_unique<Shader>(logicalDevice,
                                                        shaderPath + "\\spirvGlsl\\DebugRendering\\debug.frag.spv", S_FRAG);
                mDebuggerCache->AddShaderInfo(modelVShader->sParser.get());
                mDebuggerCache->AddShaderInfo(modelFShader->sParser.get());
                mDebuggerCache->BuildDescriptorsCache(vk::ShaderStageFlagBits::eVertex |
                                                      vk::ShaderStageFlagBits::eFragment);

                auto pushConstantRange = vk::PushConstantRange()
                                         .setOffset(0)
                                         .setStageFlags(
                                             vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
                                         .setSize(sizeof(MvpPc));

                auto layoutCreateInfo = vk::PipelineLayoutCreateInfo()
                                        .setSetLayoutCount(1)
                                        .setPushConstantRanges(pushConstantRange)
                                        .setPSetLayouts(&mDebuggerCache->dstLayout.get());

                VertexInput vertexInput = D_Vertex3D::GetVertexInput();
                AttachmentInfo colInfo = GetColorAttachmentInfo(
                    glm::vec4(0.0f), core->swapchainRef->GetFormat(), vk::AttachmentLoadOp::eLoad);
                // AttachmentInfo depthInfo = GetDepthAttachmentInfo();
                auto renderNode = renderGraph->AddPass(mDebuggerPassName);

                renderNode->SetVertShader(modelVShader.get());
                renderNode->SetFragShader(modelFShader.get());
                renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
                renderNode->SetPipelineLayoutCI(layoutCreateInfo);
                renderNode->SetVertexInput(vertexInput);
                renderNode->AddColorAttachmentOutput("modelCol", colInfo, BlendConfigs::B_OPAQUE);
                renderNode->SetRasterizationConfigs(RasterizationConfigs::R_LINE);
                renderNode->SetDepthConfig(DepthConfigs::D_NONE);
                renderNode->DependsOn("light");
                renderNode->BuildRenderGraphNode();
            }

        }
        
        void RecreateSwapChainResources() override
        {
            
        }
        void SetRenderOperation() override
        {
            if (clusterRenderer)
            {
                auto debugTask = new std::function<void()>([this]()
                {
                    SetViewCamera();
                    CreateFrustumVertices();
                    rawVerticesBuff = ResourcesManager::GetInstance()->SetBuffer(
                        "debugRawVertices", sizeof(D_Vertex3D) * raw3DVertices.size(), raw3DVertices.data());
                    auto* currImage = renderGraph->currentBackBuffer;
                    renderGraph->AddColorImageResource(mDebuggerPassName, "modelCol", currImage);
                    renderGraph->GetNode(mDebuggerPassName)->SetFramebufferSize(windowProvider->GetWindowSize());
                });
                auto renderOp = new std::function<void()>(
                    [this]()
                    {
                        vk::DeviceSize offset = 0;
                        auto node = renderGraph->GetNode(mDebuggerPassName);
                        renderGraph->currentFrameResources->commandBuffer->bindDescriptorSets(node->pipelineType,
                                                         node->pipelineLayout.get(), 0,
                                                         1,
                                                         &mDebuggerCache->dstSet, 0, nullptr);

                        renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(0, 1, &rawVerticesBuff->bufferHandle.get(), &offset);

                        pc.projView = currentViewCam->matrices.perspective * currentViewCam->matrices.view;
                        pc.model = glm::mat4(1.0);

                        renderGraph->currentFrameResources->commandBuffer->pushConstants(node->pipelineLayout.get(),
                                                    vk::ShaderStageFlagBits::eVertex |
                                                    vk::ShaderStageFlagBits::eFragment,
                                                    0, sizeof(MvpPc), &pc);
                        renderGraph->currentFrameResources->commandBuffer->draw(raw3DVertices.size(), 1, 0, 0);
                    });

                renderGraph->GetNode(mDebuggerPassName)->AddTask(debugTask);
                renderGraph->GetNode(mDebuggerPassName)->SetRenderOperation(renderOp);
            }
        }
        void ReloadShaders() override
        {
            auto node = renderGraph->GetNode(mDebuggerPassName);
            if(node)
            {
                node->RecreateResources();
            }
        }
        void SetViewCamera()
        {
            currentViewCam = &clusterRenderer->camera;   
        }


        void CreateFrustumVertices()
        {

            glm::vec4 nearTopL = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(clusterRenderer->camFrustum.points[0], 1.0);
            glm::vec4 nearTopR = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(clusterRenderer->camFrustum.points[1], 1.0);
            glm::vec4 nearBottomL = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(clusterRenderer->camFrustum.points[2], 1.0);
            glm::vec4 nearBottomR = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(clusterRenderer->camFrustum.points[3], 1.0);
            glm::vec4 farTopL = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(clusterRenderer->camFrustum.points[4], 1.0);
            glm::vec4 farTopR = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(clusterRenderer->camFrustum.points[5], 1.0);
            glm::vec4 farBottomL = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(clusterRenderer->camFrustum.points[6], 1.0);
            glm::vec4 farBottomR = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(clusterRenderer->camFrustum.points[7], 1.0);

            raw3DVertices.clear();
            
            // Left face: 0,2,6 and 0,6,4
            raw3DVertices.push_back({ nearTopL });
            raw3DVertices.push_back({ nearBottomL });
            raw3DVertices.push_back({ farBottomL });
            raw3DVertices.push_back({ nearTopL });
            raw3DVertices.push_back({ farBottomL });
            raw3DVertices.push_back({ farTopL });

            // Right face: 1,3,7 and 1,7,5
            raw3DVertices.push_back({ nearTopR });
            raw3DVertices.push_back({ nearBottomR });
            raw3DVertices.push_back({ farBottomR });
            raw3DVertices.push_back({ nearTopR });
            raw3DVertices.push_back({ farBottomR });
            raw3DVertices.push_back({ farTopR });

            // Top face: 0,1,5 and 0,5,4
            raw3DVertices.push_back({ nearTopL });
            raw3DVertices.push_back({ nearTopR });
            raw3DVertices.push_back({ farTopR });
            raw3DVertices.push_back({ nearTopL });
            raw3DVertices.push_back({ farTopR });
            raw3DVertices.push_back({ farTopL });

            // Bottom face: 2,3,7 and 2,7,6
            raw3DVertices.push_back({ nearBottomL });
            raw3DVertices.push_back({ nearBottomR });
            raw3DVertices.push_back({ farBottomR });
            raw3DVertices.push_back({ nearBottomL });
            raw3DVertices.push_back({ farBottomR });
            raw3DVertices.push_back({ farBottomL });
            
        }
        std::vector<D_Vertex3D> raw3DVertices;
        std::vector<Model> models;
        std::unique_ptr<Shader> modelFShader;
        std::unique_ptr<Shader> modelVShader;
        std::string mDebuggerPassName = "ModelDebugger";
        
        Core* core;
        RenderGraph* renderGraph;
        WindowProvider* windowProvider;
        std::unique_ptr<DescriptorCache> mDebuggerCache;

        ImageView* colAttachmentView;
        Buffer* rawVerticesBuff;
        Camera* currentViewCam;
        
        MvpPc pc{};
        ClusterRenderer* clusterRenderer = nullptr;

    };
}
#endif //DEBUGRENDERER_HPP
