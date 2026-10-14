//

// Created by carlo on 2024-12-02.
//

#ifndef FLAT_RENDERER_HPP
#define FLAT_RENDERER_HPP
#include "CodeCuda.cuh"
namespace rendering
{
using namespace engine;

class flat_renderer : public base_renderer
{
  public:
	flat_renderer(engine::core *core, ::window_provider *window_provider)
	{
		this->core           = core;
		this->render_graph    = core->render_graph_ref;
		this->window_provider = window_provider;
		create_resources();
		create_buffers();
		create_pipelines();
	}

	void create_resources()
	{
		cascades_info.cascade_count       = 5;
		cascades_info.probe_size_px        = 2;
		cascades_info.interval_count      = 2;
		cascades_info.base_interval_length = 1;
		auto image_info                  = engine::image::create_info2d(glm::uvec2(rc_resolution_w, rc_resolution_h), 1, 1,
		                                                      engine::g_32b_format,
		                                                      vk::ImageUsageFlagBits::eColorAttachment |
		                                                          vk::ImageUsageFlagBits::eSampled);
		cascades_attachments_images_views.reserve(cascades_info.cascade_count);
		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			std::string name      = "CascadeAttachment_" + std::to_string(i);
			engine::image_view  *image_view_handle = resources_manager::get_instance()->get_image(name, image_info, 0, 0);
			cascades_attachments_images_views.emplace_back(image_view_handle);
		}

		probes_gen_pc.cascade_index = 0;
		probes_gen_pc.interval_size = 2;
		probes_gen_pc.probe_size_px  = cascades_info.probe_size_px;

		rc_pc.probe_size_px        = cascades_info.probe_size_px;
		rc_pc.interval_count      = cascades_info.interval_count;
		rc_pc.base_interval_length = cascades_info.base_interval_length;

		painting_pc.radius = 20;

		auto       storage_image_info = engine::image::create_info2d(glm::uvec2(rc_resolution_w, rc_resolution_h), 1, 1,
		                                                          engine::g_32b_format,
		                                                          vk::ImageUsageFlagBits::eStorage |
		                                                              vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
		engine::image_view *light_layer       = resources_manager::get_instance()->get_image("PaintingLayer", storage_image_info, 0, 0);
		engine::image_view *occluder_layer    = resources_manager::get_instance()->get_image(
            "OccluderLayer", storage_image_info, 0, 0);
		engine::image_view *debug_layer        = resources_manager::get_instance()->get_image("DebugRaysLayer", storage_image_info, 0, 0);
		engine::image_view *fluid_sim_layer     = resources_manager::get_instance()->get_image("FluidSimLayer", storage_image_info, 0, 0);
		engine::image_view *fluid_sim_layer_info = resources_manager::get_instance()->get_image("FluidSimInfoLayer", storage_image_info, 0, 0);
		painting_layers.push_back(light_layer);
		painting_layers.push_back(occluder_layer);
		painting_layers.push_back(debug_layer);
		painting_layers.push_back(fluid_sim_layer);
		painting_layers.push_back(fluid_sim_layer_info);

		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			std::string name      = "radianceStorage_" + std::to_string(i);
			engine::image_view  *image_view_handle = resources_manager::get_instance()->get_image(name, storage_image_info, 0, 0);
			radiances_images.emplace_back(image_view_handle);
		}

		std::string resources_path = systems::os::get_instance()->get_engine_resources_path();
		std::string asset_path     = systems::os::get_instance()->get_assets_path();

		test_image = resources_manager::get_instance()->get_shipper("test_image",
		                                                        resources_path + "\\Images\\VulkanLogo.png", 1, 1,
		                                                        engine::g_shipper_format,
		                                                        layout_patterns::graphics_read);

		std::filesystem::path dir_entry(asset_path + "\\Textures\\RCascadesTextures");

