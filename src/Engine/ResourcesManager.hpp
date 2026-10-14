//

// Created by carlo on 2024-11-22.
//

#ifndef RESOURCES_MANAGER_HPP
#define RESOURCES_MANAGER_HPP

#define BASE_SIZE 8192
#define DST_SET_MAX_SIZE 1280

namespace engine
{
class resources_manager : systems::subject
{
  public:
	enum res_state
	{
		valid,
		invalid
	};
	enum class buffer_type
	{
		internal,
		external
	};

	struct buffer_update_info
	{
		res_state state;
		size_t   size;
		size_t   stride;
		void    *data;
	};
	struct img_create_configs
	{
		int  base_mip_level      = 0;
		int  base_array_layer    = 0;
		bool add_to_storage_clear = false;
	};

	struct img_update_info
	{
		res_state       buffer_state;
		std::string    path;
		uint32_t       array_layers_count;
		uint32_t       mips_count;
		vk::Format     format;
		layout_patterns dst_pattern;
		std::string    name;
		int32_t        id;
	};

	struct buffer_batch_info
	{
		std::string             name;
		size_t                  size;
		size_t                  stride;
		void                   *data;
		vk::BufferUsageFlags    buffer_usage_flags;
		vk::MemoryPropertyFlags mem_property_flags;
	};

	struct shader_batch_info
	{
		std::string path;
		shader_stage stage;
		int32_t     id;
	};

	struct dsets_info
	{
		vk::DescriptorSet dset;
		int32_t           id = -1;
	};

	struct buffer_params
	{
		std::string             name;
		vk::BufferUsageFlags    buffer_usage_flags;
		vk::MemoryPropertyFlags mem_property_flags;
		vk::DeviceSize          device_size;
		vk::DeviceSize          stride;
		void                   *data       = nullptr;
		buffer_type              is_external = buffer_type::internal;
	};

	engine::image_shipper *get_shipper(std::string name, std::string path, uint32_t array_layers_count, uint32_t mips_count,
	                         vk::Format     format,
	                         layout_patterns dst_pattern)
	{
		assert(core != nullptr && "core must be set");
		engine::image_shipper *image_shipper;
		int32_t       id = (int32_t) image_shippers.size();
		if (images_shippers_names.contains(name))
		{
			image_shipper = image_shippers.at(images_shippers_names.at(name)).get();
		}
		else
		{
			images_shippers_names.try_emplace(name, id);
			image_shippers.emplace_back(std::make_unique<engine::image_shipper>());
			images_update_infos.emplace_back(img_update_info{
			    valid, path, array_layers_count, mips_count, format, dst_pattern, name, id});
			image_shipper = get_shipper_from_name(name);
		}
		if (image_shipper->image == nullptr)
		{
			image_shipper->set_data_from_path(path);
			image_shipper->build_image(core, shipper_sampler, array_layers_count, mips_count, format, dst_pattern, name,
			                         id);
		}
		return image_shipper;
	}

	engine::image_shipper *get_shipper(std::string name, void *data, int width, int height, vk::DeviceSize size,
	                         uint32_t array_layers_count, uint32_t mips_count, vk::Format format,
	                         layout_patterns dst_pattern)
	{
		assert(core != nullptr && "core must be set");
		engine::image_shipper *image_shipper;
		int32_t       id = (int32_t) image_shippers.size();
		if (images_shippers_names.contains(name))
		{
			image_shipper = image_shippers.at(images_shippers_names.at(name)).get();
		}
		else
		{
			images_shippers_names.try_emplace(name, id);
			image_shippers.emplace_back(std::make_unique<engine::image_shipper>());
			images_update_infos.emplace_back(img_update_info{
			    valid, "", array_layers_count, mips_count, format, dst_pattern, name, id});
			image_shipper = get_shipper_from_name(name);
		}
		if (image_shipper->image == nullptr)
		{
			image_shipper->set_data_raw(data, width, height, size);
			image_shipper->build_image(core, shipper_sampler, array_layers_count, mips_count, format, dst_pattern, name,
			                         id);
		}
		return image_shipper;
	}

