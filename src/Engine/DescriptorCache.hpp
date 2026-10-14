//







// Created by carlo on 2024-11-19.
//

#ifndef DESCRIPTOR_CACHE_HPP
#define DESCRIPTOR_CACHE_HPP
namespace engine
{
#define DEFAULT_VAR_DESCRIPTOR_COUNT 64
    class Descriptor_Cache : public systems::Watcher
    {
        struct Sampler_Binding
        {
            engine::Image_View* image_view_handle;
            engine::Sampler* sampler;
        };
        struct Storage_Binding
        {
            engine::Image_View* image_view_handle;
            engine::Sampler* sampler;
        };
        struct Sampler_Array
        {
            std::vector<engine::Image_View*> image_views_array{};
            std::vector<engine::Sampler*> samplers;
        };
        struct Storage_Array
        {
            std::vector<engine::Image_View*> image_views_array{};
            std::vector<engine::Sampler*> samplers;
        };
    public:
        Descriptor_Cache(engine::Core* core)
        {
            this->core = core;
            resources_manager_ref = Resources_Manager::Get_Instance();
            resources_manager_ref->Attach(this);
            engine::Image_Shipper* default_shipper = resources_manager_ref->Get_Shipper_From_Name("default_tex");
            this->default_image_view = default_shipper->image_view_handle.get();
            this->default_sampler = resources_manager_ref->shipper_sampler;
            this->default_storage_image_view = resources_manager_ref->Get_Storage_From_Name("default_storage");
            this->default_storage_sampler = resources_manager_ref->shipper_sampler;
        }
        ~Descriptor_Cache()
        {
            resources_manager_ref->Detach(this);
            resources_manager_ref->Deallocate_Dset(dsets_info.id);
        }
        void Update_Watcher() override
        {
            Flush_Buffers();
        }

        void Add_Shader_Info(Shader_Parser* parser)
        {
            if (!parser->shader_name.empty())
            {
                shader_names.push_back(parser->shader_name);
            }
            std::vector<Shader_Resource> unique_resources;
            parser->Get_Layout(unique_resources);
            for (auto& resource : unique_resources)
            {
                if (dst_set_builder.unique_bindings.contains(resource.binding))
                {
                    systems::Logger::Get_Instance()->Log("Binding: " + std::to_string(resource.binding) + " already exist with name: " + resource.name, systems::Log_Level::L_Error);
                    continue;
                }
                engine::Buffer* ubo;
                engine::Buffer* ssbo;
                std::unique_ptr<engine::Image> storage_image;
                std::unique_ptr<engine::Image_Data> storage_image_data;
                std::unique_ptr<engine::Image_View> storage_image_view;
                switch (resource.type)
                {
                case vk::DescriptorType::eCombinedImageSampler:
                    assert(default_sampler != nullptr && "Default sampler is needed");
                    assert(default_image_view != nullptr && "Default Image view is needed");
                    image_bindings_keys.try_emplace(resource.name, resource);
                    resource_shader_names.try_emplace(resource.name, parser->shader_name);
                    if (resource.array){
                        sampler_array_resources.try_emplace(resource.binding, Sampler_Array({default_image_view}, {default_sampler}));
                    }else
                    {
                        image_samplers.try_emplace(resource.binding, Sampler_Binding(default_image_view, default_sampler));
                    }
                    break;
                case vk::DescriptorType::eStorageImage:
                    assert(default_storage_sampler != nullptr && "Default sampler is needed");
                    assert(default_storage_image_view != nullptr && "Default Image view is needed");
                    image_bindings_keys.try_emplace(resource.name, resource);
                    resource_shader_names.try_emplace(resource.name, parser->shader_name);
                    if (resource.array)
                    {
                        storage_array_resources.try_emplace(resource.binding,Storage_Array({default_storage_image_view}, {default_storage_sampler}));
                    }
                    else
                    {
                        storage_images.try_emplace(resource.binding, Storage_Binding(default_storage_image_view, default_storage_sampler));
                    }
                    
                    break;
                case vk::DescriptorType::eUniformBuffer:
                    buffer_bindings_keys.try_emplace(resource.name, resource);
                    resource_shader_names.try_emplace(resource.name, parser->shader_name);
                    // ubo = resources_manager_ref->Get_Stage_Buffer(resource.name, vk::BufferUsageFlagBits::eUniformBuffer, 1)->device_buffer.get();
                    ubo = resources_manager_ref->Get_Buffer(Resources_Manager::Buffer_Params{resource.name, vk::BufferUsageFlagBits::eUniformBuffer,
                                                         vk::MemoryPropertyFlagBits::eHostVisible |
                                                         vk::MemoryPropertyFlagBits::eHostCoherent, 1});
                    buffers_resources.try_emplace(resource.binding, ubo);
                    break;
                case vk::DescriptorType::eStorageBuffer:
                    buffer_bindings_keys.try_emplace(resource.name, resource);
                    resource_shader_names.try_emplace(resource.name, parser->shader_name);
                    // ssbo = resources_manager_ref->Get_Stage_Buffer(resource.name ,vk::BufferUsageFlagBits::eStorageBuffer, 1)->device_buffer.get();
                    ssbo = resources_manager_ref->Get_Buffer(Resources_Manager::Buffer_Params{resource.name, vk::BufferUsageFlagBits::eStorageBuffer,
                                                         vk::MemoryPropertyFlagBits::eHostVisible |
                                                         vk::MemoryPropertyFlagBits::eHostCoherent, 1});
                    buffers_resources.try_emplace(resource.binding,ssbo);
                    break;
                }
                if (resource.array)
                {
                    dst_set_builder.Add_Binding(resource.binding, resource.type, DEFAULT_VAR_DESCRIPTOR_COUNT);
                }else
                {
                    dst_set_builder.Add_Binding(resource.binding, resource.type);
                }
            }
        }

