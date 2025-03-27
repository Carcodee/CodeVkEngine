//

// Created by carlo on 2025-03-26.
//


#ifndef GIRENDERER_HPP
#define GIRENDERER_HPP

namespace Rendering
{
    class GIRenderer :public BaseRenderer
    {
    public:

        GIRenderer(Core* core, WindowProvider* windowProvider)
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
        }

        void CreateBuffers()
        {
            quadVertBuffer = ResourcesManager::GetInstance()->GetStagedBuffFromName("quad_default")->deviceBuffer.get();
            quadIndexBuffer = ResourcesManager::GetInstance()->GetStagedBuffFromName("quad_index_default")->deviceBuffer.get();
        }

        void CreatePipelines()
        {
            
            AttachmentInfo colInfo = GetColorAttachmentInfo(
                glm::vec4(0.0f), renderGraph->core->swapchainRef->GetFormat());
            std::string shaderPath = SYSTEMS::OS::GetInstance()->GetShadersPath();
            
            // auto imageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1, ENGINE::g_32bFormat,ENGINE::colorImageUsage);
            // ImageView* attachmentOutput = renderGraph->resourcesManager->GetImage("shOutput", imageInfo, 1, 1);

            auto renderNode = RenderingResManager::GetInstance()->GetTemplateNode(
                shPassName, shaderPath + "\\slang\\test\\shView.slang", shaderPath + "\\slang\\test\\shView.slang");
            renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
            renderNode->AddColorAttachmentOutput("shAttachment", colInfo, BlendConfigs::B_OPAQUE);
            
            renderNode->BuildRenderGraphNode();
        }

        void RecreateSwapChainResources() override
        {
            
        }

        void SetRenderOperation() override
        {
            auto taskOp = new std::function<void()>([this]{
                auto renderNode = renderGraph->GetNode(shPassName);
                auto currBackBuffer = renderGraph->currentBackBuffer;
                renderNode->AddColorImageResource("shAttachment", currBackBuffer);
            });
            
            auto shRenderOp = new std::function<void()>(
                [this]()
                {
                    auto& renderNode = renderGraph->renderNodes.at(shPassName);
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
                        renderGraph->resourcesManager->GetStagedBuffFromName("quad_index_default")->deviceBuffer->bufferHandle.get(), 0, vk::IndexType::eUint32);
                    renderGraph->currentFrameResources->commandBuffer->drawIndexed(
                        Vertex2D::GetQuadIndices().size(), 1, 0, 0, 0);
                });
            renderGraph->GetNode(shPassName)->AddTask(taskOp);
            renderGraph->GetNode(shPassName)->SetRenderOperation(shRenderOp);
            
        }
        

        void ReloadShaders() override
        {
        }


        std::string shPassName = "SHPass";

        Core* core;
        RenderGraph* renderGraph;
        WindowProvider* windowProvider;

        ImageView* colAttachmentView;
        Buffer* quadVertBuffer;
        Buffer* quadIndexBuffer;
    };
}

#endif //GIRENDERER_HPP