	engine::image_shipper *batch_shipper(std::string name, std::string path, uint32_t array_layers_count, uint32_t mips_count,
	                           vk::Format     format,
	                           layout_patterns dst_pattern)
	{
		if (images_shippers_names.contains(name))
		{
			systems::logger::get_instance()->log("Using texture that already exist: " + name,
			                                    systems::log_level::l_info);
			engine::image_shipper *shipper = get_shipper_from_name(name);
			return shipper;
		}
		int id = (int32_t) image_shippers.size();
		images_shippers_names.try_emplace(name, id);
		image_shippers.emplace_back(std::make_unique<engine::image_shipper>());
		images_update_infos.emplace_back(img_update_info{
		    invalid, path, array_layers_count, mips_count, format, dst_pattern, name, id});
		update_imgs_shippers = true;
		return get_shipper_from_name(name);
	}

	int batch_shader(std::string path, shader_stage stage)
	{
		if (shaders_names.contains(path))
		{
			return shaders_names.at(path);
		}
		int32_t id = shaders.size();
		shader_update_infos.push_back({path, stage, id});
		shaders_names.try_emplace(path, id);
		return id;
	}

	int batch_buffer(std::string name, vk::BufferUsageFlags buffer_usage_flags,
	                vk::MemoryPropertyFlags mem_property_flags, vk::DeviceSize device_size, vk::DeviceSize stride, void *data = nullptr)
	{
		if (buffer_names.contains(name))
		{
			return buffer_names.at(name);
		}
		int32_t id = buffers.size();
		buffer_batch_infos.push_back({name, device_size, stride, data, buffer_usage_flags, mem_property_flags});
		buffer_names.try_emplace(name, id);
		return id;
	}

	engine::image_view *set_or_create_image(std::string name, vk::ImageCreateInfo image_info, int base_mip_level, int base_array_layer)
	{
		assert(core != nullptr && "core must be set");
		if (!images_names.contains(name) && !storage_imgs_names.contains(name))
		{
			return get_image(name, image_info, base_mip_level, base_array_layer);
		}

		auto image = std::make_unique<engine::image>(core->physical_device, core->logical_device.get(), image_info);
		if (image_info.usage & vk::ImageUsageFlagBits::eStorage)
		{
			assert(storage_imgs_names.contains(name) && "Img name dont exist");
			int32_t id = (int32_t) storage_imgs_names.at(name);
			storage_imgs_views[id]->recreate(image->image_metadata.get());
			return storage_imgs_views[id].get();
		}
		else
		{
			assert(images_names.contains(name) && "Img name dont exist");
			int32_t id = (int32_t) images_names.at(name);
			image_views[id]->recreate(image->image_metadata.get());
			return image_views[id].get();
		}
	}

	engine::image_view *get_image(std::string name, vk::ImageCreateInfo image_info, int base_mip_level, int base_array_layer)
	{
		assert(core != nullptr && "core must be set");
		engine::image_view *image_view_ref = nullptr;
		if (images_names.contains(name) && (image_info.usage & vk::ImageUsageFlagBits::eStorage))
		{
			image_view_ref = get_storage_from_name(name);
			return image_view_ref;
		}
		else if (images_names.contains(name))
		{
			image_view_ref = get_image_view_from_name(name);
			return image_view_ref;
		}

		auto image = std::make_unique<engine::image>(core->physical_device, core->logical_device.get(), image_info);
		if (image_info.usage & vk::ImageUsageFlagBits::eStorage)
		{
			int32_t id = (int32_t) storage_imgs_views.size();
			assert(!storage_imgs_names.contains(name) && "Img name already exist");
			storage_imgs_names.try_emplace(name, (int32_t) id);

			storage_imgs_views.emplace_back(std::make_unique<engine::image_view>(
			    core->logical_device.get(), image->image_metadata.get(),
			    base_mip_level, image_info.mipLevels, base_array_layer,
			    image_info.arrayLayers, name, id));
			images.emplace_back(std::move(image));
			return storage_imgs_views.back().get();
		}
		else
		{
			assert(!images_names.contains(name) && "Img name already exist");
			int32_t id = (int32_t) image_views.size();
			images_names.try_emplace(name, (int32_t) image_views.size());
			image_views.emplace_back(std::make_unique<engine::image_view>(core->logical_device.get(), image->image_metadata.get(),
			                                                    base_mip_level, image_info.mipLevels, base_array_layer,
			                                                    image_info.arrayLayers, name, id));
			images.emplace_back(std::move(image));
			return image_views.back().get();
		}
	}

