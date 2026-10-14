//

// Created by carlo on 2024-12-02.
//

#ifndef FLAT_RENDERER_HPP
#define FLAT_RENDERER_HPP
#include "CodeCuda.cuh"
namespace rendering
{
using namespace engine;

class Flat_Renderer : public Base_Renderer
{
  public:
	Flat_Renderer(engine::Core *core, ::Window_Provider *window_provider)
	{
		this->core           = core;
		this->render_graph    = core->render_graph_ref;
		this->window_provider = window_provider;
		Create_Resources();
		Create_Buffers();
		Create_Pipelines();
	}

	void Create_Resources()
	{
		cascades_info.cascade_count       = 5;
		cascades_info.probe_size_px        = 2;
		cascades_info.interval_count      = 2;
		cascades_info.base_interval_length = 1;
		auto image_info                  = engine::Image::Create_Info2d(glm::uvec2(rc_resolution_w, rc_resolution_h), 1, 1,
		                                                      engine::g_32b_format,
		                                                      vk::ImageUsageFlagBits::eColorAttachment |
		                                                          vk::ImageUsageFlagBits::eSampled);
		cascades_attachments_images_views.reserve(cascades_info.cascade_count);
		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			std::string name      = "CascadeAttachment_" + std::to_string(i);
			engine::Image_View  *image_view_handle = Resources_Manager::Get_Instance()->Get_Image(name, image_info, 0, 0);
			cascades_attachments_images_views.emplace_back(image_view_handle);
		}

		probes_gen_pc.cascade_index = 0;
		probes_gen_pc.interval_size = 2;
		probes_gen_pc.probe_size_px  = cascades_info.probe_size_px;

		rc_pc.probe_size_px        = cascades_info.probe_size_px;
		rc_pc.interval_count      = cascades_info.interval_count;
		rc_pc.base_interval_length = cascades_info.base_interval_length;

		painting_pc.radius = 20;

		auto       storage_image_info = engine::Image::Create_Info2d(glm::uvec2(rc_resolution_w, rc_resolution_h), 1, 1,
		                                                          engine::g_32b_format,
		                                                          vk::ImageUsageFlagBits::eStorage |
		                                                              vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
		engine::Image_View *light_layer       = Resources_Manager::Get_Instance()->Get_Image("PaintingLayer", storage_image_info, 0, 0);
		engine::Image_View *occluder_layer    = Resources_Manager::Get_Instance()->Get_Image(
            "OccluderLayer", storage_image_info, 0, 0);
		engine::Image_View *debug_layer        = Resources_Manager::Get_Instance()->Get_Image("DebugRaysLayer", storage_image_info, 0, 0);
		engine::Image_View *fluid_sim_layer     = Resources_Manager::Get_Instance()->Get_Image("FluidSimLayer", storage_image_info, 0, 0);
		engine::Image_View *fluid_sim_layer_info = Resources_Manager::Get_Instance()->Get_Image("FluidSimInfoLayer", storage_image_info, 0, 0);
		painting_layers.push_back(light_layer);
		painting_layers.push_back(occluder_layer);
		painting_layers.push_back(debug_layer);
		painting_layers.push_back(fluid_sim_layer);
		painting_layers.push_back(fluid_sim_layer_info);

		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			std::string name      = "radianceStorage_" + std::to_string(i);
			engine::Image_View  *image_view_handle = Resources_Manager::Get_Instance()->Get_Image(name, storage_image_info, 0, 0);
			radiances_images.emplace_back(image_view_handle);
		}

		std::string resources_path = systems::Os::Get_Instance()->Get_Engine_Resources_Path();
		std::string asset_path     = systems::Os::Get_Instance()->Get_Assets_Path();

		test_image = Resources_Manager::Get_Instance()->Get_Shipper("test_image",
		                                                        resources_path + "\\Images\\VulkanLogo.png", 1, 1,
		                                                        engine::g_shipper_format,
		                                                        Layout_Patterns::Graphics_Read);

