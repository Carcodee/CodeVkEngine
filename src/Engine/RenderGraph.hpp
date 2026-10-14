//

// Created by carlo on 2024-10-02.
//

#ifndef RENDER_GRAPH_HPP
#define RENDER_GRAPH_HPP
#define DUMMY_PC_SIZE 4

namespace engine
{
struct buffer_key
{
	buffer_usage_types src_usage;
	buffer_usage_types dst_usage;
	engine::buffer          *buffer;
	std::string      name;
};
enum class render_node_type
{
	none,
	graphics,
	compute,
	cuda
};

struct render_node_configs : systems::i_serializable<render_node_configs>
{
	bool automatic_cache = true;
	bool manual_add      = false;

	render_node_configs(bool automatic_cache, bool manual_add = false)
	{
		this->automatic_cache = automatic_cache;
		this->manual_add      = manual_add;
	}

	nlohmann::json serialize() override
	{
		nlohmann::json json;
		json["automaticCache"] = automatic_cache;
		json["manualAdd"]      = manual_add;

		return json;
	}
	render_node_configs *deserialize(std::string filename) override
	{
		return this;
	}
};

class render_graph;

struct cuda_pipeline
{
	CodeCuda::CodeCudaContext *context = nullptr;
	engine::core                      *core    = nullptr;
	std::string                name    = "";

	cuda_pipeline() = default;
	engine::cuda_pipeline *c_build_cuda_pipeline()
	{
		assert(context != nullptr);
		return this;
	}
	engine::cuda_pipeline *c_export_buffer(engine::buffer *buffer)
	{
		assert(context);
		assert(buffer->get_buffer_handle());
		context->C_ImportExternalBuffer(buffer->get_buffer_handle(), buffer->device_size, buffer->stride);
		return this;
	}
	engine::cuda_pipeline *c_create_pipeline()
	{
		return this;
	}
	engine::cuda_pipeline *c_set_kernel_function(std::function<void(CodeCuda::CodeCudaContext*)> funct)
	{
		context->C_SetKernelLauncherFunction(funct);
		return this;
	}
	engine::cuda_pipeline *c_set_pre_cpu_kernel_function(std::function<void()> funct)
	{
		context->C_SetPreKernelFunction(funct);
		return this;
	}
	~cuda_pipeline() = default;
};
struct gpu_pipeline
{
	gpu_pipeline()  = default;
	~gpu_pipeline() = default;

	engine::core                        *core;
	vk::UniquePipeline           pipeline;
	vk::UniquePipelineLayout     pipeline_layout;
	vk::UniquePipelineCache      pipeline_cache;
	vk::PipelineLayoutCreateInfo pipeline_layout_ci;
	vk::PushConstantRange        push_constant_range;
	vk::PipelineBindPoint        pipeline_type;

	dynamic_render_pass                dynamic_render_pass;
	std::unique_ptr<descriptor_cache> desc_cache;

	engine::graphics_pipeline_configs                                   pipeline_configs = {};
	engine::vertex_input                                               vertex_layout;
	engine::attachment_info                                            depth_attachment      = {};
	engine::attachment_info                                            depth_attachment_input = {};
	size_t                                                    push_constant_size     = DUMMY_PC_SIZE;
	depth_configs                                              depth_config          = d_none;
	render_node_configs                                         configs              = {true, false};
	std::unordered_map<int, engine::attachment_info>           out_col_attachments_bindings;
	std::string                                               name;

	std::map<std::string, shader *> shaders = {
	    {"frag", nullptr},
	    {"vert", nullptr},
	    {"comp", nullptr},
	    {"tesc", nullptr},
	    {"tese", nullptr},
	    {"geom", nullptr}};

	engine::gpu_pipeline *g_recreate_resources()
	{
		assert(&pipeline_layout_ci != nullptr && "Pipeline layout is null");
		assert(core != nullptr && "Core is null");

		pipeline.reset();
		pipeline_layout.reset();

		g_reload_shaders();

		shader *vert_shader = shaders.at("vert");
		shader *frag_shader = shaders.at("frag");
		shader *comp_shader = shaders.at("comp");
		shader *tesc_shader = shaders.at("tesc");
		shader *tese_shader = shaders.at("tese");
		shader *geom_shader = shaders.at("geom");

		std::map<shader_stage, shader *> stages;
		g_get_shader_stages(stages);

		if (frag_shader && vert_shader)
		{
			if (configs.automatic_cache)
			{
				desc_cache->add_shader_info(vert_shader->s_parser.get());
				desc_cache->add_shader_info(frag_shader->s_parser.get());

				vk::ShaderStageFlags stage_flags =
				    vk::ShaderStageFlagBits::eFragment |
				    vk::ShaderStageFlagBits::eVertex;

				if (tesc_shader)
				{
					desc_cache->add_shader_info(tesc_shader->s_parser.get());

					if (!tese_shader)
					{
						assert(false && "Must have tese shader");
					}

					desc_cache->add_shader_info(tese_shader->s_parser.get());

					stage_flags |=
					    vk::ShaderStageFlagBits::eTessellationControl |
					    vk::ShaderStageFlagBits::eTessellationEvaluation;
				}

				if (geom_shader)
				{
					desc_cache->add_shader_info(geom_shader->s_parser.get());
					stage_flags |= vk::ShaderStageFlagBits::eGeometry;
				}

				desc_cache->build_descriptors_cache(stage_flags);

				if (push_constant_size <= 0)
				{
					g_set_push_constant_size(DUMMY_PC_SIZE);
				}

				auto painting_push_constant_ranges =
				    vk::PushConstantRange()
				        .setOffset(0)
				        .setStageFlags(
				            vk::ShaderStageFlagBits::eVertex |
				            vk::ShaderStageFlagBits::eFragment)
				        .setSize(push_constant_size);

				auto painting_layout_create_info =
				    vk::PipelineLayoutCreateInfo()
				        .setSetLayoutCount(1)
				        .setPushConstantRanges(painting_push_constant_ranges)
				        .setPSetLayouts(&desc_cache->dst_layout.get());

				g_set_pipeline_layout_ci(painting_layout_create_info);
			}
			std::vector<engine::attachment_info> col_attachments;
			col_attachments.resize(out_col_attachments_bindings.size());
			for (auto col_info : out_col_attachments_bindings)
			{
				col_attachments[col_info.first] = col_info.second;
				
			}

			std::vector<vk::Format> color_formats;
			color_formats.reserve(col_attachments.size());

			std::vector<vk::RenderingAttachmentInfo> rendering_attachment_infos;

			for (auto &col_attachment : col_attachments)
			{
				color_formats.push_back(col_attachment.format);
				rendering_attachment_infos.push_back(col_attachment.vk_attachment_info);
			}

			dynamic_render_pass.set_pipeline_rendering_info(
			    col_attachments.size(),
			    color_formats,
			    depth_attachment.format);

			pipeline_layout =
			    core->logical_device->createPipelineLayoutUnique(pipeline_layout_ci);
			
			std::vector<blend_configs> blend_configs = {};
			blend_configs.resize(out_col_attachments_bindings.size());
			for (auto col_info : out_col_attachments_bindings)
			{
				blend_configs[col_info.first] = col_info.second.blend_configs;
			}

			std::unique_ptr<engine::graphics_pipeline> graphics_pipeline =
			    std::make_unique<engine::graphics_pipeline>(
			        core->logical_device.get(),
			        stages,
			        pipeline_layout.get(),
			        dynamic_render_pass.pipeline_rendering_create_info,
			        pipeline_configs,
			        blend_configs,
			        depth_config,
			        vertex_layout,
			        pipeline_cache.get());

			pipeline     = std::move(graphics_pipeline->pipeline_handle);
			pipeline_type = vk::PipelineBindPoint::eGraphics;
		}
		else if (comp_shader)
		{
			if (configs.automatic_cache)
			{
				desc_cache->add_shader_info(comp_shader->s_parser.get());

				desc_cache->build_descriptors_cache(
				    vk::ShaderStageFlagBits::eCompute);

				if (push_constant_size <= 0)
				{
					g_set_push_constant_size(DUMMY_PC_SIZE);
				}

				auto painting_push_constant_ranges =
				    vk::PushConstantRange()
				        .setOffset(0)
				        .setStageFlags(vk::ShaderStageFlagBits::eCompute)
				        .setSize(push_constant_size);

				auto painting_layout_create_info =
				    vk::PipelineLayoutCreateInfo()
				        .setSetLayoutCount(1)
				        .setPushConstantRanges(painting_push_constant_ranges)
				        .setPSetLayouts(&desc_cache->dst_layout.get());

				g_set_pipeline_layout_ci(painting_layout_create_info);
			}

			pipeline_layout =
			    core->logical_device->createPipelineLayoutUnique(pipeline_layout_ci);

			std::unique_ptr<engine::compute_pipeline> compute_pipeline =
			    std::make_unique<engine::compute_pipeline>(
			        core->logical_device.get(),
			        comp_shader->s_module->shader_module_handle.get(),
			        pipeline_layout.get(),
			        pipeline_cache.get());

			pipeline     = std::move(compute_pipeline->pipeline_handle);
			pipeline_type = vk::PipelineBindPoint::eCompute;
		}
		else
		{
			std::cout << "Not viable shader configuration set\n";
		}
		return this;
	}

