//

// Created by carlo on 2024-11-22.
//

#ifndef RESOURCES_MANAGER_HPP
#define RESOURCES_MANAGER_HPP

#define BASE_SIZE 8192
#define DST_SET_MAX_SIZE 1280

namespace engine
{
    class Resources_Manager : systems::Subject
    {
    public:
        enum Res_State
        {
            Valid,
            Invalid
        };

        enum class Buffer_Type
        {
            Internal,
            External
        };

        struct Buffer_Update_Info
        {
            Res_State state;
            size_t size;
            size_t stride;
            void* data;
        };

        struct Img_Create_Configs
        {
            int base_mip_level = 0;
            int base_array_layer = 0;
            bool add_to_storage_clear = false;
        };

        struct Img_Update_Info
        {
            Res_State buffer_state;
            std::string path;
            uint32_t array_layers_count;
            uint32_t mips_count;
            vk::Format format;
            Layout_Patterns dst_pattern;
            std::string name;
            int32_t id;
        };

        struct Buffer_Batch_Info
        {
            std::string name;
            size_t size;
            size_t stride;
            void* data;
            vk::BufferUsageFlags buffer_usage_flags;
            vk::MemoryPropertyFlags mem_property_flags;
        };

        struct Shader_Batch_Info
        {
            std::string path;
            Shader_Stage stage;
            int32_t id;
        };

        struct Dsets_Info
        {
            vk::DescriptorSet dset;
            int32_t id = -1;
        };

        struct Buffer_Params
        {
            std::string name;
            vk::BufferUsageFlags buffer_usage_flags;
            vk::MemoryPropertyFlags mem_property_flags;
            vk::DeviceSize device_size;
            vk::DeviceSize stride;
            void* data = nullptr;
            Buffer_Type is_external = Buffer_Type::Internal;
            bool reset_every_frame = false;
        };

        engine::Image_Shipper* Get_Shipper(std::string name, std::string path, uint32_t array_layers_count,
                                           uint32_t mips_count,
                                           vk::Format format,
                                           Layout_Patterns dst_pattern)
        {
            assert(core != nullptr && "core must be set");
            engine::Image_Shipper* image_shipper;
            int32_t id = (int32_t)image_shippers.size();
            if (images_shippers_names.contains(name))
            {
                image_shipper = image_shippers.at(images_shippers_names.at(name)).get();
            }
            else
            {
                images_shippers_names.try_emplace(name, id);
                image_shippers.emplace_back(std::make_unique<engine::Image_Shipper>());
                images_update_infos.emplace_back(Img_Update_Info{
                    Valid, path, array_layers_count, mips_count, format, dst_pattern, name, id
                });
                image_shipper = Get_Shipper_From_Name(name);
            }
            if (image_shipper->image == nullptr)
            {
                image_shipper->Set_Data_From_Path(path);
                image_shipper->Build_Image(core, shipper_sampler, array_layers_count, mips_count, format, dst_pattern,
                                           name,
                                           id);
            }
            return image_shipper;
        }

        engine::Image_Shipper* Get_Shipper(std::string name, void* data, int width, int height, vk::DeviceSize size,
                                           uint32_t array_layers_count, uint32_t mips_count, vk::Format format,
                                           Layout_Patterns dst_pattern)
        {
            assert(core != nullptr && "core must be set");
            engine::Image_Shipper* image_shipper;
            int32_t id = (int32_t)image_shippers.size();
            if (images_shippers_names.contains(name))
            {
                image_shipper = image_shippers.at(images_shippers_names.at(name)).get();
            }
            else
            {
                images_shippers_names.try_emplace(name, id);
                image_shippers.emplace_back(std::make_unique<engine::Image_Shipper>());
                images_update_infos.emplace_back(Img_Update_Info{
                    Valid, "", array_layers_count, mips_count, format, dst_pattern, name, id
                });
                image_shipper = Get_Shipper_From_Name(name);
            }
            if (image_shipper->image == nullptr)
            {
                image_shipper->Set_Data_Raw(data, width, height, size);
                image_shipper->Build_Image(core, shipper_sampler, array_layers_count, mips_count, format, dst_pattern,
                                           name,
                                           id);
            }
            return image_shipper;
        }