	engine::buffer *get_buffer(buffer_params params)
	{
		assert(core != nullptr && "core must be set");
		if (buffer_names.contains(params.name))
		{
			return get_buff_from_name(params.name);
		}

		auto buffer = std::make_unique<engine::buffer>(core->physical_device, core->logical_device.get(), params.buffer_usage_flags, params.mem_property_flags, params.device_size, params.stride, params.data, params.is_external == buffer_type::external);

		buffer_names.try_emplace(params.name, (int32_t) buffers.size());
		buffers.emplace_back(std::move(buffer));
		buffers_state.push_back({valid, params.device_size, params.stride, params.data});
		return buffers.back().get();
	}

	staged_buffer *get_stage_buffer(std::string name, vk::BufferUsageFlags buffer_usage_flags, vk::DeviceSize device_size, vk::DeviceSize stride,
	                             void *data = nullptr)
	{
		assert(core != nullptr && "core must be set");
		if (staged_buffer_names.contains(name))
		{
			return get_staged_buff_from_name(name);
		}

		auto buffer = std::make_unique<staged_buffer>(
		    core->physical_device, core->logical_device.get(), buffer_usage_flags, device_size, stride);

		if (data != nullptr)
		{
			void *mapped_mem = buffer->map();
			memcpy(mapped_mem, data, device_size);
			auto command_executor = std::make_unique<execute_once_command>(core);
			auto command_buffer   = command_executor->begin_command_buffer();
			buffer->unmap(command_buffer);
			command_executor->end_command_buffer();
		}

		staged_buffer_names.try_emplace(name, (int32_t) staged_buffers.size());
		staged_buffers.emplace_back(std::move(buffer));
		staged_buffers_state.push_back({valid, device_size});
		return staged_buffers.back().get();
	}

	engine::buffer *set_buffer(std::string name, vk::DeviceSize device_size, vk::DeviceSize stride, void *data)
	{
		assert(core != nullptr && "core must be set");
		assert(buffer_names.contains(name) && "Buffer dont exist");
		assert(!staged_buffer_names.contains(name) && "buffer is in staged buffers poll");

		engine::buffer *buffer_ref = buffers.at(buffer_names.at(name)).get();
		if (device_size > buffer_ref->device_size)
		{
			buffers_state.at(buffer_names.at(name)) = {invalid, device_size, stride, data};
			invalidate_buffers                     = true;
		}
		else
		{
			// pending to handle this if is a staged resource
			engine::buffer *buffer_ref_to_change = get_buff_from_name(name);
			if (buffer_ref_to_change->mapped_mem == nullptr)
			{
				buffer_ref_to_change->map();
			}
			memcpy(buffer_ref_to_change->mapped_mem, data, device_size);
			if (buffer_ref_to_change->usage_flags == vk::BufferUsageFlagBits::eStorageBuffer)
			{
				buffer_ref_to_change->unmap();
			}
		}

		return buffers.at(buffer_names.at(name)).get();
	}