	engine::gpu_pipeline *g_build_gpu_pipeline()
	{
		assert(&pipeline_layout_ci != nullptr && "Pipeline layout is null");
		assert(core != nullptr && "Core is null");

		auto pipeline_cache_create_info = vk::PipelineCacheCreateInfo();

		pipeline_cache =
		    core->logical_device->createPipelineCacheUnique(
		        pipeline_cache_create_info);

		shader *vert_shader = shaders.at("vert");
		shader *frag_shader = shaders.at("frag");
		shader *comp_shader = shaders.at("comp");
		shader *tesc_shader = shaders.at("tesc");
		shader *tese_shader = shaders.at("tese");
		shader *geom_shader = shaders.at("geom");

		if (frag_shader && vert_shader)
		{
			if (configs.automatic_cache)
			{
				desc_cache.reset();
				desc_cache = std::make_unique<descriptor_cache>(core);

				desc_cache->add_shader_info(vert_shader->s_parser.get());
				desc_cache->add_shader_info(frag_shader->s_parser.get());

				vk::ShaderStageFlags stage_flags =
				    vk::ShaderStageFlagBits::eFragment |
				    vk::ShaderStageFlagBits::eVertex;

				if (tesc_shader)
				{
					desc_cache->add_shader_info(tesc_shader->s_parser.get());

					if (!tese_shader)
					{
						assert(false && "Must have tese shader");
					}

					desc_cache->add_shader_info(tese_shader->s_parser.get());

					stage_flags |=
					    vk::ShaderStageFlagBits::eTessellationControl |
					    vk::ShaderStageFlagBits::eTessellationEvaluation;
				}

				if (geom_shader)
				{
					desc_cache->add_shader_info(geom_shader->s_parser.get());
					stage_flags |= vk::ShaderStageFlagBits::eGeometry;
				}

				desc_cache->build_descriptors_cache(stage_flags);

				if (push_constant_size <= 0)
				{
					g_set_push_constant_size(DUMMY_PC_SIZE);
				}

				auto painting_push_constant_ranges =
				    vk::PushConstantRange()
				        .setOffset(0)
				        .setStageFlags(
				            vk::ShaderStageFlagBits::eFragment |
				            vk::ShaderStageFlagBits::eVertex)
				        .setSize(push_constant_size);

				auto painting_layout_create_info =
				    vk::PipelineLayoutCreateInfo()
				        .setSetLayoutCount(1)
				        .setPushConstantRanges(painting_push_constant_ranges)
				        .setPSetLayouts(&desc_cache->dst_layout.get());

				g_set_pipeline_layout_ci(painting_layout_create_info);
			}

			std::vector<engine::attachment_info> col_attachments;
			col_attachments.resize(out_col_attachments_bindings.size());
			for (auto col_info : out_col_attachments_bindings)
			{
				col_attachments[col_info.first] = col_info.second;
				
			}
			
			std::vector<vk::Format> color_formats;
			color_formats.reserve(col_attachments.size());

			std::vector<vk::RenderingAttachmentInfo> rendering_attachment_infos;

			for (auto &col_attachment : col_attachments)
			{
				color_formats.push_back(col_attachment.format);
				rendering_attachment_infos.push_back(col_attachment.vk_attachment_info);
			}

			dynamic_render_pass.set_pipeline_rendering_info(
			    col_attachments.size(),
			    color_formats,
			    depth_attachment.format);

			pipeline_layout =
			    core->logical_device->createPipelineLayoutUnique(pipeline_layout_ci);

			std::map<shader_stage, shader *> stages;
			g_get_shader_stages(stages);
			
			std::vector<blend_configs> blend_configs = {};
			blend_configs.resize(out_col_attachments_bindings.size());
			for (auto col_info : out_col_attachments_bindings)
			{
				blend_configs[col_info.first] = col_info.second.blend_configs;
			}
			std::unique_ptr<engine::graphics_pipeline> graphics_pipeline =
			    std::make_unique<engine::graphics_pipeline>(
			        core->logical_device.get(),
			        stages,
			        pipeline_layout.get(),
			        dynamic_render_pass.pipeline_rendering_create_info,
			        pipeline_configs,
			        blend_configs,
			        depth_config,
			        vertex_layout,
			        pipeline_cache.get());

			pipeline     = std::move(graphics_pipeline->pipeline_handle);
			pipeline_type = vk::PipelineBindPoint::eGraphics;

			std::cout << "Graphics pipeline created\n";
		}
		else if (comp_shader)
		{
			if (configs.automatic_cache)
			{
				desc_cache.reset();
				desc_cache = std::make_unique<descriptor_cache>(core);

				desc_cache->add_shader_info(comp_shader->s_parser.get());

				desc_cache->build_descriptors_cache(
				    vk::ShaderStageFlagBits::eCompute);

				if (push_constant_size <= 0)
				{
					g_set_push_constant_size(DUMMY_PC_SIZE);
				}

				auto painting_push_constant_ranges =
				    vk::PushConstantRange()
				        .setOffset(0)
				        .setStageFlags(vk::ShaderStageFlagBits::eCompute)
				        .setSize(push_constant_size);

				auto painting_layout_create_info =
				    vk::PipelineLayoutCreateInfo()
				        .setSetLayoutCount(1)
				        .setPushConstantRanges(painting_push_constant_ranges)
				        .setPSetLayouts(&desc_cache->dst_layout.get());

				g_set_pipeline_layout_ci(painting_layout_create_info);
			}

			pipeline_layout =
			    core->logical_device->createPipelineLayoutUnique(pipeline_layout_ci);

			std::unique_ptr<engine::compute_pipeline> compute_pipeline =
			    std::make_unique<engine::compute_pipeline>(
			        core->logical_device.get(),
			        comp_shader->s_module->shader_module_handle.get(),
			        pipeline_layout.get(),
			        pipeline_cache.get());

			pipeline     = std::move(compute_pipeline->pipeline_handle);
			pipeline_type = vk::PipelineBindPoint::eCompute;

			std::cout << "Compute pipeline created\n";
		}
		else
		{
			std::cout << "No compute or graphics shaders were set\n";
		}
		return this;
	}