        engine::Image_Shipper* Batch_Shipper(std::string name, std::string path, uint32_t array_layers_count,
                                             uint32_t mips_count,
                                             vk::Format format,
                                             Layout_Patterns dst_pattern)
        {
            if (images_shippers_names.contains(name))
            {
                systems::Logger::Get_Instance()->Log("Using texture that already exist: " + name,
                                                     systems::Log_Level::L_Info);
                engine::Image_Shipper* shipper = Get_Shipper_From_Name(name);
                return shipper;
            }
            int id = (int32_t)image_shippers.size();
            images_shippers_names.try_emplace(name, id);
            image_shippers.emplace_back(std::make_unique<engine::Image_Shipper>());
            images_update_infos.emplace_back(Img_Update_Info{
                Invalid, path, array_layers_count, mips_count, format, dst_pattern, name, id
            });
            update_imgs_shippers = true;
            return Get_Shipper_From_Name(name);
        }

        int Batch_Shader(std::string path, Shader_Stage stage)
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

        int Batch_Buffer(std::string name, vk::BufferUsageFlags buffer_usage_flags,
                         vk::MemoryPropertyFlags mem_property_flags, vk::DeviceSize device_size, vk::DeviceSize stride,
                         void* data = nullptr)
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

        engine::Image_View* Set_Or_Create_Image(std::string name, vk::ImageCreateInfo image_info, int base_mip_level,
                                                int base_array_layer)
        {
            assert(core != nullptr && "core must be set");
            if (!images_names.contains(name) && !storage_imgs_names.contains(name))
            {
                return Get_Image(name, image_info, base_mip_level, base_array_layer);
            }

            auto image = std::make_unique<engine::Image>(core->physical_device, core->logical_device.get(), image_info);
            if (image_info.usage & vk::ImageUsageFlagBits::eStorage)
            {
                assert(storage_imgs_names.contains(name) && "Img name dont exist");
                int32_t id = (int32_t)storage_imgs_names.at(name);
                storage_imgs_views[id]->Recreate(image->image_metadata.get());
                return storage_imgs_views[id].get();
            }
            else
            {
                assert(images_names.contains(name) && "Img name dont exist");
                int32_t id = (int32_t)images_names.at(name);
                image_views[id]->Recreate(image->image_metadata.get());
                return image_views[id].get();
            }
        }

        engine::Image_View* Get_Image(std::string name, vk::ImageCreateInfo image_info, int base_mip_level,
                                      int base_array_layer)
        {
            assert(core != nullptr && "core must be set");
            engine::Image_View* image_view_ref = nullptr;
            if (images_names.contains(name) && (image_info.usage & vk::ImageUsageFlagBits::eStorage))
            {
                image_view_ref = Get_Storage_From_Name(name);
                return image_view_ref;
            }
            else if (images_names.contains(name))
            {
                image_view_ref = Get_Image_View_From_Name(name);
                return image_view_ref;
            }

            auto image = std::make_unique<engine::Image>(core->physical_device, core->logical_device.get(), image_info);
            if (image_info.usage & vk::ImageUsageFlagBits::eStorage)
            {
                int32_t id = (int32_t)storage_imgs_views.size();
                assert(!storage_imgs_names.contains(name) && "Img name already exist");
                storage_imgs_names.try_emplace(name, (int32_t)id);

                storage_imgs_views.emplace_back(std::make_unique<engine::Image_View>(
                    core->logical_device.get(), image->image_metadata.get(),
                    base_mip_level, image_info.mipLevels, base_array_layer,
                    image_info.arrayLayers, name, id));
                images.emplace_back(std::move(image));
                return storage_imgs_views.back().get();
            }
            else
            {
                assert(!images_names.contains(name) && "Img name already exist");
                int32_t id = (int32_t)image_views.size();
                images_names.try_emplace(name, (int32_t)image_views.size());
                image_views.emplace_back(std::make_unique<engine::Image_View>(
                    core->logical_device.get(), image->image_metadata.get(),
                    base_mip_level, image_info.mipLevels, base_array_layer,
                    image_info.arrayLayers, name, id));
                images.emplace_back(std::move(image));
                return image_views.back().get();
            }
        }