	staged_buffer *set_stage_buffer(std::string name, vk::DeviceSize device_size, vk::DeviceSize stride, void *data)
	{
		// todo
		assert(core != nullptr && "core must be set");
		assert(staged_buffer_names.contains(name) && "staged buffer dont exist");
		assert(!buffer_names.contains(name) && "buffer is normal buffers poll");

		if (device_size > staged_buffers.at(staged_buffer_names.at(name))->size)
		{
			staged_buffers_state.at(staged_buffer_names.at(name)) = {invalid, device_size, stride, data};
			invalidate_buffers                                 = true;
		}
		else
		{
			staged_buffer *buffer    = get_staged_buff_from_name(name);
			void         *mapped_mem = buffer->map();
			memcpy(mapped_mem, data, device_size);
			auto command_executor = std::make_unique<execute_once_command>(core);
			auto command_buffer   = command_executor->begin_command_buffer();
			buffer->unmap(command_buffer);
			command_executor->end_command_buffer();
		}
		return staged_buffers.at(staged_buffer_names.at(name)).get();
	}

	void destroy_resources()
	{
		buffers.clear();
		staged_buffers.clear();
		storage_imgs_views.clear();
		image_views.clear();
		for (int i = 0; i < image_shippers.size(); ++i)
		{
			image_shippers[i]->clear();
		}
		image_shippers.clear();
		images.clear();
		sampler_pool.reset();
		dsets.clear();
		dsets_ids.clear();
		free_ids_bucket.clear();
		shaders.clear();
		descriptor_allocator.reset();
		files_manager.reset();
	}

	void create_batched_resources()
	{
		for (auto &buffer : buffer_batch_infos)
		{
			get_buffer(buffer_params{buffer.name, buffer.buffer_usage_flags, buffer.mem_property_flags, buffer.size, buffer.stride, buffer.data});
		}
		buffer_batch_infos.clear();
		for (auto &shader_update : shader_update_infos)
		{
			get_shader(shader_update.path, shader_update.stage);
		}
		shader_update_infos.clear();
	}

	void update_images()
	{
		if (!update_imgs_shippers)
		{
			return;
		}
		for (int i = 0; i < image_shippers.size(); i++)
		{
			img_update_info &update_info = images_update_infos[i];
			if (update_info.buffer_state == invalid)
			{
				image_shippers[i]->set_data_from_path(update_info.path);
				image_shippers[i]->build_image(core, shipper_sampler, update_info.array_layers_count,
				                             update_info.mips_count,
				                             update_info.format, update_info.dst_pattern, update_info.name,
				                             update_info.id);
				update_info.buffer_state = valid;
			}
		}
	}

	void update_buffers()
	{
		if (!invalidate_buffers)
		{
			return;
		}
		for (auto &name : buffer_names)
		{
			buffer_update_info &buffer_update_info = buffers_state.at(name.second);
			if (buffer_update_info.state == invalid)
			{
				buffers.at(buffer_names.at(name.first)).reset(new engine::buffer(core->physical_device, core->logical_device.get(), buffers.at(name.second)->usage_flags, buffers.at(name.second)->mem_property_flags, buffer_update_info.size, buffer_update_info.stride, buffer_update_info.data));
				buffer_update_info.state = valid;
			}
		}
		auto command_executor = std::make_unique<execute_once_command>(core);
		auto command_buffer   = command_executor->begin_command_buffer();
		for (auto &name : staged_buffer_names)
		{
			buffer_update_info &buffer_update_info = staged_buffers_state.at(name.second);
			if (buffer_update_info.state == invalid)
			{
				staged_buffers.at(staged_buffer_names.at(name.first)).reset(new staged_buffer(core->physical_device, core->logical_device.get(), staged_buffers.at(name.second)->device_buffer->usage_flags, buffer_update_info.size, buffer_update_info.stride));
				staged_buffer *buffer    = get_staged_buff_from_name(name.first);
				void         *mapped_mem = buffer->map();
				memcpy(mapped_mem, buffer_update_info.data, buffer_update_info.size);
				buffer->unmap(command_buffer);

				buffer_update_info.state = valid;
			}
		}
		command_executor->end_command_buffer();
		notify();
		invalidate_buffers = false;
	}