		int i = 0;
		for (auto &path_view : std::filesystem::directory_iterator(dir_entry))
		{
			rendering::material *background_material = rendering_res_manager::get_instance()->push_material(
			    "RCascadesMat_" + std::to_string(i++));
			std::string path   = path_view.path().string();
			engine::image_view  *col_ref = resources_manager::get_instance()->get_shipper(
                                                                   path + "\\Albedo.png", path + "\\Albedo.png", 1, 1, engine::g_shipper_format,
                                                                   layout_patterns::graphics_read)
			                        ->image_view_handle.get();
			background_material->set_texture(texture_type::albedo, col_ref);

			engine::image_view *norm_ref = resources_manager::get_instance()->get_shipper(
			                                                        path + "\\Normal.png", path + "\\Normal.png", 1, 1,
			                                                        engine::g_shipper_format, layout_patterns::graphics_read)
			                         ->image_view_handle.get();
			background_material->set_texture(texture_type::normal, norm_ref);

			engine::image_view *roughness_ref = resources_manager::get_instance()->get_shipper(
			                                                             path + "\\Roughness.png", path + "\\Roughness.png", 1, 1,
			                                                             engine::g_shipper_format, layout_patterns::graphics_read)
			                              ->image_view_handle.get();
			background_material->set_texture(texture_type::roughness, roughness_ref);

			engine::image_view *ao_ref = resources_manager::get_instance()->get_shipper(
			                                                      path + "\\Ao.png", path + "\\Ao.png", 1, 1,
			                                                      engine::g_shipper_format, layout_patterns::graphics_read)
			                       ->image_view_handle.get();
			background_material->set_texture(texture_type::ao, ao_ref);

			engine::image_view *height_ref = resources_manager::get_instance()->get_shipper(
			                                                          path + "\\Height.png", path + "\\Height.png", 1, 1,
			                                                          engine::g_shipper_format, layout_patterns::graphics_read)
			                           ->image_view_handle.get();
			background_material->set_texture(texture_type::height, height_ref);
			background_materials.emplace_back(background_material);
		}
		material_index_selected = 0;

		// testSpriteAnim.LoadAtlas(
		// assetPath + "\\Animations\\SmokeFreePack_v2\\Compressed\\512\\Smoke_4_512-sheet.png",
		// glm::uvec2(512, 512), 7, 7, 10);

		rendering::animator_info animation_info = {glm::uvec2(512, 512), 7, 7, -1, -1, -1, true};
		test_sprite_anim            = rendering_res_manager::get_instance()->get_animator("TestAnim",
		                                                                            asset_path +
		                                                                                "\\Animations\\SmokeFreePack_v2\\Compressed\\512\\Smoke_4_512-sheet.png",
		                                                                            30, animation_info);
		if (test_sprite_anim->images_frames.empty())
		{
			// The optional smoke atlas is not distributed with the engine.
			// ImageShipper owns and frees this pixel after uploading it.
			auto *transparent_pixel = std::calloc(4, 1);
			auto *fallback = resources_manager::get_instance()->get_shipper(
			    "EmptySmokeAtlas", transparent_pixel, 1, 1, 4, 1, 1,
			    engine::g_shipper_format, layout_patterns::graphics_read);
			test_sprite_anim->images_frames.push_back(fallback->image_view_handle.get());
			test_sprite_anim->animation_info = {glm::uvec2(1), 1, 1, 0, 1, 0.0f, true};
			test_sprite_anim->stop = true;
		}
	}

	void create_buffers()
	{
		quad_vert_buffer_ref = resources_manager::get_instance()->get_stage_buffer(
		                                                       "QuadRcVertices", vk::BufferUsageFlagBits::eVertexBuffer,
		                                                       sizeof(vertex_2d) * vertex_2d::get_quad_vertices().size(), sizeof(vertex_2d),
		                                                       vertex_2d::get_quad_vertices().data())
		                        ->device_buffer.get();
		quad_index_buffer_ref = resources_manager::get_instance()->get_stage_buffer(
		                                                        "QuadRcIndices", vk::BufferUsageFlagBits::eIndexBuffer,
		                                                        sizeof(uint32_t) * vertex_2d::get_quad_indices().size(), sizeof(uint32_t),
		                                                        vertex_2d::get_quad_indices().data())
		                         ->device_buffer.get();
	}

