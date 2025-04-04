//



// Created by carlo on 2024-09-21.
//




double deltaTime;
double previousTime;


#include "WindowAPI/WindowInclude.hpp"
#include "Systems/SystemsInclude.hpp"
#include "Engine/EngineInclude.hpp"
#include "UI/UI_Include.hpp"


#include "Rendering/RenderingInclude.hpp"

CONST int WINDOWS_WIDTH = 1024;
CONST int WINDOWS_HEIGHT = 1024;

#define ENGINE_ENABLE_DEBUGGING


void CreateRenderers(ENGINE::Core* core, WindowProvider* windowProvider, std::map<std::string, std::unique_ptr<Rendering::BaseRenderer>>& renderers)
{
    // renderers.try_emplace("ClusterRenderer", std::make_unique<Rendering::ClusterRenderer>(
    // core, windowProvider));
    // Rendering::ClusterRenderer* clusterRenderer = dynamic_cast<Rendering::ClusterRenderer*>(renderers.at("ClusterRenderer").get());
    // clusterRenderer->SetRenderOperation();
    renderers.try_emplace("GSRenderer", std::make_unique<Rendering::GSRenderer>(core, windowProvider));
    Rendering::GSRenderer* gsRenderer = dynamic_cast<Rendering::GSRenderer*>(renderers.at("GSRenderer").get());
    gsRenderer->SetRenderOperation();
    //
    // renderers.try_emplace("FlatRenderer", std::make_unique<Rendering::FlatRenderer>(core, windowProvider));
    // Rendering::FlatRenderer* flatRenderer = dynamic_cast<Rendering::FlatRenderer*>(renderers.at("FlatRenderer").get());
    // flatRenderer->SetRenderOperation();
    //
    //
    // renderers.try_emplace("GIRenderer", std::make_unique<Rendering::GIRenderer>(core, windowProvider));
    // Rendering::GIRenderer* giRenderer = dynamic_cast<Rendering::GIRenderer*>(renderers.at("GIRenderer").get());
    // giRenderer->SetRenderOperation();
}
void run(WindowProvider* windowProvider)
{
    int imageCount = 3;

    ENGINE::WindowDesc windowDesc = {};
    windowDesc.hInstance = GetModuleHandle(NULL);
    windowDesc.hWnd = glfwGetWin32Window(windowProvider->window);

    bool enableDebugging = false;
#if defined ENGINE_ENABLE_DEBUGGING
    enableDebugging = true;
#endif

    const char* glfwExtensions[] = {"VK_KHR_surface", "VK_KHR_win32_surface"};
    uint32_t glfwExtensionCount = sizeof(glfwExtensions) / sizeof(glfwExtensions[0]);

    std::unique_ptr<ENGINE::Core> core = std::make_unique<ENGINE::Core>(
        glfwExtensions, glfwExtensionCount, &windowDesc, enableDebugging);

    std::unique_ptr<ENGINE::RenderGraph> renderGraph = core->CreateRenderGraph();

    std::unique_ptr<ENGINE::InFlightQueue> inFlightQueue = std::make_unique<ENGINE::InFlightQueue>(
        core.get(), renderGraph.get(), windowDesc, imageCount, vk::PresentModeKHR::eMailbox,
        windowProvider->GetWindowSize());

    ENGINE::ResourcesManager* resourcesManager = ENGINE::ResourcesManager::GetInstance(core.get());
    
    renderGraph->CreateResManager();
    
    Rendering::RenderingResManager* renderingResManager = Rendering::RenderingResManager::GetInstance(renderGraph.get());
    // Rendering::ModelLoader::GetInstance(core.get());

    
    std::map<std::string, std::unique_ptr<Rendering::BaseRenderer>> renderers;
    CreateRenderers(core.get(), windowProvider, renderers);

    std::unique_ptr<Rendering::ImguiRenderer> imguiRenderer = std::make_unique<Rendering::ImguiRenderer>(
        renderGraph.get(), windowProvider, renderers);

    std::unique_ptr<Rendering::DebugRenderer> debugRenderer = std::make_unique<Rendering::DebugRenderer>(
        core.get(), windowProvider, renderers);
    debugRenderer->SetRenderOperation();

    //todo: error here
    while (!windowProvider->WindowShouldClose())
    {
        //handle time and frames better
        float time = windowProvider->GetTime();
        deltaTime = time - previousTime;
        previousTime = time;
        auto profiler = ENGINE::Profiler::GetInstance();
        profiler->StartProfiler();
        
        windowProvider->PollEvents();
        {
            glm::uvec2 windowSize = windowProvider->GetWindowSize();
            if (core->resizeRequested || windowProvider->framebufferResized)
            {
                std::cout << "recreated swapchain\n";
                core->WaitIdle();
                inFlightQueue.reset();
                inFlightQueue = std::make_unique<ENGINE::InFlightQueue>(
                    core.get(), renderGraph.get(), windowDesc, imageCount, vk::PresentModeKHR::eMailbox,
                    windowSize);
                windowProvider->framebufferResized = false;
                core->resizeRequested = false;
                //unused for now
                // renderGraph->UpdateAllFromMetaData();
                renderGraph->RecreateFrameResources();
                for (auto& renderer : renderers)
                {
                    renderer.second->SetRenderOperation();
                }
                debugRenderer->SetRenderOperation();
            }
            try
            {
                profiler->AddProfilerCpuSpot(legit::Colors::sunFlower, "Cpu");
                if (ImGui::IsKeyPressed(ImGuiKey_N, false))
                {
                    // renderGraph->resourcesManager->filesManager->CheckAllChanges();
                    //unused for now
                    // renderGraph->UpdateAllFromMetaData();
                }
                if (ImGui::IsKeyPressed(ImGuiKey_R, false))
                {
                    //each shader is handling his recompilation process.
                    renderGraph->RecreateNodePipelines();
                }
                if (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl, false) && ImGui::IsKeyPressed(ImGuiKey_R, false))
                {
                    renderGraph->DebugShadersCompilation();
                }

                renderingResManager->UpdateResources();
                resourcesManager->UpdateBuffers();
                resourcesManager->UpdateImages();

                inFlightQueue->BeginFrame();

                auto& currFrame = inFlightQueue->frameResources[inFlightQueue->frameIndex];

                profiler->EndProfilerCpuSpot("Cpu");

                core->renderGraphRef->ExecuteAll();


                profiler->AddProfilerCpuSpot(legit::Colors::alizarin, "Imgui");
                imguiRenderer->RenderFrame(currFrame.commandBuffer.get(),
                                           inFlightQueue->currentSwapchainImageView->imageView.get());

                profiler->EndProfilerCpuSpot("Imgui");
                profiler->AddProfilerGpuSpot(legit::Colors::carrot, "Gpu");

                resourcesManager->EndFrameDynamicUpdates(currFrame.commandBuffer.get());
                inFlightQueue->EndFrame();
                profiler->EndProfilerGpuSpot("Gpu");
            }
            catch (vk::OutOfDateKHRError err)
            {
                core->resizeRequested = true;
            }
        }

        profiler->AddProfilerGpuSpot(legit::Colors::amethyst, "WaitIdle");
        core->WaitIdle();
        profiler->EndProfilerGpuSpot("WaitIdle");
        profiler->UpdateProfiler();
    }
    imguiRenderer->Destroy();
    renderGraph->SerializeAll();
    ENGINE::ResourcesManager::GetInstance()->DestroyResources();
    windowProvider->DestroyWindow();
}

int main()
{
    std::unique_ptr<WindowProvider> windowProvider = std::make_unique<WindowProvider>(
        WINDOWS_WIDTH, WINDOWS_HEIGHT, "Vulkan Engine Template");
    windowProvider->InitGlfw();

    run(windowProvider.get());

    windowProvider->Terminate();

    return 0;
}