	void end_frame_dynamic_updates(vk::CommandBuffer command_buffer)
	{
		buffer_access_pattern src = get_src_buffer_access_pattern(buffer_usage_types::b_graphics_write);
		buffer_access_pattern dst = get_src_buffer_access_pattern(buffer_usage_types::b_transfer_dst);
		create_mem_barrier(src, dst, command_buffer);
		for (int i = 0; i < storage_imgs_to_clear.size(); ++i)
		{
			engine::image_view          *image_view_handle = get_storage_from_name(storage_imgs_to_clear[i]);
			vk::ClearColorValue clear_color(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});

			command_buffer.clearColorImage(
			    image_view_handle->image_metadata->image_handle,
			    vk::ImageLayout::eGeneral,
			    clear_color,
			    image_view_handle->get_subresource_range());
		}
		storage_imgs_to_clear.clear();
	}

	void request_storage_image_clear(std::string name)
	{
		if (!storage_imgs_names.contains(name))
		{
			systems::logger::get_instance()->log_message("Storage Img with name does not exist: " + name);
			return;
		}
		storage_imgs_to_clear.push_back(name);
	}

	//handle spv or normal path, internally it creates the spv path
	shader *get_shader(std::string path, shader_stage stage)
	{
		if (shaders_names.contains(path))
		{
			return shaders.at(shaders_names.at(path)).get();
		}
		int id = shaders.size();

		shaders.emplace_back(std::make_unique<shader>(core->logical_device.get(), path, stage));

		shaders_names.try_emplace(shaders.back()->spirv_path, id);

		shader *shader = shaders.back().get();
		return shader;
	}
	shader *get_shader_from_id(int id)
	{
		for (auto shader : shaders_names)
		{
			if (shader.second == id)
			{
				return shaders.at(id).get();
			}
		}
		assert(false && "invalid shader id");
		return nullptr;
	}
	shader *get_or_create_default_shader(std::string name, shader_stage stage, shader_compiler compiler)
	{
		std::filesystem::path target_path;
		std::filesystem::path template_path;
		switch (compiler)
		{
			case c_glsl:
				switch (stage)
				{
					case s_vert:
						if (std::filesystem::path(name).extension() != ".vert")
						{
							name += ".vert";
						}
						template_path = systems::os::get_instance()->glsl_shaders_template_path / "tVert.vert";
						break;
					case s_frag:
						if (std::filesystem::path(name).extension() != ".frag")
						{
							name += ".frag";
						}
						template_path = systems::os::get_instance()->glsl_shaders_template_path / "tFrag.frag";
						break;

					case s_comp:
						if (std::filesystem::path(name).extension() != ".comp")
						{
							name += ".comp";
						}
						template_path = systems::os::get_instance()->glsl_shaders_template_path / "tComp.comp";
						break;
					case s_tess_control:
						if (std::filesystem::path(name).extension() != ".tesc")
						{
							name += ".tesc";
						}
						template_path = systems::os::get_instance()->glsl_shaders_template_path / "tTesc.tesc";
						break;
					case s_tess_eval:
						if (std::filesystem::path(name).extension() != ".tese")
						{
							name += ".tese";
						}
						template_path = systems::os::get_instance()->glsl_shaders_template_path / "tTese.tese";
						break;
					case s_geom:
						if (std::filesystem::path(name).extension() != ".geom")
						{
							name += ".geom";
						}
						template_path = systems::os::get_instance()->glsl_shaders_template_path / "tGeom.geom";
						break;
					case s_unknown:
						break;
				}
				target_path = systems::os::get_instance()->shaders_path / "glsl" / "generated" / name;
				break;
			case c_slang:
				if (std::filesystem::path(name).extension() != ".slang")
				{
					name += ".slang";
				}
				target_path   = systems::os::get_instance()->shaders_path / "slang" / "generated" / name;
				template_path = systems::os::get_instance()->slang_shaders_template_path;
				if (stage == shader_stage::s_comp)
				{
					template_path /= "compute.slang";
				}
				else
				{
					template_path /= "quad.slang";
				}
				break;
			default:
				assert(false);
				break;
		}

		if (std::filesystem::exists(target_path))
		{
			return get_shader(target_path.string(), stage);
		}

		systems::os::get_instance()->copy_file_into(template_path.string(), target_path.string());

		std::string spirv_path = convert_shader_path_to_spirv(target_path.string(), stage);
		shader     *shader    = get_shader(target_path.string(), stage);

		// danger
		//  files_manager->add_path_to_delete(target_path.string());
		//  files_manager->add_path_to_delete(spirv_path);

		return shader;
	}

