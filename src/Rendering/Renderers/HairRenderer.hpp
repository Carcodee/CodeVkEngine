//
// Created by carlo on 2025-03-26.
//


#ifndef HAIRRENDERER_HPP
#define HAIRRENDERER_HPP

namespace Rendering
{
    using namespace ENGINE;
    class HairRenderer : public BaseRenderer
    {
    public:
        ~HairRenderer() override = default;

        HairRenderer(Core* core, WindowProvider* windowProvider)
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
        }

        void CreatePipelines()
        {
            
            AttachmentInfo colInfo = GetColorAttachmentInfo(
                glm::vec4(0.0f), renderGraph->core->swapchainRef->GetFormat());
            Shader* vShader = renderGraph->resourcesManager->CreateDefaultShader("HairVert", ShaderStage::S_VERT, ShaderCompiler::C_GLSL); 
            Shader* fShader = renderGraph->resourcesManager->CreateDefaultShader("HairFrag", ShaderStage::S_FRAG, ShaderCompiler::C_GLSL); 

            // auto imageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1, ENGINE::g_32bFormat,ENGINE::colorImageUsage);
            // ImageView* attachmentOutput = renderGraph->resourcesManager->GetImage("shOutput", imageInfo, 0, 0);
            
            auto renderNode = renderGraph->AddPass(passName);
            renderNode->SetConfigs({true});
            renderNode->SetVertShader(vShader);
            renderNode->SetFragShader(fShader);
            renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
            renderNode->SetVertexInput(Vertex2D::GetVertexInput());
            //change this
            renderNode->SetPushConstantSize(4);
            renderNode->AddColorAttachmentOutput("default_attachment", colInfo, BlendConfigs::B_OPAQUE);
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

#endif //HAIRRENDERER_HPP
