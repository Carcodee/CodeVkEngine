//

// Created by carlo on 2024-10-25.
//

#ifndef CLUSTER_RENDERER_HPP
#define CLUSTER_RENDERER_HPP

namespace rendering
{
using namespace engine;
class cluster_renderer : public base_renderer
{
  public:
	cluster_renderer(engine::core *core, ::window_provider *window_provider)
	{
		this->core           = core;
		this->render_graph_ref = core->render_graph_ref;
		this->window_provider = window_provider;
		create_resources();
		create_buffers();
		create_pipelines();
	}

	void recreate_swap_chain_resources() override
	{
	}

	void set_render_operation() override
	{
		render_graph_ref->get_node(mesh_cull_pass_name)->g_set_render_operation(new std::function<void()>([this]() {
			auto &render_node = render_graph_ref->render_nodes.at(mesh_cull_pass_name);
			render_node->g_set_buffer("indirect_cmds_buffer",
			                      rendering_res_manager::get_instance()->indirect_draw_buffer);
			render_node->g_set_buffer("meshes_spheres_buffer", meshes_spheres_compact);
			render_node->g_set_buffer("cam_props", c_props_ubo);
			render_node->g_set_buffer("cull_info_uniform", cam_frustum);
			render_node->get_curr_cmd().dispatch(rendering_res_manager::get_instance()->indirect_draws_cmd_infos.size(), 1, 1);
		}));
		render_graph_ref->get_node(mesh_cull_pass_name)->add_pre_rendering_task(new std::function<void()>([this]() {
			move_cam();
			c_props_ubo.inv_proj = glm::inverse(curr_camera->matrices.perspective);
			c_props_ubo.inv_view = glm::inverse(curr_camera->matrices.view);
			c_props_ubo.pos     = curr_camera->position;
			c_props_ubo.z_near   = curr_camera->camera_properties.z_near;
			c_props_ubo.z_far    = curr_camera->camera_properties.z_far;
			build_frustum_planes();

			meshes_spheres_compact.clear();
			std::vector<sphere> meshes_spheres_compact2;
			for (auto &model : rendering_res_manager::get_instance()->indirect_models_to_draw)
			{
				for (auto &sphere : model.second->meshes_spheres)
				{
					meshes_spheres_compact.emplace_back(sphere);
					meshes_spheres_compact2.emplace_back(sphere);
				}
			}
		}));

		auto cull_task = new std::function<void()>([this]() {
			cull_data_pc.s_width           = (int) window_provider->get_window_size().x;
			cull_data_pc.s_height          = (int) window_provider->get_window_size().y;
			cull_data_pc.point_lights_count = point_lights.size();
			cull_data_pc.x_tile_count       = static_cast<uint32_t>((core->swapchain_ref->extent.width - 1) / x_tile_size_px + 1);
			cull_data_pc.y_tile_count       = static_cast<uint32_t>((core->swapchain_ref->extent.height - 1) / y_tile_size_px + 1);

			lights_map.clear();
			for (int i = 0; i < cull_data_pc.x_tile_count * cull_data_pc.y_tile_count * z_slices_size; ++i)
			{
				lights_map.emplace_back(array_indexer{});
			}
			lights_indices.clear();
			lights_indices.reserve(lights_map.size() * point_lights.size());
			for (int i = 0; i < lights_map.size() * point_lights.size(); ++i)
			{
				lights_indices.emplace_back(-1);
			}
			c_props_ubo.inv_proj = glm::inverse(camera.matrices.perspective);
			c_props_ubo.inv_view = glm::inverse(camera.matrices.view);
			c_props_ubo.pos     = camera.position;
			c_props_ubo.z_near   = camera.camera_properties.z_near;
			c_props_ubo.z_far    = camera.camera_properties.z_far;
		});

		auto cull_render_op = new std::function<void()>(
		    [this]() {
			    auto &render_node = render_graph_ref->render_nodes.at(compute_pass_name);
			    render_node->g_set_buffer("point_lights_buffer", point_lights);
			    render_node->g_set_buffer("light_map_buffer", lights_map);
			    render_node->g_set_buffer("light_indices_buffer", lights_indices);
			    render_node->g_set_buffer("camera_properties", c_props_ubo);
			    render_node->get_curr_cmd().pushConstants(render_graph_ref->get_node(compute_pass_name)->gpu_pipeline_ref->pipeline_layout.get(),
			                                           vk::ShaderStageFlagBits::eCompute,
			                                           0, sizeof(screen_data_pc), &cull_data_pc);
			    render_node->get_curr_cmd().dispatch(cull_data_pc.x_tile_count / local_size, cull_data_pc.y_tile_count / local_size,
			                                      z_slices_size);
		    });

		render_graph_ref->get_node(compute_pass_name)->add_pre_rendering_task(cull_task);
		render_graph_ref->get_node(compute_pass_name)->g_set_render_operation(cull_render_op);

		auto render_op = new std::function<void()>(
		    [this]() {
			    vk::DeviceSize           offset = 0;
			    std::vector<engine::image_view *> textures;
			    for (auto &image : resources_manager::get_instance()->image_shippers)
			    {
				    textures.emplace_back(image->image_view_handle.get());
			    }
			    std::vector<rendering::material_packed_data> materials;
			    for (auto &mat : rendering_res_manager::get_instance()->material_packed_data)
			    {
				    materials.emplace_back(*mat);
			    }
			    if (materials.empty())
			    {
				    materials.emplace_back(material_packed_data());
			    }

			    std::vector<glm::mat4> model_mats;
			    std::vector<int>       mesh_mat_ids;
			    for (auto &model : rendering_res_manager::get_instance()->models)
			    {
				    for (int i = 0; i < model->models_mat.size(); ++i)
				    {
					    model_mats.push_back(model->models_mat[i]);
				    }
				    for (int i = 0; i < model->materials.size(); ++i)
				    {
					    mesh_mat_ids.push_back(model->materials[i]);
				    }
			    }
			    auto render_node = render_graph_ref->get_node(g_buffer_pass_name);

			    render_graph_ref->get_node(g_buffer_pass_name)->g_set_sampler_array("textures", textures);
			    render_graph_ref->get_node(g_buffer_pass_name)->g_set_buffer("materials_packed_buffer", materials);
			    render_graph_ref->get_node(g_buffer_pass_name)->g_set_buffer("mesh_materials_ids", mesh_mat_ids);
			    render_graph_ref->get_node(g_buffer_pass_name)->g_set_buffer("meshes_model_matrices", model_mats);

			    pc.proj_view = camera.matrices.perspective * camera.matrices.view;
			    render_node->get_curr_cmd().pushConstants(render_graph_ref->get_node(g_buffer_pass_name)->gpu_pipeline_ref->pipeline_layout.get(),
			                                           vk::ShaderStageFlagBits::eVertex |
			                                               vk::ShaderStageFlagBits::eFragment,
			                                           0, sizeof(mvp_pc), &pc);

			    int mesh_offset = 0;

			    for (auto &model_pair : rendering_res_manager::get_instance()->indirect_models_to_draw)
			    {
				    rendering::model *model_ref = model_pair.second;
				    render_node->get_curr_cmd().bindVertexBuffers(0, 1, &model_ref->vert_buffer->device_buffer->buffer_handle.get(),
				                                               &offset);
				    render_node->get_curr_cmd().bindIndexBuffer(model_ref->index_buffer->get_buffer(), 0, vk::IndexType::eUint32);

				    vk::DeviceSize size_offset = (mesh_offset) * sizeof(draw_indirect_indexed_cmd);
				    uint32_t       stride     = sizeof(draw_indirect_indexed_cmd);
				    render_node->get_curr_cmd().drawIndexedIndirect(
				        rendering_res_manager::get_instance()->indirect_draw_buffer->buffer_handle.get(),
				        size_offset,
				        model_ref->mesh_count,
				        stride);
				    mesh_offset += model_ref->mesh_count;
			    }
		    });

		render_graph_ref->get_node(g_buffer_pass_name)->g_set_render_operation(render_op);

		auto l_set_view_task = new std::function<void()>([this]() {
			auto render_node = render_graph_ref->get_node(light_pass_name);
			light_pc.x_tile_count  = cull_data_pc.x_tile_count;
			light_pc.y_tile_count  = cull_data_pc.y_tile_count;
			light_pc.x_tile_size_px = x_tile_size_px;
			light_pc.y_tile_size_px = y_tile_size_px;
			light_pc.z_slices     = z_slices_size;

			auto *curr_image = render_graph_ref->current_back_buffer;
			render_node->g_set_color_image_attachment_binding(0, curr_image);
			render_node->g_set_framebuffer_size(window_provider->get_window_size());
		});
		auto l_render_op    = new std::function<void()>(
            [this]() {
                auto render_node = render_graph_ref->get_node(light_pass_name);
                render_graph_ref->resources_manager->request_storage_image_clear("specularHolderStorage");
			    render_node->g_set_sampler("g_col", col_attachment_view);
			    render_node->g_set_sampler("g_normals", norm_attachment_view);
			    render_node->g_set_sampler("g_tang", tang_attachment_view);
			    render_node->g_set_sampler("g_depth", depth_attachment_view);
			    render_node->g_set_sampler("g_met_roughness", met_rough_attachment_view);
			    render_node->g_set_sampler("g_mesh_uv", uv_attachment_view);

			    render_node->g_set_storage_image("specular_holder", specular_holder);
			    render_node->g_set_buffer("camera_properties", c_props_ubo);
			    render_node->g_set_buffer("point_lights_buffer", point_lights);
			    render_node->g_set_buffer("light_map_buffer", lights_map);
			    render_node->g_set_buffer("light_indices_buffer", lights_indices);
                vk::DeviceSize offset = 0;
                render_node->get_curr_cmd().bindVertexBuffers(0, 1, &l_vertex_buffer->buffer_handle.get(), &offset);
                render_node->get_curr_cmd().bindIndexBuffer(l_index_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);

                render_node->get_curr_cmd().pushConstants(render_graph_ref->get_node(light_pass_name)->gpu_pipeline_ref->pipeline_layout.get(),
			                                              vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex,
			                                              0, sizeof(light_pc), &light_pc);
                render_node->get_curr_cmd().drawIndexed(quad_indices.size(), 1, 0,
			                                            0, 0);
            });

		render_graph_ref->get_node(light_pass_name)->add_pre_rendering_task(l_set_view_task);
		render_graph_ref->get_node(light_pass_name)->g_set_render_operation(l_render_op);
	}