        engine::Buffer* Get_Buffer(Buffer_Params params)
        {
            assert(core != nullptr && "core must be set");
            if (buffer_names.contains(params.name))
            {
                return Get_Buff_From_Name(params.name);
            }

            auto buffer = std::make_unique<engine::Buffer>(core->physical_device, core->logical_device.get(),
                                                           params.buffer_usage_flags, params.mem_property_flags,
                                                           params.device_size, params.stride, params.data,
                                                           params.is_external == Buffer_Type::External);

            buffer_names.try_emplace(params.name, (int32_t)buffers.size());
            buffers.emplace_back(std::move(buffer));
            buffers_state.push_back({Valid, params.device_size, params.stride, params.data});
            return buffers.back().get();
        }

        Staged_Buffer* Get_Stage_Buffer(std::string name, vk::BufferUsageFlags buffer_usage_flags,
                                        vk::DeviceSize device_size, vk::DeviceSize stride,
                                        void* data = nullptr)
        {
            assert(core != nullptr && "core must be set");
            if (staged_buffer_names.contains(name))
            {
                return Get_Staged_Buff_From_Name(name);
            }

            auto buffer = std::make_unique<Staged_Buffer>(
                core->physical_device, core->logical_device.get(), buffer_usage_flags, device_size, stride);

            if (data != nullptr)
            {
                void* mapped_mem = buffer->Map();
                memcpy(mapped_mem, data, device_size);
                auto command_executor = std::make_unique<Execute_Once_Command>(core);
                auto command_buffer = command_executor->Begin_Command_Buffer();
                buffer->Unmap(command_buffer);
                command_executor->End_Command_Buffer();
            }

            staged_buffer_names.try_emplace(name, (int32_t)staged_buffers.size());
            staged_buffers.emplace_back(std::move(buffer));
            staged_buffers_state.push_back({Valid, device_size});
            return staged_buffers.back().get();
        }

        engine::Buffer* Set_Buffer(std::string name, vk::DeviceSize device_size, vk::DeviceSize stride, void* data)
        {
            assert(core != nullptr && "core must be set");
            assert(buffer_names.contains(name) && "Buffer dont exist");
            assert(!staged_buffer_names.contains(name) && "buffer is in staged buffers poll");

            engine::Buffer* buffer_ref = buffers.at(buffer_names.at(name)).get();
            if (device_size > buffer_ref->device_size)
            {
                buffers_state.at(buffer_names.at(name)) = {Invalid, device_size, stride, data};
                invalidate_buffers = true;
            }
            else
            {
                // pending to handle this if is a staged resource
                engine::Buffer* buffer_ref_to_change = Get_Buff_From_Name(name);
                if (buffer_ref_to_change->mapped_mem == nullptr)
                {
                    buffer_ref_to_change->Map();
                }
                if (data != nullptr)
                {
                    memcpy(buffer_ref_to_change->mapped_mem, data, device_size);
                }else
                {
                    memset(buffer_ref_to_change->mapped_mem, 0,  device_size);
                }
                if (buffer_ref_to_change->usage_flags == vk::BufferUsageFlagBits::eStorageBuffer)
                {
                    buffer_ref_to_change->Unmap();
                }
            }

            return buffers.at(buffer_names.at(name)).get();
        }