	void create_pipelines()
	{
		auto        logical_device = core->logical_device.get();
		std::string shader_path    = systems::os::get_instance()->get_shaders_path();

		auto cuda_buffer = render_graph->resources_manager->get_buffer(engine::resources_manager::buffer_params{
		    "CudaBuffer", vk::BufferUsageFlagBits::eStorageBuffer, {}, sizeof(float) * 4 * rc_resolution_w * rc_resolution_h, sizeof(float) * 4, nullptr, engine::resources_manager::buffer_type::external});

		auto cuda_buffer_vel = render_graph->resources_manager->get_buffer(engine::resources_manager::buffer_params{
		    "CudaBufferVel", vk::BufferUsageFlagBits::eStorageBuffer, {}, sizeof(float) * rc_resolution_w * rc_resolution_h, sizeof(float), nullptr, engine::resources_manager::buffer_type::external});

		auto cuda_pi = render_graph->add_cuda_pipeline("CudaCopySmoke");
		cuda_pi
		    ->c_export_buffer(cuda_buffer)
		    ->c_set_kernel_function(*CodeCuda::FluidSimulation::C_GetKernelLauncherMappers(0).GetKernelFunct())
		    ->c_build_cuda_pipeline();
		auto cuda_node = render_graph->add_cuda_pass(cuda_pi, "CudaNode");

		auto cuda_pi_vel = render_graph->add_cuda_pipeline("CudaCopyVel");
		cuda_pi_vel
		    ->c_export_buffer(cuda_buffer_vel)
		    ->c_set_kernel_function(*CodeCuda::FluidSimulation::C_GetKernelLauncherMappers(1).GetKernelFunct())
		    ->c_build_cuda_pipeline();

		auto cuda_vel = render_graph->add_cuda_pass(cuda_pi_vel, "CudaVelNode");

		//

		paint_comp_shader = render_graph->resources_manager->get_shader(
		    shader_path + "\\slang\\test\\paintingGen.slang", s_comp);
		auto *painting_gpu_pipeline = render_graph->add_gpu_pipeline("PaintingCompute");
		painting_gpu_pipeline->g_set_comp_shader(paint_comp_shader);
		painting_gpu_pipeline->g_set_push_constant_size(sizeof(painting_pc));
		painting_gpu_pipeline->g_build_gpu_pipeline();

		auto *painting_node = render_graph->add_pass(painting_gpu_pipeline, painting_pass_name, "Graphics");
		painting_node->g_add_storage_resource(painting_layers[0]);
		painting_node->g_add_storage_resource(painting_layers[1]);
		painting_node->g_add_storage_resource(painting_layers[2]);

		auto cuda_buffer_imports = render_graph->resources_manager->get_shader(
		    shader_path + "\\slang\\test\\cudaBufferToImage.slang", s_comp);

		auto *import_buffer_node = render_graph->add_pass("CudaBufferImporterNode");
		import_buffer_node->g_set_comp_shader(cuda_buffer_imports);
		import_buffer_node->g_add_storage_resource(painting_layers[3]);
		import_buffer_node->g_add_storage_resource(painting_layers[4]);
		import_buffer_node->depends_on(cuda_node);
		import_buffer_node->depends_on(cuda_vel);
		import_buffer_node->build_render_graph_node();

		engine::vertex_input    vertex_layout = vertex_2d::get_vertex_input();
		engine::attachment_info col_info     = get_color_attachment_info(blend_configs::b_opaque,
            glm::vec4(0.0f), vk::Format::eR32G32B32A32Sfloat);

		probes_vert_shader = render_graph->resources_manager->get_shader(
		    shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", s_vert);
		probes_frag_shader = render_graph->resources_manager->get_shader(
		    shader_path + "\\spirvGlsl\\FlatRendering\\cascadeGen.frag.spv",
		    s_frag);

		auto probes_gen_gpu_pipeline = render_graph->add_gpu_pipeline("ProbesGenShader");
		probes_gen_gpu_pipeline->g_set_configs({true});
		probes_gen_gpu_pipeline->g_set_push_constant_size(sizeof(painting_pc));
		probes_gen_gpu_pipeline->g_set_vert_shader(probes_vert_shader);
		probes_gen_gpu_pipeline->g_set_frag_shader(probes_frag_shader);
		probes_gen_gpu_pipeline->g_set_vertex_input(vertex_layout);
		probes_gen_gpu_pipeline->g_add_color_attachment_output(0, col_info);
		probes_gen_gpu_pipeline->g_build_gpu_pipeline();

		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			std::string name = "ProbesGen_" + std::to_string(i);
			probes_gen_pass_names.push_back(name);
			auto render_node = render_graph->add_pass(probes_gen_gpu_pipeline, name, "Graphics_Test");
			render_node->g_set_framebuffer_size(glm::uvec2(rc_resolution_w, rc_resolution_h));
			render_node->g_set_color_image_attachment_binding(0, cascades_attachments_images_views[i]);
			render_node->depends_on(painting_pass_name);
		}

		vert_shader = render_graph->resources_manager->get_shader(
		    shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", s_vert);
		frag_shader = render_graph->resources_manager->get_shader(
		    shader_path + "\\spirvGlsl\\FlatRendering\\rCascadesOutput.frag.spv",
		    s_frag);

		// This pass writes storage images only; disable fragment color writes.
		engine::attachment_info output_col_info = get_color_attachment_info(blend_configs::b_none,
		    glm::vec4(0.0f), core->swapchain_ref->get_format());

		auto cascades_gpu_pipeline = render_graph->add_gpu_pipeline("CascadesGPUPipeline");
		cascades_gpu_pipeline->g_set_vert_shader(vert_shader);
		cascades_gpu_pipeline->g_set_frag_shader(frag_shader);
		cascades_gpu_pipeline->g_set_push_constant_size(sizeof(rc_pc));
		cascades_gpu_pipeline->g_set_vertex_input(vertex_layout);
		cascades_gpu_pipeline->g_add_color_attachment_output(0, output_col_info);
		cascades_gpu_pipeline->g_build_gpu_pipeline();

		auto render_node = render_graph->add_pass(cascades_gpu_pipeline, r_cascades_pass_name);
		render_node->g_set_framebuffer_size(glm::uvec2(rc_resolution_w, rc_resolution_h));
		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			render_node->depends_on("ProbesGen_" + std::to_string(i));
		}

		engine::attachment_info merge_col_info = get_color_attachment_info( blend_configs::b_none,
		    glm::vec4(0.0f), core->swapchain_ref->get_format(), vk::AttachmentLoadOp::eLoad,
		    vk::AttachmentStoreOp::eStore);

		merge_vert_shader = render_graph->resources_manager->get_shader(shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", s_vert);
		merge_frag_shader = render_graph->resources_manager->get_shader(shader_path + "\\spirvGlsl\\FlatRendering\\cascadesMerge.frag.spv", s_frag);

		auto shader_node = render_graph->add_gpu_pipeline("MergeShader");
		shader_node->g_set_vert_shader(merge_vert_shader);
		shader_node->g_set_frag_shader(merge_frag_shader);
		shader_node->g_set_push_constant_size(sizeof(rc_pc));
		shader_node->g_set_vertex_input(vertex_layout);
		shader_node->g_add_color_attachment_output(0, merge_col_info);
		shader_node->g_build_gpu_pipeline();

		for (int i = cascades_info.cascade_count - 2; i >= 0; i--)
		{
			std::string name            = r_merge_pass_name + "_" + std::to_string(i);
			auto        merge_render_node = render_graph->add_pass(shader_node, name, "Graphics");
			merge_render_node->g_set_framebuffer_size(glm::uvec2(rc_resolution_w, rc_resolution_h));
			merge_render_node->g_add_storage_resource(radiances_images[i]);
			merge_render_node->g_add_storage_resource(radiances_images[i + 1]);
			merge_render_node->depends_on(r_cascades_pass_name);
			if (i < cascades_info.cascade_count - 2)
			{
				std::string dependancy_name = r_merge_pass_name + "_" + std::to_string(i + 1);
			}
		}

		result_vert_shader = render_graph->resources_manager->get_shader(
		    shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", s_vert);
		result_frag_shader = render_graph->resources_manager->get_shader(
		    shader_path +
		        "\\spirvGlsl\\FlatRendering\\cascadesResult.frag.spv",
		    s_frag);

		
		engine::attachment_info result_col_info = get_color_attachment_info(blend_configs::b_opaque,
			glm::vec4(0.0f), core->swapchain_ref->get_format());
		auto result_gpu_pipeline = render_graph->add_gpu_pipeline("ResultCascadesShader");
		result_gpu_pipeline->g_set_vert_shader(result_vert_shader);
		result_gpu_pipeline->g_set_frag_shader(result_frag_shader);
		result_gpu_pipeline->g_set_vertex_input(vertex_2d::get_vertex_input());
		result_gpu_pipeline->g_set_push_constant_size(sizeof(rc_pc));
		result_gpu_pipeline->g_add_color_attachment_output(0, result_col_info);

		auto result_node = render_graph->add_pass(result_gpu_pipeline, result_pass_name, "Graphics");
		result_node->g_set_framebuffer_size(glm::uvec2(rc_resolution_w, rc_resolution_h));
		result_node->depends_on(r_merge_pass_name + "_" + std::to_string(0));
		result_node->build_render_graph_node();
	}

	void recreate_swap_chain_resources() override
	{
	}

	void set_render_operation() override
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
			    render_node->g_set_storage_image_array("painting_layers", painting_layers);
			    render_node->get_curr_cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
			                                           vk::ShaderStageFlagBits::eCompute,
			                                           0, sizeof(painting_pc), &painting_pc);
			    render_node->get_curr_cmd().dispatch(painting_pc.radius, painting_pc.radius, 1);
		    });

		render_graph->get_node(painting_pass_name)->g_set_render_operation(painting_render_op);

		auto import_cuda_buffer_node_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph->render_nodes.at("CudaBufferImporterNode");
			    render_node->g_set_storage_image("out_image", painting_layers[3]);
			    render_node->g_set_storage_image("out_image_simulation_info", painting_layers[4]);
			    render_node->g_set_buffer("simulation_buffer", render_graph->resources_manager->get_buff_from_name("CudaBuffer"));
			    render_node->g_set_buffer("simulation_buffer_info", render_graph->resources_manager->get_buff_from_name("CudaBufferVel"));

			    render_node->get_curr_cmd().dispatch((rc_resolution_w + rc_resolution_w - 1) / 8, (rc_resolution_h + rc_resolution_h - 1) / 8, 1);
		    });

		render_graph->get_node("CudaBufferImporterNode")->g_set_render_operation(import_cuda_buffer_node_op);

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

				    render_node->get_curr_cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
				                                           vk::ShaderStageFlagBits::eVertex |
				                                               vk::ShaderStageFlagBits::eFragment,
				                                           0, sizeof(probes_gen_pc), &probes_gen_pc);
				    vk::DeviceSize offset = 0;
				    render_node->get_curr_cmd().bindVertexBuffers(0, 1, &quad_vert_buffer_ref->buffer_handle.get(), &offset);
				    render_node->get_curr_cmd().bindIndexBuffer(quad_index_buffer_ref->buffer_handle.get(), 0,
				                                             vk::IndexType::eUint32);
				    render_node->get_curr_cmd().drawIndexed(vertex_2d::get_quad_indices().size(), 1, 0, 0, 0);
			    });
			render_graph->get_node(probes_gen_pass_names[i])->g_set_render_operation(probes_gen_op);
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
			render_node->g_set_color_image_attachment_binding(0, curr_image);
			render_graph->get_node(r_cascades_pass_name)->g_set_framebuffer_size(glm::uvec2(rc_resolution_w, rc_resolution_h));
		});
		auto radiance_output_op   = new std::function<void()>(
            [this]() {
                auto &render_node = render_graph->render_nodes.at(r_cascades_pass_name);
                render_node->g_set_sampler_array("cascades", cascades_attachments_images_views);
                render_node->g_set_storage_image_array("painting_layers", painting_layers);
                render_node->g_set_storage_image_array("radiances", radiances_images);
                render_node->g_set_sampler("test_image", test_image->image_view_handle.get());
                render_node->g_set_sampler_array("sprite_anims", test_sprite_anim->images_frames);
                render_node->g_set_buffer("sprite_info", test_sprite_anim->animation_info);
                render_node->g_set_sampler_array("mat_textures",
			                                    background_materials.at(material_index_selected)->convert_textures_to_vec());

                vk::DeviceSize offset = 0;
                render_node->get_curr_cmd().bindVertexBuffers(0, 1, &quad_vert_buffer_ref->buffer_handle.get(), &offset);
                render_node->get_curr_cmd().bindIndexBuffer(quad_index_buffer_ref->buffer_handle.get(), 0, vk::IndexType::eUint32);

                render_node->get_curr_cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
			                                             vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			                                             0, sizeof(rc_pc), &rc_pc);

                render_node->get_curr_cmd().drawIndexed(vertex_2d::get_quad_indices().size(), 1, 0,
			                                           0, 0);
            });
		render_graph->get_node(r_cascades_pass_name)->g_set_render_operation(radiance_output_op);
		render_graph->get_node(r_cascades_pass_name)->add_pre_rendering_task(radiance_output_task);

		for (int i = cascades_info.cascade_count - 2; i >= 0; i--)
		{
			std::string merge_name_cascades = r_merge_pass_name + "_" + std::to_string(i);
			auto        merge_task         = new std::function<void()>([this, i]() {
                int         idx       = i;
                std::string merge_name = r_merge_pass_name + "_" + std::to_string(idx);
                auto       *curr_image = render_graph->current_back_buffer;
                render_graph->get_node(merge_name)->g_set_color_image_attachment_binding(0, curr_image);
                render_graph->get_node(merge_name)->g_set_framebuffer_size(glm::uvec2(rc_resolution_w, rc_resolution_h));
            });
			auto        merge_render_op     = new std::function<void()>(
                [this, i]() {
                    int         idx        = i;
                    std::string merge_name  = r_merge_pass_name + "_" + std::to_string(idx);
                    auto       &render_node = render_graph->render_nodes.at(merge_name);
                    render_node->g_set_sampler_array("cascades", cascades_attachments_images_views);
                    render_node->g_set_storage_image_array("radiances", radiances_images);

                    rc_pc.cascade_index = idx;

                    vk::DeviceSize offset = 0;
                    render_node->get_curr_cmd().bindVertexBuffers(0, 1, &quad_vert_buffer_ref->buffer_handle.get(), &offset);
                    render_node->get_curr_cmd().bindIndexBuffer(quad_index_buffer_ref->buffer_handle.get(), 0,
				                                                        vk::IndexType::eUint32);

                    render_node->get_curr_cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
				                                                      vk::ShaderStageFlagBits::eVertex |
				                                                          vk::ShaderStageFlagBits::eFragment,
				                                                      0, sizeof(rc_pc), &rc_pc);
                    render_node->get_curr_cmd().drawIndexed(vertex_2d::get_quad_indices().size(), 1, 0,
				                                                    0, 0);
                });
			render_graph->get_node(merge_name_cascades)->g_set_render_operation(merge_render_op);
			render_graph->get_node(merge_name_cascades)->add_pre_rendering_task(merge_task);
		}

		auto result_task     = new std::function<void()>([this]() {
            auto *curr_image = render_graph->current_back_buffer;
            render_graph->get_node(result_pass_name)->g_set_color_image_attachment_binding(0, curr_image);
            render_graph->get_node(result_pass_name)->g_set_framebuffer_size(glm::uvec2(rc_resolution_w, rc_resolution_h));
        });
		auto result_render_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph->render_nodes.at(result_pass_name);
			    render_node->g_set_storage_image_array("painting_layers", painting_layers);
			    render_node->g_set_storage_image_array("radiances", radiances_images);
			    render_node->g_set_sampler("test_image", test_image->image_view_handle.get());
			    render_node->g_set_sampler_array("sprite_anims", test_sprite_anim->images_frames);
			    render_node->g_set_buffer("sprite_info", test_sprite_anim->animation_info);
			    render_node->g_set_sampler_array("mat_textures",
			                                  background_materials.at(material_index_selected)->convert_textures_to_vec());
			    render_node->g_set_buffer("light_info", light);
			    render_node->g_set_buffer("r_configs_uniform", r_configs);

			    vk::DeviceSize offset = 0;
			    render_node->get_curr_cmd().bindVertexBuffers(0, 1, &quad_vert_buffer_ref->buffer_handle.get(), &offset);
			    render_node->get_curr_cmd().bindIndexBuffer(quad_index_buffer_ref->buffer_handle.get(), 0, vk::IndexType::eUint32);

			    render_node->get_curr_cmd().pushConstants(render_node->gpu_pipeline_ref->pipeline_layout.get(),
			                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			                                           0, sizeof(rc_pc), &rc_pc);

			    render_node->get_curr_cmd().drawIndexed(vertex_2d::get_quad_indices().size(), 1, 0,
			                                         0, 0);
			    test_sprite_anim->use_frame();
		    });
		render_graph->get_node(result_pass_name)->g_set_render_operation(result_render_op);
		render_graph->get_node(result_pass_name)->add_pre_rendering_task(result_task);
	}

	void reload_shaders() override
	{
		auto *painting_node = render_graph->get_node(painting_pass_name);
		painting_node->g_recreate_resources();
		for (int i = 0; i < cascades_info.cascade_count; ++i)
		{
			auto *gen_node = render_graph->get_node(probes_gen_pass_names[i]);
			gen_node->g_recreate_resources();
		}
		auto *output_node = render_graph->get_node(r_cascades_pass_name);
		output_node->g_recreate_resources();
		for (int i = cascades_info.cascade_count - 2; i >= 0; i--)
		{
			std::string name      = r_merge_pass_name + "_" + std::to_string(i);
			auto       *merge_node = render_graph->get_node(name);
			merge_node->g_recreate_resources();
		}
		auto *result_node = render_graph->get_node(result_pass_name);
		result_node->g_recreate_resources();
	}

	engine::core           *core;
	engine::render_graph    *render_graph;
	::window_provider *window_provider;

	std::string painting_pass_name  = "PaintingPass";
	std::string r_cascades_pass_name = "rCascadesPass";
	std::string r_merge_pass_name    = "rMergePass";
	std::string result_pass_name    = "resultPass";

	shader *result_vert_shader;
	shader *result_frag_shader;

	shader                  *merge_vert_shader;
	shader                  *merge_frag_shader;
	std::vector<engine::image_view *> radiances_images;

	shader *vert_shader;
	shader *frag_shader;

	std::vector<std::string> probes_gen_pass_names;
	shader                  *probes_vert_shader;
	shader                  *probes_frag_shader;
	std::vector<engine::image_view *> cascades_attachments_images_views;

	shader                  *paint_comp_shader;
	std::vector<engine::image_view *> painting_layers;
	std::vector<rendering::material *>  background_materials;
	int                      material_index_selected = 0;
	engine::image_shipper            *test_image;

	engine::buffer *quad_vert_buffer_ref;
	engine::buffer *quad_index_buffer_ref;

	animator_2d *test_sprite_anim;

	directional_light        light{glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0, 0.0, 1.0), 0.01f};
	painting_pc              painting_pc;
	rc_pc                    rc_pc;
	radiance_cascades_configs r_configs{};
	probes_gen_pc             probes_gen_pc;
	cascades_info            cascades_info;
	int                     rc_resolution_w = 1024;
	int                     rc_resolution_h = 1024;
};
}        // namespace rendering
#endif        // FLAT_RENDERER_HPP
