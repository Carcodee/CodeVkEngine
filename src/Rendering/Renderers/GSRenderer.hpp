//



// Created by carlo on 2025-03-26.
//












#ifndef GSRENDERER_HPP
#define GSRENDERER_HPP

namespace Rendering
{
    using namespace ENGINE;
    class GSRenderer : public BaseRenderer
    {
    public:
        ~GSRenderer() override = default;

        GSRenderer(Core* core, WindowProvider* windowProvider)
        {
            this->core = core;
            this->renderGraph = core->renderGraphRef;
            this->windowProvider = windowProvider;

            CreateResources();
            CreateBuffers();
            CreatePipelines();
        }

        void CreateResources()
        {
            std::string path = SYSTEMS::OS::GetInstance()->GetAssetsPath() +
                "\\PointClouds\\Train\\train30000.ply";
            std::vector<GaussianSplat> gaussianSplats;
            // for (auto& file : std::filesystem::directory_iterator(path))
            // {
            // for (auto& value : std::filesystem::directory_iterator(file))
            // {
            //     RenderingResManager::GetInstance()->LoadGS(value.path().string(), gaussianSplats);
            //     break;
            // }
            //      
            // }
            RenderingResManager::GetInstance()->LoadGS(path, gaussianSplats);
            
            gaussians.Init(gaussianSplats, camera.cameraProperties.fov, 1024.0, 1024.0);


            auto indicesSorted = gaussians.SortByDepth(splitMvp.view);
            for (int i = 0; i < gaussians.pos.size(); ++i)
            {
                ENGINE::DrawIndirectIndexedCmd indirectCmd{};
                indirectCmd.firstIndex = 0;
                indirectCmd.firstInstance = indicesSorted[i];
                indirectCmd.indexCount = Vertex2D::GetQuadIndices().size();
                indirectCmd.instanceCount = 1;
                indirectCmd.vertexOffset = 0;
                indexedCmds.emplace_back(indirectCmd);
            }
            indirectBuffer = ENGINE::ResourcesManager::GetInstance()->GetBuffer(
                "gsIndirectDraw", vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
                vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                sizeof(ENGINE::DrawIndirectIndexedCmd) * indexedCmds.size(), indexedCmds.data()); 
        }
        void ReSort()
        {
            auto indicesSorted = gaussians.SortByDepth(splitMvp.view);
            indexedCmds.clear();
            for (int i = 0; i < gaussians.pos.size(); ++i)
            {
                ENGINE::DrawIndirectIndexedCmd indirectCmd{};
                indirectCmd.firstIndex = 0;
                indirectCmd.firstInstance = indicesSorted[i];
                indirectCmd.indexCount = Vertex2D::GetQuadIndices().size();
                indirectCmd.instanceCount = 1;
                indirectCmd.vertexOffset = 0;
                indexedCmds.emplace_back(indirectCmd);
            }
            indirectBuffer = ENGINE::ResourcesManager::GetInstance()->SetBuffer(
                "gsIndirectDraw", sizeof(ENGINE::DrawIndirectIndexedCmd) * indexedCmds.size(), indexedCmds.data()); 
        }

        void CreateBuffers()
        {
            
            gsPointsVertexBuffer = this->renderGraph->resourcesManager->GetStageBuffer(
                "gsPointsVertexBuffer", vk::BufferUsageFlagBits::eVertexBuffer,
                sizeof(Vertex2D) * Vertex2D::GetQuadVertices().size(),
                Vertex2D::GetQuadVertices().data())->deviceBuffer.get();
            gsPointsIndexBuffer = this->renderGraph->resourcesManager->GetStageBuffer(
                "gsPointsIndexBuffer", vk::BufferUsageFlagBits::eIndexBuffer,
                sizeof(Vertex2D) * Vertex2D::GetQuadIndices().size(),
                Vertex2D::GetQuadIndices().data())->deviceBuffer.get();
        }

        void CreatePipelines()
        {
            
            AttachmentInfo colInfo = GetColorAttachmentInfo(
                glm::vec4(0.0f), core->swapchainRef->GetFormat());
            auto imageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1, ENGINE::g_32bFormat,ENGINE::colorImageUsage);
            // ImageView* attachmentOutput = renderGraph->resourcesManager->GetImage("shOutput", imageInfo, 0, 0);
            
            auto renderNode = RenderingResManager::GetInstance()->GetTemplateNode_DF(passName, "GSLoad", C_GLSL);
            renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
            renderNode->SetVertexInput(Vertex2D::GetVertexInput());
            renderNode->SetPushConstantSize(sizeof(SplitMVP));
            renderNode->AddColorAttachmentOutput("DisplayAttachment", colInfo, BlendConfigs::B_ALPHA_BLEND);
            renderNode->SetGraphicsPipelineConfigs({R_FILL, T_TRIANGLE});
            renderNode->BuildRenderGraphNode();
            
