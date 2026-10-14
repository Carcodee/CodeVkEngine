//

// Created by carlo on 2024-10-02.
//

#ifndef RENDER_GRAPH_HPP
#define RENDER_GRAPH_HPP
#define DUMMY_PC_SIZE 4

namespace engine
{
struct Buffer_Key
{
	Buffer_Usage_Types src_usage;
	Buffer_Usage_Types dst_usage;
	engine::Buffer          *buffer;
	std::string      name;
};
enum class Render_Node_Type
{
	None,
	Graphics,
	Compute,
	Cuda
};

struct Render_Node_Configs : systems::I_Serializable<Render_Node_Configs>
{
	bool automatic_cache = true;
	bool manual_add      = false;

	Render_Node_Configs(bool automatic_cache, bool manual_add = false)
	{
		this->automatic_cache = automatic_cache;
		this->manual_add      = manual_add;
	}

	nlohmann::json Serialize() override
	{
		nlohmann::json json;
		json["automaticCache"] = automatic_cache;
		json["manualAdd"]      = manual_add;

		return json;
	}
	Render_Node_Configs *Deserialize(std::string filename) override
	{
		return this;
	}
};

class Render_Graph;

struct Cuda_Pipeline
{
	CodeCuda::CodeCudaContext *context = nullptr;
	engine::Core                      *core    = nullptr;
	std::string                name    = "";

	Cuda_Pipeline() = default;
	engine::Cuda_Pipeline *C_Build_Cuda_Pipeline()
	{
		assert(context != nullptr);
		return this;
	}
	engine::Cuda_Pipeline *C_Export_Buffer(engine::Buffer *buffer)
	{
		assert(context);
		assert(buffer->Get_Buffer_Handle());
		context->C_ImportExternalBuffer(buffer->Get_Buffer_Handle(), buffer->device_size, buffer->stride);
		return this;
	}
	engine::Cuda_Pipeline *C_Create_Pipeline()
	{
		return this;
	}
	engine::Cuda_Pipeline *C_Set_Kernel_Function(std::function<void(CodeCuda::CodeCudaContext*)> funct)
	{
		context->C_SetKernelLauncherFunction(funct);
		return this;
	}
	engine::Cuda_Pipeline *C_Set_Pre_Cpu_Kernel_Function(std::function<void()> funct)
	{
		context->C_SetPreKernelFunction(funct);
		return this;
	}
	~Cuda_Pipeline() = default;
};
struct Gpu_Pipeline
{
	Gpu_Pipeline()  = default;
	~Gpu_Pipeline() = default;

	engine::Core                        *core;
	vk::UniquePipeline           pipeline;
	vk::UniquePipelineLayout     pipeline_layout;
	vk::UniquePipelineCache      pipeline_cache;
	vk::PipelineLayoutCreateInfo pipeline_layout_ci;
	vk::PushConstantRange        push_constant_range;
	vk::PipelineBindPoint        pipeline_type;

	Dynamic_Render_Pass                dynamic_render_pass;
	std::unique_ptr<Descriptor_Cache> desc_cache;

	engine::Graphics_Pipeline_Configs                                   pipeline_configs = {};
	engine::Vertex_Input                                               vertex_layout;
	engine::Attachment_Info                                            depth_attachment      = {};
	engine::Attachment_Info                                            depth_attachment_input = {};
	size_t                                                    push_constant_size     = DUMMY_PC_SIZE;
	Depth_Configs                                              depth_config          = D_None;
	Render_Node_Configs                                         configs              = {true, false};
	std::unordered_map<int, engine::Attachment_Info>           out_col_attachments_bindings;
	std::string                                               name;

	std::map<std::string, Shader *> shaders = {
	    {"frag", nullptr},
	    {"vert", nullptr},
	    {"comp", nullptr},
	    {"tesc", nullptr},
	    {"tese", nullptr},
	    {"geom", nullptr}};

