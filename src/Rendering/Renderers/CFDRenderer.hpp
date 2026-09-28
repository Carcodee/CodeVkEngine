//
// Created by carlo on 2025-03-26.
//

#ifndef CFDRENDERER_HPP
#define CFDRENDERER_HPP

namespace Rendering
{
    using namespace ENGINE;

    class CFDRenderer : public BaseRenderer
    {
    public:
        ~CFDRenderer() override = default;

        CFDRenderer(Core* core, WindowProvider* windowProvider)
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
            auto imageInfo = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
                                                 vk::Format::eR32G32B32A32Sfloat,
                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eSampled);
            auto* colAttachmentView = ResourcesManager::GetInstance()->GetImage("colAttachment", imageInfo, 0, 0);
        }

        void CreatePipelines()
        {
            auto& logicalDevice = core->logicalDevice.get();
            std::string shaderPath = SYSTEMS::OS::GetInstance()->GetShadersPath();
            auto voxelizerComp = renderGraph->resourcesManager->GetShader(
                shaderPath + "\\spirvGlsl\\CFD\\voxelizeMesh.comp.spv", S_COMP);
            auto modelToVoxelize = RenderingResManager::GetInstance()->GetModel(
                SYSTEMS::OS::GetInstance()->GetAssetsPath() + "\\Models\\porsche\\scene.gltf");

            auto* voxelizerNode = renderGraph->AddPass("voxelizer");
            voxelizerNode->G_SetCompShader(voxelizerComp);
            voxelizerNode->BuildRenderGraphNode();

            voxelizerNode->G_SetRenderOperation(new std::function<void()>(
                [this]()
                {
                    glm::vec2 mouseInput = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
                    auto& renderNode = renderGraph->renderNodes.at("voxelizer");
                    renderNode->GetCurrCmd().dispatch(64 / 8, 64 / 8, 1);
                }));
        }

        void RecreateSwapChainResources() override
        {
        }

        void SetRenderOperation() override
        {
        }

        void ReloadShaders() override
        {
        }

        std::string passName = "passName";

        Core* core;
        RenderGraph* renderGraph;
        WindowProvider* windowProvider;
    };
} // namespace Rendering

#endif        // CFDRENDERER_HPP