		std::filesystem::path dir_entry(asset_path + "\\Textures\\RCascadesTextures");

		int i = 0;
		for (auto &path_view : std::filesystem::directory_iterator(dir_entry))
		{
			rendering::Material *background_material = Rendering_Res_Manager::Get_Instance()->Push_Material(
			    "RCascadesMat_" + std::to_string(i++));
			std::string path   = path_view.path().string();
			engine::Image_View  *col_ref = Resources_Manager::Get_Instance()->Get_Shipper(
                                                                   path + "\\Albedo.png", path + "\\Albedo.png", 1, 1, engine::g_shipper_format,
                                                                   Layout_Patterns::Graphics_Read)
			                        ->image_view_handle.get();
			background_material->Set_Texture(Texture_Type::Albedo, col_ref);

			engine::Image_View *norm_ref = Resources_Manager::Get_Instance()->Get_Shipper(
			                                                        path + "\\Normal.png", path + "\\Normal.png", 1, 1,
			                                                        engine::g_shipper_format, Layout_Patterns::Graphics_Read)
			                         ->image_view_handle.get();
			background_material->Set_Texture(Texture_Type::Normal, norm_ref);

			engine::Image_View *roughness_ref = Resources_Manager::Get_Instance()->Get_Shipper(
			                                                             path + "\\Roughness.png", path + "\\Roughness.png", 1, 1,
			                                                             engine::g_shipper_format, Layout_Patterns::Graphics_Read)
			                              ->image_view_handle.get();
			background_material->Set_Texture(Texture_Type::Roughness, roughness_ref);

			engine::Image_View *ao_ref = Resources_Manager::Get_Instance()->Get_Shipper(
			                                                      path + "\\Ao.png", path + "\\Ao.png", 1, 1,
			                                                      engine::g_shipper_format, Layout_Patterns::Graphics_Read)
			                       ->image_view_handle.get();
			background_material->Set_Texture(Texture_Type::Ao, ao_ref);

			engine::Image_View *height_ref = Resources_Manager::Get_Instance()->Get_Shipper(
			                                                          path + "\\Height.png", path + "\\Height.png", 1, 1,
			                                                          engine::g_shipper_format, Layout_Patterns::Graphics_Read)
			                           ->image_view_handle.get();
			background_material->Set_Texture(Texture_Type::Height, height_ref);
			background_materials.emplace_back(background_material);
		}
		material_index_selected = 0;

		// testSpriteAnim.LoadAtlas(
		// assetPath + "\\Animations\\SmokeFreePack_v2\\Compressed\\512\\Smoke_4_512-sheet.png",
		// glm::uvec2(512, 512), 7, 7, 10);

		rendering::Animator_Info animation_info = {glm::uvec2(512, 512), 7, 7, -1, -1, -1, true};
		test_sprite_anim            = Rendering_Res_Manager::Get_Instance()->Get_Animator("TestAnim",
		                                                                            asset_path +
		                                                                                "\\Animations\\SmokeFreePack_v2\\Compressed\\512\\Smoke_4_512-sheet.png",
		                                                                            30, animation_info);
		if (test_sprite_anim->images_frames.empty())
		{
			// The optional smoke atlas is not distributed with the engine.
			// ImageShipper owns and frees this pixel after uploading it.
			auto *transparent_pixel = std::calloc(4, 1);
			auto *fallback = Resources_Manager::Get_Instance()->Get_Shipper(
			    "EmptySmokeAtlas", transparent_pixel, 1, 1, 4, 1, 1,
			    engine::g_shipper_format, Layout_Patterns::Graphics_Read);
			test_sprite_anim->images_frames.push_back(fallback->image_view_handle.get());
			test_sprite_anim->animation_info = {glm::uvec2(1), 1, 1, 0, 1, 0.0f, true};
			test_sprite_anim->stop = true;
		}
	}