	engine::gpu_pipeline *g_set_sampler(std::string name, engine::image_view *image_view_handle, engine::sampler *sampler = nullptr)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->set_sampler(name, image_view_handle, sampler);
		return this;
	}
	engine::gpu_pipeline *g_set_sampler_array(std::string name, std::vector<engine::image_view *> &image_views, std::vector<engine::sampler *> *samplers = nullptr)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->set_sampler_array(name, image_views, samplers);
		return this;
	}
	engine::gpu_pipeline *g_set_storage_image(std::string name, engine::image_view *image_view_handle, engine::sampler *sampler = nullptr)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->set_storage_image(name, image_view_handle, sampler);
		return this;
	}
	engine::gpu_pipeline *g_set_storage_image_array(std::string name, std::vector<engine::image_view *> &image_views,
	                                  std::vector<engine::sampler *> *samplers = nullptr)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->set_storage_image_array(name, image_views, samplers);
		return this;
	}
	template <typename value_type>
	engine::gpu_pipeline *g_set_buffer(std::string name, value_type &buffer_data)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->set_buffer<value_type>(name, buffer_data);
		return this;
	}
	template <typename value_type>
	engine::gpu_pipeline *g_set_buffer(std::string name, std::vector<value_type> &buffer_data)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->set_buffer<value_type>(name, buffer_data);
		return this;
	}
	engine::gpu_pipeline *g_set_buffer(std::string name, engine::buffer *buffer_data)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->set_buffer(name, buffer_data);
		return this;
	}

	engine::gpu_pipeline *g_set_pipeline_layout_ci(vk::PipelineLayoutCreateInfo create_info)
	{
		this->pipeline_layout_ci = create_info;

		if (create_info.pPushConstantRanges != nullptr)
		{
			this->push_constant_range.offset =
			    create_info.pPushConstantRanges->offset;

			this->push_constant_range.size =
			    create_info.pPushConstantRanges->size;

			this->push_constant_range.stageFlags =
			    create_info.pPushConstantRanges->stageFlags;

			this->pipeline_layout_ci.setPushConstantRanges(
			    this->push_constant_range);
		}
		return this;
	}

	engine::gpu_pipeline *g_set_push_constant_size(size_t size)
	{
		push_constant_size = size;
		return this;
	}

	engine::gpu_pipeline *g_set_graphics_pipeline_configs(
	    engine::graphics_pipeline_configs pipeline_configs)
	{
		this->pipeline_configs = pipeline_configs;
		return this;
	}

	engine::gpu_pipeline *g_set_depth_config(depth_configs depht_config)
	{
		this->depth_config = depht_config;
		return this;
	}

	engine::gpu_pipeline *g_set_configs(render_node_configs configs)
	{
		this->configs = configs;
		return this;
	}
	engine::gpu_pipeline *g_set_vertex_input(engine::vertex_input vertex_layout)
	{
		this->vertex_layout = vertex_layout;
		return this;
	}

	engine::gpu_pipeline *g_set_vert_shader(shader *shader)
	{
		shaders.at("vert") = shader;
		return this;
	}

	engine::gpu_pipeline *g_set_frag_shader(shader *shader)
	{
		shaders.at("frag") = shader;
		return this;
	}

	engine::gpu_pipeline *g_set_comp_shader(shader *shader)
	{
		shaders.at("comp") = shader;
		return this;
	}

	engine::gpu_pipeline *g_set_tes_control_shader(shader *shader)
	{
		shaders.at("tesc") = shader;
		return this;
	}

	engine::gpu_pipeline *g_set_tes_eval_shader(shader *shader)
	{
		shaders.at("tese") = shader;
		return this;
	}

	engine::gpu_pipeline *g_set_geom_shader(shader *shader)
	{
		shaders.at("geom") = shader;
		return this;
	}

	bool g_reload_shaders()
	{
		shader *vert_shader = shaders.at("vert");
		shader *frag_shader = shaders.at("frag");
		shader *comp_shader = shaders.at("comp");
		shader *tesc_shader = shaders.at("tesc");
		shader *tese_shader = shaders.at("tese");
		shader *geom_shader = shaders.at("geom");

		// todo
		bool is_reloaded = false;
		if (vert_shader != nullptr && frag_shader != nullptr)
		{
			vert_shader->reload();
			frag_shader->reload();
		}

		if (tesc_shader != nullptr)
		{
			tesc_shader->reload();
		}

		if (tese_shader != nullptr)
		{
			tese_shader->reload();
		}

		if (geom_shader != nullptr)
		{
			geom_shader->reload();
		}

		if (comp_shader != nullptr)
		{
			comp_shader->reload();
		}
		return is_reloaded;
	}

	engine::gpu_pipeline *g_get_shader_stages(std::map<shader_stage, shader *> &stages)
	{
		shader *vert_shader = shaders.at("vert");
		shader *frag_shader = shaders.at("frag");
		shader *comp_shader = shaders.at("comp");
		shader *tesc_shader = shaders.at("tesc");
		shader *tese_shader = shaders.at("tese");
		shader *geom_shader = shaders.at("geom");

		if (vert_shader)
		{
			stages.try_emplace(s_vert, vert_shader);
		}

		if (frag_shader)
		{
			stages.try_emplace(s_frag, frag_shader);
		}

		if (comp_shader)
		{
			stages.try_emplace(s_comp, comp_shader);
		}

		if (tesc_shader)
		{
			stages.try_emplace(s_tess_control, tesc_shader);
		}

		if (tese_shader)
		{
			stages.try_emplace(s_tess_eval, tese_shader);
		}

		if (geom_shader)
		{
			stages.try_emplace(s_geom, geom_shader);
		}
		return this;
	}

	engine::gpu_pipeline *g_add_color_attachment_output(int binding, engine::attachment_info vk_attachment_info)
	{
		if (!out_col_attachments_bindings.contains(binding))
		{
			out_col_attachments_bindings.try_emplace(binding, vk_attachment_info);
		}
		else
		{
			std::cout << "Attachment: " << "\"" << name << "\"" << " already exist";
		}
		return this;
	}

	engine::gpu_pipeline *g_set_depth_attachment_output(engine::attachment_info depth)
	{
		depth_attachment = depth;
		return this;
	}

	engine::gpu_pipeline *g_reset()
	{
		pipeline.reset();
		pipeline_layout.reset();
		pipeline_cache.reset();
		desc_cache.reset();
		pipeline_layout_ci  = vk::PipelineLayoutCreateInfo();
		push_constant_range = vk::PushConstantRange();
		pipeline_type      = vk::PipelineBindPoint::eGraphics;

		dynamic_render_pass.reset();
		pipeline_configs.rasterization_configs = r_fill;
		pipeline_configs.topology_configs      = t_triangle;
		depth_config = d_none;
		vertex_layout.binding_description.clear();
		vertex_layout.input_description.clear();
		push_constant_size = 4;
		depth_attachment = {};
		return this;
	}
};

struct render_graph_node : systems::i_serializable<engine::render_graph_node>
{
	render_graph_node() = default;

	nlohmann::json serialize() override
	{
		nlohmann::json json;
		std::string    text     = json.dump(3);
		std::string    filename = systems::os::get_instance()->get_engine_resources_path() + "\\RenderNodes\\pass_" + pass_name + ".json";
		if (!filename.empty())
		{
			systems::os::get_instance()->write_file(filename, text.c_str(), text.size());
		}
		return text;
	}

	engine::render_graph_node *deserialize(std::string filename) override
	{
		if (!std::filesystem::exists(filename))
		{
			systems::logger::get_instance()->log_message(
			    "Pass with name: (" + pass_name + ") does not have a serialized file, the pass may be null");
			return this;
		}
		return this;
	}
	engine::render_graph_node *g_recreate_resources()
	{
		gpu_pipeline_ref->g_recreate_resources();
		return this;
	}

	engine::render_graph_node *build_render_graph_node()
	{
		if (gpu_pipeline_ref)
		{
			gpu_pipeline_ref->g_build_gpu_pipeline();
			if (gpu_pipeline_ref->pipeline_type == vk::PipelineBindPoint::eGraphics)
			{
				node_kind = render_node_type::graphics;
			}
			else
			{
				node_kind = render_node_type::compute;
			}
		}
		else
		{
			node_kind = render_node_type::cuda;
			cuda_pipeline->c_build_cuda_pipeline();
		}
		return this;
	}
	
	engine::render_graph_node* c_set_kernel_function(std::function<void(CodeCuda::CodeCudaContext*)> funct)
	{
		cuda_pipeline->c_set_kernel_function(std::move(funct));
		return this;
	}
	
	engine::render_graph_node* c_set_pre_kernel_function(std::function<void()> funct)
	{
		cuda_pipeline->c_set_pre_cpu_kernel_function(std::move(funct));
		return this;
	}


	engine::render_graph_node *enqueue_node()
	{
		return this;
	}

	vk::PipelineBindPoint g_get_pipeline_type()
	{
		assert(cuda_pipeline == nullptr && "this call should not happen in a cuda pipeline");
		return gpu_pipeline_ref->pipeline_type;
	}

	engine::render_graph_node *transition_images(vk::CommandBuffer command_buffer)
	{
		for (auto &storage_image : storage_images)
		{
			layout_patterns dst_pattern = empty;
			switch (node_kind)
			{
				case render_node_type::graphics:
					dst_pattern = graphics_write;
					break;
				case render_node_type::compute:
					dst_pattern = compute_write;
					break;
				case render_node_type::cuda:
					dst_pattern = compute_write;
					break;
				default:
					assert(false && "pipeline type is unknown");
			}
			if (is_image_transition_needed(storage_image.second->image_metadata->current_layout, dst_pattern))
			{
				transition_image(storage_image.second->image_metadata, dst_pattern,
				                storage_image.second->get_subresource_range(),
				                command_buffer);
			}
		}
		for (auto &sampler : sampled_images)
		{
			layout_patterns dst_pattern = empty;
			switch (node_kind)
			{
				case render_node_type::graphics:
					dst_pattern = graphics_read;
					break;
				case render_node_type::compute:
					dst_pattern = compute;
					break;
				case render_node_type::cuda:
					dst_pattern = compute;
					break;
				default:
					assert(false && "pipeline type is unknown");
			}
			// TODO: Better transition for sampler images
			//  if (is_image_transition_needed(sampler.second->image_metadata->current_layout, dst_pattern))
			//  {
			//  }
			transition_image(sampler.second->image_metadata, dst_pattern, sampler.second->get_subresource_range(),
			                command_buffer);
		}
		return this;
	}

