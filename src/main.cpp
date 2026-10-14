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


void create_renderers(engine::core* core, ::window_provider* window_provider, std::map<std::string, std::unique_ptr<rendering::base_renderer>>& renderers)
{
    renderers.try_emplace("ClusterRenderer", std::make_unique<rendering::cluster_renderer>(
    core, window_provider));
    rendering::cluster_renderer* cluster_renderer = dynamic_cast<rendering::cluster_renderer*>(renderers.at("ClusterRenderer").get());
    cluster_renderer->set_render_operation();

	renderers.try_emplace("CFDRenderer", std::make_unique<rendering::cfd_renderer>(core, window_provider));
	rendering::cfd_renderer* cfd_renderer = dynamic_cast<rendering::cfd_renderer*>(renderers.at("CFDRenderer").get());
	cfd_renderer->set_render_operation();

    // renderers.try_emplace("HairRenderer", std::make_unique<rendering::hair_renderer>(core, window_provider));
    // rendering::hair_renderer* hair_renderer = dynamic_cast<rendering::hair_renderer*>(renderers.at("HairRenderer").get());
    // hair_renderer->set_render_operation();
    //
    // renderers.try_emplace("GSRenderer", std::make_unique<rendering::gs_renderer>(core, window_provider));
    // rendering::gs_renderer* gs_renderer = dynamic_cast<rendering::gs_renderer*>(renderers.at("GSRenderer").get());
    // gs_renderer->set_render_operation();
    
    // renderers.try_emplace("FlatRenderer", std::make_unique<rendering::flat_renderer>(core, window_provider));
    // rendering::flat_renderer* flat_renderer = dynamic_cast<rendering::flat_renderer*>(renderers.at("FlatRenderer").get());
    // flat_renderer->set_render_operation();


    //
    // renderers.try_emplace("GIRenderer", std::make_unique<rendering::gi_renderer>(core, window_provider));
    // rendering::gi_renderer* gi_renderer = dynamic_cast<rendering::gi_renderer*>(renderers.at("GIRenderer").get());
    // gi_renderer->set_render_operation();
}
void run(::window_provider* window_provider)
{
	CodeCuda::C_InitEngine();
	
    int image_count = 3;

    engine::window_desc window_desc = {};
    window_desc.h_instance = GetModuleHandle(NULL);
    window_desc.h_wnd = glfwGetWin32Window(window_provider->window);
	
    bool enable_debugging = false;
#if defined ENGINE_ENABLE_DEBUGGING
    enable_debugging = true;
#endif

    const char* glfw_extensions[] = {"VK_KHR_surface", "VK_KHR_win32_surface"};
    uint32_t glfw_extension_count = sizeof(glfw_extensions) / sizeof(glfw_extensions[0]);

    std::unique_ptr<engine::core> core = std::make_unique<engine::core>(
        glfw_extensions, glfw_extension_count, &window_desc, enable_debugging);
	

    std::unique_ptr<engine::render_graph> render_graph = core->create_render_graph();

    std::unique_ptr<engine::in_flight_queue> in_flight_queue = std::make_unique<engine::in_flight_queue>(
        core.get(), render_graph.get(), window_desc, image_count, vk::PresentModeKHR::eMailbox,
        window_provider->get_window_size());

    engine::resources_manager* resources_manager = engine::resources_manager::get_instance(core.get());
    
    render_graph->create_res_manager();
	
	rendering::rendering_res_manager* rendering_res_manager = rendering::rendering_res_manager::get_instance();
    // rendering::ModelLoader::get_instance(core.get());
    
    std::map<std::string, std::unique_ptr<rendering::base_renderer>> renderers;
    create_renderers(core.get(), window_provider, renderers);

    std::unique_ptr<rendering::imgui_renderer> imgui_renderer =nullptr;
	
	if (render_graph->debug_ui)
	{
		imgui_renderer = std::make_unique<rendering::imgui_renderer>(render_graph.get(), window_provider, renderers);
	}
	
    std::unique_ptr<rendering::debug_renderer> debug_renderer = std::make_unique<rendering::debug_renderer>(
        core.get(), window_provider, renderers);
	   
    debug_renderer->set_render_operation();
	
	render_graph->create_utility();

    //todo: error here
    while (!window_provider->window_should_close())
    {
        //handle time and frames better
        float time = window_provider->get_time();
        delta_time = time - previous_time;
        previous_time = time;
        auto profiler = engine::profiler::get_instance();
        profiler->start_profiler();
        
        window_provider->poll_events();
        {
            glm::uvec2 window_size = window_provider->get_window_size();
            if (core->resize_requested || window_provider->framebuffer_resized)
            {
                std::cout << "recreated swapchain\n";
                core->wait_idle();
                in_flight_queue.reset();
                in_flight_queue = std::make_unique<engine::in_flight_queue>(
                    core.get(), render_graph.get(), window_desc, image_count, vk::PresentModeKHR::eMailbox,
                    window_size);
                window_provider->framebuffer_resized = false;
                core->resize_requested = false;
                //unused for now
             // render_graph->update_all_from_meta_data();
            	//this is problematic because recreating the image right now modify the pointer location
             render_graph->recreate_frame_resources();
                // render_graph->BuildRenderOperations();
             //    for (auto& renderer : renderers)
             //    {
             //        renderer.second->set_render_operation();
             //    }
             //    debug_renderer->set_render_operation();
            }
            try
            {
                if (ImGui::IsKeyPressed(ImGuiKey_N, false))
                {
                    // render_graph->resources_manager->files_manager->check_all_changes();
                    //unused for now
                    // render_graph->update_all_from_meta_data();
                }
                if (ImGui::IsKeyPressed(ImGuiKey_R, false))
                {
                    //each shader is handling his recompilation process.
                    render_graph->recreate_node_pipelines();
                }
                if (ImGui::IsKeyPressed(ImGuiKey_0, false))
                {
                    render_graph->debug_shaders_compilation();
                }

                rendering_res_manager->update_resources();
                core->render_graph_ref->prepare_rendering();
            	
                // in_flight_queue->begin_parallel_threads();
                in_flight_queue->begin_frame();
            	
                core->render_graph_ref->execute_rendering();
                if (imgui_renderer != nullptr)
            	{
					profiler->add_profiler_cpu_spot(legit::Colors::alizarin, "Imgui");
                    imgui_renderer->render_frame(core->queue_worker_manager->get_worker_queue("UI")->get_current_cmd(0),
											   in_flight_queue->current_swapchain_image_view->image_view_handle.get());
					profiler->end_profiler_cpu_spot("Imgui");
            	}
                resources_manager->end_frame_dynamic_updates(render_graph->sorted_queue_batches.back().command_buffer);
                // in_flight_queue->end_parallel_threads();
                in_flight_queue->end_frame();
                render_graph->end_rendering();
            }
            catch (vk::OutOfDateKHRError err)
            {
                core->resize_requested = true;
            }
        }

        profiler->add_profiler_gpu_spot(legit::Colors::amethyst, "WaitIdle");
        core->wait_idle();
        profiler->end_profiler_gpu_spot("WaitIdle");
        profiler->update_profiler();
    }
	if (imgui_renderer != nullptr)
	{
		imgui_renderer->destroy();
	}
    render_graph->serialize_all();
    engine::resources_manager::get_instance()->destroy_resources();
    window_provider->destroy_window();
	
	CodeCuda::C_ShutDownEngine();
}

int main()
{
    std::unique_ptr<::window_provider> window_provider = std::make_unique<::window_provider>(
        windows_width, windows_height, "Vulkan Engine Template");
    window_provider->init_glfw();

    run(window_provider.get());

    window_provider->terminate();

    return 0;
}