	void reload_shaders() override
	{
		auto *g_render_node     = render_graph_ref->get_node(g_buffer_pass_name);
		auto *render_node      = render_graph_ref->get_node(light_pass_name);
		auto *c_render_node     = render_graph_ref->get_node(compute_pass_name);
		auto *mesh_c_render_node = render_graph_ref->get_node(mesh_cull_pass_name);

		g_render_node->g_recreate_resources();
		render_node->g_recreate_resources();
		c_render_node->g_recreate_resources();
		mesh_c_render_node->g_recreate_resources();
	}

	void create_resources()
	{
		auto image_info = engine::image::create_info2d(window_provider->get_window_size(), 1, 1,
		                                     vk::Format::eR32G32B32A32Sfloat,
		                                     vk::ImageUsageFlagBits::eColorAttachment |
		                                         vk::ImageUsageFlagBits::eSampled);

		auto storage_info = engine::image::create_info2d(window_provider->get_window_size(), 1, 1,
		                                       vk::Format::eR32G32B32A32Sfloat,
		                                       vk::ImageUsageFlagBits::eStorage |
		                                           vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

		auto depth_image_info = engine::image::create_info2d(window_provider->get_window_size(), 1, 1,
		                                          core->swapchain_ref->depth_format,
		                                          vk::ImageUsageFlagBits::eDepthStencilAttachment |
		                                              vk::ImageUsageFlagBits::eSampled);

		col_attachment_view      = resources_manager::get_instance()->get_image("colAttachment", image_info, 0, 0);
		norm_attachment_view     = resources_manager::get_instance()->get_image("normAttachment", image_info, 0, 0);
		tang_attachment_view     = resources_manager::get_instance()->get_image("tangAttachment", image_info, 0, 0);
		met_rough_attachment_view = resources_manager::get_instance()->get_image("metRoughnessAttachmentView", image_info, 0, 0);
		uv_attachment_view       = resources_manager::get_instance()->get_image("uvAttachmentView", image_info, 0, 0);

		depth_attachment_view = resources_manager::get_instance()->get_image("depthAttachment", depth_image_info, 0, 0);
		specular_holder      = resources_manager::get_instance()->get_image("specularHolderStorage", storage_info, 0, 0);

		// gbuff
		camera.set_perspective(
		    45.0f, (float) window_provider->get_window_size().x / (float) window_provider->get_window_size().y,
		    0.1f, 512.0f);
		debug_cam.set_perspective(
		    45.0f, (float) window_provider->get_window_size().x / (float) window_provider->get_window_size().y,
		    0.1f, 512.0f);

		curr_camera = &camera;

		camera.set_look_at(glm::vec3(0.0f, 0.0f, 1.0f));
		camera.position = glm::vec3(0.0f);

		std::string path = systems::os::get_instance()->get_assets_path();
		rendering_res_manager::get_instance()->push_model_to_indirect_batch(path + "\\Models\\porsche\\scene.gltf");

		// compute
		std::random_device rd;
		std::mt19937       gen(rd());

		point_lights.reserve(1);
		for (int i = 0; i < 1; ++i)
		{
			std::uniform_real_distribution<> distribution_pos(-10.0f, 10.0f);
			std::uniform_real_distribution<> distribution_col(0.0f, 1.0f);
			glm::vec3                        pos = glm::vec3(distribution_pos(gen), distribution_pos(gen), distribution_pos(gen));

			glm::vec3 col = glm::vec3(distribution_col(gen), distribution_col(gen), distribution_col(gen));

			std::uniform_real_distribution<> distribution_intensity(0.5f, 2.0f);
			float                            intensity = static_cast<float>(distribution_intensity(gen));

			std::uniform_real_distribution<> distribution_radius(0.5f, 10.0f);
			float                            radius = 2.0f;

			std::uniform_real_distribution<> distribution_attenuation(0.3f, 10.0f);
			float                            l_attenuation = 0.01f;
			float                            q_attenuation = static_cast<float>(distribution_attenuation(gen));

			point_lights.emplace_back(point_light{pos, col, radius, intensity, l_attenuation, 0.0f});
		}

		cull_data_pc.s_width           = window_provider->get_window_size().x;
		cull_data_pc.s_height          = window_provider->get_window_size().y;
		cull_data_pc.point_lights_count = 0;
		cull_data_pc.x_tile_count       = static_cast<uint32_t>((core->swapchain_ref->extent.width - 1) / x_tile_size_px + 1);
		cull_data_pc.y_tile_count       = static_cast<uint32_t>((core->swapchain_ref->extent.height - 1) / y_tile_size_px + 1);
		cull_data_pc.x_tile_size_px      = x_tile_size_px;
		cull_data_pc.y_tile_size_px      = y_tile_size_px;
		cull_data_pc.point_lights_count = (int) point_lights.size();

		lights_map.reserve(cull_data_pc.x_tile_count * cull_data_pc.y_tile_count * z_slices_size);

		for (int i = 0; i < lights_map.capacity(); ++i)
		{
			lights_map.emplace_back(array_indexer{});
		}

		lights_indices.reserve(lights_map.size() * point_lights.size());
		for (int i = 0; i < lights_map.size() * point_lights.size(); ++i)
		{
			lights_indices.emplace_back(-1);
		}

		// light
		quad_vert    = vertex_2d::get_quad_vertices();
		quad_indices = vertex_2d::get_quad_indices();

		c_props_ubo.inv_proj = glm::inverse(camera.matrices.perspective);
		c_props_ubo.inv_view = glm::inverse(camera.matrices.view);
		c_props_ubo.pos     = camera.position;
		c_props_ubo.z_near   = camera.camera_properties.z_near;
		c_props_ubo.z_far    = camera.camera_properties.z_far;

		light_pc.x_tile_count  = cull_data_pc.x_tile_count;
		light_pc.y_tile_count  = cull_data_pc.y_tile_count;
		light_pc.x_tile_size_px = x_tile_size_px;
		light_pc.y_tile_size_px = y_tile_size_px;
		light_pc.z_slices     = z_slices_size;
	}

	void create_buffers()
	{
		rendering_res_manager::get_instance()->build_indirect_buffers();

		l_vertex_buffer = resources_manager::get_instance()->get_buffer(resources_manager::buffer_params{
		    "lVertexBuffer", vk::BufferUsageFlagBits::eVertexBuffer,
		    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		    sizeof(vertex_2d) * quad_vert.size(), sizeof(vertex_2d),quad_vert.data()});

		l_index_buffer = resources_manager::get_instance()->get_buffer(resources_manager::buffer_params{
		    "lIndexBuffer", vk::BufferUsageFlagBits::eIndexBuffer,
		    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		    sizeof(uint32_t) * quad_indices.size(),sizeof(vertex_2d), quad_indices.data()});
	}

	void create_pipelines()
	{
		auto &logical_device = core->logical_device.get();

		// Cull meshes
		std::string shader_path = systems::os::get_instance()->get_shaders_path();

		cull_meshes_comp_shader = render_graph_ref->resources_manager->get_shader(shader_path + "\\spirvGlsl\\Compute\\meshCull.comp.spv", s_comp);

		auto *mesh_cull_render_node = render_graph_ref->add_pass(mesh_cull_pass_name);
		mesh_cull_render_node->g_set_comp_shader(cull_meshes_comp_shader);
		mesh_cull_render_node->build_render_graph_node();

		// Cull pass//

		cull_comp_shader = render_graph_ref->resources_manager->get_shader(shader_path + "\\spirvGlsl\\Compute\\lightCulling.comp.spv", s_comp);

		auto *cull_render_node = render_graph_ref->add_pass(compute_pass_name);
		cull_render_node->g_set_comp_shader(cull_comp_shader);
		cull_render_node->g_set_push_constant_size(sizeof(screen_data_pc));
		cull_render_node->build_render_graph_node();

		// gbuffer

		g_vert_shader = render_graph_ref->resources_manager->get_shader(shader_path + "\\spirvGlsl\\ClusterRendering\\gBuffer.vert.spv", s_vert);
		g_frag_shader = render_graph_ref->resources_manager->get_shader(shader_path + "\\spirvGlsl\\ClusterRendering\\gBuffer.frag.spv", s_frag);

		engine::vertex_input    vertex_layout = m_vertex_3d::get_vertex_input();
		engine::attachment_info col_info     = get_color_attachment_info(blend_configs::b_opaque,
            glm::vec4(0.0f), vk::Format::eR32G32B32A32Sfloat);
		engine::attachment_info depth_info  = get_depth_attachment_info();
		auto           render_node = render_graph_ref->add_pass(g_buffer_pass_name);

		render_node->g_set_vert_shader(g_vert_shader);
		render_node->g_set_frag_shader(g_frag_shader);
		render_node->g_set_framebuffer_size(window_provider->get_window_size());
		render_node->g_set_vertex_input(vertex_layout);
		render_node->g_set_push_constant_size(sizeof(mvp_pc));
		render_node->g_add_color_attachment_output(0, col_info);
		render_node->g_add_color_attachment_output(1, col_info);
		render_node->g_add_color_attachment_output(2, col_info);
		render_node->g_add_color_attachment_output(3, col_info);
		render_node->g_add_color_attachment_output(4, col_info);
		render_node->g_set_depth_attachment_output(depth_info);
		render_node->g_set_depth_config(depth_configs::d_enable);
		render_node->g_set_color_image_attachment_binding(0, col_attachment_view);
		render_node->g_set_color_image_attachment_binding(1, norm_attachment_view);
		render_node->g_set_color_image_attachment_binding(2, tang_attachment_view);
		render_node->g_set_color_image_attachment_binding(3, met_rough_attachment_view);
		render_node->g_set_color_image_attachment_binding(4, uv_attachment_view);
		render_node->g_set_depth_image_resource(depth_attachment_view);
		render_node->g_set_graphics_pipeline_configs({r_fill, t_triangle});
		render_node->add_buffer_sync({b_compute_write, b_draw_indirect});
		render_node->depends_on(mesh_cull_pass_name);
		render_node->build_render_graph_node();

		// light pass//

		l_vert_shader = render_graph_ref->resources_manager->get_shader(shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", s_vert);
		l_frag_shader = render_graph_ref->resources_manager->get_shader(shader_path + "\\spirvGlsl\\ClusterRendering\\light.frag.spv", s_frag);

		engine::attachment_info l_col_info = get_color_attachment_info(blend_configs::b_opaque,
		    glm::vec4(0.0f), core->swapchain_ref->get_format());

		engine::vertex_input l_vertex_input = vertex_2d::get_vertex_input();

		auto l_render_node = render_graph_ref->add_pass(light_pass_name);
		l_render_node->g_set_vert_shader(l_vert_shader);
		l_render_node->g_set_frag_shader(l_frag_shader);
		l_render_node->g_set_push_constant_size(sizeof(light_pc));
		l_render_node->g_set_framebuffer_size(window_provider->get_window_size());
		l_render_node->g_set_vertex_input(l_vertex_input);
		l_render_node->g_add_color_attachment_output(0, l_col_info);
		l_render_node->g_add_sampler_resource(col_attachment_view);
		l_render_node->g_add_sampler_resource(norm_attachment_view);
		l_render_node->g_add_sampler_resource(tang_attachment_view);
		l_render_node->g_add_sampler_resource(met_rough_attachment_view);
		l_render_node->g_add_sampler_resource(uv_attachment_view);
		l_render_node->g_add_sampler_resource(depth_attachment_view);
		l_render_node->g_add_storage_resource(specular_holder);
		l_render_node->depends_on(compute_pass_name);
		l_render_node->build_render_graph_node();
	}
	glm::vec4 u_get_plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		glm::vec3 v1 = p1 - p0;
		glm::vec3 v2 = p2 - p0;

		glm::vec3 norm;
		norm = normalize(cross(v1, v2));

		float d = dot(norm, p0);

		glm::vec4 plane = glm::vec4(norm, d);
		return plane;
	}

	glm::vec4 screen_to_view_ndc(glm::mat4 inv_proj, float depth, glm::vec2 ndc_coords)
	{
		glm::vec4 ndc_pos  = glm::vec4(ndc_coords, depth, 1.0);
		glm::vec4 view_pos = inv_proj * ndc_pos;
		view_pos           = view_pos / view_pos.w;
		return view_pos;
	}
	void build_frustum_planes()
	{
		glm::vec3 near_top_l    = screen_to_view_ndc(c_props_ubo.inv_proj, 0.0, glm::vec2(-1.0, 1.0));
		glm::vec3 near_top_r    = screen_to_view_ndc(c_props_ubo.inv_proj, 0.0, glm::vec2(1.0, 1.0));
		glm::vec3 near_bottom_l = screen_to_view_ndc(c_props_ubo.inv_proj, 0.0, glm::vec2(-1.0, -1.0));
		glm::vec3 near_bottom_r = screen_to_view_ndc(c_props_ubo.inv_proj, 0.0, glm::vec2(1.0, -1.0));

		glm::vec3 far_top_l    = screen_to_view_ndc(c_props_ubo.inv_proj, 1.0, glm::vec2(-1.0, 1.0));
		glm::vec3 far_top_r    = screen_to_view_ndc(c_props_ubo.inv_proj, 1.0, glm::vec2(1.0, 1.0));
		glm::vec3 far_bottom_l = screen_to_view_ndc(c_props_ubo.inv_proj, 1.0, glm::vec2(-1.0, -1.0));
		glm::vec3 far_bottom_r = screen_to_view_ndc(c_props_ubo.inv_proj, 1.0, glm::vec2(1.0, -1.0));

		cam_frustum.points[0] = near_top_l;
		cam_frustum.points[1] = near_top_r;
		cam_frustum.points[2] = near_bottom_l;
		cam_frustum.points[3] = near_bottom_r;

		cam_frustum.points[4] = far_top_l;
		cam_frustum.points[5] = far_top_r;
		cam_frustum.points[6] = far_bottom_l;
		cam_frustum.points[7] = far_bottom_r;

		// left
		cam_frustum.planes[0] = u_get_plane(near_bottom_l, far_bottom_l, near_top_l);

		// Right Plane
		cam_frustum.planes[1] = u_get_plane(near_top_r, far_top_r, near_bottom_r);

		// Top Plane
		cam_frustum.planes[2] = u_get_plane(near_top_l, far_top_l, near_top_r);

		// Bottom Plane
		cam_frustum.planes[3] = u_get_plane(near_bottom_r, far_bottom_r, near_bottom_l);

		// Near Plane
		cam_frustum.planes[4] = u_get_plane(near_top_l, near_bottom_r, near_bottom_l);

		// Far Plane
		cam_frustum.planes[5] = u_get_plane(far_top_l, far_bottom_l, far_bottom_r);
	}
	void move_cam()
	{
		glm::vec2 input = glm::vec2(0.0f);
		if (glfwGetKey(window_provider->window, GLFW_KEY_W))
		{
			input += glm::vec2(0.0f, 1.0f);
		}
		if (glfwGetKey(window_provider->window, GLFW_KEY_S))
		{
			input += glm::vec2(0.0f, -1.0f);
		}
		if (glfwGetKey(window_provider->window, GLFW_KEY_D))
		{
			input += glm::vec2(1.0f, 0.0f);
		}
		if (glfwGetKey(window_provider->window, GLFW_KEY_A))
		{
			input += glm::vec2(-1.0f, 0.0f);
		}
		if (glfwGetKey(window_provider->window, GLFW_KEY_LEFT_SHIFT))
		{
			camera.movement_speed = 40;
		}
		else
		{
			camera.movement_speed = 5;
		}
		input                = glm::clamp(input, glm::vec2(-1.0, -1.0), glm::vec2(1.0, 1.0));
		glm::vec2 mouse_input = glm::vec2(-ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		camera.mouse_input    = mouse_input;
		if (glfwGetMouseButton(window_provider->window, GLFW_MOUSE_BUTTON_2))
		{
			camera.rotate_camera();
			camera.move(delta_time, input);
		}
		else
		{
			camera.first_mouse = true;
		}
		camera.update_cam();
	};

	::window_provider *window_provider;
	engine::core           *core;
	engine::render_graph    *render_graph_ref;

	shader *g_vert_shader;
	shader *g_frag_shader;

	shader *l_vert_shader;
	shader *l_frag_shader;

	shader *cull_comp_shader;
	shader *cull_meshes_comp_shader;

	engine::image_view *col_attachment_view;
	engine::image_view *norm_attachment_view;
	engine::image_view *tang_attachment_view;
	engine::image_view *met_rough_attachment_view;
	engine::image_view *depth_attachment_view;
	engine::image_view *uv_attachment_view;
	engine::image_view *specular_holder;

	staged_buffer *vertex_buffer;
	staged_buffer *index_buffer;

	engine::buffer *l_vertex_buffer;
	engine::buffer *l_index_buffer;

	std::string g_buffer_pass_name  = "gBuffer";
	std::string compute_pass_name  = "cullLight";
	std::string light_pass_name    = "light";
	std::string mesh_cull_pass_name = "cullMesh";

	// gbuff
	rendering::camera  camera     = {glm::vec3(5.0f), rendering::camera::camera_mode::e_free};
	rendering::camera  debug_cam   = {glm::vec3(10.0f), rendering::camera::camera_mode::e_free};
	rendering::camera *curr_camera = nullptr;
	mvp_pc   pc{};

	// culling
	std::vector<point_light>   point_lights;
	std::vector<array_indexer> lights_map;
	std::vector<int32_t>      lights_indices;
	std::vector<sphere>       meshes_spheres_compact;
	screen_data_pc              cull_data_pc{};
	uint32_t                  x_tile_size_px = 512;
	uint32_t                  y_tile_size_px = 512;
	uint32_t                  z_slices_size = 4;
	uint32_t                  local_size   = 1;

	// light
	std::vector<vertex_2d> quad_vert;
	std::vector<uint32_t> quad_indices;
	c_props_ubo             c_props_ubo;
	light_pc               light_pc{};
	frustum               cam_frustum;
};
}        // namespace rendering

#endif        // CLUSTER_RENDERER_HPP