        void Build_Descriptors_Cache(vk::ShaderStageFlags stage_flags)
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

            dst_layout = dst_set_builder.Build_Bindings(core->logical_device.get(), stage_flags, &bindings_flags_create_info, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT);

            if (dst_set != nullptr)
        	{
                assert(dsets_info.id > -1 && "you are trying to deallocate an dset without dset info");
                resources_manager_ref->Deallocate_Dset(dsets_info.id);
        	}
            dsets_info = resources_manager_ref->Allocate_Dset(dst_layout.get());
            dst_set = dsets_info.dset;
            for (auto& buff_binding : buffer_bindings_keys)
            {
                engine::Buffer* buffer = buffers_resources.at(buff_binding.second.binding);
                writer_builder.Add_Write_Buffer(buff_binding.second.binding, buffer->descriptor, buff_binding.second.type);
            }

            for (auto image_binding : image_bindings_keys)
            {
                if (image_binding.second.array && image_binding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    Sampler_Array& image_array = sampler_array_resources.at(image_binding.second.binding);
                    writer_builder.Add_Images_Array(image_binding.second.binding, image_array.image_views_array,
                                                 image_array.samplers, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                 image_binding.second.type);
                    continue;
                }else if(image_binding.second.array && image_binding.second.type == vk::DescriptorType::eStorageImage)
                {
                    Storage_Array& image_array = storage_array_resources.at(image_binding.second.binding);
                    writer_builder.Add_Images_Array(image_binding.second.binding, image_array.image_views_array,
                                                 image_array.samplers, vk::ImageLayout::eGeneral,
                                                 image_binding.second.type);
                    continue;
                }
                if (image_binding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    Sampler_Binding& sampler_binding = image_samplers.at(image_binding.second.binding);
                    writer_builder.Add_Write_Image(image_binding.second.binding, sampler_binding.image_view_handle,
                                                sampler_binding.sampler->sampler_handle.get(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal, image_binding.second.type);
                }
                else if (image_binding.second.type == vk::DescriptorType::eStorageImage)
                {
                    Storage_Binding& binding = storage_images.at(image_binding.second.binding);
                    writer_builder.Add_Write_Image(image_binding.second.binding, binding.image_view_handle,
                                                binding.sampler->sampler_handle.get(), vk::ImageLayout::eGeneral,
                                                image_binding.second.type);
                }

            }
            writer_builder.Update_Set(core->logical_device.get(), dsets_info.dset);
        }

