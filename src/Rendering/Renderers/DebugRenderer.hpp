//
// Created by carlo on 2024-12-04.
//

#ifndef DEBUGRENDERER_HPP
#define DEBUGRENDERER_HPP

namespace Rendering
{
    using namespace ENGINE;

    class DebugRenderer : public BaseRenderer
    {
    public:
        ~DebugRenderer() override = default;

        DebugRenderer(Core* core, WindowProvider* windowProvider,
                      std::map<std::string, std::unique_ptr<BaseRenderer>>& renderers)
        {
            this->core = core;
            this->renderGraph = core->renderGraphRef;
            this->windowProvider = windowProvider;
            if (renderers.contains("ClusterRenderer"))
            {
                this->clusterRenderer = dynamic_cast<ClusterRenderer*>(renderers.at("ClusterRenderer").get());
            }

            if (renderers.contains("CFDRenderer"))
            {
                this->CFDRenderer = dynamic_cast<Rendering::CFDRenderer*>(renderers.at("CFDRenderer").get());
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

            // image view not used right now
            //  colAttachmentView = ResourcesManager::GetInstance()->GetImage("debugColAttachment", imageInfo, 0, 0);
            SetViewCamera();
        }

        void CreateBuffers()
        {
            rawVerticesBuff = ResourcesManager::GetInstance()->GetBuffer(
                ResourcesManager::BufferParams{
                    "debugRawVertices", vk::BufferUsageFlagBits::eVertexBuffer,
                    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                    1
                });

            auto* models_voxels = ResourcesManager::GetInstance()->GetBuffer(
                ResourcesManager::BufferParams{
                    "models_voxels", vk::BufferUsageFlagBits::eStorageBuffer,
                    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                    CFDRenderer->voxelGrid.pcVoxelGrid.GetSize(), sizeof(glm::mat4)
                });
        }

        void CreatePipelines()
        {
            std::string shaderPath = SYSTEMS::OS::GetInstance()->GetShadersPath();

            AttachmentInfo colInfo = GetColorAttachmentInfo(BlendConfigs::B_OPAQUE,
                                                            glm::vec4(0.0f), core->swapchainRef->GetFormat(),
                                                            vk::AttachmentLoadOp::eLoad);
            if (clusterRenderer)
            {
                auto modelVShader = renderGraph->resourcesManager->GetShader(
                    shaderPath + "\\spirvGlsl\\DebugRendering\\debug.vert.spv", S_VERT);

                auto modelFShader = renderGraph->resourcesManager->GetShader(
                    shaderPath + "\\spirvGlsl\\DebugRendering\\debug.frag.spv", S_FRAG);

                auto renderNode = renderGraph->AddPass(mDebuggerPassName);
                renderNode->G_SetVertShader(modelVShader);
                renderNode->G_SetFragShader(modelFShader);
                renderNode->G_SetFramebufferSize(renderGraph->currentBackBuffer->imageData->GetImageSize());
                renderNode->G_SetPushConstantSize(sizeof(MvpPc));
                renderNode->G_SetVertexInput( D_Vertex3D::GetVertexInput());
                renderNode->G_AddColorAttachmentOutput(0, colInfo);
                renderNode->G_SetColorImageAttachmentBinding(0, renderGraph->currentBackBuffer);
                renderNode->G_SetGraphicsPipelineConfigs({RasterizationConfigs::R_LINE, TopologyConfigs::T_TRIANGLE});
                renderNode->G_SetDepthConfig(DepthConfigs::D_NONE);
                renderNode->DependsOn("light");
                renderNode->BuildRenderGraphNode();

                auto debugTask = new std::function<void()>([this]()
                {
                    SetViewCamera();
                    CreateFrustumVertices();
                    rawVerticesBuff = ResourcesManager::GetInstance()->SetBuffer(
                        "debugRawVertices", sizeof(D_Vertex3D) * raw3DVertices.size(), sizeof(D_Vertex3D),
                        raw3DVertices.data());
                });
                auto renderOp = new std::function<void()>(
                    [this]()
                    {
                        vk::DeviceSize offset = 0;
                        auto node = renderGraph->GetNode(mDebuggerPassName);

                        node->GetCurrCmd().bindVertexBuffers(0, 1, &rawVerticesBuff->bufferHandle.get(), &offset);

                        pc.projView = currentViewCam->GetViewProj();
                        pc.model = glm::mat4(1.0);

                        node->GetCurrCmd().pushConstants(node->GPUPipelineRef->pipelineLayout.get(),
                                                         vk::ShaderStageFlagBits::eVertex |
                                                         vk::ShaderStageFlagBits::eFragment,
                                                         0, sizeof(MvpPc), &pc);
                        node->GetCurrCmd().draw(raw3DVertices.size(), 1, 0, 0);
                    });

                renderGraph->GetNode(mDebuggerPassName)->AddPreRenderingTask(debugTask);
                renderGraph->GetNode(mDebuggerPassName)->G_SetRenderOperation(renderOp);
            }
            if (CFDRenderer)
            {

                auto modelVShader = renderGraph->resourcesManager->GetShader(
                    shaderPath + "\\spirvGlsl\\DebugRendering\\debugCFD.vert.spv", S_VERT);
                auto modelFShader = renderGraph->resourcesManager->GetShader(
                    shaderPath + "\\spirvGlsl\\DebugRendering\\debugCFD.frag.spv", S_FRAG);
                auto renderNode = renderGraph->AddPass("voxelizer_viewer");
                renderNode->G_SetVertShader(modelVShader);
                renderNode->G_SetFragShader(modelFShader);
                renderNode->G_SetFramebufferSize(renderGraph->currentBackBuffer->imageData->GetImageSize());
                renderNode->G_SetPushConstantSize(sizeof(DebugVoxelsMVP));
                renderNode->G_SetVertexInput(D_Vertex3D::GetVertexInput());
                renderNode->G_AddColorAttachmentOutput(0, colInfo);
                renderNode->G_SetColorImageAttachmentBinding(0, renderGraph->currentBackBuffer);
                renderNode->G_SetGraphicsPipelineConfigs({RasterizationConfigs::R_LINE, TopologyConfigs::T_TRIANGLE});
                renderNode->G_SetDepthConfig(DepthConfigs::D_NONE);
                if (clusterRenderer)
                {
                    renderNode->DependsOn("light");
                    renderNode->DependsOn("ModelDebugger");
                }
                renderNode->DependsOn("voxelizer");
                renderNode->BuildRenderGraphNode();

                renderNode->G_SetRenderOperation(new std::function<void()>(
                    [this]()
                    {
                        debugVoxelsPc.projView = currentViewCam->GetViewProj();
                        debugVoxelsPc.origin = CFDRenderer->voxelGrid.pcVoxelGrid.origin;
                        debugVoxelsPc.cellSize = CFDRenderer->voxelGrid.pcVoxelGrid.cellSize;
                        debugVoxelsPc.dimensions = CFDRenderer->voxelGrid.pcVoxelGrid.dimensions;

                        vk::DeviceSize offset = 0;
                        auto node = renderGraph->GetNode("voxelizer_viewer");

                        node->GetCurrCmd().pushConstants(node->GPUPipelineRef->pipelineLayout.get(),
                                                         vk::ShaderStageFlagBits::eVertex |
                                                         vk::ShaderStageFlagBits::eFragment,
                                                         0, sizeof(DebugVoxelsMVP), &debugVoxelsPc);
                        node->GetCurrCmd().bindVertexBuffers(
                            0, 1, &renderGraph->resourcesManager->GetStagedBuffFromName("cube_verts_default")->deviceBuffer->bufferHandle.
                                                get(), &offset);
                        node->GetCurrCmd().bindIndexBuffer(
                            renderGraph->resourcesManager->GetStagedBuffFromName("cube_index_default")->deviceBuffer->bufferHandle.get(), 0,
                            vk::IndexType::eUint32);
                        node->GetCurrCmd().drawIndexed(36, CFDRenderer->voxelGrid.pcVoxelGrid.GetSize(), 0, 0, 0);
                    }));
            }
        }

        void RecreateSwapChainResources() override
        {
        }

        void SetRenderOperation() override
        {
        }

        void ReloadShaders() override
        {
            auto node = renderGraph->GetNode(mDebuggerPassName);
            if (node)
            {
                node->G_RecreateResources();
            }
        }

        void SetViewCamera()
        {
            currentViewCam = &clusterRenderer->camera;
        }


        void CreateFrustumVertices()
        {
            glm::vec4 nearTopL = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(
                clusterRenderer->camFrustum.points[0], 1.0);
            glm::vec4 nearTopR = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(
                clusterRenderer->camFrustum.points[1], 1.0);
            glm::vec4 nearBottomL = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(
                clusterRenderer->camFrustum.points[2], 1.0);
            glm::vec4 nearBottomR = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(
                clusterRenderer->camFrustum.points[3], 1.0);
            glm::vec4 farTopL = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(
                clusterRenderer->camFrustum.points[4], 1.0);
            glm::vec4 farTopR = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(
                clusterRenderer->camFrustum.points[5], 1.0);
            glm::vec4 farBottomL = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(
                clusterRenderer->camFrustum.points[6], 1.0);
            glm::vec4 farBottomR = glm::inverse(clusterRenderer->currCamera->matrices.view) * glm::vec4(
                clusterRenderer->camFrustum.points[7], 1.0);

            raw3DVertices.clear();

            // Left face: 0,2,6 and 0,6,4
            raw3DVertices.push_back({nearTopL});
            raw3DVertices.push_back({nearBottomL});
            raw3DVertices.push_back({farBottomL});
            raw3DVertices.push_back({nearTopL});
            raw3DVertices.push_back({farBottomL});
            raw3DVertices.push_back({farTopL});

            // Right face: 1,3,7 and 1,7,5
            raw3DVertices.push_back({nearTopR});
            raw3DVertices.push_back({nearBottomR});
            raw3DVertices.push_back({farBottomR});
            raw3DVertices.push_back({nearTopR});
            raw3DVertices.push_back({farBottomR});
            raw3DVertices.push_back({farTopR});

            // Top face: 0,1,5 and 0,5,4
            raw3DVertices.push_back({nearTopL});
            raw3DVertices.push_back({nearTopR});
            raw3DVertices.push_back({farTopR});
            raw3DVertices.push_back({nearTopL});
            raw3DVertices.push_back({farTopR});
            raw3DVertices.push_back({farTopL});

            // Bottom face: 2,3,7 and 2,7,6
            raw3DVertices.push_back({nearBottomL});
            raw3DVertices.push_back({nearBottomR});
            raw3DVertices.push_back({farBottomR});
            raw3DVertices.push_back({nearBottomL});
            raw3DVertices.push_back({farBottomR});
            raw3DVertices.push_back({farBottomL});
        }

        std::vector<D_Vertex3D> raw3DVertices;
        std::vector<Model> models;
        std::unique_ptr<Shader> modelFShader;
        std::unique_ptr<Shader> modelVShader;
        std::string mDebuggerPassName = "ModelDebugger";

        Core* core;
        RenderGraph* renderGraph;
        WindowProvider* windowProvider;

        ImageView* colAttachmentView;
        Buffer* rawVerticesBuff;
        Camera* currentViewCam;

        MvpPc pc{};
        DebugVoxelsMVP debugVoxelsPc{};
        ClusterRenderer* clusterRenderer = nullptr;
        CFDRenderer* CFDRenderer = nullptr;
    };
} // namespace Rendering
#endif        // DEBUGRENDERER_HPP