        Staged_Buffer* Set_Stage_Buffer(std::string name, vk::DeviceSize device_size, vk::DeviceSize stride, void* data)
        {
            // todo
            assert(core != nullptr && "core must be set");
            assert(staged_buffer_names.contains(name) && "staged buffer dont exist");
            assert(!buffer_names.contains(name) && "buffer is normal buffers poll");

            if (device_size > staged_buffers.at(staged_buffer_names.at(name))->size)
            {
                staged_buffers_state.at(staged_buffer_names.at(name)) = {Invalid, device_size, stride, data};
                invalidate_buffers = true;
            }
            else
            {
                Staged_Buffer* buffer = Get_Staged_Buff_From_Name(name);
                void* mapped_mem = buffer->Map();
                memcpy(mapped_mem, data, device_size);
                auto command_executor = std::make_unique<Execute_Once_Command>(core);
                auto command_buffer = command_executor->Begin_Command_Buffer();
                buffer->Unmap(command_buffer);
                command_executor->End_Command_Buffer();
            }
            return staged_buffers.at(staged_buffer_names.at(name)).get();
        }

        void Destroy_Resources()
        {
            buffers.clear();
            staged_buffers.clear();
            storage_imgs_views.clear();
            image_views.clear();
            for (int i = 0; i < image_shippers.size(); ++i)
            {
                image_shippers[i]->Clear();
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

        void Create_Batched_Resources()
        {
            for (auto& buffer : buffer_batch_infos)
            {
                Get_Buffer(Buffer_Params{
                    buffer.name, buffer.buffer_usage_flags, buffer.mem_property_flags, buffer.size, buffer.stride,
                    buffer.data
                });
            }
            buffer_batch_infos.clear();
            for (auto& shader_update : shader_update_infos)
            {
                Get_Shader(shader_update.path, shader_update.stage);
            }
            shader_update_infos.clear();
        }

        void Update_Images()
        {
            if (!update_imgs_shippers)
            {
                return;
            }
            for (int i = 0; i < image_shippers.size(); i++)
            {
                Img_Update_Info& update_info = images_update_infos[i];
                if (update_info.buffer_state == Invalid)
                {
                    image_shippers[i]->Set_Data_From_Path(update_info.path);
                    image_shippers[i]->Build_Image(core, shipper_sampler, update_info.array_layers_count,
                                                   update_info.mips_count,
                                                   update_info.format, update_info.dst_pattern, update_info.name,
                                                   update_info.id);
                    update_info.buffer_state = Valid;
                }
            }
        }

        void Update_Buffers()
        {
            for (auto& buffer : buffers)
            {
            }
            if (!invalidate_buffers)
            {
                return;
            }
            for (auto& name : buffer_names)
            {
                Buffer_Update_Info& buffer_update_info = buffers_state.at(name.second);
                if (buffer_update_info.state == Invalid)
                {
                    buffers.at(buffer_names.at(name.first)).reset(new engine::Buffer(
                        core->physical_device, core->logical_device.get(), buffers.at(name.second)->usage_flags,
                        buffers.at(name.second)->mem_property_flags, buffer_update_info.size, buffer_update_info.stride,
                        buffer_update_info.data));
                    buffer_update_info.state = Valid;
                }
            }
            auto command_executor = std::make_unique<Execute_Once_Command>(core);
            auto command_buffer = command_executor->Begin_Command_Buffer();
            for (auto& name : staged_buffer_names)
            {
                Buffer_Update_Info& buffer_update_info = staged_buffers_state.at(name.second);
                if (buffer_update_info.state == Invalid)
                {
                    staged_buffers.at(staged_buffer_names.at(name.first)).reset(
                        new Staged_Buffer(core->physical_device, core->logical_device.get(),
                                          staged_buffers.at(name.second)->device_buffer->usage_flags,
                                          buffer_update_info.size, buffer_update_info.stride));
                    Staged_Buffer* buffer = Get_Staged_Buff_From_Name(name.first);
                    void* mapped_mem = buffer->Map();
                    memcpy(mapped_mem, buffer_update_info.data, buffer_update_info.size);
                    buffer->Unmap(command_buffer);

                    buffer_update_info.state = Valid;
                }
            }
            command_executor->End_Command_Buffer();
            Notify();
            invalidate_buffers = false;
        }

        void End_Frame_Dynamic_Updates(vk::CommandBuffer command_buffer)
        {
            Buffer_Access_Pattern src = Get_Src_Buffer_Access_Pattern(Buffer_Usage_Types::B_Graphics_Write);
            Buffer_Access_Pattern dst = Get_Src_Buffer_Access_Pattern(Buffer_Usage_Types::B_Transfer_Dst);
            Create_Mem_Barrier(src, dst, command_buffer);
            for (int i = 0; i < storage_imgs_to_clear.size(); ++i)
            {
                engine::Image_View* image_view_handle = Get_Storage_From_Name(storage_imgs_to_clear[i]);
                vk::ClearColorValue clear_color(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});

                command_buffer.clearColorImage(
                    image_view_handle->image_metadata->image_handle,
                    vk::ImageLayout::eGeneral,
                    clear_color,
                    image_view_handle->Get_Subresource_Range());
            }
            storage_imgs_to_clear.clear();
        }

        void Request_Storage_Image_Clear(std::string name)
        {
            if (!storage_imgs_names.contains(name))
            {
                systems::Logger::Get_Instance()->Log_Message("Storage Img with name does not exist: " + name);
                return;
            }
            storage_imgs_to_clear.push_back(name);
        }

        //handle spv or normal path, internally it creates the spv path
        Shader* Get_Shader(std::string path, Shader_Stage stage)
        {
            if (shaders_names.contains(path))
            {
                return shaders.at(shaders_names.at(path)).get();
            }
            int id = shaders.size();

            shaders.emplace_back(std::make_unique<Shader>(core->logical_device.get(), path, stage));

            shaders_names.try_emplace(shaders.back()->spirv_path, id);

            Shader* shader = shaders.back().get();
            return shader;
        }

        Shader* Get_Shader_From_Id(int id)
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

        Shader* Get_Or_Create_Default_Shader(std::string name, Shader_Stage stage, Shader_Compiler compiler)
        {
            std::filesystem::path target_path;
            std::filesystem::path template_path;
            switch (compiler)
            {
            case C_Glsl:
                switch (stage)
                {
                case S_Vert:
                    if (std::filesystem::path(name).extension() != ".vert")
                    {
                        name += ".vert";
                    }
                    template_path = systems::Os::Get_Instance()->glsl_shaders_template_path / "tVert.vert";
                    break;
                case S_Frag:
                    if (std::filesystem::path(name).extension() != ".frag")
                    {
                        name += ".frag";
                    }
                    template_path = systems::Os::Get_Instance()->glsl_shaders_template_path / "tFrag.frag";
                    break;

                case S_Comp:
                    if (std::filesystem::path(name).extension() != ".comp")
                    {
                        name += ".comp";
                    }
                    template_path = systems::Os::Get_Instance()->glsl_shaders_template_path / "tComp.comp";
                    break;
                case S_Tess_Control:
                    if (std::filesystem::path(name).extension() != ".tesc")
                    {
                        name += ".tesc";
                    }
                    template_path = systems::Os::Get_Instance()->glsl_shaders_template_path / "tTesc.tesc";
                    break;
                case S_Tess_Eval:
                    if (std::filesystem::path(name).extension() != ".tese")
                    {
                        name += ".tese";
                    }
                    template_path = systems::Os::Get_Instance()->glsl_shaders_template_path / "tTese.tese";
                    break;
                case S_Geom:
                    if (std::filesystem::path(name).extension() != ".geom")
                    {
                        name += ".geom";
                    }
                    template_path = systems::Os::Get_Instance()->glsl_shaders_template_path / "tGeom.geom";
                    break;
                case S_Unknown:
                    break;
                }
                target_path = systems::Os::Get_Instance()->shaders_path / "glsl" / "generated" / name;
                break;
            case C_Slang:
                if (std::filesystem::path(name).extension() != ".slang")
                {
                    name += ".slang";
                }
                target_path = systems::Os::Get_Instance()->shaders_path / "slang" / "generated" / name;
                template_path = systems::Os::Get_Instance()->slang_shaders_template_path;
                if (stage == Shader_Stage::S_Comp)
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
                return Get_Shader(target_path.string(), stage);
            }

            systems::Os::Get_Instance()->Copy_File_Into(template_path.string(), target_path.string());

            std::string spirv_path = Convert_Shader_Path_To_Spirv(target_path.string(), stage);
            Shader* shader = Get_Shader(target_path.string(), stage);

            // danger
            //  files_manager->Add_Path_To_Delete(target_path.string());
            //  files_manager->Add_Path_To_Delete(spirv_path);

            return shader;
        }

        engine::Vertex_Input* Get_Vertex_Input(std::string name)
        {
            if (vertices_inputs_names.contains(name))
            {
                return vertices_inputs.at(vertices_inputs_names.at(name)).get();
            }
            vertices_inputs_names.try_emplace(name, vertices_inputs.size());
            vertices_inputs.emplace_back(std::make_unique<engine::Vertex_Input>());
            engine::Vertex_Input* vertex_layout = vertices_inputs.back().get();
            return vertex_layout;
        }

        engine::Vertex_Input* Get_Vertex_Input_From_Id(int id)
        {
            for (auto& name : vertices_inputs_names)
            {
                if (name.second == id)
                {
                    return vertices_inputs.at(name.second).get();
                }
            }
            assert(false && "invalid vertex id");
        }

        Dsets_Info Allocate_Dset(vk::DescriptorSetLayout dst_set_layout)
        {
            if (!free_ids_bucket.empty())
            {
                for (auto& id : free_ids_bucket)
                {
                    systems::Logger::Get_Instance()->Log("Free Id: " + std::to_string(id));
                }
                int32_t id = free_ids_bucket.front();
                free_ids_bucket.erase(free_ids_bucket.begin());
                dsets.at(id) = descriptor_allocator->Allocate(core->logical_device.get(), dst_set_layout);
                return Dsets_Info{dsets.at(id).get(), id};
            }
            else
            {
                int32_t id = dsets.size();
                dsets_ids.insert(id);
                dsets.emplace_back(descriptor_allocator->Allocate(core->logical_device.get(), dst_set_layout));
                return Dsets_Info{dsets.back().get(), id};
            }
        }

        void Deallocate_Dset(int id)
        {
            if (dsets_ids.contains(id))
            {
                dsets.at(id).reset();
                free_ids_bucket.emplace_back(id);
            }
        }

        engine::Image_View* Get_Image_View_From_Name(std::string name)
        {
            if (!images_names.contains(name))
            {
                systems::Logger::Get_Instance()->Log("Img View With Name: " + name + "Does not exist");
                return nullptr;
            }
            return image_views.at(images_names.at(name)).get();
        }

        engine::Image_Shipper* Get_Shipper_From_Name(std::string name)
        {
            if (!images_shippers_names.contains(name))
            {
                systems::Logger::Get_Instance()->Log("Img Shipper With Name: " + name + "Does not exist");
                return nullptr;
            }
            return image_shippers.at(images_shippers_names.at(name)).get();
        }

        engine::Image_View* Get_Storage_From_Name(std::string name)
        {
            if (!storage_imgs_names.contains(name))
            {
                return nullptr;
            }
            return storage_imgs_views.at(storage_imgs_names.at(name)).get();
        }

        engine::Image_View* Get_Image_View_From_Id(int id)
        {
            for (auto& image_name : images_names)
            {
                if (image_name.second == id)
                {
                    return image_views.at(id).get();
                }
            }
            systems::Logger::Get_Instance()->Log("Img View With id: " + std::to_string(id) + "Does not exist");
            return nullptr;
        }

        engine::Image_Shipper* Get_Shipper_From_Id(int id)
        {
            for (auto& image_name : images_shippers_names)
            {
                if (image_name.second == id)
                {
                    return image_shippers.at(id).get();
                }
            }
            systems::Logger::Get_Instance()->Log("Img View With id: " + std::to_string(id) + "Does not exist");
            return nullptr;
        }

        engine::Image_View* Get_Storage_From_Id(int id)
        {
            for (auto& image_name : storage_imgs_names)
            {
                if (image_name.second == id)
                {
                    return storage_imgs_views.at(id).get();
                }
            }
            systems::Logger::Get_Instance()->Log("Img View With id: " + std::to_string(id) + "Does not exist");
            return nullptr;
        }

        engine::Buffer* Get_Buff_From_Name(std::string name)
        {
            if (!buffer_names.contains(name))
            {
                return nullptr;
            }
            return buffers.at(buffer_names.at(name)).get();
        }

        Staged_Buffer* Get_Staged_Buff_From_Name(std::string name)
        {
            if (!staged_buffer_names.contains(name))
            {
                return nullptr;
            }
            return staged_buffers.at(staged_buffer_names.at(name)).get();
        }

        int Get_Shipper_Id(std::string name)
        {
            return images_shippers_names.at(name);
        }

        static Resources_Manager* Get_Instance(engine::Core* core_ref = nullptr)
        {
            if (instance == nullptr && core_ref != nullptr)
            {
                instance = new Resources_Manager(core_ref);
            }
            return instance;
        }

        Resources_Manager(engine::Core* core_refs)
        {
            this->core = core_refs;
            staged_buffers.reserve(BASE_SIZE);
            buffers.reserve(BASE_SIZE);
            storage_imgs_views.reserve(BASE_SIZE);
            image_views.reserve(BASE_SIZE);
            images.reserve(BASE_SIZE);
            files_manager = std::make_unique<systems::Files_Manager>();
            descriptor_allocator = std::make_unique<engine::Descriptor_Allocator>();
            descriptor_allocator->Begin_Pool(core->logical_device.get(), DST_SET_MAX_SIZE, pool_size_ratios);

            sampler_pool = std::make_unique<engine::Sampler_Pool>();

            shipper_sampler = sampler_pool->Add_Sampler(core->logical_device.get(), vk::SamplerAddressMode::eRepeat,
                                                        vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);

            std::string default_texture_path = systems::Os::Get_Instance()->Get_Engine_Resources_Path() +
                "\\Images\\default_texture.jpg";

            engine::Image_Shipper* shipper = Get_Shipper("default_tex", default_texture_path, 1, 1, g_shipper_format,
                                                         Layout_Patterns::Graphics_Read);

            auto image_info = engine::Image::Create_Info2d(
                glm::uvec2(core->swapchain_ref->extent.width, core->swapchain_ref->extent.height), 1, 1,
                engine::g_32b_format,
                vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);

            engine::Image_View* default_storage = Get_Image("default_storage", image_info, 0, 0);

            auto image_info_bf = engine::Image::Create_Info2d(
                glm::uvec2(core->swapchain_ref->extent.width, core->swapchain_ref->extent.height), 1, 1,
                core->swapchain_ref->Get_Format(),
                vk::ImageUsageFlagBits::eColorAttachment |
                vk::ImageUsageFlagBits::eSampled);
            engine::Image_View* image_view_handle = Get_Image("bf", image_info_bf, 0, 0);

            engine::Buffer* quad_vert_buffer = Get_Stage_Buffer(
                                                   "quad_default", vk::BufferUsageFlagBits::eVertexBuffer,
                                                   sizeof(engine::Vertex_2d) * engine::Vertex_2d::Get_Quad_Vertices().
                                                   size(),
                                                   sizeof(engine::Vertex_2d),
                                                   engine::Vertex_2d::Get_Quad_Vertices().data())
                                               ->device_buffer.get();

            engine::Buffer* quad_index_buffer = Get_Stage_Buffer(
                                                    "quad_index_default", vk::BufferUsageFlagBits::eIndexBuffer,
                                                    sizeof(uint32_t) * engine::Vertex_2d::Get_Quad_Indices().size(),
                                                    sizeof(uint32_t),
                                                    engine::Vertex_2d::Get_Quad_Indices().data())
                                                ->device_buffer.get();

            engine::Buffer* cube_verts = Get_Stage_Buffer(
                                             "cube_verts_default", vk::BufferUsageFlagBits::eVertexBuffer,
                                             sizeof(D_Vertex_3d) * D_Vertex_3d::Get_Cube_Vertices().size(),
                                             sizeof(D_Vertex_3d),
                                             D_Vertex_3d::Get_Cube_Vertices().data())
                                         ->device_buffer.get();
            engine::Buffer* cube_index = Get_Stage_Buffer(
                                             "cube_index_default", vk::BufferUsageFlagBits::eIndexBuffer,
                                             sizeof(uint32_t) * D_Vertex_3d::Get_Cube_Indices().size(),
                                             sizeof(uint32_t),
                                             D_Vertex_3d::Get_Cube_Indices().data())
                                         ->device_buffer.get();
        }

        ~Resources_Manager() = default;

        void Attach(systems::Watcher* watcher) override
        {
            watchers.push_back(watcher);
        }

        void Detach(systems::Watcher* watcher) override
        {
            watchers.erase(std::remove(watchers.begin(), watchers.end(), watcher), watchers.end());
        }

        void Notify() override
        {
            for (auto& watcher : watchers)
            {
                if (watcher == nullptr)
                {
                    systems::Logger::Get_Instance()->Log_Message("Watcher is null");
                    continue;
                }
                watcher->Update_Watcher();
            }
        }

        std::vector<systems::Watcher*> watchers;

        std::unordered_map<std::string, int32_t> buffer_names;
        std::unordered_map<std::string, int32_t> staged_buffer_names;
        std::unordered_map<std::string, int32_t> images_names;
        std::unordered_map<std::string, int32_t> storage_imgs_names;
        std::unordered_map<std::string, int32_t> images_shippers_names;
        std::unordered_map<std::string, int32_t> shaders_names;
        std::unordered_map<std::string, int32_t> vertices_inputs_names;

        std::set<int32_t> dsets_ids;
        std::deque<int32_t> free_ids_bucket;

        std::vector<std::unique_ptr<engine::Buffer>> buffers;
        std::vector<std::unique_ptr<Staged_Buffer>> staged_buffers;
        std::vector<std::unique_ptr<engine::Image_View>> image_views;
        std::vector<std::unique_ptr<engine::Image_View>> storage_imgs_views;
        std::vector<std::unique_ptr<engine::Image_Shipper>> image_shippers;
        std::vector<std::unique_ptr<engine::Image>> images;
        std::vector<std::string> storage_imgs_to_clear;
        std::unique_ptr<engine::Sampler_Pool> sampler_pool;
        std::unique_ptr<engine::Descriptor_Allocator> descriptor_allocator;
        std::vector<vk::UniqueDescriptorSet> dsets;
        std::vector<std::unique_ptr<Shader>> shaders;
        std::vector<std::unique_ptr<engine::Vertex_Input>> vertices_inputs;

        std::vector<Img_Update_Info> images_update_infos;
        std::vector<Buffer_Update_Info> buffers_state;
        std::vector<Buffer_Update_Info> staged_buffers_state;

        std::vector<Shader_Batch_Info> shader_update_infos;
        std::vector<Buffer_Batch_Info> buffer_batch_infos;

        std::vector<engine::Descriptor_Allocator::Pool_Size_Ratio> pool_size_ratios = {
            {vk::DescriptorType::eSampler, 1.5f},
            {vk::DescriptorType::eStorageBuffer, 1.5f},
            {vk::DescriptorType::eUniformBuffer, 1.5f},
            // Array bindings reserve 1000 descriptors each, including unused entries.
            // With 100 sets, reserve 100000 descriptors of each image type.
            {vk::DescriptorType::eStorageImage, 1.5f},
            {vk::DescriptorType::eCombinedImageSampler, 1.5f},
        };
        std::unique_ptr<systems::Files_Manager> files_manager;

        engine::Sampler* shipper_sampler;

        bool invalidate_buffers = false;
        bool update_imgs_shippers = false;
        bool update_imgs = false;

        engine::Core* core;
        static Resources_Manager* instance;
    };

    Resources_Manager* Resources_Manager::instance = nullptr;
} // namespace engine

#endif        // RESOURCES_MANAGER_HPP