	void Create_Buffers()
	{
		quad_vert_buffer_ref = Resources_Manager::Get_Instance()->Get_Stage_Buffer(
		                                                       "QuadRcVertices", vk::BufferUsageFlagBits::eVertexBuffer,
		                                                       sizeof(Vertex_2d) * Vertex_2d::Get_Quad_Vertices().size(), sizeof(Vertex_2d),
		                                                       Vertex_2d::Get_Quad_Vertices().data())
		                        ->device_buffer.get();
		quad_index_buffer_ref = Resources_Manager::Get_Instance()->Get_Stage_Buffer(
		                                                        "QuadRcIndices", vk::BufferUsageFlagBits::eIndexBuffer,
		                                                        sizeof(uint32_t) * Vertex_2d::Get_Quad_Indices().size(), sizeof(uint32_t),
		                                                        Vertex_2d::Get_Quad_Indices().data())
		                         ->device_buffer.get();
	}

	void Create_Pipelines()
	{
		auto        logical_device = core->logical_device.get();
		std::string shader_path    = systems::Os::Get_Instance()->Get_Shaders_Path();

		auto cuda_buffer = render_graph->resources_manager->Get_Buffer(engine::Resources_Manager::Buffer_Params{
		    "CudaBuffer", vk::BufferUsageFlagBits::eStorageBuffer, {}, sizeof(float) * 4 * rc_resolution_w * rc_resolution_h, sizeof(float) * 4, nullptr, engine::Resources_Manager::Buffer_Type::External});

		auto cuda_buffer_vel = render_graph->resources_manager->Get_Buffer(engine::Resources_Manager::Buffer_Params{
		    "CudaBufferVel", vk::BufferUsageFlagBits::eStorageBuffer, {}, sizeof(float) * rc_resolution_w * rc_resolution_h, sizeof(float), nullptr, engine::Resources_Manager::Buffer_Type::External});

		auto cuda_pi = render_graph->Add_Cuda_Pipeline("CudaCopySmoke");
		cuda_pi
		    ->C_Export_Buffer(cuda_buffer)
		    ->C_Set_Kernel_Function(*CodeCuda::FluidSimulation::C_GetKernelLauncherMappers(0).GetKernelFunct())
		    ->C_Build_Cuda_Pipeline();
		auto cuda_node = render_graph->Add_Cuda_Pass(cuda_pi, "CudaNode");

		auto cuda_pi_vel = render_graph->Add_Cuda_Pipeline("CudaCopyVel");
		cuda_pi_vel
		    ->C_Export_Buffer(cuda_buffer_vel)
		    ->C_Set_Kernel_Function(*CodeCuda::FluidSimulation::C_GetKernelLauncherMappers(1).GetKernelFunct())
		    ->C_Build_Cuda_Pipeline();

		auto cuda_vel = render_graph->Add_Cuda_Pass(cuda_pi_vel, "CudaVelNode");

		//

		paint_comp_shader = render_graph->resources_manager->Get_Shader(
		    shader_path + "\\slang\\test\\paintingGen.slang", S_Comp);
		auto *painting_gpu_pipeline = render_graph->Add_Gpu_Pipeline("PaintingCompute");
		painting_gpu_pipeline->G_Set_Comp_Shader(paint_comp_shader);
		painting_gpu_pipeline->G_Set_Push_Constant_Size(sizeof(painting_pc));
		painting_gpu_pipeline->G_Build_Gpu_Pipeline();

		auto *painting_node = render_graph->Add_Pass(painting_gpu_pipeline, painting_pass_name, "Graphics");
		painting_node->G_Add_Storage_Resource(painting_layers[0]);
		painting_node->G_Add_Storage_Resource(painting_layers[1]);
		painting_node->G_Add_Storage_Resource(painting_layers[2]);

		auto cuda_buffer_imports = render_graph->resources_manager->Get_Shader(
		    shader_path + "\\slang\\test\\cudaBufferToImage.slang", S_Comp);

		auto *import_buffer_node = render_graph->Add_Pass("CudaBufferImporterNode");
		import_buffer_node->G_Set_Comp_Shader(cuda_buffer_imports);
		import_buffer_node->G_Add_Storage_Resource(painting_layers[3]);
		import_buffer_node->G_Add_Storage_Resource(painting_layers[4]);
		import_buffer_node->Depends_On(cuda_node);
		import_buffer_node->Depends_On(cuda_vel);
		import_buffer_node->Build_Render_Graph_Node();

		engine::Vertex_Input    vertex_layout = Vertex_2d::Get_Vertex_Input();
		engine::Attachment_Info col_info     = Get_Color_Attachment_Info(Blend_Configs::B_Opaque,
            glm::vec4(0.0f), vk::Format::eR32G32B32A32Sfloat);

		probes_vert_shader = render_graph->resources_manager->Get_Shader(
		    shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", S_Vert);
		probes_frag_shader = render_graph->resources_manager->Get_Shader(
		    shader_path + "\\spirvGlsl\\FlatRendering\\cascadeGen.frag.spv",
		    S_Frag);

		auto probes_gen_gpu_pipeline = render_graph->Add_Gpu_Pipeline("ProbesGenShader");
		probes_gen_gpu_pipeline->G_Set_Configs({true});
		probes_gen_gpu_pipeline->G_Set_Push_Constant_Size(sizeof(painting_pc));
		probes_gen_gpu_pipeline->G_Set_Vert_Shader(probes_vert_shader);
		probes_gen_gpu_pipeline->G_Set_Frag_Shader(probes_frag_shader);
		probes_gen_gpu_pipeline->G_Set_Vertex_Input(vertex_layout);
		probes_gen_gpu_pipeline->G_Add_Color_Attachment_Output(0, col_info);
		probes_gen_gpu_pipeline->G_Build_Gpu_Pipeline();

		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			std::string name = "ProbesGen_" + std::to_string(i);
			probes_gen_pass_names.push_back(name);
			auto render_node = render_graph->Add_Pass(probes_gen_gpu_pipeline, name, "Graphics_Test");
			render_node->G_Set_Framebuffer_Size(glm::uvec2(rc_resolution_w, rc_resolution_h));
			render_node->G_Set_Color_Image_Attachment_Binding(0, cascades_attachments_images_views[i]);
			render_node->Depends_On(painting_pass_name);
		}

		vert_shader = render_graph->resources_manager->Get_Shader(
		    shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", S_Vert);
		frag_shader = render_graph->resources_manager->Get_Shader(
		    shader_path + "\\spirvGlsl\\FlatRendering\\rCascadesOutput.frag.spv",
		    S_Frag);

		// This pass writes storage images only; disable fragment color writes.
		engine::Attachment_Info output_col_info = Get_Color_Attachment_Info(Blend_Configs::B_None,
		    glm::vec4(0.0f), core->swapchain_ref->Get_Format());

		auto cascades_gpu_pipeline = render_graph->Add_Gpu_Pipeline("CascadesGPUPipeline");
		cascades_gpu_pipeline->G_Set_Vert_Shader(vert_shader);
		cascades_gpu_pipeline->G_Set_Frag_Shader(frag_shader);
		cascades_gpu_pipeline->G_Set_Push_Constant_Size(sizeof(rc_pc));
		cascades_gpu_pipeline->G_Set_Vertex_Input(vertex_layout);
		cascades_gpu_pipeline->G_Add_Color_Attachment_Output(0, output_col_info);
		cascades_gpu_pipeline->G_Build_Gpu_Pipeline();

		auto render_node = render_graph->Add_Pass(cascades_gpu_pipeline, r_cascades_pass_name);
		render_node->G_Set_Framebuffer_Size(glm::uvec2(rc_resolution_w, rc_resolution_h));
		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			render_node->Depends_On("ProbesGen_" + std::to_string(i));
		}

		engine::Attachment_Info merge_col_info = Get_Color_Attachment_Info( Blend_Configs::B_None,
		    glm::vec4(0.0f), core->swapchain_ref->Get_Format(), vk::AttachmentLoadOp::eLoad,
		    vk::AttachmentStoreOp::eStore);

		merge_vert_shader = render_graph->resources_manager->Get_Shader(shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", S_Vert);
		merge_frag_shader = render_graph->resources_manager->Get_Shader(shader_path + "\\spirvGlsl\\FlatRendering\\cascadesMerge.frag.spv", S_Frag);

		auto shader_node = render_graph->Add_Gpu_Pipeline("MergeShader");
		shader_node->G_Set_Vert_Shader(merge_vert_shader);
		shader_node->G_Set_Frag_Shader(merge_frag_shader);
		shader_node->G_Set_Push_Constant_Size(sizeof(rc_pc));
		shader_node->G_Set_Vertex_Input(vertex_layout);
		shader_node->G_Add_Color_Attachment_Output(0, merge_col_info);
		shader_node->G_Build_Gpu_Pipeline();

		for (int i = cascades_info.cascade_count - 2; i >= 0; i--)
		{
			std::string name            = r_merge_pass_name + "_" + std::to_string(i);
			auto        merge_render_node = render_graph->Add_Pass(shader_node, name, "Graphics");
			merge_render_node->G_Set_Framebuffer_Size(glm::uvec2(rc_resolution_w, rc_resolution_h));
			merge_render_node->G_Add_Storage_Resource(radiances_images[i]);
			merge_render_node->G_Add_Storage_Resource(radiances_images[i + 1]);
			merge_render_node->Depends_On(r_cascades_pass_name);
			if (i < cascades_info.cascade_count - 2)
			{
				std::string dependancy_name = r_merge_pass_name + "_" + std::to_string(i + 1);
			}
		}

		result_vert_shader = render_graph->resources_manager->Get_Shader(
		    shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", S_Vert);
		result_frag_shader = render_graph->resources_manager->Get_Shader(
		    shader_path +
		        "\\spirvGlsl\\FlatRendering\\cascadesResult.frag.spv",
		    S_Frag);

		
		engine::Attachment_Info result_col_info = Get_Color_Attachment_Info(Blend_Configs::B_Opaque,
			glm::vec4(0.0f), core->swapchain_ref->Get_Format());
		auto result_gpu_pipeline = render_graph->Add_Gpu_Pipeline("ResultCascadesShader");
		result_gpu_pipeline->G_Set_Vert_Shader(result_vert_shader);
		result_gpu_pipeline->G_Set_Frag_Shader(result_frag_shader);
		result_gpu_pipeline->G_Set_Vertex_Input(Vertex_2d::Get_Vertex_Input());
		result_gpu_pipeline->G_Set_Push_Constant_Size(sizeof(rc_pc));
		result_gpu_pipeline->G_Add_Color_Attachment_Output(0, result_col_info);

		auto result_node = render_graph->Add_Pass(result_gpu_pipeline, result_pass_name, "Graphics");
		result_node->G_Set_Framebuffer_Size(glm::uvec2(rc_resolution_w, rc_resolution_h));
		result_node->Depends_On(r_merge_pass_name + "_" + std::to_string(0));
		result_node->Build_Render_Graph_Node();
	}

	void Recreate_Swap_Chain_Resources() override
	{
	}

	void Set_Render_Operation() override
	{
		auto painting_render_op = new std::function<void()>(
		    [this]() {
			    glm::vec2 mouse_input = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
			    painting_pc.x_mouse_pos = mouse_input.x;
			    painting_pc.y_mouse_pos = mouse_input.y;
			    // if (glfwGetMouseButton(window_provider->window, GLFW_MOUSE_BUTTON_2))
			    // {
			    //  paintingPc.painting = 1;
			    // }
			    // else
			    // {
			    //  paintingPc.painting = 0;
			    // }

			    auto &render_node = render_graph->render_nodes.at(painting_pass_name);
			    render_node->G_Set_Storage_Image_Array("painting_layers", painting_layers);
			    render_node->Get_Curr_Cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
			                                           vk::ShaderStageFlagBits::eCompute,
			                                           0, sizeof(painting_pc), &painting_pc);
			    render_node->Get_Curr_Cmd().dispatch(painting_pc.radius, painting_pc.radius, 1);
		    });

		render_graph->Get_Node(painting_pass_name)->G_Set_Render_Operation(painting_render_op);

		auto import_cuda_buffer_node_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph->render_nodes.at("CudaBufferImporterNode");
			    render_node->G_Set_Storage_Image("out_image", painting_layers[3]);
			    render_node->G_Set_Storage_Image("out_image_simulation_info", painting_layers[4]);
			    render_node->G_Set_Buffer("simulation_buffer", render_graph->resources_manager->Get_Buff_From_Name("CudaBuffer"));
			    render_node->G_Set_Buffer("simulation_buffer_info", render_graph->resources_manager->Get_Buff_From_Name("CudaBufferVel"));

			    render_node->Get_Curr_Cmd().dispatch((rc_resolution_w + rc_resolution_w - 1) / 8, (rc_resolution_h + rc_resolution_h - 1) / 8, 1);
		    });