	engine::render_graph_node *sync_buffers(vk::CommandBuffer command_buffer)
	{
		for (auto &pair : buffers)
		{
			buffer_key           buffer     = pair.second;
			buffer_access_pattern src_pattern = get_src_buffer_access_pattern(buffer.src_usage);
			buffer_access_pattern dst_pattern = get_src_buffer_access_pattern(buffer.dst_usage);
			create_mem_barrier(src_pattern, dst_pattern, command_buffer);
		}
		return this;
	}
	engine::render_graph_node *g_set_sampler(std::string name, engine::image_view *image_view_handle, engine::sampler *sampler = nullptr)
	{
		gpu_pipeline_ref->g_set_sampler(name, image_view_handle, sampler);
		return this;
	}
	engine::render_graph_node *g_set_sampler_array(std::string name, std::vector<engine::image_view *> &image_views, std::vector<engine::sampler *> *samplers = nullptr)
	{
		gpu_pipeline_ref->g_set_sampler_array(name, image_views, samplers);
		return this;
	}
	engine::render_graph_node *g_set_storage_image(std::string name, engine::image_view *image_view_handle, engine::sampler *sampler = nullptr)
	{
		gpu_pipeline_ref->g_set_storage_image(name, image_view_handle, sampler);
		return this;
	}
	engine::render_graph_node *g_set_storage_image_array(std::string name, std::vector<engine::image_view *> &image_views,
	                                      std::vector<engine::sampler *> *samplers = nullptr)
	{
		gpu_pipeline_ref->g_set_storage_image_array(name, image_views, samplers);
		return this;
	}
	template <typename value_type>
	engine::render_graph_node *g_set_buffer(std::string name, value_type &buffer_data)
	{
		gpu_pipeline_ref->g_set_buffer<value_type>(name, buffer_data);
		return this;
	}
	template <typename value_type>
	engine::render_graph_node *g_set_buffer(std::string name, std::vector<value_type> &buffer_data)
	{
		gpu_pipeline_ref->g_set_buffer<value_type>(name, buffer_data);
		return this;
	}
	engine::render_graph_node *g_set_buffer(std::string name, engine::buffer *buffer_data)
	{
		gpu_pipeline_ref->g_set_buffer(name, buffer_data);
		return this;
	}
	engine::render_graph_node *g_reload_shaders()
	{
		gpu_pipeline_ref->g_reload_shaders();
		return this;
	}
	engine::render_graph_node *validate_node_type()
	{
		assert(!(gpu_pipeline_ref && cuda_pipeline) && "you only can have either gpu or cuda pipeline not both");
		if (node_kind != render_node_type::none)
		{
			return this;
		}
		if (gpu_pipeline_ref == nullptr)
		{
			node_kind = render_node_type::cuda;
			return this;
		}
		if (gpu_pipeline_ref->pipeline_type == vk::PipelineBindPoint::eCompute)
		{
			node_kind = render_node_type::compute;
		}
		else
		{
			node_kind = render_node_type::graphics;
		}
		return this;
	}

	engine::render_graph_node *g_execute_pass(vk::CommandBuffer command_buffer)
	{
		assert(images_attachment_outputs.size() == gpu_pipeline_ref->out_col_attachments_bindings.size() && "Not all color attachments were set");
		assert(!images_attachment_outputs.empty() && "No color attachaments were set");
		assert(!(gpu_pipeline_ref->depth_attachment.format != vk::Format::eUndefined && depth_image == nullptr) && "there is no depth attachment set");

		g_set_framebuffer_size(images_attachment_outputs.at(0)->image_metadata->get_image_size());
		gpu_pipeline_ref->dynamic_render_pass.set_viewport(frame_buffer_size, frame_buffer_size);
		command_buffer.setViewport(0, 1, &gpu_pipeline_ref->dynamic_render_pass.viewport);
		command_buffer.setScissor(0, 1, &gpu_pipeline_ref->dynamic_render_pass.scissor);

		std::vector<vk::RenderingAttachmentInfo> attachment_infos;
		attachment_infos.resize(gpu_pipeline_ref->out_col_attachments_bindings.size());
		
		for (auto &image_pair : images_attachment_outputs)
		{
			if (is_image_transition_needed(image_pair.second->image_metadata->current_layout, color_attachment))
			{
				transition_image(image_pair.second->image_metadata, color_attachment,
				                image_pair.second->get_subresource_range(), command_buffer);
			}
			attachment_infos[image_pair.first] = (gpu_pipeline_ref->out_col_attachments_bindings[image_pair.first].vk_attachment_info);
			attachment_infos[image_pair.first].setImageView(image_pair.second->image_view_handle.get());
			attachment_infos[image_pair.first].imageLayout = image_pair.second->image_metadata->current_pattern.layout;
		}
		if (depth_image)
		{
			if (depth_image->image_metadata->current_layout != depth_attachment)
			{
				transition_image(depth_image->image_metadata, depth_attachment, depth_image->get_subresource_range(),
				                command_buffer);
			}
		}

		transition_images(command_buffer);
		sync_buffers(command_buffer);
		if (depth_image != nullptr)
		{
			gpu_pipeline_ref->depth_attachment.vk_attachment_info.imageView = depth_image->image_view_handle.get();
		}
		gpu_pipeline_ref->dynamic_render_pass.set_render_info(attachment_infos, frame_buffer_size, &gpu_pipeline_ref->depth_attachment.vk_attachment_info);

		command_buffer.bindPipeline(gpu_pipeline_ref->pipeline_type, gpu_pipeline_ref->pipeline.get());
		command_buffer.beginRendering(gpu_pipeline_ref->dynamic_render_pass.render_info);
		command_buffer.bindDescriptorSets(gpu_pipeline_ref->pipeline_type,
		                                 gpu_pipeline_ref->pipeline_layout.get(), 0,
		                                 1,
		                                 &gpu_pipeline_ref->desc_cache->dst_set, 0, nullptr);
		(*render_operations)();
		command_buffer.endRendering();
		return this;
	}

	engine::render_graph_node *g_execute_compute(vk::CommandBuffer command_buffer)
	{
		transition_images(command_buffer);
		sync_buffers(command_buffer);
		command_buffer.bindPipeline(gpu_pipeline_ref->pipeline_type, gpu_pipeline_ref->pipeline.get());
		command_buffer.bindDescriptorSets(gpu_pipeline_ref->pipeline_type,
		                                 gpu_pipeline_ref->pipeline_layout.get(), 0,
		                                 1,
		                                 &gpu_pipeline_ref->desc_cache->dst_set, 0, nullptr);
		(*render_operations)();
		return this;
	}

	engine::render_graph_node *c_execute(vk::CommandBuffer command_buffer)
	{
		transition_images(command_buffer);
		sync_buffers(command_buffer);
		assert(this->cuda_pipeline);
		this->cuda_pipeline->context->C_ExecuteCPU();
		return this;
	}

	engine::render_graph_node *execute(vk::CommandBuffer command_buffer)
	{
		for (int i = 0; i < pre_rendering_tasks.size(); ++i)
		{
			if (pre_rendering_tasks[i] != nullptr)
			{
				(*pre_rendering_tasks[i])();
			}
		}
		switch (node_kind)
		{
			case render_node_type::graphics:
				g_execute_pass(command_buffer);
				break;
			case render_node_type::compute:
				g_execute_compute(command_buffer);
				break;
			case render_node_type::cuda:
				c_execute(command_buffer);
				break;
			default:
				assert(false && "Unsuported pipeline type");
				break;
		}
		return this;
	}

	engine::render_graph_node *g_set_vertex_input(engine::vertex_input vertex_layout)
	{
		this->gpu_pipeline_ref->g_set_vertex_input(vertex_layout);
		return this;
	}

	engine::render_graph_node *g_set_framebuffer_size(glm::uvec2 size)
	{
		this->frame_buffer_size = size;
		return this;
	}

	engine::render_graph_node *g_set_render_operation(std::function<void()> *render_operations)
	{
		assert(gpu_pipeline_ref != nullptr && "Render operations are only for gpu pipelines");
		if (this->render_operations)
		{
			delete (this->render_operations);
		}
		this->render_operations = render_operations;
		return this;
	}

	engine::render_graph_node *add_pre_rendering_task(std::function<void()> *task)
	{
		this->pre_rendering_tasks.push_back(task);
		return this;
	}

	engine::render_graph_node *g_set_pipeline_layout_ci(vk::PipelineLayoutCreateInfo create_info)
	{
		gpu_pipeline_ref->g_set_pipeline_layout_ci(create_info);
		return this;
	}

	engine::render_graph_node *g_set_push_constant_size(size_t size)
	{
		gpu_pipeline_ref->g_set_push_constant_size(size);
		return this;
	}

	engine::render_graph_node *g_set_graphics_pipeline_configs(engine::graphics_pipeline_configs pipeline_configs)
	{
		gpu_pipeline_ref->g_set_graphics_pipeline_configs(pipeline_configs);
		return this;
	}

	engine::render_graph_node *g_set_depth_config(depth_configs depht_config)
	{
		gpu_pipeline_ref->g_set_depth_config(depht_config);
		return this;
	}