	engine::vertex_input *get_vertex_input(std::string name)
	{
		if (vertices_inputs_names.contains(name))
		{
			return vertices_inputs.at(vertices_inputs_names.at(name)).get();
		}
		vertices_inputs_names.try_emplace(name, vertices_inputs.size());
		vertices_inputs.emplace_back(std::make_unique<engine::vertex_input>());
		engine::vertex_input *vertex_layout = vertices_inputs.back().get();
		return vertex_layout;
	}
	engine::vertex_input *get_vertex_input_from_id(int id)
	{
		for (auto &name : vertices_inputs_names)
		{
			if (name.second == id)
			{
				return vertices_inputs.at(name.second).get();
			}
		}
		assert(false && "invalid vertex id");
	}

	dsets_info allocate_dset(vk::DescriptorSetLayout dst_set_layout)
	{
		if (!free_ids_bucket.empty())
		{
			for (auto &id : free_ids_bucket)
			{
				systems::logger::get_instance()->log("Free Id: " + std::to_string(id));
			}
			int32_t id = free_ids_bucket.front();
			free_ids_bucket.erase(free_ids_bucket.begin());
			dsets.at(id) = descriptor_allocator->allocate(core->logical_device.get(), dst_set_layout);
			return dsets_info{dsets.at(id).get(), id};
		}
		else
		{
			int32_t id = dsets.size();
			dsets_ids.insert(id);
			dsets.emplace_back(descriptor_allocator->allocate(core->logical_device.get(), dst_set_layout));
			return dsets_info{dsets.back().get(), id};
		}
	}

	void deallocate_dset(int id)
	{
		if (dsets_ids.contains(id))
		{
			dsets.at(id).reset();
			free_ids_bucket.emplace_back(id);
		}
	}

	engine::image_view *get_image_view_from_name(std::string name)
	{
		if (!images_names.contains(name))
		{
			systems::logger::get_instance()->log("Img View With Name: " + name + "Does not exist");
			return nullptr;
		}
		return image_views.at(images_names.at(name)).get();
	}

	engine::image_shipper *get_shipper_from_name(std::string name)
	{
		if (!images_shippers_names.contains(name))
		{
			systems::logger::get_instance()->log("Img Shipper With Name: " + name + "Does not exist");
			return nullptr;
		}
		return image_shippers.at(images_shippers_names.at(name)).get();
	}

	engine::image_view *get_storage_from_name(std::string name)
	{
		if (!storage_imgs_names.contains(name))
		{
			return nullptr;
		}
		return storage_imgs_views.at(storage_imgs_names.at(name)).get();
	}

	engine::image_view *get_image_view_from_id(int id)
	{
		for (auto &image_name : images_names)
		{
			if (image_name.second == id)
			{
				return image_views.at(id).get();
			}
		}
		systems::logger::get_instance()->log("Img View With id: " + std::to_string(id) + "Does not exist");
		return nullptr;
	}

	engine::image_shipper *get_shipper_from_id(int id)
	{
		for (auto &image_name : images_shippers_names)
		{
			if (image_name.second == id)
			{
				return image_shippers.at(id).get();
			}
		}
		systems::logger::get_instance()->log("Img View With id: " + std::to_string(id) + "Does not exist");
		return nullptr;
	}

