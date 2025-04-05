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
                "\\PointClouds\\goat_skull_ply_1\\Goat skull.ply";
            std::vector<glm::vec3> positions;
            RenderingResManager::GetInstance()->LoadPLY(path, positions);
            gaussians.Init(positions);
        }

        void CreateBuffers()
        {
            
            gsPointsVertexBuffer = this->renderGraph->resourcesManager->GetStageBuffer(
                "gsPointsVertexBuffer", vk::BufferUsageFlagBits::eVertexBuffer,
                sizeof(GS_Vertex3D) * gaussians.gsVertex3Ds.size(),
                gaussians.gsVertex3Ds.data())->deviceBuffer.get();
        }

        void CreatePipelines()
        {
            
            AttachmentInfo colInfo = GetColorAttachmentInfo(
                glm::vec4(0.0f), core->swapchainRef->GetFormat());
            auto imageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1, ENGINE::g_32bFormat,ENGINE::colorImageUsage);
            // ImageView* attachmentOutput = renderGraph->resourcesManager->GetImage("shOutput", imageInfo, 0, 0);
            
            auto renderNode = RenderingResManager::GetInstance()->GetTemplateNode_DF(passName, "GSLoad", C_GLSL);
            renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
            renderNode->SetVertexInput(GS_Vertex3D::GetVertexInput());
            renderNode->SetPushConstantSize(sizeof(MvpPc));
            renderNode->AddColorAttachmentOutput("DisplayAttachment", colInfo, BlendConfigs::B_OPAQUE);
            renderNode->SetGraphicsPipelineConfigs({R_POINT, T_POINT_LIST});
            renderNode->BuildRenderGraphNode();
            
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
                    auto renderNode = renderGraph->GetNode(passName);
                    renderNode->AddColorImageResource("DisplayAttachment", renderGraph->currentBackBuffer);
                });
            auto renderOp = new std::function<void()>(
                [this]()
                {

                    
                    auto& renderNode = renderGraph->renderNodes.at(passName);
                    renderNode->descCache->SetBuffer("GSMats", gaussians.covarianceMats);
                    // renderNode->descCache->SetBuffer("GSCols", gaussians.cols);
                    // renderNode->descCache->SetBuffer("GSAlphas", gaussians.alphas);
                    
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
                        0, sizeof(MvpPc), &mvpPc);
                    
                    renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(
                        0, 1,
                        &gsPointsVertexBuffer->bufferHandle.get(),
                        &offset);
                    
                    renderGraph->currentFrameResources->commandBuffer->draw(
                        gaussians.gsVertex3Ds.size(), 1, 0, 0);
                    
                    
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
            mvpPc.projView = camera.matrices.perspective * camera.matrices.view;
        }

        void ReloadShaders() override
        {
        }


        std::string passName = "gsDisplayPass";

        Core* core;
        RenderGraph* renderGraph;
        WindowProvider* windowProvider;
        ENGINE::Buffer* gsPointsVertexBuffer = nullptr;
        ArraysOfGaussians gaussians = {};
        MvpPc mvpPc = {};
        Camera camera = {glm::vec3(5.0f), Camera::CameraMode::E_FREE};

        ImageView* colAttachmentView;
    };
}

#endif //GSRENDERER_HPP
