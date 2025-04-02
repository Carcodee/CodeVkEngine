//
// Created by carlo on 2025-03-26.
//


#ifndef GSRENDERER_HPP
#define GSRENDERER_HPP

namespace Rendering
{
    using namespace ENGINE;
    class GSRenderer : BaseRenderer
    {
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
            ENGINE::Buffer* gsPointsVertexBuffer = this->renderGraph->resourcesManager->GetStageBuffer(
                "gsPointsVertexBuffer", vk::BufferUsageFlagBits::eVertexBuffer,
                sizeof(Vertex2D) * Vertex2D::GetQuadVertices().size(),
                Vertex2D::GetQuadVertices().data())->deviceBuffer.get();

            ENGINE::Buffer* gsPointsIndexBuffer = this->renderGraph->resourcesManager->GetStageBuffer(
                "gsPointsIndexBuffer", vk::BufferUsageFlagBits::eIndexBuffer,
                sizeof(uint32_t) * Vertex2D::GetQuadIndices().size(),
                Vertex2D::GetQuadIndices().data())->deviceBuffer.get();
        }

        void CreateBuffers()
        {
        }

        void CreatePipelines()
        {
            
            AttachmentInfo colInfo = GetColorAttachmentInfo(
                glm::vec4(0.0f), g_32bFormat);
            auto imageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1, ENGINE::g_32bFormat,ENGINE::colorImageUsage);
            ImageView* attachmentOutput = renderGraph->resourcesManager->GetImage("shOutput", imageInfo, 1, 1);
            
            auto renderNode = RenderingResManager::GetInstance()->GetTemplateNode_DF("GSLoader", "GSLoad");
            renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
            renderNode->SetVertexInput(D_Vertex3D::GetVertexInput());
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
                    auto renderNode = renderGraph->GetNode(passName);
                    renderNode->AddColorImageResource("default_attachment", renderGraph->currentBackBuffer);
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
                    renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(
                        0, 1,
                        &renderGraph->resourcesManager->GetStagedBuffFromName("quad_default")->deviceBuffer->bufferHandle.get(),
                        &offset);
                    renderGraph->currentFrameResources->commandBuffer->bindIndexBuffer(
                        renderGraph->resourcesManager->GetStagedBuffFromName("quad_index_default")->deviceBuffer->
                                     bufferHandle.get(), 0, vk::IndexType::eUint32);
                    renderGraph->currentFrameResources->commandBuffer->drawIndexed(
                        Vertex2D::GetQuadIndices().size(), 1, 0, 0, 0);
                });
            renderGraph->GetNode(passName)->SetRenderOperation(renderOp);
            renderGraph->GetNode(passName)->AddTask(taskOp);
        }
        

        void ReloadShaders() override
        {
        }


        std::string passName = "passName";

        Core* core;
        RenderGraph* renderGraph;
        WindowProvider* windowProvider;

        ImageView* colAttachmentView;
    };
}

#endif //GSRENDERER_HPP
