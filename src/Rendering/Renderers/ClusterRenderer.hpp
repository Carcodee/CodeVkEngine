//

// Created by carlo on 2024-10-25.
//

#ifndef CLUSTER_RENDERER_HPP
#define CLUSTER_RENDERER_HPP

namespace rendering
{
using namespace engine;
class Cluster_Renderer : public Base_Renderer
{
  public:
	Cluster_Renderer(engine::Core *core, ::Window_Provider *window_provider)
	{
		this->core           = core;
		this->render_graph_ref = core->render_graph_ref;
		this->window_provider = window_provider;
		Create_Resources();
		Create_Buffers();
		Create_Pipelines();
	}

	void Recreate_Swap_Chain_Resources() override
	{
	}

	void Set_Render_Operation() override
	{
		render_graph_ref->Get_Node(mesh_cull_pass_name)->G_Set_Render_Operation(new std::function<void()>([this]() {
			auto &render_node = render_graph_ref->render_nodes.at(mesh_cull_pass_name);
			render_node->G_Set_Buffer("Indirect_Cmds_Buffer",
			                      Rendering_Res_Manager::Get_Instance()->indirect_draw_buffer);
			render_node->G_Set_Buffer("Meshes_Spheres_Buffer", meshes_spheres_compact);
			render_node->G_Set_Buffer("Cam_Props", c_props_ubo);
			render_node->G_Set_Buffer("Cull_Info_Uniform", cam_frustum);
			render_node->Get_Curr_Cmd().dispatch(Rendering_Res_Manager::Get_Instance()->indirect_draws_cmd_infos.size(), 1, 1);
		}));
		render_graph_ref->Get_Node(mesh_cull_pass_name)->Add_Pre_Rendering_Task(new std::function<void()>([this]() {
			Move_Cam();
			c_props_ubo.inv_proj = glm::inverse(curr_camera->matrices.perspective);
			c_props_ubo.inv_view = glm::inverse(curr_camera->matrices.view);
			c_props_ubo.pos     = curr_camera->position;
			c_props_ubo.z_near   = curr_camera->camera_properties.z_near;
			c_props_ubo.z_far    = curr_camera->camera_properties.z_far;
			Build_Frustum_Planes();

			meshes_spheres_compact.clear();
			std::vector<Sphere> meshes_spheres_compact2;
			for (auto &model : Rendering_Res_Manager::Get_Instance()->indirect_models_to_draw)
			{
				for (auto &sphere : model.second->meshes_spheres)
				{
					meshes_spheres_compact.emplace_back(sphere);
					meshes_spheres_compact2.emplace_back(sphere);
				}
			}
		}));

		auto cull_task = new std::function<void()>([this]() {
			cull_data_pc.s_width           = (int) window_provider->Get_Window_Size().x;
			cull_data_pc.s_height          = (int) window_provider->Get_Window_Size().y;
			cull_data_pc.point_lights_count = point_lights.size();
			cull_data_pc.x_tile_count       = static_cast<uint32_t>((core->swapchain_ref->extent.width - 1) / x_tile_size_px + 1);
			cull_data_pc.y_tile_count       = static_cast<uint32_t>((core->swapchain_ref->extent.height - 1) / y_tile_size_px + 1);

			lights_map.clear();
			for (int i = 0; i < cull_data_pc.x_tile_count * cull_data_pc.y_tile_count * z_slices_size; ++i)
			{
				lights_map.emplace_back(Array_Indexer{});
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
			    render_node->G_Set_Buffer("Point_Lights_Buffer", point_lights);
			    render_node->G_Set_Buffer("Light_Map_Buffer", lights_map);
			    render_node->G_Set_Buffer("Light_Indices_Buffer", lights_indices);
			    render_node->G_Set_Buffer("Camera_Properties", c_props_ubo);
			    render_node->Get_Curr_Cmd().pushConstants(render_graph_ref->Get_Node(compute_pass_name)->gpu_pipeline_ref->pipeline_layout.get(),
			                                           vk::ShaderStageFlagBits::eCompute,
			                                           0, sizeof(Screen_Data_Pc), &cull_data_pc);
			    render_node->Get_Curr_Cmd().dispatch(cull_data_pc.x_tile_count / local_size, cull_data_pc.y_tile_count / local_size,
			                                      z_slices_size);
		    });

		render_graph_ref->Get_Node(compute_pass_name)->Add_Pre_Rendering_Task(cull_task);
		render_graph_ref->Get_Node(compute_pass_name)->G_Set_Render_Operation(cull_render_op);

		auto render_op = new std::function<void()>(
		    [this]() {
			    vk::DeviceSize           offset = 0;
			    std::vector<engine::Image_View *> textures;
			    for (auto &image : Resources_Manager::Get_Instance()->image_shippers)
			    {
				    textures.emplace_back(image->image_view_handle.get());
			    }
			    std::vector<rendering::Material_Packed_Data> materials;
			    for (auto &mat : Rendering_Res_Manager::Get_Instance()->material_packed_data)
			    {
				    materials.emplace_back(*mat);
			    }
			    if (materials.empty())
			    {
				    materials.emplace_back(Material_Packed_Data());
			    }

			    std::vector<glm::mat4> model_mats;
			    std::vector<int>       mesh_mat_ids;
			    for (auto &model : Rendering_Res_Manager::Get_Instance()->models)
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
			    auto render_node = render_graph_ref->Get_Node(g_buffer_pass_name);

			    render_graph_ref->Get_Node(g_buffer_pass_name)->G_Set_Sampler_Array("textures", textures);
			    render_graph_ref->Get_Node(g_buffer_pass_name)->G_Set_Buffer("Materials_Packed_Buffer", materials);
			    render_graph_ref->Get_Node(g_buffer_pass_name)->G_Set_Buffer("Mesh_Materials_Ids", mesh_mat_ids);
			    render_graph_ref->Get_Node(g_buffer_pass_name)->G_Set_Buffer("Meshes_Model_Matrices", model_mats);

			    pc.proj_view = camera.matrices.perspective * camera.matrices.view;
			    render_node->Get_Curr_Cmd().pushConstants(render_graph_ref->Get_Node(g_buffer_pass_name)->gpu_pipeline_ref->pipeline_layout.get(),
			                                           vk::ShaderStageFlagBits::eVertex |
			                                               vk::ShaderStageFlagBits::eFragment,
			                                           0, sizeof(Mvp_Pc), &pc);

			    int mesh_offset = 0;

			    for (auto &model_pair : Rendering_Res_Manager::Get_Instance()->indirect_models_to_draw)
			    {
				    rendering::Model *model_ref = model_pair.second;
				    render_node->Get_Curr_Cmd().bindVertexBuffers(0, 1, &model_ref->vert_buffer->device_buffer->buffer_handle.get(),
				                                               &offset);
				    render_node->Get_Curr_Cmd().bindIndexBuffer(model_ref->index_buffer->Get_Buffer(), 0, vk::IndexType::eUint32);

				    vk::DeviceSize size_offset = (mesh_offset) * sizeof(Draw_Indirect_Indexed_Cmd);
				    uint32_t       stride     = sizeof(Draw_Indirect_Indexed_Cmd);
				    render_node->Get_Curr_Cmd().drawIndexedIndirect(
				        Rendering_Res_Manager::Get_Instance()->indirect_draw_buffer->buffer_handle.get(),
				        size_offset,
				        model_ref->mesh_count,
				        stride);
				    mesh_offset += model_ref->mesh_count;
			    }
		    });

		render_graph_ref->Get_Node(g_buffer_pass_name)->G_Set_Render_Operation(render_op);

		auto l_set_view_task = new std::function<void()>([this]() {
			auto render_node = render_graph_ref->Get_Node(light_pass_name);
			light_pc.x_tile_count  = cull_data_pc.x_tile_count;
			light_pc.y_tile_count  = cull_data_pc.y_tile_count;
			light_pc.x_tile_size_px = x_tile_size_px;
			light_pc.y_tile_size_px = y_tile_size_px;
			light_pc.z_slices     = z_slices_size;

			auto *curr_image = render_graph_ref->current_back_buffer;
			render_node->G_Set_Color_Image_Attachment_Binding(0, curr_image);
			render_node->G_Set_Framebuffer_Size(window_provider->Get_Window_Size());
		});
		auto l_render_op    = new std::function<void()>(
            [this]() {
                auto render_node = render_graph_ref->Get_Node(light_pass_name);
                render_graph_ref->resources_manager->Request_Storage_Image_Clear("specularHolderStorage");
			    render_node->G_Set_Sampler("g_col", col_attachment_view);
			    render_node->G_Set_Sampler("g_normals", norm_attachment_view);
			    render_node->G_Set_Sampler("g_tang", tang_attachment_view);
			    render_node->G_Set_Sampler("g_depth", depth_attachment_view);
			    render_node->G_Set_Sampler("g_met_roughness", met_rough_attachment_view);
			    render_node->G_Set_Sampler("g_mesh_uv", uv_attachment_view);

			    render_node->G_Set_Storage_Image("specular_holder", specular_holder);
			    render_node->G_Set_Buffer("Camera_Properties", c_props_ubo);
			    render_node->G_Set_Buffer("Point_Lights_Buffer", point_lights);
			    render_node->G_Set_Buffer("Light_Map_Buffer", lights_map);
			    render_node->G_Set_Buffer("Light_Indices_Buffer", lights_indices);
                vk::DeviceSize offset = 0;
                render_node->Get_Curr_Cmd().bindVertexBuffers(0, 1, &l_vertex_buffer->buffer_handle.get(), &offset);
                render_node->Get_Curr_Cmd().bindIndexBuffer(l_index_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);

                render_node->Get_Curr_Cmd().pushConstants(render_graph_ref->Get_Node(light_pass_name)->gpu_pipeline_ref->pipeline_layout.get(),
			                                              vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex,
			                                              0, sizeof(light_pc), &light_pc);
                render_node->Get_Curr_Cmd().drawIndexed(quad_indices.size(), 1, 0,
			                                            0, 0);
            });

		render_graph_ref->Get_Node(light_pass_name)->Add_Pre_Rendering_Task(l_set_view_task);
		render_graph_ref->Get_Node(light_pass_name)->G_Set_Render_Operation(l_render_op);
	}

	void Reload_Shaders() override
	{
		auto *g_render_node     = render_graph_ref->Get_Node(g_buffer_pass_name);
		auto *render_node      = render_graph_ref->Get_Node(light_pass_name);
		auto *c_render_node     = render_graph_ref->Get_Node(compute_pass_name);
		auto *mesh_c_render_node = render_graph_ref->Get_Node(mesh_cull_pass_name);

		g_render_node->G_Recreate_Resources();
		render_node->G_Recreate_Resources();
		c_render_node->G_Recreate_Resources();
		mesh_c_render_node->G_Recreate_Resources();
	}

	void Create_Resources()
	{
		auto image_info = engine::Image::Create_Info2d(window_provider->Get_Window_Size(), 1, 1,
		                                     vk::Format::eR32G32B32A32Sfloat,
		                                     vk::ImageUsageFlagBits::eColorAttachment |
		                                         vk::ImageUsageFlagBits::eSampled);

		auto storage_info = engine::Image::Create_Info2d(window_provider->Get_Window_Size(), 1, 1,
		                                       vk::Format::eR32G32B32A32Sfloat,
		                                       vk::ImageUsageFlagBits::eStorage |
		                                           vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

		auto depth_image_info = engine::Image::Create_Info2d(window_provider->Get_Window_Size(), 1, 1,
		                                          core->swapchain_ref->depth_format,
		                                          vk::ImageUsageFlagBits::eDepthStencilAttachment |
		                                              vk::ImageUsageFlagBits::eSampled);

		col_attachment_view      = Resources_Manager::Get_Instance()->Get_Image("colAttachment", image_info, 0, 0);
		norm_attachment_view     = Resources_Manager::Get_Instance()->Get_Image("normAttachment", image_info, 0, 0);
		tang_attachment_view     = Resources_Manager::Get_Instance()->Get_Image("tangAttachment", image_info, 0, 0);
		met_rough_attachment_view = Resources_Manager::Get_Instance()->Get_Image("metRoughnessAttachmentView", image_info, 0, 0);
		uv_attachment_view       = Resources_Manager::Get_Instance()->Get_Image("uvAttachmentView", image_info, 0, 0);

		depth_attachment_view = Resources_Manager::Get_Instance()->Get_Image("depthAttachment", depth_image_info, 0, 0);
		specular_holder      = Resources_Manager::Get_Instance()->Get_Image("specularHolderStorage", storage_info, 0, 0);

		// gbuff
		camera.Set_Perspective(
		    45.0f, (float) window_provider->Get_Window_Size().x / (float) window_provider->Get_Window_Size().y,
		    0.1f, 512.0f);
		debug_cam.Set_Perspective(
		    45.0f, (float) window_provider->Get_Window_Size().x / (float) window_provider->Get_Window_Size().y,
		    0.1f, 512.0f);

		curr_camera = &camera;

		camera.Set_Look_At(glm::vec3(0.0f, 0.0f, 1.0f));
		camera.position = glm::vec3(0.0f);

		std::string path = systems::Os::Get_Instance()->Get_Assets_Path();
		Rendering_Res_Manager::Get_Instance()->Push_Model_To_Indirect_Batch(path + "\\Models\\porsche\\scene.gltf");

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

			point_lights.emplace_back(Point_Light{pos, col, radius, intensity, l_attenuation, 0.0f});
		}

		cull_data_pc.s_width           = window_provider->Get_Window_Size().x;
		cull_data_pc.s_height          = window_provider->Get_Window_Size().y;
		cull_data_pc.point_lights_count = 0;
		cull_data_pc.x_tile_count       = static_cast<uint32_t>((core->swapchain_ref->extent.width - 1) / x_tile_size_px + 1);
		cull_data_pc.y_tile_count       = static_cast<uint32_t>((core->swapchain_ref->extent.height - 1) / y_tile_size_px + 1);
		cull_data_pc.x_tile_size_px      = x_tile_size_px;
		cull_data_pc.y_tile_size_px      = y_tile_size_px;
		cull_data_pc.point_lights_count = (int) point_lights.size();

		lights_map.reserve(cull_data_pc.x_tile_count * cull_data_pc.y_tile_count * z_slices_size);

		for (int i = 0; i < lights_map.capacity(); ++i)
		{
			lights_map.emplace_back(Array_Indexer{});
		}

		lights_indices.reserve(lights_map.size() * point_lights.size());
		for (int i = 0; i < lights_map.size() * point_lights.size(); ++i)
		{
			lights_indices.emplace_back(-1);
		}

		// light
		quad_vert    = Vertex_2d::Get_Quad_Vertices();
		quad_indices = Vertex_2d::Get_Quad_Indices();

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

	void Create_Buffers()
	{
		Rendering_Res_Manager::Get_Instance()->Build_Indirect_Buffers();

		l_vertex_buffer = Resources_Manager::Get_Instance()->Get_Buffer(Resources_Manager::Buffer_Params{
		    "lVertexBuffer", vk::BufferUsageFlagBits::eVertexBuffer,
		    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		    sizeof(Vertex_2d) * quad_vert.size(), sizeof(Vertex_2d),quad_vert.data()});

		l_index_buffer = Resources_Manager::Get_Instance()->Get_Buffer(Resources_Manager::Buffer_Params{
		    "lIndexBuffer", vk::BufferUsageFlagBits::eIndexBuffer,
		    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		    sizeof(uint32_t) * quad_indices.size(),sizeof(Vertex_2d), quad_indices.data()});
	}

	void Create_Pipelines()
	{
		auto &logical_device = core->logical_device.get();

		// Cull meshes
		std::string shader_path = systems::Os::Get_Instance()->Get_Shaders_Path();

		cull_meshes_comp_shader = render_graph_ref->resources_manager->Get_Shader(shader_path + "\\spirvGlsl\\Compute\\meshCull.comp.spv", S_Comp);

		auto *mesh_cull_render_node = render_graph_ref->Add_Pass(mesh_cull_pass_name);
		mesh_cull_render_node->G_Set_Comp_Shader(cull_meshes_comp_shader);
		mesh_cull_render_node->Build_Render_Graph_Node();

		// Cull pass//

		cull_comp_shader = render_graph_ref->resources_manager->Get_Shader(shader_path + "\\spirvGlsl\\Compute\\lightCulling.comp.spv", S_Comp);

		auto *cull_render_node = render_graph_ref->Add_Pass(compute_pass_name);
		cull_render_node->G_Set_Comp_Shader(cull_comp_shader);
		cull_render_node->G_Set_Push_Constant_Size(sizeof(Screen_Data_Pc));
		cull_render_node->Build_Render_Graph_Node();

		// gbuffer

		g_vert_shader = render_graph_ref->resources_manager->Get_Shader(shader_path + "\\spirvGlsl\\ClusterRendering\\gBuffer.vert.spv", S_Vert);
		g_frag_shader = render_graph_ref->resources_manager->Get_Shader(shader_path + "\\spirvGlsl\\ClusterRendering\\gBuffer.frag.spv", S_Frag);

		engine::Vertex_Input    vertex_layout = M_Vertex_3d::Get_Vertex_Input();
		engine::Attachment_Info col_info     = Get_Color_Attachment_Info(Blend_Configs::B_Opaque,
            glm::vec4(0.0f), vk::Format::eR32G32B32A32Sfloat);
		engine::Attachment_Info depth_info  = Get_Depth_Attachment_Info();
		auto           render_node = render_graph_ref->Add_Pass(g_buffer_pass_name);

		render_node->G_Set_Vert_Shader(g_vert_shader);
		render_node->G_Set_Frag_Shader(g_frag_shader);
		render_node->G_Set_Framebuffer_Size(window_provider->Get_Window_Size());
		render_node->G_Set_Vertex_Input(vertex_layout);
		render_node->G_Set_Push_Constant_Size(sizeof(Mvp_Pc));
		render_node->G_Add_Color_Attachment_Output(0, col_info);
		render_node->G_Add_Color_Attachment_Output(1, col_info);
		render_node->G_Add_Color_Attachment_Output(2, col_info);
		render_node->G_Add_Color_Attachment_Output(3, col_info);
		render_node->G_Add_Color_Attachment_Output(4, col_info);
		render_node->G_Set_Depth_Attachment_Output(depth_info);
		render_node->G_Set_Depth_Config(Depth_Configs::D_Enable);
		render_node->G_Set_Color_Image_Attachment_Binding(0, col_attachment_view);
		render_node->G_Set_Color_Image_Attachment_Binding(1, norm_attachment_view);
		render_node->G_Set_Color_Image_Attachment_Binding(2, tang_attachment_view);
		render_node->G_Set_Color_Image_Attachment_Binding(3, met_rough_attachment_view);
		render_node->G_Set_Color_Image_Attachment_Binding(4, uv_attachment_view);
		render_node->G_Set_Depth_Image_Resource(depth_attachment_view);
		render_node->G_Set_Graphics_Pipeline_Configs({R_Fill, T_Triangle});
		render_node->Add_Buffer_Sync({B_Compute_Write, B_Draw_Indirect});
		render_node->Depends_On(mesh_cull_pass_name);
		render_node->Build_Render_Graph_Node();

		// light pass//

		l_vert_shader = render_graph_ref->resources_manager->Get_Shader(shader_path + "\\spirvGlsl\\Common\\Quad.vert.spv", S_Vert);
		l_frag_shader = render_graph_ref->resources_manager->Get_Shader(shader_path + "\\spirvGlsl\\ClusterRendering\\light.frag.spv", S_Frag);

		engine::Attachment_Info l_col_info = Get_Color_Attachment_Info(Blend_Configs::B_Opaque,
		    glm::vec4(0.0f), core->swapchain_ref->Get_Format());

		engine::Vertex_Input l_vertex_input = Vertex_2d::Get_Vertex_Input();

		auto l_render_node = render_graph_ref->Add_Pass(light_pass_name);
		l_render_node->G_Set_Vert_Shader(l_vert_shader);
		l_render_node->G_Set_Frag_Shader(l_frag_shader);
		l_render_node->G_Set_Push_Constant_Size(sizeof(light_pc));
		l_render_node->G_Set_Framebuffer_Size(window_provider->Get_Window_Size());
		l_render_node->G_Set_Vertex_Input(l_vertex_input);
		l_render_node->G_Add_Color_Attachment_Output(0, l_col_info);
		l_render_node->G_Add_Sampler_Resource(col_attachment_view);
		l_render_node->G_Add_Sampler_Resource(norm_attachment_view);
		l_render_node->G_Add_Sampler_Resource(tang_attachment_view);
		l_render_node->G_Add_Sampler_Resource(met_rough_attachment_view);
		l_render_node->G_Add_Sampler_Resource(uv_attachment_view);
		l_render_node->G_Add_Sampler_Resource(depth_attachment_view);
		l_render_node->G_Add_Storage_Resource(specular_holder);
		l_render_node->Depends_On(compute_pass_name);
		l_render_node->Build_Render_Graph_Node();
	}
	glm::vec4 U_Get_Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		glm::vec3 v1 = p1 - p0;
		glm::vec3 v2 = p2 - p0;

		glm::vec3 norm;
		norm = normalize(cross(v1, v2));

		float d = dot(norm, p0);

		glm::vec4 plane = glm::vec4(norm, d);
		return plane;
	}

	glm::vec4 Screen_To_View_Ndc(glm::mat4 inv_proj, float depth, glm::vec2 ndc_coords)
	{
		glm::vec4 ndc_pos  = glm::vec4(ndc_coords, depth, 1.0);
		glm::vec4 view_pos = inv_proj * ndc_pos;
		view_pos           = view_pos / view_pos.w;
		return view_pos;
	}
	void Build_Frustum_Planes()
	{
		glm::vec3 near_top_l    = Screen_To_View_Ndc(c_props_ubo.inv_proj, 0.0, glm::vec2(-1.0, 1.0));
		glm::vec3 near_top_r    = Screen_To_View_Ndc(c_props_ubo.inv_proj, 0.0, glm::vec2(1.0, 1.0));
		glm::vec3 near_bottom_l = Screen_To_View_Ndc(c_props_ubo.inv_proj, 0.0, glm::vec2(-1.0, -1.0));
		glm::vec3 near_bottom_r = Screen_To_View_Ndc(c_props_ubo.inv_proj, 0.0, glm::vec2(1.0, -1.0));

		glm::vec3 far_top_l    = Screen_To_View_Ndc(c_props_ubo.inv_proj, 1.0, glm::vec2(-1.0, 1.0));
		glm::vec3 far_top_r    = Screen_To_View_Ndc(c_props_ubo.inv_proj, 1.0, glm::vec2(1.0, 1.0));
		glm::vec3 far_bottom_l = Screen_To_View_Ndc(c_props_ubo.inv_proj, 1.0, glm::vec2(-1.0, -1.0));
		glm::vec3 far_bottom_r = Screen_To_View_Ndc(c_props_ubo.inv_proj, 1.0, glm::vec2(1.0, -1.0));

		cam_frustum.points[0] = near_top_l;
		cam_frustum.points[1] = near_top_r;
		cam_frustum.points[2] = near_bottom_l;
		cam_frustum.points[3] = near_bottom_r;

		cam_frustum.points[4] = far_top_l;
		cam_frustum.points[5] = far_top_r;
		cam_frustum.points[6] = far_bottom_l;
		cam_frustum.points[7] = far_bottom_r;

		// left
		cam_frustum.planes[0] = U_Get_Plane(near_bottom_l, far_bottom_l, near_top_l);

		// Right Plane
		cam_frustum.planes[1] = U_Get_Plane(near_top_r, far_top_r, near_bottom_r);

		// Top Plane
		cam_frustum.planes[2] = U_Get_Plane(near_top_l, far_top_l, near_top_r);

		// Bottom Plane
		cam_frustum.planes[3] = U_Get_Plane(near_bottom_r, far_bottom_r, near_bottom_l);

		// Near Plane
		cam_frustum.planes[4] = U_Get_Plane(near_top_l, near_bottom_r, near_bottom_l);

		// Far Plane
		cam_frustum.planes[5] = U_Get_Plane(far_top_l, far_bottom_l, far_bottom_r);
	}
	void Move_Cam()
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
			camera.Rotate_Camera();
			camera.Move(delta_time, input);
		}
		else
		{
			camera.first_mouse = true;
		}
		camera.Update_Cam();
	};