	engine::render_graph_node *g_set_vert_shader(shader *shader)
	{
		gpu_pipeline_ref->g_set_vert_shader(shader);
		return this;
	}

	engine::render_graph_node *g_set_frag_shader(shader *shader)
	{
		gpu_pipeline_ref->g_set_frag_shader(shader);
		return this;
	}

	engine::render_graph_node *g_set_comp_shader(shader *shader)
	{
		gpu_pipeline_ref->g_set_comp_shader(shader);
		return this;
	}

	engine::render_graph_node *g_set_tes_control_shader(shader *shader)
	{
		gpu_pipeline_ref->g_set_tes_control_shader(shader);
		return this;
	}

	engine::render_graph_node *g_set_tes_eval_shader(shader *shader)
	{
		gpu_pipeline_ref->g_set_tes_eval_shader(shader);
		return this;
	}

	engine::render_graph_node *g_set_geom_shader(shader *shader)
	{
		gpu_pipeline_ref->g_set_geom_shader(shader);
		return this;
	}

	engine::render_graph_node *g_add_color_attachment_output(int binding, engine::attachment_info vk_attachment_info)
	{
		gpu_pipeline_ref->g_add_color_attachment_output(binding, vk_attachment_info);
		return this;
	}

	engine::render_graph_node *g_set_depth_attachment_output(engine::attachment_info depth)
	{
		gpu_pipeline_ref->g_set_depth_attachment_output(depth);
		return this;
	}
	engine::render_graph_node *g_set_depth_image_resource(engine::image_view *image_view_handle)
	{
		this->depth_image = image_view_handle;
		return this;
	}

	engine::render_graph_node *activate_node(bool value)
	{
		this->active = value;
		return this;
	}

	engine::render_graph_node *g_set_color_image_attachment_binding(int binding, engine::image_view *image_view_handle)
	{
		assert(image_view_handle && "Name does not exist or image view is null");
		assert(gpu_pipeline_ref->out_col_attachments_bindings.contains(binding) && "Binding specified does not exist");
		if (!image_attachments_names.contains(image_view_handle->name))
		{
			image_attachments_names.try_emplace(image_view_handle->name, binding);
			assert(!images_attachment_outputs.contains(binding) && "this binding was already set");
			images_attachment_outputs.try_emplace(binding, image_view_handle);
		}
		else
		{
			images_attachment_outputs.at(image_attachments_names.at(image_view_handle->name)) = image_view_handle;
		}
		return this;
	}

	engine::render_graph_node *g_add_sampler_resource(engine::image_view *image_view_handle)
	{
		assert(image_view_handle && "Name does not exist or image view is null");
		if (!sampled_images.contains(image_view_handle->name))
		{
			sampled_images.try_emplace(image_view_handle->name, image_view_handle);
		}
		else
		{
			sampled_images.at(image_view_handle->name) = image_view_handle;
		}
		return this;
	}

	engine::render_graph_node *g_add_storage_resource(engine::image_view *image_view_handle)
	{
		assert(image_view_handle && "Name does not exist or image view is null");
		if (!storage_images.contains(image_view_handle->name))
		{
			storage_images.try_emplace(image_view_handle->name, image_view_handle);
		}
		else
		{
			storage_images.at(image_view_handle->name) = image_view_handle;
		}
		return this;
	}

	engine::render_graph_node *add_buffer_sync(buffer_key buffer)
	{
		if (!buffers.contains(buffer.name))
		{
			buffers.try_emplace(buffer.name, buffer);
		}
		else
		{
			buffers.at(buffer.name) = buffer;
		}
		return this;
	}
	// We change the image view if the name already exist when using resources

	
	engine::render_graph_node *depends_on(engine::render_graph_node* node)
	{
		if (!dependencies.contains(node->pass_name))
		{
			dependencies.insert(node->pass_name);
			systems::logger::get_instance()->log_message("Renderpass: (" + this->pass_name + ") Depends On-> (" + node->pass_name + ")");
		}
		return this;
	}
	engine::render_graph_node *depends_on(std::string dependency)
	{
		if (!dependencies.contains(dependency))
		{
			dependencies.insert(dependency);
			systems::logger::get_instance()->log_message("Renderpass: (" + this->pass_name + ") Depends On-> (" + dependency + ")");
		}
		return this;
	}

	engine::render_graph_node *clear_operations()
	{
		delete render_operations;
		for (auto &task : pre_rendering_tasks)
		{
			delete task;
		}
		render_operations = nullptr;
		pre_rendering_tasks.clear();
		return this;
	}
	engine::render_graph_node *set_curr_cmd(vk::CommandBuffer cmd)
	{
		curr_cmd = cmd;
		return this;
	}
	vk::CommandBuffer &get_curr_cmd()
	{
		assert(curr_cmd != nullptr && "Cmd must be valid");
		return curr_cmd;
	}
	engine::render_graph_node *g_set_configs(render_node_configs configs)
	{
		gpu_pipeline_ref->g_set_configs(configs);
		return this;
	}
	engine::render_graph_node *g_reset()
	{
		gpu_pipeline_ref->g_reset();
		active          = false;
		frame_buffer_size = {0, 0};
		worker_queue_name = "Graphics";
		curr_cmd         = nullptr;
		depth_image      = nullptr;

		images_attachment_outputs.clear();
		storage_images.clear();
		sampled_images.clear();
		buffers.clear();
		clear_operations();
		dependencies.clear();
		return this;
	}

	// unused

	engine::gpu_pipeline  *gpu_pipeline_ref = nullptr;
	engine::cuda_pipeline *cuda_pipeline   = nullptr;

	bool           wait_for_resources_creation = false;
	render_node_type node_kind                 = render_node_type::none;
	std::string    path;
	std::string    pass_name;
	std::string    worker_queue_name = "Graphics";
	bool           active          = false;

	std::set<std::string> dependencies;

	const std::unordered_map<std::string, int> &get_image_attachment_names() const
	{
		return image_attachments_names;
	}

	const std::unordered_map<int, engine::image_view *> &get_image_attachment_outputs() const
	{
		return images_attachment_outputs;
	}

	const std::unordered_map<std::string, engine::image_view *> &get_storage_images() const
	{
		return storage_images;
	}

	const std::unordered_map<std::string, engine::image_view *> &get_sampled_images() const
	{
		return sampled_images;
	}

	const std::unordered_map<std::string, buffer_key> &get_buffers() const
	{
		return buffers;
	}

	engine::image_view *get_depth_image() const
	{
		return depth_image;
	}

	glm::uvec2 get_frame_buffer_size() const
	{
		return frame_buffer_size;
	}

  private:
	friend class render_graph;

	engine::core             *core;
	engine::image_view        *depth_image = nullptr;
	vk::CommandBuffer curr_cmd;

	glm::uvec2                      frame_buffer_size = {0, 0};
	std::vector<engine::render_graph_node *> *nodes_to_execute_ref;

	std::unordered_map<std::string, int> image_attachments_names;
	std::unordered_map<int, engine::image_view *>             images_attachment_outputs;

	std::unordered_map<std::string, engine::image_view *> storage_images;
	std::unordered_map<std::string, engine::image_view *> sampled_images;
	std::unordered_map<std::string, buffer_key>   buffers;

	std::function<void()>               *render_operations = nullptr;
	std::vector<std::function<void()> *> pre_rendering_tasks;

	resources_manager *res_manager_ref;
};
struct queue_nodes_batch
{
	std::vector<engine::render_graph_node *> sorted_nodes;
	std::string                    queue_name;
	vk::CommandBuffer              command_buffer;
	int                            id;
	int                            pool_id_used;
	void                           execute_cuda(engine::core *core)
	{
		auto queue_ref = core->queue_worker_manager->get_worker_queue(queue_name);
		for (int i = 0; i < sorted_nodes.size(); ++i)
		{
			auto &node = sorted_nodes[i];
			node->cuda_pipeline->context->C_WaitExternalSemaphore(queue_ref->timeline_value);
			node->cuda_pipeline->context->C_ExecuteKernel();
			node->cuda_pipeline->context->C_SignalExternalSemaphore(++queue_ref->timeline_value);
		}
	}
};

class render_graph
{
  public:
	engine::core             *core;
	resources_manager *resources_manager;

	engine::image_view *current_back_buffer_swapchain;
	// this is the backbuffer that will be blitted to the swapchain
	engine::image_view *current_back_buffer;
	size_t     frame_index;
	bool       debug_ui = true;

	std::unordered_map<std::string, std::unique_ptr<engine::gpu_pipeline>>     gpu_pipelines;
	std::unordered_map<std::string, std::unique_ptr<engine::cuda_pipeline>>    cuda_pipelines;
	std::unordered_map<std::string, std::unique_ptr<engine::render_graph_node>> render_nodes;
	// todo
	std::vector<engine::render_graph_node *> nodes_to_execute;
	std::vector<engine::render_graph_node *> sequential_render_nodes;
	std::vector<engine::render_graph_node *> sorted_by_dep_nodes;