            renderNode->descCache->SetBuffer("GSScale", gaussians.scales);
            renderNode->descCache->SetBuffer("GSRot", gaussians.rots);
            renderNode->descCache->SetBuffer("GSPos", gaussians.pos);
            renderNode->descCache->SetBuffer("GSCols", gaussians.cols);
            renderNode->descCache->SetBuffer("GSAlphas", gaussians.alphas);
            renderNode->descCache->SetBuffer("HFov", gaussians.hFovFocal);
            
        }

        void RecreateSwapChainResources() override
        {
        }

        void SetRenderOperation() override
        {
            auto taskOp = new std::function<void()>(
                [this]
                {
                    MoveCam();
                    splitMvp.model = glm::identity<glm::mat4>();
                    
                    auto renderNode = renderGraph->GetNode(passName);
                    renderNode->AddColorImageResource("DisplayAttachment", renderGraph->currentBackBuffer);
                });
            auto renderOp = new std::function<void()>(
                [this]()
                {

                    
                    auto& renderNode = renderGraph->renderNodes.at(passName);
                    
                    renderGraph->currentFrameResources->commandBuffer->bindDescriptorSets(renderNode->pipelineType,
                                                     renderNode->pipelineLayout.get(), 0,
                                                     1,
                                                     &renderNode->descCache->dstSet, 0, nullptr);
                    renderGraph->currentFrameResources->commandBuffer->bindPipeline(renderNode->pipelineType, renderNode->pipeline.get());
                    vk::DeviceSize offset = 0;
                    
                    renderGraph->currentFrameResources->commandBuffer->pushConstants(
                        renderGraph->GetNode(passName)->pipelineLayout.get(),
                        vk::ShaderStageFlagBits::eVertex |
                        vk::ShaderStageFlagBits::eFragment,
                        0, sizeof(SplitMVP), &splitMvp);
                    
                    renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(
                        0, 1,
                        &gsPointsVertexBuffer->bufferHandle.get(),
                        &offset);

                    renderGraph->currentFrameResources->commandBuffer->bindIndexBuffer(
                        gsPointsIndexBuffer->bufferHandle.get(), 0, vk::IndexType::eUint32);
                    // renderGraph->currentFrameResources->commandBuffer->drawIndexed(
                    //     Vertex2D::GetQuadIndices().size(), gaussians.pos.size(), 0, 0, 0);

                    vk::DeviceSize sizeOffset = 0;
                    uint32_t stride = sizeof(DrawIndirectIndexedCmd);
                    renderGraph->currentFrameResources->commandBuffer->drawIndexedIndirect(
                        indirectBuffer->bufferHandle.get(),
                        sizeOffset,
                        gaussians.pos.size(),
                        stride);
                    
                });
            renderGraph->GetNode(passName)->SetRenderOperation(renderOp);
            renderGraph->GetNode(passName)->AddTask(taskOp);
        }

        void MoveCam()
        {
            glm::vec2 input = glm::vec2(0.0f);
            if (glfwGetKey(windowProvider->window, GLFW_KEY_W)) { input += glm::vec2(0.0f, 1.0f); }
            if (glfwGetKey(windowProvider->window, GLFW_KEY_S)) { input += glm::vec2(0.0f, -1.0f); }
            if (glfwGetKey(windowProvider->window, GLFW_KEY_D)) { input += glm::vec2(1.0f, 0.0f); }
            if (glfwGetKey(windowProvider->window, GLFW_KEY_A)) { input += glm::vec2(-1.0f, 0.0f); }
            if (glfwGetKey(windowProvider->window, GLFW_KEY_LEFT_SHIFT))
            {
                camera.movementSpeed = 40;
            }
            else
            {
                camera.movementSpeed = 5;
            }
            input = glm::clamp(input, glm::vec2(-1.0, -1.0), glm::vec2(1.0, 1.0));
            glm::vec2 mouseInput = glm::vec2(-ImGui::GetMousePos().x, ImGui::GetMousePos().y);
            camera.mouseInput = mouseInput;
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
            splitMvp.view = camera.matrices.view;
            splitMvp.proj = camera.matrices.perspective;
        }

        void ReloadShaders() override
        {
        }


        std::string passName = "gsDisplayPass";

        Core* core;
        RenderGraph* renderGraph;
        WindowProvider* windowProvider;
        ENGINE::Buffer* gsPointsVertexBuffer = nullptr;
        ENGINE::Buffer* gsPointsIndexBuffer = nullptr;
        ArraysOfGaussians gaussians = {};
        SplitMVP splitMvp = {};
        Camera camera = {glm::vec3(5.0f), Camera::CameraMode::E_FREE};

        Buffer* indirectBuffer; 
        ImageView* colAttachmentView;
        std::vector<DrawIndirectIndexedCmd> indexedCmds;
    };
}

#endif //GSRENDERER_HPP
