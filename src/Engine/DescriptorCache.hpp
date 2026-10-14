//







// Created by carlo on 2024-11-19.
//

#ifndef DESCRIPTOR_CACHE_HPP
#define DESCRIPTOR_CACHE_HPP
namespace engine
{
#define DEFAULT_VAR_DESCRIPTOR_COUNT 64
    class descriptor_cache : public systems::watcher
    {
        struct sampler_binding
        {
            engine::image_view* image_view_handle;
            engine::sampler* sampler;
        };
        struct storage_binding
        {
            engine::image_view* image_view_handle;
            engine::sampler* sampler;
        };
        struct sampler_array
        {
            std::vector<engine::image_view*> image_views_array{};
            std::vector<engine::sampler*> samplers;
        };
        struct storage_array
        {
            std::vector<engine::image_view*> image_views_array{};
            std::vector<engine::sampler*> samplers;
        };
    public:
        descriptor_cache(engine::core* core)
        {
            this->core = core;
            resources_manager_ref = resources_manager::get_instance();
            resources_manager_ref->attach(this);
            engine::image_shipper* default_shipper = resources_manager_ref->get_shipper_from_name("default_tex");
            this->default_image_view = default_shipper->image_view_handle.get();
            this->default_sampler = resources_manager_ref->shipper_sampler;
            this->default_storage_image_view = resources_manager_ref->get_storage_from_name("default_storage");
            this->default_storage_sampler = resources_manager_ref->shipper_sampler;
        }
        ~descriptor_cache()
        {
            resources_manager_ref->detach(this);
            resources_manager_ref->deallocate_dset(dsets_info.id);
        }
        void update_watcher() override
        {
            flush_buffers();
        }

        void add_shader_info(shader_parser* parser)
        {
            if (!parser->shader_name.empty())
            {
                shader_names.push_back(parser->shader_name);
            }
            std::vector<shader_resource> unique_resources;
            parser->get_layout(unique_resources);
            for (auto& resource : unique_resources)
            {
                if (dst_set_builder.unique_bindings.contains(resource.binding))
                {
                    systems::logger::get_instance()->log("Binding: " + std::to_string(resource.binding) + " already exist with name: " + resource.name, systems::log_level::l_error);
                    continue;
                }
                engine::buffer* ubo;
                engine::buffer* ssbo;
                std::unique_ptr<engine::image> storage_image;
                std::unique_ptr<engine::image_data> storage_image_data;
                std::unique_ptr<engine::image_view> storage_image_view;
                switch (resource.type)
                {
                case vk::DescriptorType::eCombinedImageSampler:
                    assert(default_sampler != nullptr && "Default sampler is needed");
                    assert(default_image_view != nullptr && "Default Image view is needed");
                    image_bindings_keys.try_emplace(resource.name, resource);
                    resource_shader_names.try_emplace(resource.name, parser->shader_name);
                    if (resource.array){
                        sampler_array_resources.try_emplace(resource.binding, sampler_array({default_image_view}, {default_sampler}));
                    }else
                    {
                        image_samplers.try_emplace(resource.binding, sampler_binding(default_image_view, default_sampler));
                    }
                    break;
                case vk::DescriptorType::eStorageImage:
                    assert(default_storage_sampler != nullptr && "Default sampler is needed");
                    assert(default_storage_image_view != nullptr && "Default Image view is needed");
                    image_bindings_keys.try_emplace(resource.name, resource);
                    resource_shader_names.try_emplace(resource.name, parser->shader_name);
                    if (resource.array)
                    {
                        storage_array_resources.try_emplace(resource.binding,storage_array({default_storage_image_view}, {default_storage_sampler}));
                    }
                    else
                    {
                        storage_images.try_emplace(resource.binding, storage_binding(default_storage_image_view, default_storage_sampler));
                    }
                    
                    break;
                case vk::DescriptorType::eUniformBuffer:
                    buffer_bindings_keys.try_emplace(resource.name, resource);
                    resource_shader_names.try_emplace(resource.name, parser->shader_name);
                    // ubo = resources_manager_ref->get_stage_buffer(resource.name, vk::BufferUsageFlagBits::eUniformBuffer, 1)->device_buffer.get();
                    ubo = resources_manager_ref->get_buffer(resources_manager::buffer_params{resource.name, vk::BufferUsageFlagBits::eUniformBuffer,
                                                         vk::MemoryPropertyFlagBits::eHostVisible |
                                                         vk::MemoryPropertyFlagBits::eHostCoherent, 1});
                    buffers_resources.try_emplace(resource.binding, ubo);
                    break;
                case vk::DescriptorType::eStorageBuffer:
                    buffer_bindings_keys.try_emplace(resource.name, resource);
                    resource_shader_names.try_emplace(resource.name, parser->shader_name);
                    // ssbo = resources_manager_ref->get_stage_buffer(resource.name ,vk::BufferUsageFlagBits::eStorageBuffer, 1)->device_buffer.get();
                    ssbo = resources_manager_ref->get_buffer(resources_manager::buffer_params{resource.name, vk::BufferUsageFlagBits::eStorageBuffer,
                                                         vk::MemoryPropertyFlagBits::eHostVisible |
                                                         vk::MemoryPropertyFlagBits::eHostCoherent, 1});
                    buffers_resources.try_emplace(resource.binding,ssbo);
                    break;
                }
                if (resource.array)
                {
                    dst_set_builder.add_binding(resource.binding, resource.type, DEFAULT_VAR_DESCRIPTOR_COUNT);
                }else
                {
                    dst_set_builder.add_binding(resource.binding, resource.type);
                }
            }
        }