	std::vector<queue_nodes_batch>                                      sorted_queue_batches;

	render_graph(engine::core *core)
	{
		this->core = core;
	}

	void serialize_all()
	{
		for (auto node : sequential_render_nodes)
		{
			node->serialize();
		}
	}

	engine::gpu_pipeline *get_template_gpu_pipeline(std::string name, std::string shader_name, shader_compiler compiler)
	{
		shader *v_shader     = resources_manager->get_or_create_default_shader(shader_name, shader_stage::s_vert, compiler);
		shader *f_shader     = resources_manager->get_or_create_default_shader(shader_name, shader_stage::s_frag, compiler);
		auto    gpu_pipeline = add_gpu_pipeline(name);
		gpu_pipeline->g_set_configs({true});
		gpu_pipeline->g_set_vert_shader(v_shader);
		gpu_pipeline->g_set_frag_shader(f_shader);
		gpu_pipeline->g_set_vertex_input(vertex_2d::get_vertex_input());
		return gpu_pipeline;
	}

	engine::render_graph_node *get_template_node_df(std::string name, std::string shader_name, shader_compiler compiler)
	{
		shader *v_shader    = resources_manager->get_or_create_default_shader(shader_name, shader_stage::s_vert, compiler);
		shader *f_shader    = resources_manager->get_or_create_default_shader(shader_name, shader_stage::s_frag, compiler);
		auto    render_node = add_pass(name);
		render_node->g_set_configs({true});
		render_node->g_set_vert_shader(v_shader);
		render_node->g_set_frag_shader(f_shader);
		render_node->g_set_vertex_input(vertex_2d::get_vertex_input());
		return render_node;
	}

	engine::render_graph_node *get_template_compute_node_df(std::string name, std::string shader_name, shader_compiler compiler)
	{
		shader *shader     = resources_manager->get_or_create_default_shader(shader_name, shader_stage::s_comp, compiler);
		auto    render_node = add_pass(name);
		render_node->g_set_configs({true});
		render_node->g_set_comp_shader(shader);
		return render_node;
	}
	engine::render_graph_node *get_template_node(std::string name, std::string v_path, std::string f_path)
	{
		engine::attachment_info col_info = get_color_attachment_info(
		    blend_configs::b_opaque, glm::vec4(0.0f), g_32b_format);
		shader *v_shader    = resources_manager->get_shader(v_path, shader_stage::s_vert);
		shader *f_shader    = resources_manager->get_shader(f_path, shader_stage::s_frag);
		auto    render_node = add_pass(name);
		render_node->g_set_configs({true});
		render_node->g_set_vert_shader(v_shader);
		render_node->g_set_frag_shader(f_shader);
		render_node->g_set_vertex_input(vertex_2d::get_vertex_input());
		// change this
		render_node->g_set_push_constant_size(4);
		return render_node;
	}

	engine::render_graph_node *get_template_compute_node(std::string name, std::string path)
	{
		shader *shader     = resources_manager->get_shader(path, shader_stage::s_comp);
		auto    render_node = add_pass(name);
		render_node->g_set_configs({true});
		render_node->g_set_comp_shader(shader);
		// change this
		render_node->g_set_push_constant_size(4);
		return render_node;
	}
	void create_utility_shaders()
	{
		auto          *blitter_shader = get_template_gpu_pipeline("BlitterPipeline", "Blitter", shader_compiler::c_glsl);
		engine::attachment_info col_info       = get_color_attachment_info(b_opaque,
            glm::vec4(0.0f), core->swapchain_ref->get_format());
		blitter_shader->g_add_color_attachment_output(0, col_info);
		blitter_shader->g_build_gpu_pipeline();
	}
	engine::gpu_pipeline *get_blitter_shader()
	{
		auto *blitter_shader = get_gpu_pipeline("BlitterPipeline");
		return blitter_shader;
	}
	engine::render_graph_node *blit_to_backbuffer()
	{
		auto *blitter_node = add_pass(get_blitter_shader(), "BlitterNode");

		blitter_node->build_render_graph_node();
		blitter_node->enqueue_node();

		get_node("BlitterNode")->g_set_sampler("main_tex", current_back_buffer);
		blitter_node->add_pre_rendering_task( new std::function<void()>([this]() {
			// there is one thing that I may change and is that when I set the sampler from outside of the task
			// function and I reload the shaders the ref of the sampler is gone because I reset the descriptor set, I should change that
			get_node("BlitterNode")->g_set_color_image_attachment_binding(0,current_back_buffer_swapchain);
			get_node("BlitterNode")->g_set_framebuffer_size(current_back_buffer_swapchain->image_metadata->get_image_size());
		}));
		blitter_node->g_set_render_operation(new std::function<void()>(
		    [this]() {
			    auto           render_node = get_node("BlitterNode");
			    vk::DeviceSize offset     = 0;
			    render_node->get_curr_cmd().bindVertexBuffers(
			        0, 1,
			        &resources_manager->get_staged_buff_from_name("quad_default")->device_buffer->buffer_handle.get(),
			        &offset);
			    render_node->get_curr_cmd().bindIndexBuffer(
			        resources_manager->get_staged_buff_from_name("quad_index_default")->device_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);

			    render_node->get_curr_cmd().drawIndexed(vertex_2d::get_quad_indices().size(), 1, 0,
			                                         0, 0);
		    }));
		return blitter_node;
	}

	void load_external_passes()
	{
		std::string              nodes_path = systems::os::get_instance()->get_engine_resources_path() + "\\RenderNodes";
		std::vector<std::string> paths;
		assert(std::filesystem::exists(nodes_path));
		for (auto path : std::filesystem::directory_iterator(nodes_path))
		{
			size_t      pos      = path.path().string().find('_');
			std::string pass_name = path.path().string().substr(pos + 1, path.path().string().length());
			size_t      ext_pos   = pass_name.find_last_of('.');
			pass_name             = pass_name.substr(0, ext_pos);

			engine::render_graph_node *node = add_pass(pass_name);
			if (!render_nodes.contains(node->pass_name))
			{
				node->deserialize(path.path().string());
			}
			else
			{
				systems::logger::get_instance()->log_message("Render Node already is in ram");
				node->deserialize(path.path().string());
			}
		}
		recreate_node_pipelines();
	}
	void enqueue_node(engine::render_graph_node *node)
	{
		nodes_to_execute.push_back(node);
	}
	void update_all_from_meta_data()
	{
		for (auto &node : sequential_render_nodes)
		{
			node->deserialize(node->path);
			systems::logger::get_instance()->log_message("Render Node: (" + node->pass_name + ") updated");
		}
	}

	void create_res_manager()
	{
		resources_manager  = resources_manager::get_instance();
		current_back_buffer = resources_manager->get_image_view_from_name("bf");
	}

	engine::render_graph_node *get_node(std::string name)
	{
		if (render_nodes.contains(name))
		{
			return render_nodes.at(name).get();
		}
		else
		{
			print_invalid_resource("Renderpass", name);
			return nullptr;
		}
	}

	engine::gpu_pipeline *get_gpu_pipeline(std::string name)
	{
		if (!gpu_pipelines.contains(name))
		{
			return nullptr;
		}
		auto gpu_pipeline = gpu_pipelines.at(name).get();
		assert(gpu_pipeline != nullptr && "gpu pipeline is null");
		return gpu_pipeline;
	}

	engine::cuda_pipeline *get_cuda_pipeline(std::string name)
	{
		if (!cuda_pipelines.contains(name))
		{
			return nullptr;
		}
		auto cuda_pi = cuda_pipelines.at(name).get();
		assert(cuda_pi != nullptr && "cuda pipeline is null");
		return cuda_pi;
	}
	// used in cases shaders in case we want to reuse shaders with different dsets
	engine::gpu_pipeline *add_gpu_pipeline(std::string name)
	{
		if (!gpu_pipelines.contains(name))
		{
			auto gpu_pipeline             = std::make_unique<engine::gpu_pipeline>();
			gpu_pipeline->core            = core;
			gpu_pipeline->name            = name;
			gpu_pipelines.try_emplace(name, std::move(gpu_pipeline));
			return gpu_pipelines.at(name).get();
		}

		auto gpu_pipeline = gpu_pipelines.at(name).get();
		assert(gpu_pipeline != nullptr && "Pipeline is null");
		return gpu_pipeline;
	}