        void Update_Descriptor()
        {
            for (auto& buff_binding : buffer_bindings_keys)
            {
                engine::Buffer* buffer = buffers_resources.at(buff_binding.second.binding);
                writer_builder.Add_Write_Buffer(buff_binding.second.binding, buffer->descriptor, buff_binding.second.type);
            }

            for (auto& image_binding : image_bindings_keys)
            {
                if (image_binding.second.array && image_binding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    Sampler_Array& image_array = sampler_array_resources.at(image_binding.second.binding);
                    writer_builder.Add_Images_Array(image_binding.second.binding, image_array.image_views_array,
                                                 image_array.samplers, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                 image_binding.second.type);
                    continue;
                }else if(image_binding.second.array && image_binding.second.type == vk::DescriptorType::eStorageImage)
                {
                    Storage_Array& image_array = storage_array_resources.at(image_binding.second.binding);
                    writer_builder.Add_Images_Array(image_binding.second.binding, image_array.image_views_array,
                                                 image_array.samplers, vk::ImageLayout::eGeneral,
                                                 image_binding.second.type);
                    continue;
                }
                if (image_binding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    Sampler_Binding& sampler_binding = image_samplers.at(image_binding.second.binding);
                    writer_builder.Add_Write_Image(image_binding.second.binding, sampler_binding.image_view_handle,
                                                sampler_binding.sampler->sampler_handle.get(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal, image_binding.second.type);
                }else if (image_binding.second.type == vk::DescriptorType::eStorageImage)
                {
                    Storage_Binding& binding = storage_images.at(image_binding.second.binding);
                    writer_builder.Add_Write_Image(image_binding.second.binding, binding.image_view_handle,
                                                binding.sampler->sampler_handle.get(), vk::ImageLayout::eGeneral,
                                                image_binding.second.type);
                }
                
            }
            writer_builder.Update_Set(core->logical_device.get(), dsets_info.dset);
        }
        bool Has_Bindings()
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
        template<typename Value_Type>
        void Set_Buffer(std::string name, std::vector<Value_Type>& buffer_data)
        {
            engine::Buffer* buffer_ref = Get_Buffer_By_Name(name);
            if (buffer_ref==nullptr){return;}
            // resources_manager_ref->Set_Stage_Buffer(
            //     name,
            //     sizeof(T) * bufferData.size(),
            //     bufferData.data());
            resources_manager_ref->Set_Buffer(
                name,
                sizeof(Value_Type) * buffer_data.size(),
                sizeof(Value_Type),
                buffer_data.data());
            
        }

        template <typename Value_Type>
        void Set_Buffer(std::string name, Value_Type& buffer_data)
        {
            engine::Buffer* buffer_ref = Get_Buffer_By_Name(name);
            if (buffer_ref==nullptr){return;}
            // resources_manager_ref->Set_Stage_Buffer(
            //     name,
            //     sizeof(T),
            //     &bufferData);
            resources_manager_ref->Set_Buffer(
                name,
                sizeof(Value_Type),
                sizeof(Value_Type),
                &buffer_data);
        }

        void Set_Buffer(std::string name, engine::Buffer* buffer_data)
        {
            engine::Buffer* buffer_ref = Get_Buffer_By_Name(name);
            assert(buffer_data != nullptr && "Buffer sets from here should never be nullptrs");
            if (buffer_ref == nullptr)
        	{
        		return;
        	}
            Shader_Resource& binding = buffer_bindings_keys.at(name);
            
            //buffer updates is handled outside the descriptor cache
            if (buffer_ref != buffer_data)
            {
                buffers_resources.at(binding.binding) = buffer_data;
                Update_Descriptor();
            }
        }
        void Set_Sampler(std::string name, engine::Image_View* image_view_handle, engine::Sampler* sampler = nullptr)
        {
            Sampler_Binding* sampler_binding = Get_Sampler_By_Name(name);
            if (sampler_binding == nullptr)
        	{
        		return;
        	}
        	
            Shader_Resource& resource = image_bindings_keys.at(name);
            assert(resource.type == vk::DescriptorType::eCombinedImageSampler && "The image view is not a sampler image");

            if (sampler_binding == nullptr){return;}
            if (sampler_binding->image_view_handle == image_view_handle) { return; }

            if (sampler)
            {
                sampler_binding->sampler = sampler;
            }
            sampler_binding->image_view_handle = image_view_handle;
            Update_Descriptor();
        }

        void Set_Storage_Image(std::string name, engine::Image_View* image_view_handle, engine::Sampler* sampler = nullptr)
        {
             Storage_Binding* storage_binding = Get_Storage_Image_By_Name(name);
            if (storage_binding == nullptr){return;}
            Shader_Resource& resource = image_bindings_keys.at(name);
            assert(resource.type == vk::DescriptorType::eStorageImage && "The image view is not a storage image");
            if (storage_binding->image_view_handle == image_view_handle) { return; }

            if (sampler)
            {
                storage_binding->sampler = sampler;
            }
            storage_binding->image_view_handle = image_view_handle;
            Update_Descriptor();
        }

        void Set_Storage_Image_Array(std::string name, std::vector<engine::Image_View*>& image_views,
                           std::vector<engine::Sampler*>* samplers = nullptr)
        {
            Storage_Array* image_array_resource = Get_Storage_Array_By_Name(name);
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
                        std::vector<engine::Sampler*>& samplers_ref = *samplers;
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
                        std::vector<engine::Sampler*>& samplers_ref = *samplers;
                        image_array_resource->samplers.push_back(samplers_ref[i]);
                    }
                }
                Update_Descriptor();
            }
        }
        void Set_Sampler_Array(std::string name, std::vector<engine::Image_View*>& image_views, std::vector<engine::Sampler*>* samplers = nullptr)
        {
            Sampler_Array* image_array_resource = Get_Sampler_Array_By_Name(name);
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
                        std::vector<engine::Sampler*>& samplers_ref = *samplers;
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
                        std::vector<engine::Sampler*>& samplers_ref = *samplers;
                        image_array_resource->samplers.push_back(samplers_ref[i]);
                    }
                }
                Update_Descriptor();
            }
        }
        engine::Buffer* Get_Buffer_By_Name(std::string name)
        {
            if (!buffer_bindings_keys.contains(name))
            {
                std::string text = "The buffer with name: " + name + ": does not exist on shaders" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            Shader_Resource& binding = buffer_bindings_keys.at(name);
            if (!buffers_resources.contains(binding.binding))
            {
                std::string text = "The buffer with name: " + name + ": is present in the bindings but is not registered as a buffer" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return buffers_resources.at(binding.binding);
        }
        Sampler_Binding* Get_Sampler_By_Name(std::string name)
        {
            if (!image_bindings_keys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            Shader_Resource& binding = image_bindings_keys.at(name);
            if (!image_samplers.contains(binding.binding))
            {
                std::string text = "The image with name: " + name + ": is present in the bindings but is not registered as a sampler image" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return &image_samplers.at(binding.binding);
        }

        Storage_Binding* Get_Storage_Image_By_Name(std::string name)
        {
            if (!image_bindings_keys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            Shader_Resource& binding = image_bindings_keys.at(name);
            if (!storage_images.contains(binding.binding))
            {
                std::string text = "The image with name: " + name + ": is present in the image bindings but is not registered as a storage image" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return &storage_images.at(binding.binding);
        }       
        Sampler_Array* Get_Sampler_Array_By_Name(std::string name)
        {
            if (!image_bindings_keys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            Shader_Resource& binding = image_bindings_keys.at(name);
            if (!sampler_array_resources.contains(binding.binding))
            {
                std::string text = "The image array with name: " + name + ": is present in the bindings but is not registered as a resource this should not happen" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return &sampler_array_resources.at(binding.binding);
        }
        Storage_Array* Get_Storage_Array_By_Name(std::string name)
        {
            if (!image_bindings_keys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            Shader_Resource& binding = image_bindings_keys.at(name);
            if (!storage_array_resources.contains(binding.binding))
            {
                std::string text = "The image array with name: " + name + ": is present in the bindings but is not registered as a resource this should not happen" + Get_Shader_Log_Suffix(name) + "\n";
                std::cout << text;
                return nullptr;
            }
            return &storage_array_resources.at(binding.binding);
        }
        void Flush_Buffers()
        {
            for (auto key : buffer_bindings_keys)
            {
                // buffers_resources.at(key.second.binding) = resources_manager_ref->Get_Staged_Buff_From_Name(key.first)->
                                                                               // deviceBuffer.get();
                buffers_resources.at(key.second.binding) = resources_manager_ref->Get_Buff_From_Name(key.first);
            }
            Update_Descriptor();
        }
        bool Has_Any_Data()
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

        Resources_Manager* resources_manager_ref;
        std::unordered_map<std::string, Shader_Resource> buffer_bindings_keys;
        std::unordered_map<std::string, Shader_Resource> image_bindings_keys;
        std::unordered_map<std::string, std::string> resource_shader_names;
        std::vector<std::string> shader_names;
        
        std::map<uint32_t, engine::Buffer*> buffers_resources;
        std::map<uint32_t, Sampler_Binding> image_samplers;
        std::map<uint32_t, Storage_Binding> storage_images;
        std::map<uint32_t, Sampler_Array> sampler_array_resources;
        std::map<uint32_t, Storage_Array> storage_array_resources;
        
        Descriptor_Writer_Builder writer_builder;
        vk::UniqueDescriptorSetLayout dst_layout;
        vk::DescriptorSet dst_set;
        Resources_Manager::Dsets_Info dsets_info = {};
        
        Descriptor_Layout_Builder dst_set_builder;
        vk::DescriptorSetLayout* dst_set_layout_handle;
        
        engine::Core* core;
        engine::Image_View* default_image_view;
        engine::Sampler* default_sampler;

    private:
        std::string Get_Shader_Log_Suffix(const std::string& resource_name)
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
        
        engine::Image_View* default_storage_image_view;
        engine::Sampler* default_storage_sampler;
        bool use_shared_pool;
         
    };
   
}

#endif //DESCRIPTOR_CACHE_HPP