	engine::image_view *get_storage_from_id(int id)
	{
		for (auto &image_name : storage_imgs_names)
		{
			if (image_name.second == id)
			{
				return storage_imgs_views.at(id).get();
			}
		}
		systems::logger::get_instance()->log("Img View With id: " + std::to_string(id) + "Does not exist");
		return nullptr;
	}

	engine::buffer *get_buff_from_name(std::string name)
	{
		if (!buffer_names.contains(name))
		{
			return nullptr;
		}
		return buffers.at(buffer_names.at(name)).get();
	}

	staged_buffer *get_staged_buff_from_name(std::string name)
	{
		if (!staged_buffer_names.contains(name))
		{
			return nullptr;
		}
		return staged_buffers.at(staged_buffer_names.at(name)).get();
	}

	int get_shipper_id(std::string name)
	{
		return images_shippers_names.at(name);
	}

	static resources_manager *get_instance(engine::core *core_ref = nullptr)
	{
		if (instance == nullptr && core_ref != nullptr)
		{
			instance = new resources_manager(core_ref);
		}
		return instance;
	}

	resources_manager(engine::core *core_refs)
	{
		this->core = core_refs;
		staged_buffers.reserve(BASE_SIZE);
		buffers.reserve(BASE_SIZE);
		storage_imgs_views.reserve(BASE_SIZE);
		image_views.reserve(BASE_SIZE);
		images.reserve(BASE_SIZE);
		files_manager        = std::make_unique<systems::files_manager>();
		descriptor_allocator = std::make_unique<engine::descriptor_allocator>();
		descriptor_allocator->begin_pool(core->logical_device.get(), DST_SET_MAX_SIZE, pool_size_ratios);

		sampler_pool = std::make_unique<engine::sampler_pool>();

		shipper_sampler = sampler_pool->add_sampler(core->logical_device.get(), vk::SamplerAddressMode::eRepeat,
		                                         vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);

		std::string default_texture_path = systems::os::get_instance()->get_engine_resources_path() +
		                                 "\\Images\\default_texture.jpg";

		engine::image_shipper *shipper = get_shipper("default_tex", default_texture_path, 1, 1, g_shipper_format,
		                                   layout_patterns::graphics_read);

		auto image_info = engine::image::create_info2d(
		    glm::uvec2(core->swapchain_ref->extent.width, core->swapchain_ref->extent.height), 1, 1,
		    engine::g_32b_format,
		    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);

		engine::image_view *default_storage = get_image("default_storage", image_info, 0, 0);

		auto       image_info_bf = engine::image::create_info2d(glm::uvec2(core->swapchain_ref->extent.width, core->swapchain_ref->extent.height), 1, 1,
		                                             core->swapchain_ref->get_format(),
		                                             vk::ImageUsageFlagBits::eColorAttachment |
		                                                 vk::ImageUsageFlagBits::eSampled);
		engine::image_view *image_view_handle   = get_image("bf", image_info_bf, 0, 0);

		engine::buffer *quad_vert_buffer = get_stage_buffer(
		                                     "quad_default", vk::BufferUsageFlagBits::eVertexBuffer,
		                                     sizeof(engine::vertex_2d) * engine::vertex_2d::get_quad_vertices().size(),
		                                     sizeof(engine::vertex_2d), engine::vertex_2d::get_quad_vertices().data())
		                                     ->device_buffer.get();

		engine::buffer *quad_index_buffer = get_stage_buffer(
		                                      "quad_index_default", vk::BufferUsageFlagBits::eIndexBuffer,
		                                      sizeof(uint32_t) * engine::vertex_2d::get_quad_indices().size(), sizeof(uint32_t),
		                                      engine::vertex_2d::get_quad_indices().data())
		                                      ->device_buffer.get();

		engine::buffer* cube_verts = get_stage_buffer(
															   "cube_verts_default", vk::BufferUsageFlagBits::eVertexBuffer,
															   sizeof(d_vertex_3d) * d_vertex_3d::get_cube_vertices().size(), sizeof(d_vertex_3d),
															   d_vertex_3d::get_cube_vertices().data())
								->device_buffer.get();
		engine::buffer* cube_index= get_stage_buffer(
																"cube_index_default", vk::BufferUsageFlagBits::eIndexBuffer,
																sizeof(uint32_t) * d_vertex_3d::get_cube_indices().size(), sizeof(uint32_t),
																d_vertex_3d::get_cube_indices().data())
								 ->device_buffer.get();
	}