	// Used in cases we want to have multiple independent cuda pipelines
	engine::cuda_pipeline *add_cuda_pipeline(std::string name)
	{
		if (!cuda_pipelines.contains(name))
		{
			auto cuda_pipeline     = std::make_unique<engine::cuda_pipeline>();
			auto queue_ref         = core->queue_worker_manager->get_worker_queue("CUDA");
			cuda_pipeline->context = new CodeCuda::CodeCudaContext();
			cuda_pipeline->context->C_InitFromExternalDevice(core->device_uuid.data(), VK_UUID_SIZE);
			cuda_pipeline->context->C_ImportExternalSemaphore(queue_ref->get_exportable_handle());
			cuda_pipeline->name = name;
			cuda_pipelines.try_emplace(name, std::move(cuda_pipeline));
			return cuda_pipelines.at(name).get();
		}

		auto cuda_pipeline = cuda_pipelines.at(name).get();
		assert(cuda_pipeline != nullptr && "Pipeline is null");
		return cuda_pipeline;
	}

	engine::render_graph_node *add_cuda_pass(engine::cuda_pipeline *pipeline, std::string name, std::string worker_queue_name = "CUDA")
	{
		if (!render_nodes.contains(name))
		{
			assert(pipeline);
			if (!cuda_pipelines.contains(pipeline->name))
			{
				add_cuda_pipeline("CUDA_PIPELINE_"+name);
				systems::logger::get_instance()->log("CUDA Pipeline was created (CUDA_PIPELINE_"+name + ")");
			}
			auto render_graph_node               = std::make_unique<engine::render_graph_node>();
			render_graph_node->cuda_pipeline      = pipeline;
			render_graph_node->pass_name          = name;
			render_graph_node->core              = core;
			render_graph_node->nodes_to_execute_ref = &nodes_to_execute;
			render_graph_node->res_manager_ref     = resources_manager;
			render_graph_node->worker_queue_name   = worker_queue_name;
			render_graph_node->active            = true;
			render_graph_node->path              = systems::os::get_instance()->get_engine_resources_path() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			render_nodes.try_emplace(name, std::move(render_graph_node));
			sequential_render_nodes.push_back(render_nodes.at(name).get());
			return render_nodes.at(name).get();
		}
		return render_nodes.at(name).get();
	}
	engine::render_graph_node *add_cuda_pass(std::string name, std::string worker_queue_name = "CUDA")
	{
		if (!render_nodes.contains(name))
		{
			auto render_graph_node               = std::make_unique<engine::render_graph_node>();
			render_graph_node->cuda_pipeline      = add_cuda_pipeline("CUDA_PIPELINE_" + name);
			render_graph_node->pass_name          = name;
			render_graph_node->core              = core;
			render_graph_node->nodes_to_execute_ref = &nodes_to_execute;
			render_graph_node->res_manager_ref     = resources_manager;
			render_graph_node->worker_queue_name   = worker_queue_name;
			render_graph_node->active            = true;
			render_graph_node->path              = systems::os::get_instance()->get_engine_resources_path() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			render_nodes.try_emplace(name, std::move(render_graph_node));
			sequential_render_nodes.push_back(render_nodes.at(name).get());
			return render_nodes.at(name).get();
		}
		return render_nodes.at(name).get();
	}

	engine::render_graph_node *add_pass(engine::gpu_pipeline *gpu_pipeline, std::string name, std::string worker_queue_name = "Graphics")
	{
		if (!render_nodes.contains(name))
		{
			assert(gpu_pipeline != nullptr && "Pipeline is null");
			auto render_graph_node               = std::make_unique<engine::render_graph_node>();
			render_graph_node->gpu_pipeline_ref    = gpu_pipeline;
			render_graph_node->pass_name          = name;
			render_graph_node->core              = core;
			render_graph_node->nodes_to_execute_ref = &nodes_to_execute;
			render_graph_node->res_manager_ref     = resources_manager;
			render_graph_node->worker_queue_name   = worker_queue_name;
			render_graph_node->active            = true;
			render_graph_node->path              = systems::os::get_instance()->get_engine_resources_path() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			render_nodes.try_emplace(name, std::move(render_graph_node));
			sequential_render_nodes.push_back(render_nodes.at(name).get());
			return render_nodes.at(name).get();
		}
		return render_nodes.at(name).get();
	}

	engine::render_graph_node *add_pass(std::string name, std::string worker_queue_name = "Graphics")
	{
		if (!render_nodes.contains(name))
		{
			auto gpu_pipeline = add_gpu_pipeline("GPUPipeline_" + name);
			assert(gpu_pipeline != nullptr && "GPUPipeline_ is null");
			auto render_graph_node               = std::make_unique<engine::render_graph_node>();
			render_graph_node->gpu_pipeline_ref    = gpu_pipeline;
			render_graph_node->pass_name          = name;
			render_graph_node->core              = core;
			render_graph_node->nodes_to_execute_ref = &nodes_to_execute;
			render_graph_node->res_manager_ref     = resources_manager;
			render_graph_node->worker_queue_name   = worker_queue_name;
			render_graph_node->active            = true;
			render_graph_node->path              = systems::os::get_instance()->get_engine_resources_path() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			render_nodes.try_emplace(name, std::move(render_graph_node));
			sequential_render_nodes.push_back(render_nodes.at(name).get());
			return render_nodes.at(name).get();
		}
		return render_nodes.at(name).get();
	}

	engine::image_view *add_sampler_resource(std::string pass_name, engine::image_view *image_view_handle)
	{
		assert(image_view_handle && "ImageView is null");
		if (render_nodes.contains(pass_name))
		{
			render_nodes.at(pass_name)->g_add_sampler_resource(image_view_handle);
		}
		else
		{
			std::cout << "Renderpass: " << pass_name << " does not exist, saving the image anyways. \n";
		}
		return image_view_handle;
	}

	engine::image_view *add_storage_resource(std::string pass_name, engine::image_view *image_view_handle)
	{
		assert(image_view_handle && "ImageView is null");
		if (render_nodes.contains(pass_name))
		{
			render_nodes.at(pass_name)->g_add_storage_resource(image_view_handle);
		}
		else
		{
			std::cout << "Renderpass: " << pass_name << " does not exist, saving the image anyways. \n";
		}
		return image_view_handle;
	}

	buffer_key &add_buffer_sync(std::string pass_name, buffer_key buffer)
	{
		assert(buffer.buffer && "ImageView is null");
		if (render_nodes.contains(pass_name))
		{
			render_nodes.at(pass_name)->add_buffer_sync(buffer);
		}
		else
		{
			std::cout << "Renderpass: " << pass_name << " does not exist, saving the image anyways. \n";
		}
		// std::cout << "Image with name: \"" << name << "\" has changed \n";
		return buffer;
	}

	void recreate_frame_resources()
	{
		// auto image_info_bf = image::create_info2d(glm::uvec2(core->swapchain_ref->extent.width, core->swapchain_ref->extent.height), 1, 1,
		// 											 core->swapchain_ref->get_format(),
		// 											 vk::ImageUsageFlagBits::eColorAttachment |
		// 												 vk::ImageUsageFlagBits::eSampled);
		// current_back_buffer = resources_manager->set_or_create_image("bf", image_info_bf, 0, 0);
	}
	void recreate_node_pipelines()
	{
		for (auto &node : render_nodes)
		{
			if (node.second->gpu_pipeline_ref)
			{
				node.second->g_recreate_resources();
			}
		}
		systems::logger::get_instance()->log_message("Graphics Pipelines Recreated");
	}