	engine::Gpu_Pipeline *G_Recreate_Resources()
	{
		assert(&pipeline_layout_ci != nullptr && "Pipeline layout is null");
		assert(core != nullptr && "Core is null");

		pipeline.reset();
		pipeline_layout.reset();

		G_Reload_Shaders();

		Shader *vert_shader = shaders.at("vert");
		Shader *frag_shader = shaders.at("frag");
		Shader *comp_shader = shaders.at("comp");
		Shader *tesc_shader = shaders.at("tesc");
		Shader *tese_shader = shaders.at("tese");
		Shader *geom_shader = shaders.at("geom");

		std::map<Shader_Stage, Shader *> stages;
		G_Get_Shader_Stages(stages);

		if (frag_shader && vert_shader)
		{
			if (configs.automatic_cache)
			{
				desc_cache->Add_Shader_Info(vert_shader->s_parser.get());
				desc_cache->Add_Shader_Info(frag_shader->s_parser.get());

				vk::ShaderStageFlags stage_flags =
				    vk::ShaderStageFlagBits::eFragment |
				    vk::ShaderStageFlagBits::eVertex;

				if (tesc_shader)
				{
					desc_cache->Add_Shader_Info(tesc_shader->s_parser.get());

					if (!tese_shader)
					{
						assert(false && "Must have tese shader");
					}

					desc_cache->Add_Shader_Info(tese_shader->s_parser.get());

					stage_flags |=
					    vk::ShaderStageFlagBits::eTessellationControl |
					    vk::ShaderStageFlagBits::eTessellationEvaluation;
				}

				if (geom_shader)
				{
					desc_cache->Add_Shader_Info(geom_shader->s_parser.get());
					stage_flags |= vk::ShaderStageFlagBits::eGeometry;
				}

				desc_cache->Build_Descriptors_Cache(stage_flags);

				if (push_constant_size <= 0)
				{
					G_Set_Push_Constant_Size(DUMMY_PC_SIZE);
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

				G_Set_Pipeline_Layout_Ci(painting_layout_create_info);
			}
			std::vector<engine::Attachment_Info> col_attachments;
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

			dynamic_render_pass.Set_Pipeline_Rendering_Info(
			    col_attachments.size(),
			    color_formats,
			    depth_attachment.format);

			pipeline_layout =
			    core->logical_device->createPipelineLayoutUnique(pipeline_layout_ci);
			
			std::vector<Blend_Configs> blend_configs = {};
			blend_configs.resize(out_col_attachments_bindings.size());
			for (auto col_info : out_col_attachments_bindings)
			{
				blend_configs[col_info.first] = col_info.second.blend_configs;
			}

			std::unique_ptr<engine::Graphics_Pipeline> graphics_pipeline =
			    std::make_unique<engine::Graphics_Pipeline>(
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
				desc_cache->Add_Shader_Info(comp_shader->s_parser.get());

				desc_cache->Build_Descriptors_Cache(
				    vk::ShaderStageFlagBits::eCompute);

				if (push_constant_size <= 0)
				{
					G_Set_Push_Constant_Size(DUMMY_PC_SIZE);
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

				G_Set_Pipeline_Layout_Ci(painting_layout_create_info);
			}

			pipeline_layout =
			    core->logical_device->createPipelineLayoutUnique(pipeline_layout_ci);

			std::unique_ptr<engine::Compute_Pipeline> compute_pipeline =
			    std::make_unique<engine::Compute_Pipeline>(
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

	engine::Gpu_Pipeline *G_Build_Gpu_Pipeline()
	{
		assert(&pipeline_layout_ci != nullptr && "Pipeline layout is null");
		assert(core != nullptr && "Core is null");

		auto pipeline_cache_create_info = vk::PipelineCacheCreateInfo();

		pipeline_cache =
		    core->logical_device->createPipelineCacheUnique(
		        pipeline_cache_create_info);

		Shader *vert_shader = shaders.at("vert");
		Shader *frag_shader = shaders.at("frag");
		Shader *comp_shader = shaders.at("comp");
		Shader *tesc_shader = shaders.at("tesc");
		Shader *tese_shader = shaders.at("tese");
		Shader *geom_shader = shaders.at("geom");

		if (frag_shader && vert_shader)
		{
			if (configs.automatic_cache)
			{
				desc_cache.reset();
				desc_cache = std::make_unique<Descriptor_Cache>(core);

				desc_cache->Add_Shader_Info(vert_shader->s_parser.get());
				desc_cache->Add_Shader_Info(frag_shader->s_parser.get());

				vk::ShaderStageFlags stage_flags =
				    vk::ShaderStageFlagBits::eFragment |
				    vk::ShaderStageFlagBits::eVertex;

				if (tesc_shader)
				{
					desc_cache->Add_Shader_Info(tesc_shader->s_parser.get());

					if (!tese_shader)
					{
						assert(false && "Must have tese shader");
					}

					desc_cache->Add_Shader_Info(tese_shader->s_parser.get());

					stage_flags |=
					    vk::ShaderStageFlagBits::eTessellationControl |
					    vk::ShaderStageFlagBits::eTessellationEvaluation;
				}

				if (geom_shader)
				{
					desc_cache->Add_Shader_Info(geom_shader->s_parser.get());
					stage_flags |= vk::ShaderStageFlagBits::eGeometry;
				}

				desc_cache->Build_Descriptors_Cache(stage_flags);

				if (push_constant_size <= 0)
				{
					G_Set_Push_Constant_Size(DUMMY_PC_SIZE);
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

				G_Set_Pipeline_Layout_Ci(painting_layout_create_info);
			}

			std::vector<engine::Attachment_Info> col_attachments;
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

			dynamic_render_pass.Set_Pipeline_Rendering_Info(
			    col_attachments.size(),
			    color_formats,
			    depth_attachment.format);

			pipeline_layout =
			    core->logical_device->createPipelineLayoutUnique(pipeline_layout_ci);

			std::map<Shader_Stage, Shader *> stages;
			G_Get_Shader_Stages(stages);
			
			std::vector<Blend_Configs> blend_configs = {};
			blend_configs.resize(out_col_attachments_bindings.size());
			for (auto col_info : out_col_attachments_bindings)
			{
				blend_configs[col_info.first] = col_info.second.blend_configs;
			}
			std::unique_ptr<engine::Graphics_Pipeline> graphics_pipeline =
			    std::make_unique<engine::Graphics_Pipeline>(
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
				desc_cache = std::make_unique<Descriptor_Cache>(core);

				desc_cache->Add_Shader_Info(comp_shader->s_parser.get());

				desc_cache->Build_Descriptors_Cache(
				    vk::ShaderStageFlagBits::eCompute);

				if (push_constant_size <= 0)
				{
					G_Set_Push_Constant_Size(DUMMY_PC_SIZE);
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

				G_Set_Pipeline_Layout_Ci(painting_layout_create_info);
			}

			pipeline_layout =
			    core->logical_device->createPipelineLayoutUnique(pipeline_layout_ci);

			std::unique_ptr<engine::Compute_Pipeline> compute_pipeline =
			    std::make_unique<engine::Compute_Pipeline>(
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

	engine::Gpu_Pipeline *G_Set_Sampler(std::string name, engine::Image_View *image_view_handle, engine::Sampler *sampler = nullptr)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->Set_Sampler(name, image_view_handle, sampler);
		return this;
	}
	engine::Gpu_Pipeline *G_Set_Sampler_Array(std::string name, std::vector<engine::Image_View *> &image_views, std::vector<engine::Sampler *> *samplers = nullptr)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->Set_Sampler_Array(name, image_views, samplers);
		return this;
	}
	engine::Gpu_Pipeline *G_Set_Storage_Image(std::string name, engine::Image_View *image_view_handle, engine::Sampler *sampler = nullptr)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->Set_Storage_Image(name, image_view_handle, sampler);
		return this;
	}
	engine::Gpu_Pipeline *G_Set_Storage_Image_Array(std::string name, std::vector<engine::Image_View *> &image_views,
	                                  std::vector<engine::Sampler *> *samplers = nullptr)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->Set_Storage_Image_Array(name, image_views, samplers);
		return this;
	}
	template <typename Value_Type>
	engine::Gpu_Pipeline *G_Set_Buffer(std::string name, Value_Type &buffer_data)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->Set_Buffer<Value_Type>(name, buffer_data);
		return this;
	}
	template <typename Value_Type>
	engine::Gpu_Pipeline *G_Set_Buffer(std::string name, std::vector<Value_Type> &buffer_data)
	{
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->Set_Buffer<Value_Type>(name, buffer_data);
		return this;
	}
	engine::Gpu_Pipeline *G_Set_Buffer(std::string name, engine::Buffer *buffer_data)
	{
		assert(buffer_data != nullptr && "Buffer sets from here should never be nullptrs");
		assert(desc_cache && " Is not possible to set a shader value before building the node");
		desc_cache->Set_Buffer(name, buffer_data);
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Pipeline_Layout_Ci(vk::PipelineLayoutCreateInfo create_info)
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

	engine::Gpu_Pipeline *G_Set_Push_Constant_Size(size_t size)
	{
		push_constant_size = size;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Graphics_Pipeline_Configs(
	    engine::Graphics_Pipeline_Configs pipeline_configs)
	{
		this->pipeline_configs = pipeline_configs;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Depth_Config(Depth_Configs depht_config)
	{
		this->depth_config = depht_config;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Configs(Render_Node_Configs configs)
	{
		this->configs = configs;
		return this;
	}
	engine::Gpu_Pipeline *G_Set_Vertex_Input(engine::Vertex_Input vertex_layout)
	{
		this->vertex_layout = vertex_layout;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Vert_Shader(Shader *shader)
	{
		shaders.at("vert") = shader;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Frag_Shader(Shader *shader)
	{
		shaders.at("frag") = shader;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Comp_Shader(Shader *shader)
	{
		shaders.at("comp") = shader;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Tes_Control_Shader(Shader *shader)
	{
		shaders.at("tesc") = shader;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Tes_Eval_Shader(Shader *shader)
	{
		shaders.at("tese") = shader;
		return this;
	}

	engine::Gpu_Pipeline *G_Set_Geom_Shader(Shader *shader)
	{
		shaders.at("geom") = shader;
		return this;
	}

	bool G_Reload_Shaders()
	{
		Shader *vert_shader = shaders.at("vert");
		Shader *frag_shader = shaders.at("frag");
		Shader *comp_shader = shaders.at("comp");
		Shader *tesc_shader = shaders.at("tesc");
		Shader *tese_shader = shaders.at("tese");
		Shader *geom_shader = shaders.at("geom");

		// todo
		bool is_reloaded = false;
		if (vert_shader != nullptr && frag_shader != nullptr)
		{
			vert_shader->Reload();
			frag_shader->Reload();
		}

		if (tesc_shader != nullptr)
		{
			tesc_shader->Reload();
		}

		if (tese_shader != nullptr)
		{
			tese_shader->Reload();
		}

		if (geom_shader != nullptr)
		{
			geom_shader->Reload();
		}

		if (comp_shader != nullptr)
		{
			comp_shader->Reload();
		}
		return is_reloaded;
	}

	engine::Gpu_Pipeline *G_Get_Shader_Stages(std::map<Shader_Stage, Shader *> &stages)
	{
		Shader *vert_shader = shaders.at("vert");
		Shader *frag_shader = shaders.at("frag");
		Shader *comp_shader = shaders.at("comp");
		Shader *tesc_shader = shaders.at("tesc");
		Shader *tese_shader = shaders.at("tese");
		Shader *geom_shader = shaders.at("geom");

		if (vert_shader)
		{
			stages.try_emplace(S_Vert, vert_shader);
		}

		if (frag_shader)
		{
			stages.try_emplace(S_Frag, frag_shader);
		}

		if (comp_shader)
		{
			stages.try_emplace(S_Comp, comp_shader);
		}

		if (tesc_shader)
		{
			stages.try_emplace(S_Tess_Control, tesc_shader);
		}

		if (tese_shader)
		{
			stages.try_emplace(S_Tess_Eval, tese_shader);
		}

		if (geom_shader)
		{
			stages.try_emplace(S_Geom, geom_shader);
		}
		return this;
	}

	engine::Gpu_Pipeline *G_Add_Color_Attachment_Output(int binding, engine::Attachment_Info vk_attachment_info)
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

	engine::Gpu_Pipeline *G_Set_Depth_Attachment_Output(engine::Attachment_Info depth)
	{
		depth_attachment = depth;
		return this;
	}

	engine::Gpu_Pipeline *G_Reset()
	{
		pipeline.reset();
		pipeline_layout.reset();
		pipeline_cache.reset();
		desc_cache.reset();
		pipeline_layout_ci  = vk::PipelineLayoutCreateInfo();
		push_constant_range = vk::PushConstantRange();
		pipeline_type      = vk::PipelineBindPoint::eGraphics;

		dynamic_render_pass.Reset();
		pipeline_configs.rasterization_configs = R_Fill;
		pipeline_configs.topology_configs      = T_Triangle;
		depth_config = D_None;
		vertex_layout.binding_description.clear();
		vertex_layout.input_description.clear();
		push_constant_size = 4;
		depth_attachment = {};
		return this;
	}
};

struct Render_Graph_Node : systems::I_Serializable<engine::Render_Graph_Node>
{
	Render_Graph_Node() = default;

	nlohmann::json Serialize() override
	{
		nlohmann::json json;
		std::string    text     = json.dump(3);
		std::string    filename = systems::Os::Get_Instance()->Get_Engine_Resources_Path() + "\\RenderNodes\\pass_" + pass_name + ".json";
		if (!filename.empty())
		{
			systems::Os::Get_Instance()->Write_File(filename, text.c_str(), text.size());
		}
		return text;
	}

	engine::Render_Graph_Node *Deserialize(std::string filename) override
	{
		if (!std::filesystem::exists(filename))
		{
			systems::Logger::Get_Instance()->Log_Message(
			    "Pass with name: (" + pass_name + ") does not have a serialized file, the pass may be null");
			return this;
		}
		return this;
	}
	engine::Render_Graph_Node *G_Recreate_Resources()
	{
		gpu_pipeline_ref->G_Recreate_Resources();
		return this;
	}

	engine::Render_Graph_Node *Build_Render_Graph_Node()
	{
		if (gpu_pipeline_ref)
		{
			gpu_pipeline_ref->G_Build_Gpu_Pipeline();
			if (gpu_pipeline_ref->pipeline_type == vk::PipelineBindPoint::eGraphics)
			{
				node_kind = Render_Node_Type::Graphics;
			}
			else
			{
				node_kind = Render_Node_Type::Compute;
			}
		}
		else
		{
			node_kind = Render_Node_Type::Cuda;
			cuda_pipeline->C_Build_Cuda_Pipeline();
		}
		return this;
	}
	
	engine::Render_Graph_Node* C_Set_Kernel_Function(std::function<void(CodeCuda::CodeCudaContext*)> funct)
	{
		cuda_pipeline->C_Set_Kernel_Function(std::move(funct));
		return this;
	}
	
	engine::Render_Graph_Node* C_Set_Pre_Kernel_Function(std::function<void()> funct)
	{
		cuda_pipeline->C_Set_Pre_Cpu_Kernel_Function(std::move(funct));
		return this;
	}


	engine::Render_Graph_Node *Enqueue_Node()
	{
		return this;
	}

	vk::PipelineBindPoint G_Get_Pipeline_Type()
	{
		assert(cuda_pipeline == nullptr && "this call should not happen in a cuda pipeline");
		return gpu_pipeline_ref->pipeline_type;
	}

	engine::Render_Graph_Node *Transition_Images(vk::CommandBuffer command_buffer)
	{
		for (auto &storage_image : storage_images)
		{
			Layout_Patterns dst_pattern = Empty;
			switch (node_kind)
			{
				case Render_Node_Type::Graphics:
					dst_pattern = Graphics_Write;
					break;
				case Render_Node_Type::Compute:
					dst_pattern = Compute_Write;
					break;
				case Render_Node_Type::Cuda:
					dst_pattern = Compute_Write;
					break;
				default:
					assert(false && "pipeline type is unknown");
			}
			if (Is_Image_Transition_Needed(storage_image.second->image_metadata->current_layout, dst_pattern))
			{
				Transition_Image(storage_image.second->image_metadata, dst_pattern,
				                storage_image.second->Get_Subresource_Range(),
				                command_buffer);
			}
		}
		for (auto &sampler : sampled_images)
		{
			Layout_Patterns dst_pattern = Empty;
			switch (node_kind)
			{
				case Render_Node_Type::Graphics:
					dst_pattern = Graphics_Read;
					break;
				case Render_Node_Type::Compute:
					dst_pattern = Compute;
					break;
				case Render_Node_Type::Cuda:
					dst_pattern = Compute;
					break;
				default:
					assert(false && "pipeline type is unknown");
			}
			// TODO: Better transition for sampler images
			//  if (Is_Image_Transition_Needed(sampler.second->image_metadata->current_layout, dst_pattern))
			//  {
			//  }
			Transition_Image(sampler.second->image_metadata, dst_pattern, sampler.second->Get_Subresource_Range(),
			                command_buffer);
		}
		return this;
	}

	engine::Render_Graph_Node *Sync_Buffers(vk::CommandBuffer command_buffer)
	{
		for (auto &pair : buffers)
		{
			Buffer_Key           buffer     = pair.second;
			Buffer_Access_Pattern src_pattern = Get_Src_Buffer_Access_Pattern(buffer.src_usage);
			Buffer_Access_Pattern dst_pattern = Get_Src_Buffer_Access_Pattern(buffer.dst_usage);
			Create_Mem_Barrier(src_pattern, dst_pattern, command_buffer);
		}
		return this;
	}
	engine::Render_Graph_Node *G_Set_Sampler(std::string name, engine::Image_View *image_view_handle, engine::Sampler *sampler = nullptr)
	{
		gpu_pipeline_ref->G_Set_Sampler(name, image_view_handle, sampler);
		return this;
	}
	engine::Render_Graph_Node *G_Set_Sampler_Array(std::string name, std::vector<engine::Image_View *> &image_views, std::vector<engine::Sampler *> *samplers = nullptr)
	{
		gpu_pipeline_ref->G_Set_Sampler_Array(name, image_views, samplers);
		return this;
	}
	engine::Render_Graph_Node *G_Set_Storage_Image(std::string name, engine::Image_View *image_view_handle, engine::Sampler *sampler = nullptr)
	{
		gpu_pipeline_ref->G_Set_Storage_Image(name, image_view_handle, sampler);
		return this;
	}
	engine::Render_Graph_Node *G_Set_Storage_Image_Array(std::string name, std::vector<engine::Image_View *> &image_views,
	                                      std::vector<engine::Sampler *> *samplers = nullptr)
	{
		gpu_pipeline_ref->G_Set_Storage_Image_Array(name, image_views, samplers);
		return this;
	}
	template <typename Value_Type>
	engine::Render_Graph_Node *G_Set_Buffer(std::string name, Value_Type &buffer_data)
	{
		gpu_pipeline_ref->G_Set_Buffer<Value_Type>(name, buffer_data);
		return this;
	}
	template <typename Value_Type>
	engine::Render_Graph_Node *G_Set_Buffer(std::string name, std::vector<Value_Type> &buffer_data)
	{
		gpu_pipeline_ref->G_Set_Buffer<Value_Type>(name, buffer_data);
		return this;
	}
	engine::Render_Graph_Node *G_Set_Buffer(std::string name, engine::Buffer *buffer_data)
	{
		assert(buffer_data != nullptr && "Buffer sets from here should never be nullptrs");
		gpu_pipeline_ref->G_Set_Buffer(name, buffer_data);
		return this;
	}
	engine::Render_Graph_Node *G_Reload_Shaders()
	{
		gpu_pipeline_ref->G_Reload_Shaders();
		return this;
	}
	engine::Render_Graph_Node *Validate_Node_Type()
	{
		assert(!(gpu_pipeline_ref && cuda_pipeline) && "you only can have either gpu or cuda pipeline not both");
		if (node_kind != Render_Node_Type::None)
		{
			return this;
		}
		if (gpu_pipeline_ref == nullptr)
		{
			node_kind = Render_Node_Type::Cuda;
			return this;
		}
		if (gpu_pipeline_ref->pipeline_type == vk::PipelineBindPoint::eCompute)
		{
			node_kind = Render_Node_Type::Compute;
		}
		else
		{
			node_kind = Render_Node_Type::Graphics;
		}
		return this;
	}

	engine::Render_Graph_Node *G_Execute_Pass(vk::CommandBuffer command_buffer)
	{
		assert(images_attachment_outputs.size() == gpu_pipeline_ref->out_col_attachments_bindings.size() && "Not all color attachments were set");
		assert(!images_attachment_outputs.empty() && "No color attachaments were set");
		assert(!(gpu_pipeline_ref->depth_attachment.format != vk::Format::eUndefined && depth_image == nullptr) && "there is no depth attachment set");

		G_Set_Framebuffer_Size(images_attachment_outputs.at(0)->image_metadata->Get_Image_Size());
		gpu_pipeline_ref->dynamic_render_pass.Set_Viewport(frame_buffer_size, frame_buffer_size);
		command_buffer.setViewport(0, 1, &gpu_pipeline_ref->dynamic_render_pass.viewport);
		command_buffer.setScissor(0, 1, &gpu_pipeline_ref->dynamic_render_pass.scissor);

		std::vector<vk::RenderingAttachmentInfo> attachment_infos;
		attachment_infos.resize(gpu_pipeline_ref->out_col_attachments_bindings.size());
		
		for (auto &image_pair : images_attachment_outputs)
		{
			if (Is_Image_Transition_Needed(image_pair.second->image_metadata->current_layout, Color_Attachment))
			{
				Transition_Image(image_pair.second->image_metadata, Color_Attachment,
				                image_pair.second->Get_Subresource_Range(), command_buffer);
			}
			attachment_infos[image_pair.first] = (gpu_pipeline_ref->out_col_attachments_bindings[image_pair.first].vk_attachment_info);
			attachment_infos[image_pair.first].setImageView(image_pair.second->image_view_handle.get());
			attachment_infos[image_pair.first].imageLayout = image_pair.second->image_metadata->current_pattern.layout;
		}
		if (depth_image)
		{
			if (depth_image->image_metadata->current_layout != Depth_Attachment)
			{
				Transition_Image(depth_image->image_metadata, Depth_Attachment, depth_image->Get_Subresource_Range(),
				                command_buffer);
			}
		}

		Transition_Images(command_buffer);
		Sync_Buffers(command_buffer);
		if (depth_image != nullptr)
		{
			gpu_pipeline_ref->depth_attachment.vk_attachment_info.imageView = depth_image->image_view_handle.get();
		}
		gpu_pipeline_ref->dynamic_render_pass.Set_Render_Info(attachment_infos, frame_buffer_size, &gpu_pipeline_ref->depth_attachment.vk_attachment_info);

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

	engine::Render_Graph_Node *G_Execute_Compute(vk::CommandBuffer command_buffer)
	{
		Transition_Images(command_buffer);
		Sync_Buffers(command_buffer);
		command_buffer.bindPipeline(gpu_pipeline_ref->pipeline_type, gpu_pipeline_ref->pipeline.get());
		command_buffer.bindDescriptorSets(gpu_pipeline_ref->pipeline_type,
		                                 gpu_pipeline_ref->pipeline_layout.get(), 0,
		                                 1,
		                                 &gpu_pipeline_ref->desc_cache->dst_set, 0, nullptr);
		(*render_operations)();
		return this;
	}

	engine::Render_Graph_Node *C_Execute(vk::CommandBuffer command_buffer)
	{
		Transition_Images(command_buffer);
		Sync_Buffers(command_buffer);
		assert(this->cuda_pipeline);
		this->cuda_pipeline->context->C_ExecuteCPU();
		return this;
	}

	engine::Render_Graph_Node *Execute(vk::CommandBuffer command_buffer)
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
			case Render_Node_Type::Graphics:
				G_Execute_Pass(command_buffer);
				break;
			case Render_Node_Type::Compute:
				G_Execute_Compute(command_buffer);
				break;
			case Render_Node_Type::Cuda:
				C_Execute(command_buffer);
				break;
			default:
				assert(false && "Unsuported pipeline type");
				break;
		}
		return this;
	}

	engine::Render_Graph_Node *G_Set_Vertex_Input(engine::Vertex_Input vertex_layout)
	{
		this->gpu_pipeline_ref->G_Set_Vertex_Input(vertex_layout);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Framebuffer_Size(glm::uvec2 size)
	{
		this->frame_buffer_size = size;
		return this;
	}

	engine::Render_Graph_Node *G_Set_Render_Operation(std::function<void()> *render_operations)
	{
		assert(gpu_pipeline_ref != nullptr && "Render operations are only for gpu pipelines");
		if (this->render_operations)
		{
			delete (this->render_operations);
		}
		this->render_operations = render_operations;
		return this;
	}

	engine::Render_Graph_Node *Add_Pre_Rendering_Task(std::function<void()> *task)
	{
		this->pre_rendering_tasks.push_back(task);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Pipeline_Layout_Ci(vk::PipelineLayoutCreateInfo create_info)
	{
		gpu_pipeline_ref->G_Set_Pipeline_Layout_Ci(create_info);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Push_Constant_Size(size_t size)
	{
		gpu_pipeline_ref->G_Set_Push_Constant_Size(size);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Graphics_Pipeline_Configs(engine::Graphics_Pipeline_Configs pipeline_configs)
	{
		gpu_pipeline_ref->G_Set_Graphics_Pipeline_Configs(pipeline_configs);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Depth_Config(Depth_Configs depht_config)
	{
		gpu_pipeline_ref->G_Set_Depth_Config(depht_config);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Vert_Shader(Shader *shader)
	{
		gpu_pipeline_ref->G_Set_Vert_Shader(shader);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Frag_Shader(Shader *shader)
	{
		gpu_pipeline_ref->G_Set_Frag_Shader(shader);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Comp_Shader(Shader *shader)
	{
		gpu_pipeline_ref->G_Set_Comp_Shader(shader);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Tes_Control_Shader(Shader *shader)
	{
		gpu_pipeline_ref->G_Set_Tes_Control_Shader(shader);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Tes_Eval_Shader(Shader *shader)
	{
		gpu_pipeline_ref->G_Set_Tes_Eval_Shader(shader);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Geom_Shader(Shader *shader)
	{
		gpu_pipeline_ref->G_Set_Geom_Shader(shader);
		return this;
	}

	engine::Render_Graph_Node *G_Add_Color_Attachment_Output(int binding, engine::Attachment_Info vk_attachment_info)
	{
		gpu_pipeline_ref->G_Add_Color_Attachment_Output(binding, vk_attachment_info);
		return this;
	}

	engine::Render_Graph_Node *G_Set_Depth_Attachment_Output(engine::Attachment_Info depth)
	{
		gpu_pipeline_ref->G_Set_Depth_Attachment_Output(depth);
		return this;
	}
	engine::Render_Graph_Node *G_Set_Depth_Image_Resource(engine::Image_View *image_view_handle)
	{
		this->depth_image = image_view_handle;
		return this;
	}

	engine::Render_Graph_Node *Activate_Node(bool value)
	{
		this->active = value;
		return this;
	}

	engine::Render_Graph_Node *G_Set_Color_Image_Attachment_Binding(int binding, engine::Image_View *image_view_handle)
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

	engine::Render_Graph_Node *G_Add_Sampler_Resource(engine::Image_View *image_view_handle)
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

	engine::Render_Graph_Node *G_Add_Storage_Resource(engine::Image_View *image_view_handle)
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

	engine::Render_Graph_Node *Add_Buffer_Sync(Buffer_Key buffer)
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

	
	engine::Render_Graph_Node *Depends_On(engine::Render_Graph_Node* node)
	{
		if (!dependencies.contains(node->pass_name))
		{
			dependencies.insert(node->pass_name);
			systems::Logger::Get_Instance()->Log_Message("Renderpass: (" + this->pass_name + ") Depends On-> (" + node->pass_name + ")");
		}
		return this;
	}
	engine::Render_Graph_Node *Depends_On(std::string dependency)
	{
		if (!dependencies.contains(dependency))
		{
			dependencies.insert(dependency);
			systems::Logger::Get_Instance()->Log_Message("Renderpass: (" + this->pass_name + ") Depends On-> (" + dependency + ")");
		}
		return this;
	}

	engine::Render_Graph_Node *Clear_Operations()
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
	engine::Render_Graph_Node *Set_Curr_Cmd(vk::CommandBuffer cmd)
	{
		curr_cmd = cmd;
		return this;
	}
	vk::CommandBuffer &Get_Curr_Cmd()
	{
		assert(curr_cmd != nullptr && "Cmd must be valid");
		return curr_cmd;
	}
	engine::Render_Graph_Node *G_Set_Configs(Render_Node_Configs configs)
	{
		gpu_pipeline_ref->G_Set_Configs(configs);
		return this;
	}
	engine::Render_Graph_Node *G_Reset()
	{
		gpu_pipeline_ref->G_Reset();
		active          = false;
		frame_buffer_size = {0, 0};
		worker_queue_name = "Graphics";
		curr_cmd         = nullptr;
		depth_image      = nullptr;

		images_attachment_outputs.clear();
		storage_images.clear();
		sampled_images.clear();
		buffers.clear();
		Clear_Operations();
		dependencies.clear();
		return this;
	}

	// unused

	engine::Gpu_Pipeline  *gpu_pipeline_ref = nullptr;
	engine::Cuda_Pipeline *cuda_pipeline   = nullptr;

	bool           wait_for_resources_creation = false;
	Render_Node_Type node_kind                 = Render_Node_Type::None;
	std::string    path;
	std::string    pass_name;
	std::string    worker_queue_name = "Graphics";
	bool           active          = false;

	std::set<std::string> dependencies;

	const std::unordered_map<std::string, int> &Get_Image_Attachment_Names() const
	{
		return image_attachments_names;
	}

	const std::unordered_map<int, engine::Image_View *> &Get_Image_Attachment_Outputs() const
	{
		return images_attachment_outputs;
	}

	const std::unordered_map<std::string, engine::Image_View *> &Get_Storage_Images() const
	{
		return storage_images;
	}

	const std::unordered_map<std::string, engine::Image_View *> &Get_Sampled_Images() const
	{
		return sampled_images;
	}

	const std::unordered_map<std::string, Buffer_Key> &Get_Buffers() const
	{
		return buffers;
	}

	engine::Image_View *Get_Depth_Image() const
	{
		return depth_image;
	}

	glm::uvec2 Get_Frame_Buffer_Size() const
	{
		return frame_buffer_size;
	}

  private:
	friend class Render_Graph;

	engine::Core             *core;
	engine::Image_View        *depth_image = nullptr;
	vk::CommandBuffer curr_cmd;

	glm::uvec2                      frame_buffer_size = {0, 0};
	std::vector<engine::Render_Graph_Node *> *nodes_to_execute_ref;

	std::unordered_map<std::string, int> image_attachments_names;
	std::unordered_map<int, engine::Image_View *>             images_attachment_outputs;

	std::unordered_map<std::string, engine::Image_View *> storage_images;
	std::unordered_map<std::string, engine::Image_View *> sampled_images;
	std::unordered_map<std::string, Buffer_Key>   buffers;

	std::function<void()>               *render_operations = nullptr;
	std::vector<std::function<void()> *> pre_rendering_tasks;

	Resources_Manager *res_manager_ref;
};
struct Queue_Nodes_Batch
{
	std::vector<engine::Render_Graph_Node *> sorted_nodes;
	std::string                    queue_name;
	vk::CommandBuffer              command_buffer;
	int                            id;
	int                            pool_id_used;
	void                           Execute_Cuda(engine::Core *core)
	{
		auto queue_ref = core->queue_worker_manager->Get_Worker_Queue(queue_name);
		for (int i = 0; i < sorted_nodes.size(); ++i)
		{
			auto &node = sorted_nodes[i];
			node->cuda_pipeline->context->C_WaitExternalSemaphore(queue_ref->timeline_value);
			node->cuda_pipeline->context->C_ExecuteKernel();
			node->cuda_pipeline->context->C_SignalExternalSemaphore(++queue_ref->timeline_value);
		}
	}
};

class Render_Graph
{
  public:
	engine::Core             *core;
	Resources_Manager *resources_manager;

	engine::Image_View *current_back_buffer_swapchain;
	// this is the backbuffer that will be blitted to the swapchain
	engine::Image_View *current_back_buffer;
	size_t     frame_index;
	bool       debug_ui = true;

	std::unordered_map<std::string, std::unique_ptr<engine::Gpu_Pipeline>>     gpu_pipelines;
	std::unordered_map<std::string, std::unique_ptr<engine::Cuda_Pipeline>>    cuda_pipelines;
	std::unordered_map<std::string, std::unique_ptr<engine::Render_Graph_Node>> render_nodes;
	// todo
	std::vector<engine::Render_Graph_Node *> nodes_to_execute;
	std::vector<engine::Render_Graph_Node *> sequential_render_nodes;
	std::vector<engine::Render_Graph_Node *> sorted_by_dep_nodes;

	std::vector<Queue_Nodes_Batch>                                      sorted_queue_batches;

	Render_Graph(engine::Core *core)
	{
		this->core = core;
	}

	void Serialize_All()
	{
		for (auto node : sequential_render_nodes)
		{
			node->Serialize();
		}
	}

	engine::Gpu_Pipeline *Get_Template_Gpu_Pipeline(std::string name, std::string shader_name, Shader_Compiler compiler)
	{
		Shader *v_shader     = resources_manager->Get_Or_Create_Default_Shader(shader_name, Shader_Stage::S_Vert, compiler);
		Shader *f_shader     = resources_manager->Get_Or_Create_Default_Shader(shader_name, Shader_Stage::S_Frag, compiler);
		auto    gpu_pipeline = Add_Gpu_Pipeline(name);
		gpu_pipeline->G_Set_Configs({true});
		gpu_pipeline->G_Set_Vert_Shader(v_shader);
		gpu_pipeline->G_Set_Frag_Shader(f_shader);
		gpu_pipeline->G_Set_Vertex_Input(Vertex_2d::Get_Vertex_Input());
		return gpu_pipeline;
	}

	engine::Render_Graph_Node *Get_Template_Node_Df(std::string name, std::string shader_name, Shader_Compiler compiler)
	{
		Shader *v_shader    = resources_manager->Get_Or_Create_Default_Shader(shader_name, Shader_Stage::S_Vert, compiler);
		Shader *f_shader    = resources_manager->Get_Or_Create_Default_Shader(shader_name, Shader_Stage::S_Frag, compiler);
		auto    render_node = Add_Pass(name);
		render_node->G_Set_Configs({true});
		render_node->G_Set_Vert_Shader(v_shader);
		render_node->G_Set_Frag_Shader(f_shader);
		render_node->G_Set_Vertex_Input(Vertex_2d::Get_Vertex_Input());
		return render_node;
	}

	engine::Render_Graph_Node *Get_Template_Compute_Node_Df(std::string name, std::string shader_name, Shader_Compiler compiler)
	{
		Shader *shader     = resources_manager->Get_Or_Create_Default_Shader(shader_name, Shader_Stage::S_Comp, compiler);
		auto    render_node = Add_Pass(name);
		render_node->G_Set_Configs({true});
		render_node->G_Set_Comp_Shader(shader);
		return render_node;
	}
	engine::Render_Graph_Node *Get_Template_Node(std::string name, std::string v_path, std::string f_path)
	{
		engine::Attachment_Info col_info = Get_Color_Attachment_Info(
		    Blend_Configs::B_Opaque, glm::vec4(0.0f), g_32b_format);
		Shader *v_shader    = resources_manager->Get_Shader(v_path, Shader_Stage::S_Vert);
		Shader *f_shader    = resources_manager->Get_Shader(f_path, Shader_Stage::S_Frag);
		auto    render_node = Add_Pass(name);
		render_node->G_Set_Configs({true});
		render_node->G_Set_Vert_Shader(v_shader);
		render_node->G_Set_Frag_Shader(f_shader);
		render_node->G_Set_Vertex_Input(Vertex_2d::Get_Vertex_Input());
		// change this
		render_node->G_Set_Push_Constant_Size(4);
		return render_node;
	}

	engine::Render_Graph_Node *Get_Template_Compute_Node(std::string name, std::string path)
	{
		Shader *shader     = resources_manager->Get_Shader(path, Shader_Stage::S_Comp);
		auto    render_node = Add_Pass(name);
		render_node->G_Set_Configs({true});
		render_node->G_Set_Comp_Shader(shader);
		// change this
		render_node->G_Set_Push_Constant_Size(4);
		return render_node;
	}
	void Create_Utility_Shaders()
	{
		auto          *blitter_shader = Get_Template_Gpu_Pipeline("BlitterPipeline", "Blitter", Shader_Compiler::C_Glsl);
		engine::Attachment_Info col_info       = Get_Color_Attachment_Info(B_Opaque,
            glm::vec4(0.0f), core->swapchain_ref->Get_Format());
		blitter_shader->G_Add_Color_Attachment_Output(0, col_info);
		blitter_shader->G_Build_Gpu_Pipeline();
	}
	engine::Gpu_Pipeline *Get_Blitter_Shader()
	{
		auto *blitter_shader = Get_Gpu_Pipeline("BlitterPipeline");
		return blitter_shader;
	}
	engine::Render_Graph_Node *Blit_To_Backbuffer()
	{
		auto *blitter_node = Add_Pass(Get_Blitter_Shader(), "BlitterNode");

		blitter_node->Build_Render_Graph_Node();
		blitter_node->Enqueue_Node();

		Get_Node("BlitterNode")->G_Set_Sampler("main_tex", current_back_buffer);
		blitter_node->Add_Pre_Rendering_Task( new std::function<void()>([this]() {
			// there is one thing that I may change and is that when I set the sampler from outside of the task
			// function and I reload the shaders the ref of the sampler is gone because I reset the descriptor set, I should change that
			Get_Node("BlitterNode")->G_Set_Color_Image_Attachment_Binding(0,current_back_buffer_swapchain);
			Get_Node("BlitterNode")->G_Set_Framebuffer_Size(current_back_buffer_swapchain->image_metadata->Get_Image_Size());
		}));
		blitter_node->G_Set_Render_Operation(new std::function<void()>(
		    [this]() {
			    auto           render_node = Get_Node("BlitterNode");
			    vk::DeviceSize offset     = 0;
			    render_node->Get_Curr_Cmd().bindVertexBuffers(
			        0, 1,
			        &resources_manager->Get_Staged_Buff_From_Name("quad_default")->device_buffer->buffer_handle.get(),
			        &offset);
			    render_node->Get_Curr_Cmd().bindIndexBuffer(
			        resources_manager->Get_Staged_Buff_From_Name("quad_index_default")->device_buffer->buffer_handle.get(), 0, vk::IndexType::eUint32);

			    render_node->Get_Curr_Cmd().drawIndexed(Vertex_2d::Get_Quad_Indices().size(), 1, 0,
			                                         0, 0);
		    }));
		return blitter_node;
	}

	void Load_External_Passes()
	{
		std::string              nodes_path = systems::Os::Get_Instance()->Get_Engine_Resources_Path() + "\\RenderNodes";
		std::vector<std::string> paths;
		assert(std::filesystem::exists(nodes_path));
		for (auto path : std::filesystem::directory_iterator(nodes_path))
		{
			size_t      pos      = path.path().string().find('_');
			std::string pass_name = path.path().string().substr(pos + 1, path.path().string().length());
			size_t      ext_pos   = pass_name.find_last_of('.');
			pass_name             = pass_name.substr(0, ext_pos);

			engine::Render_Graph_Node *node = Add_Pass(pass_name);
			if (!render_nodes.contains(node->pass_name))
			{
				node->Deserialize(path.path().string());
			}
			else
			{
				systems::Logger::Get_Instance()->Log_Message("Render Node already is in ram");
				node->Deserialize(path.path().string());
			}
		}
		Recreate_Node_Pipelines();
	}
	void Enqueue_Node(engine::Render_Graph_Node *node)
	{
		nodes_to_execute.push_back(node);
	}
	void Update_All_From_Meta_Data()
	{
		for (auto &node : sequential_render_nodes)
		{
			node->Deserialize(node->path);
			systems::Logger::Get_Instance()->Log_Message("Render Node: (" + node->pass_name + ") updated");
		}
	}

	void Create_Res_Manager()
	{
		resources_manager  = Resources_Manager::Get_Instance();
		current_back_buffer = resources_manager->Get_Image_View_From_Name("bf");
	}

	engine::Render_Graph_Node *Get_Node(std::string name)
	{
		if (render_nodes.contains(name))
		{
			return render_nodes.at(name).get();
		}
		else
		{
			Print_Invalid_Resource("Renderpass", name);
			return nullptr;
		}
	}

	engine::Gpu_Pipeline *Get_Gpu_Pipeline(std::string name)
	{
		if (!gpu_pipelines.contains(name))
		{
			return nullptr;
		}
		auto gpu_pipeline = gpu_pipelines.at(name).get();
		assert(gpu_pipeline != nullptr && "gpu pipeline is null");
		return gpu_pipeline;
	}

	engine::Cuda_Pipeline *Get_Cuda_Pipeline(std::string name)
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
	engine::Gpu_Pipeline *Add_Gpu_Pipeline(std::string name)
	{
		if (!gpu_pipelines.contains(name))
		{
			auto gpu_pipeline             = std::make_unique<engine::Gpu_Pipeline>();
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
	engine::Cuda_Pipeline *Add_Cuda_Pipeline(std::string name)
	{
		if (!cuda_pipelines.contains(name))
		{
			auto cuda_pipeline     = std::make_unique<engine::Cuda_Pipeline>();
			auto queue_ref         = core->queue_worker_manager->Get_Worker_Queue("CUDA");
			cuda_pipeline->context = new CodeCuda::CodeCudaContext();
			cuda_pipeline->context->C_InitFromExternalDevice(core->device_uuid.data(), VK_UUID_SIZE);
			cuda_pipeline->context->C_ImportExternalSemaphore(queue_ref->Get_Exportable_Handle());
			cuda_pipeline->name = name;
			cuda_pipelines.try_emplace(name, std::move(cuda_pipeline));
			return cuda_pipelines.at(name).get();
		}

		auto cuda_pipeline = cuda_pipelines.at(name).get();
		assert(cuda_pipeline != nullptr && "Pipeline is null");
		return cuda_pipeline;
	}

	engine::Render_Graph_Node *Add_Cuda_Pass(engine::Cuda_Pipeline *pipeline, std::string name, std::string worker_queue_name = "CUDA")
	{
		if (!render_nodes.contains(name))
		{
			assert(pipeline);
			if (!cuda_pipelines.contains(pipeline->name))
			{
				Add_Cuda_Pipeline("CUDA_PIPELINE_"+name);
				systems::Logger::Get_Instance()->Log("CUDA Pipeline was created (CUDA_PIPELINE_"+name + ")");
			}
			auto render_graph_node               = std::make_unique<engine::Render_Graph_Node>();
			render_graph_node->cuda_pipeline      = pipeline;
			render_graph_node->pass_name          = name;
			render_graph_node->core              = core;
			render_graph_node->nodes_to_execute_ref = &nodes_to_execute;
			render_graph_node->res_manager_ref     = resources_manager;
			render_graph_node->worker_queue_name   = worker_queue_name;
			render_graph_node->active            = true;
			render_graph_node->path              = systems::Os::Get_Instance()->Get_Engine_Resources_Path() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			render_nodes.try_emplace(name, std::move(render_graph_node));
			sequential_render_nodes.push_back(render_nodes.at(name).get());
			return render_nodes.at(name).get();
		}
		return render_nodes.at(name).get();
	}
	engine::Render_Graph_Node *Add_Cuda_Pass(std::string name, std::string worker_queue_name = "CUDA")
	{
		if (!render_nodes.contains(name))
		{
			auto render_graph_node               = std::make_unique<engine::Render_Graph_Node>();
			render_graph_node->cuda_pipeline      = Add_Cuda_Pipeline("CUDA_PIPELINE_" + name);
			render_graph_node->pass_name          = name;
			render_graph_node->core              = core;
			render_graph_node->nodes_to_execute_ref = &nodes_to_execute;
			render_graph_node->res_manager_ref     = resources_manager;
			render_graph_node->worker_queue_name   = worker_queue_name;
			render_graph_node->active            = true;
			render_graph_node->path              = systems::Os::Get_Instance()->Get_Engine_Resources_Path() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			render_nodes.try_emplace(name, std::move(render_graph_node));
			sequential_render_nodes.push_back(render_nodes.at(name).get());
			return render_nodes.at(name).get();
		}
		return render_nodes.at(name).get();
	}

	engine::Render_Graph_Node *Add_Pass(engine::Gpu_Pipeline *gpu_pipeline, std::string name, std::string worker_queue_name = "Graphics")
	{
		if (!render_nodes.contains(name))
		{
			assert(gpu_pipeline != nullptr && "Pipeline is null");
			auto render_graph_node               = std::make_unique<engine::Render_Graph_Node>();
			render_graph_node->gpu_pipeline_ref    = gpu_pipeline;
			render_graph_node->pass_name          = name;
			render_graph_node->core              = core;
			render_graph_node->nodes_to_execute_ref = &nodes_to_execute;
			render_graph_node->res_manager_ref     = resources_manager;
			render_graph_node->worker_queue_name   = worker_queue_name;
			render_graph_node->active            = true;
			render_graph_node->path              = systems::Os::Get_Instance()->Get_Engine_Resources_Path() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			render_nodes.try_emplace(name, std::move(render_graph_node));
			sequential_render_nodes.push_back(render_nodes.at(name).get());
			return render_nodes.at(name).get();
		}
		return render_nodes.at(name).get();
	}

	engine::Render_Graph_Node *Add_Pass(std::string name, std::string worker_queue_name = "Graphics")
	{
		if (!render_nodes.contains(name))
		{
			auto gpu_pipeline = Add_Gpu_Pipeline("GPUPipeline_" + name);
			assert(gpu_pipeline != nullptr && "GPUPipeline_ is null");
			auto render_graph_node               = std::make_unique<engine::Render_Graph_Node>();
			render_graph_node->gpu_pipeline_ref    = gpu_pipeline;
			render_graph_node->pass_name          = name;
			render_graph_node->core              = core;
			render_graph_node->nodes_to_execute_ref = &nodes_to_execute;
			render_graph_node->res_manager_ref     = resources_manager;
			render_graph_node->worker_queue_name   = worker_queue_name;
			render_graph_node->active            = true;
			render_graph_node->path              = systems::Os::Get_Instance()->Get_Engine_Resources_Path() + "\\RenderNodes\\pass_" +
			                        name + ".json";
			render_nodes.try_emplace(name, std::move(render_graph_node));
			sequential_render_nodes.push_back(render_nodes.at(name).get());
			return render_nodes.at(name).get();
		}
		return render_nodes.at(name).get();
	}

	engine::Image_View *Add_Sampler_Resource(std::string pass_name, engine::Image_View *image_view_handle)
	{
		assert(image_view_handle && "ImageView is null");
		if (render_nodes.contains(pass_name))
		{
			render_nodes.at(pass_name)->G_Add_Sampler_Resource(image_view_handle);
		}
		else
		{
			std::cout << "Renderpass: " << pass_name << " does not exist, saving the image anyways. \n";
		}
		return image_view_handle;
	}

	engine::Image_View *Add_Storage_Resource(std::string pass_name, engine::Image_View *image_view_handle)
	{
		assert(image_view_handle && "ImageView is null");
		if (render_nodes.contains(pass_name))
		{
			render_nodes.at(pass_name)->G_Add_Storage_Resource(image_view_handle);
		}
		else
		{
			std::cout << "Renderpass: " << pass_name << " does not exist, saving the image anyways. \n";
		}
		return image_view_handle;
	}

	Buffer_Key &Add_Buffer_Sync(std::string pass_name, Buffer_Key buffer)
	{
		assert(buffer.buffer && "ImageView is null");
		if (render_nodes.contains(pass_name))
		{
			render_nodes.at(pass_name)->Add_Buffer_Sync(buffer);
		}
		else
		{
			std::cout << "Renderpass: " << pass_name << " does not exist, saving the image anyways. \n";
		}
		// std::cout << "Image with name: \"" << name << "\" has changed \n";
		return buffer;
	}

	void Recreate_Frame_Resources()
	{
		// auto image_info_bf = image::Create_Info2d(glm::uvec2(core->swapchain_ref->extent.width, core->swapchain_ref->extent.height), 1, 1,
		// 											 core->swapchain_ref->Get_Format(),
		// 											 vk::ImageUsageFlagBits::eColorAttachment |
		// 												 vk::ImageUsageFlagBits::eSampled);
		// current_back_buffer = resources_manager->Set_Or_Create_Image("bf", image_info_bf, 0, 0);
	}
	void Recreate_Node_Pipelines()
	{
		for (auto &node : render_nodes)
		{
			if (node.second->gpu_pipeline_ref)
			{
				node.second->G_Recreate_Resources();
			}
		}
		systems::Logger::Get_Instance()->Log_Message("Graphics Pipelines Recreated");
	}

	void Debug_Shaders_Compilation()
	{
		std::string path = systems::Os::Get_Instance()->Get_Shaders_Path() + "\\compile.bat";
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
	void Create_Utility()
	{
		Create_Utility_Shaders();
		Blit_To_Backbuffer();
	}
	void Sort_Nodes_By_Dep(std::vector<engine::Render_Graph_Node *> &nodes_sorted_out)
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
			nodes_sorted_out.emplace_back(Get_Node(solved_nodes_ordered[i]));
		}
	}
	void Build_Queue_Passes_Batches(std::vector<engine::Render_Graph_Node *> &sorted_nodes, std::vector<Queue_Nodes_Batch> &batches)
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
			auto             *queue_ref  = core->queue_worker_manager->Get_Worker_Queue("Graphics");
			vk::CommandBuffer cmd       = queue_ref->Request_Queue_Cmd(pool_cmd_id);
			batches.emplace_back(Queue_Nodes_Batch{std::vector<engine::Render_Graph_Node *>{}, "Graphics", cmd, batch_idx});
			batches.back().pool_id_used = pool_cmd_id;
			batch_idx++;
		}
		for (int i = 0; i < sorted_nodes.size(); ++i)
		{
			if (batches.empty() || batches.back().queue_name != sorted_nodes[i]->worker_queue_name)
			{
				batch_idx++;
				auto             *queue_ref  = core->queue_worker_manager->Get_Worker_Queue(sorted_nodes[i]->worker_queue_name);
				int               pool_cmd_id = 0;
				vk::CommandBuffer cmd       = queue_ref->Request_Queue_Cmd(pool_cmd_id);
				batches.emplace_back(Queue_Nodes_Batch{std::vector<engine::Render_Graph_Node *>{sorted_nodes[i]}, queue_ref->name, cmd, batch_idx});
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
			auto             *queue_ref  = core->queue_worker_manager->Get_Worker_Queue("Graphics");
			vk::CommandBuffer cmd       = queue_ref->Request_Queue_Cmd(pool_cmd_id);
			batches.emplace_back(Queue_Nodes_Batch{std::vector<engine::Render_Graph_Node *>{}, "Graphics", cmd, batch_idx});
			batches.back().pool_id_used = pool_cmd_id;
			batch_idx++;
		}
		if (debug_ui)
		{
			auto *queue_ref  = core->queue_worker_manager->Get_Worker_Queue("UI");
			int   pool_cmd_id = 0;
			batches.emplace_back(Queue_Nodes_Batch{std::vector<engine::Render_Graph_Node *>{}, "UI", queue_ref->Request_Queue_Cmd(pool_cmd_id), batch_idx + 1});
			batches.back().pool_id_used = pool_cmd_id;
		}
	}

	void Resolve_Nodes_Dependancies(std::vector<engine::Render_Graph_Node *> &nodes_to_resolve)
	{
		for (int i = nodes_to_resolve.size() - 1; i >= 0; i--)
		{
			engine::Render_Graph_Node *node = nodes_to_resolve[i];
			for (int j = i - 1; j >= 0; j--)
			{
				engine::Render_Graph_Node *to_check_node = nodes_to_resolve[j];

				for (auto &image : to_check_node->image_attachments_names)
				{
					if (node->image_attachments_names.contains(image.first))
					{
						node->Depends_On(to_check_node->pass_name);
						break;
					}
					if (node->sampled_images.contains(image.first))
					{
						node->Depends_On(to_check_node->pass_name);
						break;
					}
				}
				for (auto &image : to_check_node->sampled_images)
				{
					if (node->sampled_images.contains(image.first))
					{
						node->Depends_On(to_check_node->pass_name);
						break;
					}
				}
				for (auto &image : to_check_node->storage_images)
				{
					if (node->storage_images.contains(image.first))
					{
						node->Depends_On(to_check_node->pass_name);
						break;
					}
				}
				for (auto &buffer : to_check_node->buffers)
				{
					if (node->buffers.contains(buffer.first))
					{
						node->Depends_On(to_check_node->pass_name);
						break;
					}
				}
			}
		}
	}

	void Execute_Queue_Batch(Queue_Nodes_Batch &queue_nodes_batch)
	{
		std::vector<std::string> all_passes_names;
		int                      idx = 0;

		auto profiler = Profiler::Get_Instance();
		for (auto &render_node : queue_nodes_batch.sorted_nodes)
		{
			if (!render_node->active)
			{
				continue;
			}

			profiler->Add_Profiler_Cpu_Spot(legit::Colors::getColor(idx), "Pass: " + render_node->pass_name);
			engine::Render_Graph_Node *node = render_node;
			node->Set_Curr_Cmd(queue_nodes_batch.command_buffer);
			auto       *queue_ref  = core->queue_worker_manager->Get_Worker_Queue(node->worker_queue_name);
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
				engine::Render_Graph_Node *depen_node = render_nodes.at(depen_name).get();
				if (!depen_node->active)
				{
					profiler->End_Profiler_Cpu_Spot("Pass: " + render_node->pass_name);
					systems::Logger::Get_Instance()->Log_Message(
					    "Pass with name: (" + render_node->pass_name + ") needs (" + depen_name + ") and is not active");
					// if a dependency is not active we skip the node
					continue;
				}
				Buffer_Usage_Types    last_node_type    = (depen_node->node_kind == Render_Node_Type::Graphics) ? B_Graphics_Write : B_Compute_Write;
				Buffer_Usage_Types    curr_node_type    = (node->node_kind == Render_Node_Type::Graphics) ? B_Graphics_Write : B_Compute_Write;
				Buffer_Access_Pattern last_node_pattern = Get_Src_Buffer_Access_Pattern(last_node_type);
				Buffer_Access_Pattern curr_node_pattern = Get_Src_Buffer_Access_Pattern(curr_node_type);
				Create_Mem_Barrier(last_node_pattern, curr_node_pattern, node->Get_Curr_Cmd());
			}
			if (queue_ref->is_main_threat)
			{
				node->Execute(node->Get_Curr_Cmd());
			}
			all_passes_names.push_back(node->pass_name);
			profiler->End_Profiler_Cpu_Spot("Pass: " + render_node->pass_name);
		}
	}
	void Prepare_Rendering()
	{
		Profiler::Get_Instance()->Add_Profiler_Cpu_Spot(legit::Colors::belizeHole, "Rendergraph prepare cpu");
		resources_manager->Update_Buffers();
		resources_manager->Update_Images();
		for (auto node : sequential_render_nodes)
		{
			node->Validate_Node_Type();
		}
		sorted_by_dep_nodes.clear();
		sorted_queue_batches.clear();
		Resolve_Nodes_Dependancies(sequential_render_nodes);
		if (sequential_render_nodes.size() > 1)
		{
			Get_Node("BlitterNode")->Depends_On(sequential_render_nodes[sequential_render_nodes.size() - 2]->pass_name);
		}
		Sort_Nodes_By_Dep(sorted_by_dep_nodes);
		Build_Queue_Passes_Batches(sorted_by_dep_nodes, sorted_queue_batches);
		Profiler::Get_Instance()->End_Profiler_Cpu_Spot("Rendergraph prepare cpu");
	}
	void Execute_Rendering()
	{
		Profiler::Get_Instance()->Add_Profiler_Cpu_Spot(legit::Colors::belizeHole, "Rendergraph execute cpu");
		assert(!sorted_queue_batches.empty() && "There must queues");
		for (auto &queue_batch : sorted_queue_batches)
		{
			Execute_Queue_Batch(queue_batch);
		}

		Profiler::Get_Instance()->End_Profiler_Cpu_Spot("Rendergraph execute cpu");
	}
	void End_Rendering()
	{
		core->queue_worker_manager->Reset_Pool_Usage();
	}

	~Render_Graph()
	{
		for (auto &pi : cuda_pipelines)
		{
			pi.second->context->C_Shutdown();
		}
	};
};

}        // namespace engine

#endif        // RENDER_GRAPH_HPP
