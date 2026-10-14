//




// Created by carlo on 2024-09-21.
//


double delta_time;
double previous_time;


#include "WindowAPI/WindowInclude.hpp"
#include "Utility/UtilityInclude.hpp"
#include "Systems/SystemsInclude.hpp"
#include "Engine/EngineInclude.hpp"
#include "UI/UI_Include.hpp"


#include "Rendering/RenderingInclude.hpp"

CONST int windows_width = 1024;
CONST int windows_height = 1024;

#define ENGINE_ENABLE_DEBUGGING


void Create_Renderers(engine::Core* core, ::Window_Provider* window_provider, std::map<std::string, std::unique_ptr<rendering::Base_Renderer>>& renderers)
{
    renderers.try_emplace("ClusterRenderer", std::make_unique<rendering::Cluster_Renderer>(
    core, window_provider));
    rendering::Cluster_Renderer* cluster_renderer = dynamic_cast<rendering::Cluster_Renderer*>(renderers.at("ClusterRenderer").get());
    cluster_renderer->Set_Render_Operation();

	renderers.try_emplace("CFDRenderer", std::make_unique<rendering::Cfd_Renderer>(core, window_provider));
	rendering::Cfd_Renderer* cfd_renderer = dynamic_cast<rendering::Cfd_Renderer*>(renderers.at("CFDRenderer").get());
	cfd_renderer->Set_Render_Operation();

    // renderers.try_emplace("HairRenderer", std::make_unique<rendering::Hair_Renderer>(core, window_provider));
    // rendering::Hair_Renderer* hair_renderer = dynamic_cast<rendering::Hair_Renderer*>(renderers.at("HairRenderer").get());
    // hair_renderer->Set_Render_Operation();
    //
    // renderers.try_emplace("GSRenderer", std::make_unique<rendering::Gs_Renderer>(core, window_provider));
    // rendering::Gs_Renderer* gs_renderer = dynamic_cast<rendering::Gs_Renderer*>(renderers.at("GSRenderer").get());
    // gs_renderer->Set_Render_Operation();
    
    // renderers.try_emplace("FlatRenderer", std::make_unique<rendering::Flat_Renderer>(core, window_provider));
    // rendering::Flat_Renderer* flat_renderer = dynamic_cast<rendering::Flat_Renderer*>(renderers.at("FlatRenderer").get());
    // flat_renderer->Set_Render_Operation();


    //
    // renderers.try_emplace("GIRenderer", std::make_unique<rendering::Gi_Renderer>(core, window_provider));
    // rendering::Gi_Renderer* gi_renderer = dynamic_cast<rendering::Gi_Renderer*>(renderers.at("GIRenderer").get());
    // gi_renderer->Set_Render_Operation();
}
void Run(::Window_Provider* window_provider)
{
	CodeCuda::C_InitEngine();
	
    int image_count = 3;

    engine::Window_Desc window_desc = {};
    window_desc.h_instance = GetModuleHandle(NULL);
    window_desc.h_wnd = glfwGetWin32Window(window_provider->window);
	
    bool enable_debugging = false;
#if defined ENGINE_ENABLE_DEBUGGING
    enable_debugging = true;
#endif

    const char* glfw_extensions[] = {"VK_KHR_surface", "VK_KHR_win32_surface"};
    uint32_t glfw_extension_count = sizeof(glfw_extensions) / sizeof(glfw_extensions[0]);

    std::unique_ptr<engine::Core> core = std::make_unique<engine::Core>(
        glfw_extensions, glfw_extension_count, &window_desc, enable_debugging);
	

    std::unique_ptr<engine::Render_Graph> render_graph = core->Create_Render_Graph();

    std::unique_ptr<engine::In_Flight_Queue> in_flight_queue = std::make_unique<engine::In_Flight_Queue>(
        core.get(), render_graph.get(), window_desc, image_count, vk::PresentModeKHR::eMailbox,
        window_provider->Get_Window_Size());

    engine::Resources_Manager* resources_manager = engine::Resources_Manager::Get_Instance(core.get());
    
    render_graph->Create_Res_Manager();
	
	rendering::Rendering_Res_Manager* rendering_res_manager = rendering::Rendering_Res_Manager::Get_Instance();
    // rendering::ModelLoader::Get_Instance(core.get());
    
    std::map<std::string, std::unique_ptr<rendering::Base_Renderer>> renderers;
    Create_Renderers(core.get(), window_provider, renderers);

    std::unique_ptr<rendering::Imgui_Renderer> imgui_renderer =nullptr;
	
	if (render_graph->debug_ui)
	{
		imgui_renderer = std::make_unique<rendering::Imgui_Renderer>(render_graph.get(), window_provider, renderers);
	}
	
    std::unique_ptr<rendering::Debug_Renderer> debug_renderer = std::make_unique<rendering::Debug_Renderer>(
        core.get(), window_provider, renderers);
	   
    debug_renderer->Set_Render_Operation();
	
	render_graph->Create_Utility();

	rendering::Cfd_Renderer* cfd_renderer = dynamic_cast<rendering::Cfd_Renderer*>(renderers.at("CFDRenderer").get());
    //todo: error here
    while (!window_provider->Window_Should_Close())
    {
        //handle time and frames better
        float time = window_provider->Get_Time();
        delta_time = time - previous_time;
        previous_time = time;
        auto profiler = engine::Profiler::Get_Instance();
        profiler->Start_Profiler();
        
        window_provider->Poll_Events();
        {
            glm::uvec2 window_size = window_provider->Get_Window_Size();
            if (core->resize_requested || window_provider->framebuffer_resized)
            {
                std::cout << "recreated swapchain\n";
                core->Wait_Idle();
                in_flight_queue.reset();
                in_flight_queue = std::make_unique<engine::In_Flight_Queue>(
                    core.get(), render_graph.get(), window_desc, image_count, vk::PresentModeKHR::eMailbox,
                    window_size);
                window_provider->framebuffer_resized = false;
                core->resize_requested = false;
                //unused for now
             // render_graph->Update_All_From_Meta_Data();
            	//this is problematic because recreating the image right now modify the pointer location
             render_graph->Recreate_Frame_Resources();
                // render_graph->BuildRenderOperations();
             //    for (auto& renderer : renderers)
             //    {
             //        renderer.second->Set_Render_Operation();
             //    }
             //    debug_renderer->Set_Render_Operation();
            }
            try
            {
                if (ImGui::IsKeyPressed(ImGuiKey_N, false))
                {
                    // render_graph->resources_manager->files_manager->Check_All_Changes();
                    //unused for now
                    // render_graph->Update_All_From_Meta_Data();
                }
                if (ImGui::IsKeyPressed(ImGuiKey_R, false))
                {
                    //each shader is handling his recompilation process.
                    render_graph->Recreate_Node_Pipelines();
                }
                if (ImGui::IsKeyPressed(ImGuiKey_0, false))
                {
                    render_graph->Debug_Shaders_Compilation();
                }
                rendering_res_manager->Update_Resources();
                core->render_graph_ref->Prepare_Rendering();
            	
                // in_flight_queue->Begin_Parallel_Threads();
                in_flight_queue->Begin_Frame();
            	
                core->render_graph_ref->Execute_Rendering();
                if (imgui_renderer != nullptr)
            	{
					profiler->Add_Profiler_Cpu_Spot(legit::Colors::alizarin, "Imgui");
                    imgui_renderer->Render_Frame(core->queue_worker_manager->Get_Worker_Queue("UI")->Get_Current_Cmd(0),
											   in_flight_queue->current_swapchain_image_view->image_view_handle.get());
					profiler->End_Profiler_Cpu_Spot("Imgui");
            	}
                resources_manager->End_Frame_Dynamic_Updates(render_graph->sorted_queue_batches.back().command_buffer);
                // in_flight_queue->End_Parallel_Threads();
                in_flight_queue->End_Frame();
                render_graph->End_Rendering();
            }
            catch (vk::OutOfDateKHRError err)
            {
                core->resize_requested = true;
            }
        }

        profiler->Add_Profiler_Gpu_Spot(legit::Colors::amethyst, "WaitIdle");
        core->Wait_Idle();
        profiler->End_Profiler_Gpu_Spot("WaitIdle");
        profiler->Update_Profiler();
    }
	if (imgui_renderer != nullptr)
	{
		imgui_renderer->Destroy();
	}
    render_graph->Serialize_All();
    engine::Resources_Manager::Get_Instance()->Destroy_Resources();
    window_provider->Destroy_Window();
	
	CodeCuda::C_ShutDownEngine();
}

int main()
{
    std::unique_ptr<::Window_Provider> window_provider = std::make_unique<::Window_Provider>(
        windows_width, windows_height, "Vulkan Engine Template");
    window_provider->Init_Glfw();

    Run(window_provider.get());

    window_provider->Terminate();

    return 0;
}