		render_graph->Get_Node("CudaBufferImporterNode")->G_Set_Render_Operation(import_cuda_buffer_node_op);

		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			auto probes_gen_op = new std::function<void()>(
			    [this, i]() {
				    int idx            = i;
				    int interval_size_pc = cascades_info.interval_count;
				    int grid_size_pc     = cascades_info.probe_size_px;
				    for (int j = 0; j < idx; ++j)
				    {
					    interval_size_pc *= 2;
					    grid_size_pc *= 2;
				    }
				    probes_gen_pc.cascade_index = idx;
				    probes_gen_pc.interval_size = interval_size_pc;
				    probes_gen_pc.probe_size_px  = grid_size_pc;
				    auto &render_node         = render_graph->render_nodes.at(probes_gen_pass_names[idx]);

				    render_node->Get_Curr_Cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
				                                           vk::ShaderStageFlagBits::eVertex |
				                                               vk::ShaderStageFlagBits::eFragment,
				                                           0, sizeof(probes_gen_pc), &probes_gen_pc);
				    vk::DeviceSize offset = 0;
				    render_node->Get_Curr_Cmd().bindVertexBuffers(0, 1, &quad_vert_buffer_ref->buffer_handle.get(), &offset);
				    render_node->Get_Curr_Cmd().bindIndexBuffer(quad_index_buffer_ref->buffer_handle.get(), 0,
				                                             vk::IndexType::eUint32);
				    render_node->Get_Curr_Cmd().drawIndexed(Vertex_2d::Get_Quad_Indices().size(), 1, 0, 0, 0);
			    });
			render_graph->Get_Node(probes_gen_pass_names[i])->G_Set_Render_Operation(probes_gen_op);
		}

		auto radiance_output_task = new std::function<void()>([this]() {
			rc_pc.cascades_count      = cascades_info.cascade_count;
			rc_pc.probe_size_px        = cascades_info.probe_size_px;
			rc_pc.interval_count      = cascades_info.interval_count;
			rc_pc.base_interval_length = cascades_info.base_interval_length;
			rc_pc.f_width             = rc_resolution_w;
			rc_pc.f_height            = rc_resolution_h;

			auto *curr_image = render_graph->current_back_buffer;
			auto &render_node = render_graph->render_nodes.at(r_cascades_pass_name);
			render_node->G_Set_Color_Image_Attachment_Binding(0, curr_image);
			render_graph->Get_Node(r_cascades_pass_name)->G_Set_Framebuffer_Size(glm::uvec2(rc_resolution_w, rc_resolution_h));
		});
		auto radiance_output_op   = new std::function<void()>(
            [this]() {
                auto &render_node = render_graph->render_nodes.at(r_cascades_pass_name);
                render_node->G_Set_Sampler_Array("cascades", cascades_attachments_images_views);
                render_node->G_Set_Storage_Image_Array("painting_layers", painting_layers);
                render_node->G_Set_Storage_Image_Array("radiances", radiances_images);
                render_node->G_Set_Sampler("test_image", test_image->image_view_handle.get());
                render_node->G_Set_Sampler_Array("sprite_anims", test_sprite_anim->images_frames);
                render_node->G_Set_Buffer("Sprite_Info", test_sprite_anim->animation_info);
                render_node->G_Set_Sampler_Array("mat_textures",
			                                    background_materials.at(material_index_selected)->Convert_Textures_To_Vec());

                vk::DeviceSize offset = 0;
                render_node->Get_Curr_Cmd().bindVertexBuffers(0, 1, &quad_vert_buffer_ref->buffer_handle.get(), &offset);
                render_node->Get_Curr_Cmd().bindIndexBuffer(quad_index_buffer_ref->buffer_handle.get(), 0, vk::IndexType::eUint32);

                render_node->Get_Curr_Cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
			                                             vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			                                             0, sizeof(rc_pc), &rc_pc);

                render_node->Get_Curr_Cmd().drawIndexed(Vertex_2d::Get_Quad_Indices().size(), 1, 0,
			                                           0, 0);
            });
		render_graph->Get_Node(r_cascades_pass_name)->G_Set_Render_Operation(radiance_output_op);
		render_graph->Get_Node(r_cascades_pass_name)->Add_Pre_Rendering_Task(radiance_output_task);

		for (int i = cascades_info.cascade_count - 2; i >= 0; i--)
		{
			std::string merge_name_cascades = r_merge_pass_name + "_" + std::to_string(i);
			auto        merge_task         = new std::function<void()>([this, i]() {
                int         idx       = i;
                std::string merge_name = r_merge_pass_name + "_" + std::to_string(idx);
                auto       *curr_image = render_graph->current_back_buffer;
                render_graph->Get_Node(merge_name)->G_Set_Color_Image_Attachment_Binding(0, curr_image);
                render_graph->Get_Node(merge_name)->G_Set_Framebuffer_Size(glm::uvec2(rc_resolution_w, rc_resolution_h));
            });
			auto        merge_render_op     = new std::function<void()>(
                [this, i]() {
                    int         idx        = i;
                    std::string merge_name  = r_merge_pass_name + "_" + std::to_string(idx);
                    auto       &render_node = render_graph->render_nodes.at(merge_name);
                    render_node->G_Set_Sampler_Array("cascades", cascades_attachments_images_views);
                    render_node->G_Set_Storage_Image_Array("radiances", radiances_images);

                    rc_pc.cascade_index = idx;

                    vk::DeviceSize offset = 0;
                    render_node->Get_Curr_Cmd().bindVertexBuffers(0, 1, &quad_vert_buffer_ref->buffer_handle.get(), &offset);
                    render_node->Get_Curr_Cmd().bindIndexBuffer(quad_index_buffer_ref->buffer_handle.get(), 0,
				                                                        vk::IndexType::eUint32);

                    render_node->Get_Curr_Cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
				                                                      vk::ShaderStageFlagBits::eVertex |
				                                                          vk::ShaderStageFlagBits::eFragment,
				                                                      0, sizeof(rc_pc), &rc_pc);
                    render_node->Get_Curr_Cmd().drawIndexed(Vertex_2d::Get_Quad_Indices().size(), 1, 0,
				                                                    0, 0);
                });
			render_graph->Get_Node(merge_name_cascades)->G_Set_Render_Operation(merge_render_op);
			render_graph->Get_Node(merge_name_cascades)->Add_Pre_Rendering_Task(merge_task);
		}

		auto result_task     = new std::function<void()>([this]() {
            auto *curr_image = render_graph->current_back_buffer;
            render_graph->Get_Node(result_pass_name)->G_Set_Color_Image_Attachment_Binding(0, curr_image);
            render_graph->Get_Node(result_pass_name)->G_Set_Framebuffer_Size(glm::uvec2(rc_resolution_w, rc_resolution_h));
        });
		auto result_render_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph->render_nodes.at(result_pass_name);
			    render_node->G_Set_Storage_Image_Array("painting_layers", painting_layers);
			    render_node->G_Set_Storage_Image_Array("radiances", radiances_images);
			    render_node->G_Set_Sampler("test_image", test_image->image_view_handle.get());
			    render_node->G_Set_Sampler_Array("sprite_anims", test_sprite_anim->images_frames);
			    render_node->G_Set_Buffer("Sprite_Info", test_sprite_anim->animation_info);
			    render_node->G_Set_Sampler_Array("mat_textures",
			                                  background_materials.at(material_index_selected)->Convert_Textures_To_Vec());
			    render_node->G_Set_Buffer("Light_Info", light);
			    render_node->G_Set_Buffer("R_Configs_Uniform", r_configs);

			    vk::DeviceSize offset = 0;
			    render_node->Get_Curr_Cmd().bindVertexBuffers(0, 1, &quad_vert_buffer_ref->buffer_handle.get(), &offset);
			    render_node->Get_Curr_Cmd().bindIndexBuffer(quad_index_buffer_ref->buffer_handle.get(), 0, vk::IndexType::eUint32);

			    render_node->Get_Curr_Cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
			                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			                                           0, sizeof(rc_pc), &rc_pc);

			    render_node->Get_Curr_Cmd().drawIndexed(Vertex_2d::Get_Quad_Indices().size(), 1, 0,
			                                         0, 0);
			    test_sprite_anim->Use_Frame();
		    });
		render_graph->Get_Node(result_pass_name)->G_Set_Render_Operation(result_render_op);
		render_graph->Get_Node(result_pass_name)->Add_Pre_Rendering_Task(result_task);
	}

	void Reload_Shaders() override
	{
		auto *painting_node = render_graph->Get_Node(painting_pass_name);
		painting_node->G_Recreate_Resources();
		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			auto *gen_node = render_graph->Get_Node(probes_gen_pass_names[i]);
			gen_node->G_Recreate_Resources();
		}
		auto *output_node = render_graph->Get_Node(r_cascades_pass_name);
		output_node->G_Recreate_Resources();
		for (int i = cascades_info.cascade_count - 2; i >= 0; i--)
		{
			std::string name      = r_merge_pass_name + "_" + std::to_string(i);
			auto       *merge_node = render_graph->Get_Node(name);
			merge_node->G_Recreate_Resources();
		}
		auto *result_node = render_graph->Get_Node(result_pass_name);
		result_node->G_Recreate_Resources();
	}

	engine::Core           *core;
	engine::Render_Graph    *render_graph;
	::Window_Provider *window_provider;

	std::string painting_pass_name  = "PaintingPass";
	std::string r_cascades_pass_name = "rCascadesPass";
	std::string r_merge_pass_name    = "rMergePass";
	std::string result_pass_name    = "resultPass";

	Shader *result_vert_shader;
	Shader *result_frag_shader;

	Shader                  *merge_vert_shader;
	Shader                  *merge_frag_shader;
	std::vector<engine::Image_View *> radiances_images;

	Shader *vert_shader;
	Shader *frag_shader;

	std::vector<std::string> probes_gen_pass_names;
	Shader                  *probes_vert_shader;
	Shader                  *probes_frag_shader;
	std::vector<engine::Image_View *> cascades_attachments_images_views;

	Shader                  *paint_comp_shader;
	std::vector<engine::Image_View *> painting_layers;
	std::vector<rendering::Material *>  background_materials;
	int                      material_index_selected = 0;
	engine::Image_Shipper            *test_image;

	engine::Buffer *quad_vert_buffer_ref;
	engine::Buffer *quad_index_buffer_ref;

	Animator_2d *test_sprite_anim;

	Directional_Light        light{glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0, 0.0, 1.0), 0.01f};
	Painting_Pc              painting_pc;
	Rc_Pc                    rc_pc;
	Radiance_Cascades_Configs r_configs{};
	Probes_Gen_Pc             probes_gen_pc;
	Cascades_Info            cascades_info;
	int                     rc_resolution_w = 1024;
	int                     rc_resolution_h = 1024;
};
}        // namespace rendering
#endif        // FLAT_RENDERER_HPP