	void debug_shaders_compilation()
	{
		std::string path = systems::os::get_instance()->get_shaders_path() + "\\compile.bat";
		int result = std::system(("\"\"" + path + "\"\"").c_str());
		if (result == 0)
		{
			std::cout << "Shaders compiled\n";
		}
		else
		{
			assert(false && "reload shaders failed");
		}
	}
	void create_utility()
	{
		create_utility_shaders();
		blit_to_backbuffer();
	}
	void sort_nodes_by_dep(std::vector<engine::render_graph_node *> &nodes_sorted_out)
	{
		std::vector<std::string> solved_nodes_ordered;
		std::set<std::string>    solved_nodes_names;
		solved_nodes_ordered.reserve(sequential_render_nodes.size());
		for (auto &node : sequential_render_nodes)
		{
			if (node->dependencies.empty())
			{
				solved_nodes_ordered.emplace_back(node->pass_name);
				if (!solved_nodes_names.contains(node->pass_name))
				{
					solved_nodes_names.insert(node->pass_name);
				}
			}
		}

		int idx        = 0;
		int curr_search = 0;
		int max_search  = 10000;
		while (solved_nodes_ordered.size() < sequential_render_nodes.size() && curr_search <= max_search)
		{
			auto &node = sequential_render_nodes[idx];
			if (solved_nodes_names.contains(node->pass_name))
			{
				curr_search++;
				idx = (idx + 1) % sequential_render_nodes.size();
				continue;
			}
			int to_solve_dep = node->dependencies.size();
			int solved_dep  = 0;
			for (auto &dep_name : node->dependencies)
			{
				if (solved_nodes_names.contains(dep_name))
				{
					solved_dep++;
				}
			}
			if (to_solve_dep == solved_dep)
			{
				if (!solved_nodes_names.contains(node->pass_name))
				{
					solved_nodes_ordered.emplace_back(node->pass_name);
					solved_nodes_names.insert(node->pass_name);
				}
			}
			curr_search++;
			idx = (idx + 1) % sequential_render_nodes.size();
		}
		bool reorder_done = solved_nodes_ordered.size() == sequential_render_nodes.size();
		nodes_sorted_out.clear();
		nodes_sorted_out.reserve(solved_nodes_ordered.size());
		for (int i = 0; i < solved_nodes_ordered.size(); ++i)
		{
			nodes_sorted_out.emplace_back(get_node(solved_nodes_ordered[i]));
		}
	}
	void build_queue_passes_batches(std::vector<engine::render_graph_node *> &sorted_nodes, std::vector<queue_nodes_batch> &batches)
	{
		assert(batches.empty() && "Queue batches should be empty before building a new queue batch");
		if (sorted_nodes.empty())
		{
			return;
		}
		int batch_idx = 0;

		// if the first queue is cuda we need to add a "bridge sync point with image adquire semaphore"
		if (sorted_nodes[0]->worker_queue_name == "CUDA")
		{
			int               pool_cmd_id = 0;
			auto             *queue_ref  = core->queue_worker_manager->get_worker_queue("Graphics");
			vk::CommandBuffer cmd       = queue_ref->request_queue_cmd(pool_cmd_id);
			batches.emplace_back(queue_nodes_batch{std::vector<engine::render_graph_node *>{}, "Graphics", cmd, batch_idx});
			batches.back().pool_id_used = pool_cmd_id;
			batch_idx++;
		}
		for (int i = 0; i < sorted_nodes.size(); ++i)
		{
			if (batches.empty() || batches.back().queue_name != sorted_nodes[i]->worker_queue_name)
			{
				batch_idx++;
				auto             *queue_ref  = core->queue_worker_manager->get_worker_queue(sorted_nodes[i]->worker_queue_name);
				int               pool_cmd_id = 0;
				vk::CommandBuffer cmd       = queue_ref->request_queue_cmd(pool_cmd_id);
				batches.emplace_back(queue_nodes_batch{std::vector<engine::render_graph_node *>{sorted_nodes[i]}, queue_ref->name, cmd, batch_idx});
				batches.back().pool_id_used = pool_cmd_id;
			}
			else
			{
				batches.back().sorted_nodes.emplace_back(sorted_nodes[i]);
			}
		}
		// if the first queue is cuda we need to add a "bridge sync point with rendering finished semaphore"
		if (sorted_nodes.back()->worker_queue_name == "CUDA")
		{
			int               pool_cmd_id = 0;
			auto             *queue_ref  = core->queue_worker_manager->get_worker_queue("Graphics");
			vk::CommandBuffer cmd       = queue_ref->request_queue_cmd(pool_cmd_id);
			batches.emplace_back(queue_nodes_batch{std::vector<engine::render_graph_node *>{}, "Graphics", cmd, batch_idx});
			batches.back().pool_id_used = pool_cmd_id;
			batch_idx++;
		}
		if (debug_ui)
		{
			auto *queue_ref  = core->queue_worker_manager->get_worker_queue("UI");
			int   pool_cmd_id = 0;
			batches.emplace_back(queue_nodes_batch{std::vector<engine::render_graph_node *>{}, "UI", queue_ref->request_queue_cmd(pool_cmd_id), batch_idx + 1});
			batches.back().pool_id_used = pool_cmd_id;
		}
	}

	void resolve_nodes_dependancies(std::vector<engine::render_graph_node *> &nodes_to_resolve)
	{
		for (int i = nodes_to_resolve.size() - 1; i >= 0; i--)
		{
			engine::render_graph_node *node = nodes_to_resolve[i];
			for (int j = i - 1; j >= 0; j--)
			{
				engine::render_graph_node *to_check_node = nodes_to_resolve[j];

				for (auto &image : to_check_node->image_attachments_names)
				{
					if (node->image_attachments_names.contains(image.first))
					{
						node->depends_on(to_check_node->pass_name);
						break;
					}
					if (node->sampled_images.contains(image.first))
					{
						node->depends_on(to_check_node->pass_name);
						break;
					}
				}
				for (auto &image : to_check_node->sampled_images)
				{
					if (node->sampled_images.contains(image.first))
					{
						node->depends_on(to_check_node->pass_name);
						break;
					}
				}
				for (auto &image : to_check_node->storage_images)
				{
					if (node->storage_images.contains(image.first))
					{
						node->depends_on(to_check_node->pass_name);
						break;
					}
				}
				for (auto &buffer : to_check_node->buffers)
				{
					if (node->buffers.contains(buffer.first))
					{
						node->depends_on(to_check_node->pass_name);
						break;
					}
				}
			}
		}
	}

	void execute_queue_batch(queue_nodes_batch &queue_nodes_batch)
	{
		std::vector<std::string> all_passes_names;
		int                      idx = 0;

		auto profiler = profiler::get_instance();
		for (auto &render_node : queue_nodes_batch.sorted_nodes)
		{
			if (!render_node->active)
			{
				continue;
			}

			profiler->add_profiler_cpu_spot(legit::Colors::getColor(idx), "Pass: " + render_node->pass_name);
			engine::render_graph_node *node = render_node;
			node->set_curr_cmd(queue_nodes_batch.command_buffer);
			auto       *queue_ref  = core->queue_worker_manager->get_worker_queue(node->worker_queue_name);
			bool        depen_need = false;
			std::string depen_name = "";
			for (auto &pass_name : all_passes_names)
			{
				if (node->dependencies.contains(pass_name))
				{
					depen_need = true;
					depen_name = pass_name;
				}
			}
			if (depen_need)
			{
				engine::render_graph_node *depen_node = render_nodes.at(depen_name).get();
				if (!depen_node->active)
				{
					profiler->end_profiler_cpu_spot("Pass: " + render_node->pass_name);
					systems::logger::get_instance()->log_message(
					    "Pass with name: (" + render_node->pass_name + ") needs (" + depen_name + ") and is not active");
					// if a dependency is not active we skip the node
					continue;
				}
				buffer_usage_types    last_node_type    = (depen_node->node_kind == render_node_type::graphics) ? b_graphics_write : b_compute_write;
				buffer_usage_types    curr_node_type    = (node->node_kind == render_node_type::graphics) ? b_graphics_write : b_compute_write;
				buffer_access_pattern last_node_pattern = get_src_buffer_access_pattern(last_node_type);
				buffer_access_pattern curr_node_pattern = get_src_buffer_access_pattern(curr_node_type);
				create_mem_barrier(last_node_pattern, curr_node_pattern, node->get_curr_cmd());
			}
			if (queue_ref->is_main_threat)
			{
				node->execute(node->get_curr_cmd());
			}
			all_passes_names.push_back(node->pass_name);
			profiler->end_profiler_cpu_spot("Pass: " + render_node->pass_name);
		}
	}
	void prepare_rendering()
	{
		profiler::get_instance()->add_profiler_cpu_spot(legit::Colors::belizeHole, "Rendergraph prepare cpu");
		resources_manager->update_buffers();
		resources_manager->update_images();
		for (auto node : sequential_render_nodes)
		{
			node->validate_node_type();
		}
		sorted_by_dep_nodes.clear();
		sorted_queue_batches.clear();
		resolve_nodes_dependancies(sequential_render_nodes);
		if (sequential_render_nodes.size() > 1)
		{
			get_node("BlitterNode")->depends_on(sequential_render_nodes[sequential_render_nodes.size() - 2]->pass_name);
		}
		sort_nodes_by_dep(sorted_by_dep_nodes);
		build_queue_passes_batches(sorted_by_dep_nodes, sorted_queue_batches);
		profiler::get_instance()->end_profiler_cpu_spot("Rendergraph prepare cpu");
	}
	void execute_rendering()
	{
		profiler::get_instance()->add_profiler_cpu_spot(legit::Colors::belizeHole, "Rendergraph execute cpu");
		assert(!sorted_queue_batches.empty() && "There must queues");
		for (auto &queue_batch : sorted_queue_batches)
		{
			execute_queue_batch(queue_batch);
		}

		profiler::get_instance()->end_profiler_cpu_spot("Rendergraph execute cpu");
	}
	void end_rendering()
	{
		core->queue_worker_manager->reset_pool_usage();
	}

	~render_graph()
	{
		for (auto &pi : cuda_pipelines)
		{
			pi.second->context->C_Shutdown();
		}
	};
};

}        // namespace engine

#endif        // RENDER_GRAPH_HPP