        void build_descriptors_cache(vk::ShaderStageFlags stage_flags)
        {

            assert((image_bindings_keys.size() + buffer_bindings_keys.size()) == dst_set_builder.bindings.size() && "Resources and builder must have the same size");
            std::vector<vk::DescriptorBindingFlags> binding_flags;
            for (int i = 0; i < dst_set_builder.bindings.size(); ++i)
            {
                binding_flags.push_back(
                    vk::DescriptorBindingFlagBits::ePartiallyBound);
            }
             auto bindings_flags_create_info = vk::DescriptorSetLayoutBindingFlagsCreateInfo()
            .setBindingCount((uint32_t)dst_set_builder.bindings.size())
            .setPBindingFlags(binding_flags.data());

            dst_layout = dst_set_builder.build_bindings(core->logical_device.get(), stage_flags, &bindings_flags_create_info, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT);

            if (dst_set != nullptr)
        	{
                assert(dsets_info.id > -1 && "you are trying to deallocate an dset without dset info");
                resources_manager_ref->deallocate_dset(dsets_info.id);
        	}
            dsets_info = resources_manager_ref->allocate_dset(dst_layout.get());
            dst_set = dsets_info.dset;
            for (auto& buff_binding : buffer_bindings_keys)
            {
                engine::buffer* buffer = buffers_resources.at(buff_binding.second.binding);
                writer_builder.add_write_buffer(buff_binding.second.binding, buffer->descriptor, buff_binding.second.type);
            }

            for (auto image_binding : image_bindings_keys)
            {
                if (image_binding.second.array && image_binding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    sampler_array& image_array = sampler_array_resources.at(image_binding.second.binding);
                    writer_builder.add_images_array(image_binding.second.binding, image_array.image_views_array,
                                                 image_array.samplers, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                 image_binding.second.type);
                    continue;
                }else if(image_binding.second.array && image_binding.second.type == vk::DescriptorType::eStorageImage)
                {
                    storage_array& image_array = storage_array_resources.at(image_binding.second.binding);
                    writer_builder.add_images_array(image_binding.second.binding, image_array.image_views_array,
                                                 image_array.samplers, vk::ImageLayout::eGeneral,
                                                 image_binding.second.type);
                    continue;
                }
                if (image_binding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    sampler_binding& sampler_binding = image_samplers.at(image_binding.second.binding);
                    writer_builder.add_write_image(image_binding.second.binding, sampler_binding.image_view_handle,
                                                sampler_binding.sampler->sampler_handle.get(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal, image_binding.second.type);
                }
                else if (image_binding.second.type == vk::DescriptorType::eStorageImage)
                {
                    storage_binding& binding = storage_images.at(image_binding.second.binding);
                    writer_builder.add_write_image(image_binding.second.binding, binding.image_view_handle,
                                                binding.sampler->sampler_handle.get(), vk::ImageLayout::eGeneral,
                                                image_binding.second.type);
                }

            }
            writer_builder.update_set(core->logical_device.get(), dsets_info.dset);
        }

        void update_descriptor()
        {
            for (auto& buff_binding : buffer_bindings_keys)
            {
                engine::buffer* buffer = buffers_resources.at(buff_binding.second.binding);
                writer_builder.add_write_buffer(buff_binding.second.binding, buffer->descriptor, buff_binding.second.type);
            }

            for (auto& image_binding : image_bindings_keys)
            {
                if (image_binding.second.array && image_binding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    sampler_array& image_array = sampler_array_resources.at(image_binding.second.binding);
                    writer_builder.add_images_array(image_binding.second.binding, image_array.image_views_array,
                                                 image_array.samplers, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                 image_binding.second.type);
                    continue;
                }else if(image_binding.second.array && image_binding.second.type == vk::DescriptorType::eStorageImage)
                {
                    storage_array& image_array = storage_array_resources.at(image_binding.second.binding);
                    writer_builder.add_images_array(image_binding.second.binding, image_array.image_views_array,
                                                 image_array.samplers, vk::ImageLayout::eGeneral,
                                                 image_binding.second.type);
                    continue;
                }
                if (image_binding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    sampler_binding& sampler_binding = image_samplers.at(image_binding.second.binding);
                    writer_builder.add_write_image(image_binding.second.binding, sampler_binding.image_view_handle,
                                                sampler_binding.sampler->sampler_handle.get(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal, image_binding.second.type);
                }else if (image_binding.second.type == vk::DescriptorType::eStorageImage)
                {
                    storage_binding& binding = storage_images.at(image_binding.second.binding);
                    writer_builder.add_write_image(image_binding.second.binding, binding.image_view_handle,
                                                binding.sampler->sampler_handle.get(), vk::ImageLayout::eGeneral,
                                                image_binding.second.type);
                }
                
            }
            writer_builder.update_set(core->logical_device.get(), dsets_info.dset);
        }
        bool has_bindings()
        {
            if (!sampler_array_resources.empty())
            {
                return true;
            }
            if (!storage_array_resources.empty())
            {
                return true;
            }
            if (!buffers_resources.empty())
            {
                return true;
            }
            if (!image_samplers.empty())
            {
                return true;
            }
            if (!storage_images.empty())
            {
                return true;
            }
            return false;
        }
        template<typename value_type>
        void set_buffer(std::string name, std::vector<value_type>& buffer_data)
        {
            engine::buffer* buffer_ref = get_buffer_by_name(name);
            if (buffer_ref==nullptr){return;}
            // resources_manager_ref->set_stage_buffer(
            //     name,
            //     sizeof(T) * bufferData.size(),
            //     bufferData.data());
            resources_manager_ref->set_buffer(
                name,
                sizeof(value_type) * buffer_data.size(),
                sizeof(value_type),
                buffer_data.data());
            
        }

        template <typename value_type>
        void set_buffer(std::string name, value_type& buffer_data)
        {
            engine::buffer* buffer_ref = get_buffer_by_name(name);
            if (buffer_ref==nullptr){return;}
            // resources_manager_ref->set_stage_buffer(
            //     name,
            //     sizeof(T),
            //     &bufferData);
            resources_manager_ref->set_buffer(
                name,
                sizeof(value_type),
                sizeof(value_type),
                &buffer_data);
        }

        void set_buffer(std::string name, engine::buffer* buffer_data)
        {
            engine::buffer* buffer_ref = get_buffer_by_name(name);
            if (buffer_ref == nullptr)
        	{
        		return;
        	}
            shader_resource& binding = buffer_bindings_keys.at(name);
            
            //buffer updates is handled outside the descriptor cache
            if (buffer_ref != buffer_data)
            {
                buffers_resources.at(binding.binding) = buffer_data;
                update_descriptor();
            }
        }
        void set_sampler(std::string name, engine::image_view* image_view_handle, engine::sampler* sampler = nullptr)
        {
            sampler_binding* sampler_binding = get_sampler_by_name(name);
            if (sampler_binding == nullptr)
        	{
        		return;
        	}
        	
            shader_resource& resource = image_bindings_keys.at(name);
            assert(resource.type == vk::DescriptorType::eCombinedImageSampler && "The image view is not a sampler image");

            if (sampler_binding == nullptr){return;}
            if (sampler_binding->image_view_handle == image_view_handle) { return; }

            if (sampler)
            {
                sampler_binding->sampler = sampler;
            }
            sampler_binding->image_view_handle = image_view_handle;
            update_descriptor();
        }

        void set_storage_image(std::string name, engine::image_view* image_view_handle, engine::sampler* sampler = nullptr)
        {
             storage_binding* storage_binding = get_storage_image_by_name(name);
            if (storage_binding == nullptr){return;}
            shader_resource& resource = image_bindings_keys.at(name);
            assert(resource.type == vk::DescriptorType::eStorageImage && "The image view is not a storage image");
            if (storage_binding->image_view_handle == image_view_handle) { return; }

            if (sampler)
            {
                storage_binding->sampler = sampler;
            }
            storage_binding->image_view_handle = image_view_handle;
            update_descriptor();
        }

        void set_storage_image_array(std::string name, std::vector<engine::image_view*>& image_views,
                           std::vector<engine::sampler*>* samplers = nullptr)
        {
            storage_array* image_array_resource = get_storage_array_by_name(name);
            if (image_array_resource == nullptr) return;

            bool update = false;
            if (image_views.size() != image_array_resource->image_views_array.size())
            {
                update = true;
            }else
            {
                for (int i = 0; i < image_array_resource->image_views_array.size(); ++i)
                {
                    if (image_array_resource->image_views_array[i] != image_views[i])
                    {
                        update = true;
                        break;
                    }
                }
            }
            if (samplers != nullptr && !update)
            {
                if (samplers->size() != image_array_resource->samplers.size())
                {
                    update = true;
                }
                else
                {
                    for (int i = 0; i < image_array_resource->samplers.size(); ++i)
                    {
                        std::vector<engine::sampler*>& samplers_ref = *samplers;
                        if (image_array_resource->samplers[i] != samplers_ref[i])
                        {
                            update = true;
                            break;
                        }
                    }
                }               
            }
            if (update)
            {

                image_array_resource->image_views_array.clear();
                image_array_resource->image_views_array.reserve((uint32_t)image_views.size());
                for (int i = 0; i < image_views.size(); ++i)
                {
                    image_array_resource->image_views_array.push_back(image_views[i]);
                }
                if (samplers != nullptr)
                {
                    image_array_resource->samplers.clear();
                    image_array_resource->samplers.reserve((uint32_t)samplers->size());
                    for (int i = 0; i < samplers->size(); ++i)
                    {
                        std::vector<engine::sampler*>& samplers_ref = *samplers;
                        image_array_resource->samplers.push_back(samplers_ref[i]);
                    }
                }
                update_descriptor();
            }
        }
        void set_sampler_array(std::string name, std::vector<engine::image_view*>& image_views, std::vector<engine::sampler*>* samplers = nullptr)
        {
            sampler_array* image_array_resource = get_sampler_array_by_name(name);
            if (image_array_resource == nullptr) return;

            bool update = false;
            if (image_views.size() != image_array_resource->image_views_array.size())
            {
                update = true;
            }else
            {
                for (int i = 0; i < image_array_resource->image_views_array.size(); ++i)
                {
                    if (image_array_resource->image_views_array[i] != image_views[i])
                    {
                        update = true;
                        break;
                    }
                }
            }
            if (samplers != nullptr && !update)
            {
                if (samplers->size() != image_array_resource->samplers.size())
                {
                    update = true;
                }
                else
                {
                    for (int i = 0; i < image_array_resource->samplers.size(); ++i)
                    {
                        std::vector<engine::sampler*>& samplers_ref = *samplers;
                        if (image_array_resource->samplers[i] != samplers_ref[i])
                        {
                            update = true;
                            break;
                        }
                    }
                }               
            }
            if (update)
            {

                image_array_resource->image_views_array.clear();
                image_array_resource->image_views_array.reserve((uint32_t)image_views.size());
                for (int i = 0; i < image_views.size(); ++i)
                {
                    image_array_resource->image_views_array.push_back(image_views[i]);
                }
                if (samplers != nullptr)
                {
                    image_array_resource->samplers.clear();
                    image_array_resource->samplers.reserve((uint32_t)samplers->size());
                    for (int i = 0; i < samplers->size(); ++i)
                    {
                        std::vector<engine::sampler*>& samplers_ref = *samplers;
                        image_array_resource->samplers.push_back(samplers_ref[i]);
                    }
                }
                update_descriptor();
            }
        }
        engine::buffer* get_buffer_by_name(std::string name)
        {
            if (!buffer_bindings_keys.contains(name))
            {
                std::string text = "The buffer with name: " + name + ": does not exist on shaders" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            shader_resource& binding = buffer_bindings_keys.at(name);
            if (!buffers_resources.contains(binding.binding))
            {
                std::string text = "The buffer with name: " + name + ": is present in the bindings but is not registered as a buffer" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return buffers_resources.at(binding.binding);
        }
        sampler_binding* get_sampler_by_name(std::string name)
        {
            if (!image_bindings_keys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            shader_resource& binding = image_bindings_keys.at(name);
            if (!image_samplers.contains(binding.binding))
            {
                std::string text = "The image with name: " + name + ": is present in the bindings but is not registered as a sampler image" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return &image_samplers.at(binding.binding);
        }

        storage_binding* get_storage_image_by_name(std::string name)
        {
            if (!image_bindings_keys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            shader_resource& binding = image_bindings_keys.at(name);
            if (!storage_images.contains(binding.binding))
            {
                std::string text = "The image with name: " + name + ": is present in the image bindings but is not registered as a storage image" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return &storage_images.at(binding.binding);
        }       
        sampler_array* get_sampler_array_by_name(std::string name)
        {
            if (!image_bindings_keys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            shader_resource& binding = image_bindings_keys.at(name);
            if (!sampler_array_resources.contains(binding.binding))
            {
                std::string text = "The image array with name: " + name + ": is present in the bindings but is not registered as a resource this should not happen" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return &sampler_array_resources.at(binding.binding);
        }
        storage_array* get_storage_array_by_name(std::string name)
        {
            if (!image_bindings_keys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            shader_resource& binding = image_bindings_keys.at(name);
            if (!storage_array_resources.contains(binding.binding))
            {
                std::string text = "The image array with name: " + name + ": is present in the bindings but is not registered as a resource this should not happen" + get_shader_log_suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return &storage_array_resources.at(binding.binding);
        }
        void flush_buffers()
        {
            for (auto key : buffer_bindings_keys)
            {
                // buffers_resources.at(key.second.binding) = resources_manager_ref->get_staged_buff_from_name(key.first)->
                                                                               // deviceBuffer.get();
                buffers_resources.at(key.second.binding) = resources_manager_ref->get_buff_from_name(key.first);
            }
            update_descriptor();
        }
        bool has_any_data()
        {
            if (!buffer_bindings_keys.empty())
        	{
			    return true;
        	}
            if (!image_bindings_keys.empty())
        	{
			    return true;
        	}
        	return false;
        }

        resources_manager* resources_manager_ref;
        std::unordered_map<std::string, shader_resource> buffer_bindings_keys;
        std::unordered_map<std::string, shader_resource> image_bindings_keys;
        std::unordered_map<std::string, std::string> resource_shader_names;
        std::vector<std::string> shader_names;
        
        std::map<uint32_t, engine::buffer*> buffers_resources;
        std::map<uint32_t, sampler_binding> image_samplers;
        std::map<uint32_t, storage_binding> storage_images;
        std::map<uint32_t, sampler_array> sampler_array_resources;
        std::map<uint32_t, storage_array> storage_array_resources;
        
        descriptor_writer_builder writer_builder;
        vk::UniqueDescriptorSetLayout dst_layout;
        vk::DescriptorSet dst_set;
        resources_manager::dsets_info dsets_info = {};
        
        descriptor_layout_builder dst_set_builder;
        vk::DescriptorSetLayout* dst_set_layout_handle;
        
        engine::core* core;
        engine::image_view* default_image_view;
        engine::sampler* default_sampler;

    private:
        std::string get_shader_log_suffix(const std::string& resource_name)
        {
            if (resource_shader_names.contains(resource_name) && !resource_shader_names.at(resource_name).empty())
            {
                return " shader: " + resource_shader_names.at(resource_name);
            }
            if (shader_names.empty())
            {
                return "";
            }
            std::string text = " shaders: ";
            for (int i = 0; i < shader_names.size(); ++i)
            {
                if (i > 0)
                {
                    text += ", ";
                }
                text += shader_names[i];
            }
            return text;
        }
        
        engine::image_view* default_storage_image_view;
        engine::sampler* default_storage_sampler;
        bool use_shared_pool;
         
    };
   
}

#endif //DESCRIPTOR_CACHE_HPP