	::Window_Provider *window_provider;
	engine::Core           *core;
	engine::Render_Graph    *render_graph_ref;

	Shader *g_vert_shader;
	Shader *g_frag_shader;

	Shader *l_vert_shader;
	Shader *l_frag_shader;

	Shader *cull_comp_shader;
	Shader *cull_meshes_comp_shader;

	engine::Image_View *col_attachment_view;
	engine::Image_View *norm_attachment_view;
	engine::Image_View *tang_attachment_view;
	engine::Image_View *met_rough_attachment_view;
	engine::Image_View *depth_attachment_view;
	engine::Image_View *uv_attachment_view;
	engine::Image_View *specular_holder;

	Staged_Buffer *vertex_buffer;
	Staged_Buffer *index_buffer;

	engine::Buffer *l_vertex_buffer;
	engine::Buffer *l_index_buffer;

	std::string g_buffer_pass_name  = "gBuffer";
	std::string compute_pass_name  = "cullLight";
	std::string light_pass_name    = "light";
	std::string mesh_cull_pass_name = "cullMesh";

	// gbuff
	rendering::Camera  camera     = {glm::vec3(5.0f), rendering::Camera::Camera_Mode::E_Free};
	rendering::Camera  debug_cam   = {glm::vec3(10.0f), rendering::Camera::Camera_Mode::E_Free};
	rendering::Camera *curr_camera = nullptr;
	Mvp_Pc   pc{};

	// culling
	std::vector<Point_Light>   point_lights;
	std::vector<Array_Indexer> lights_map;
	std::vector<int32_t>      lights_indices;
	std::vector<Sphere>       meshes_spheres_compact;
	Screen_Data_Pc              cull_data_pc{};
	uint32_t                  x_tile_size_px = 512;
	uint32_t                  y_tile_size_px = 512;
	uint32_t                  z_slices_size = 4;
	uint32_t                  local_size   = 1;

	// light
	std::vector<Vertex_2d> quad_vert;
	std::vector<uint32_t> quad_indices;
	C_Props_Ubo             c_props_ubo;
	Light_Pc               light_pc{};
	Frustum               cam_frustum;
};
}        // namespace rendering

#endif        // CLUSTER_RENDERER_HPP