	~resources_manager() = default;

	void attach(systems::watcher *watcher) override
	{
		watchers.push_back(watcher);
	}

	void detach(systems::watcher *watcher) override
	{
		watchers.erase(std::remove(watchers.begin(), watchers.end(), watcher), watchers.end());
	}

	void notify() override
	{
		for (auto &watcher : watchers)
		{
			if (watcher == nullptr)
			{
				systems::logger::get_instance()->log_message("Watcher is null");
				continue;
			}
			watcher->update_watcher();
		}
	}

	std::vector<systems::watcher *> watchers;

	std::unordered_map<std::string, int32_t> buffer_names;
	std::unordered_map<std::string, int32_t> staged_buffer_names;
	std::unordered_map<std::string, int32_t> images_names;
	std::unordered_map<std::string, int32_t> storage_imgs_names;
	std::unordered_map<std::string, int32_t> images_shippers_names;
	std::unordered_map<std::string, int32_t> shaders_names;
	std::unordered_map<std::string, int32_t> vertices_inputs_names;

	std::set<int32_t>   dsets_ids;
	std::deque<int32_t> free_ids_bucket;

	std::vector<std::unique_ptr<engine::buffer>>       buffers;
	std::vector<std::unique_ptr<staged_buffer>> staged_buffers;
	std::vector<std::unique_ptr<engine::image_view>>    image_views;
	std::vector<std::unique_ptr<engine::image_view>>    storage_imgs_views;
	std::vector<std::unique_ptr<engine::image_shipper>> image_shippers;
	std::vector<std::unique_ptr<engine::image>>        images;
	std::vector<std::string>                   storage_imgs_to_clear;
	std::unique_ptr<engine::sampler_pool>               sampler_pool;
	std::unique_ptr<engine::descriptor_allocator>       descriptor_allocator;
	std::vector<vk::UniqueDescriptorSet>       dsets;
	std::vector<std::unique_ptr<shader>>       shaders;
	std::vector<std::unique_ptr<engine::vertex_input>>  vertices_inputs;

	std::vector<img_update_info>    images_update_infos;
	std::vector<buffer_update_info> buffers_state;
	std::vector<buffer_update_info> staged_buffers_state;

	std::vector<shader_batch_info> shader_update_infos;
	std::vector<buffer_batch_info> buffer_batch_infos;

	std::vector<engine::descriptor_allocator::pool_size_ratio> pool_size_ratios = {
	    {vk::DescriptorType::eSampler, 1.5f},
	    {vk::DescriptorType::eStorageBuffer, 1.5f},
	    {vk::DescriptorType::eUniformBuffer, 1.5f},
	    // Array bindings reserve 1000 descriptors each, including unused entries.
	    // With 100 sets, reserve 100000 descriptors of each image type.
	    {vk::DescriptorType::eStorageImage, 1.5f},
	    {vk::DescriptorType::eCombinedImageSampler, 1.5f},
	};
	std::unique_ptr<systems::files_manager> files_manager;

	engine::sampler *shipper_sampler;

	bool invalidate_buffers  = false;
	bool update_imgs_shippers = false;
	bool update_imgs         = false;

	engine::core                    *core;
	static resources_manager *instance;
};

resources_manager *resources_manager::instance = nullptr;
}        // namespace engine

#endif        // RESOURCES_MANAGER_HPP
