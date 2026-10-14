//
// Created by carlo on 2024-10-08.
//




#ifndef IMAGE_SHIPPER_HPP
#define IMAGE_SHIPPER_HPP

namespace engine
{
    struct image_shipper
    {
        bool set_data_from_path(const std::string &path)
        {
            int width = 0, height = 0, channels = 0;
            stbi_uc* pixels_data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			if (!pixels_data)
			{
				data      = nullptr;
				size      = 0;
				image_size = {};
				return false;
			}
        	//4 - 1 byte per channel - R G B A
            size = width * height * 4;
            data = static_cast<void*>(pixels_data);
            image_size = {width, height};
			return true;
        }

        void set_data_raw(void* data, int width, int height, vk::DeviceSize size)
        {
            this->data = data;
            image_size = {width, height};
            this->size =size;
        }

        void build_image(engine::core* core, engine::sampler* sampler, uint32_t array_layers_count, uint32_t mips_count, vk::Format format, layout_patterns dst_pattern, std::string name, int32_t id, std::string queue_name = "Graphics")
        {
            assert(this->data && "variable \"data\" is not set or is invalid");
            vk::ImageUsageFlags usage = get_general_usage_flags(format);
            vk::ImageCreateInfo create_info = engine::image::create_info2d(image_size, mips_count, array_layers_count, format, usage);
            

            image = std::make_unique<engine::image>(core->physical_device, core->logical_device.get(), create_info);

            image_view_handle = std::make_unique<engine::image_view>(core->logical_device.get(), image->image_metadata.get(),
                                                    0, mips_count, 0, array_layers_count, name, id);

            
            auto command_executor = std::make_unique<execute_once_command>(core, queue_name);
            auto command_buffer = command_executor->begin_command_buffer();
            std::unique_ptr<engine::buffer> staged_buffer = std::make_unique<engine::buffer>(core->physical_device, core->logical_device.get(), vk::BufferUsageFlagBits::eTransferSrc,
                                                    vk::MemoryPropertyFlagBits::eHostVisible |
                                                    vk::MemoryPropertyFlagBits::eHostCoherent, size, 0);
            
            void* buffer_mem_block = staged_buffer->map();
            memcpy(buffer_mem_block, this->data, size);
            staged_buffer->unmap();
            
            transition_image(image->image_metadata.get(), transfer_dst, image_view_handle->get_subresource_range(), command_buffer);
            copy_buffer_to_image(command_buffer, staged_buffer->buffer_handle.get(), &image_view_handle->image_metadata->image_handle,
                              image_size);
            
            transition_image(image_view_handle->image_metadata, dst_pattern, image_view_handle->get_subresource_range(), command_buffer);
            command_executor->end_command_buffer();

            if (data)
            {
                free(this->data);
                data = nullptr;
            }
            this->sampler = sampler;
        }
        void build_cpu_image(std::string name, int32_t id, bool flip_y = false)
        {
            assert(this->data && "variable \"data\" is not set or is invalid");
            image_cpu.resize(static_cast<size_t>(image_size.x) * image_size.y);
                const auto *data_as_bytes = static_cast<const stbi_uc *>(data);
            for (int y = 0; y < image_size.y; ++y)
        	{
                for (int x = 0; x < image_size.x; ++x)
        		{
                    size_t y_true = flip_y ? image_size.y - 1 - y : y;
                    const size_t pixel_index =y_true * image_size.x + x;
                    const size_t byte_index  = pixel_index * 4;

                        image_cpu[static_cast<size_t>(y) * image_size.x + x] = glm::vec4(
						static_cast<float>(data_as_bytes[byte_index + 0]) / 255.0f,
						static_cast<float>(data_as_bytes[byte_index + 1]) / 255.0f,
						static_cast<float>(data_as_bytes[byte_index + 2]) / 255.0f,
						static_cast<float>(data_as_bytes[byte_index + 3]) / 255.0f);
        		}
        	}
        	
        	if (data)
        	{
        		free(this->data);
        		data = nullptr;
        	}
        }
    	
        std::vector<glm::vec4> ship_image_cpu()
        {
            return std::move(image_cpu);
        }

        std::unique_ptr<engine::image> ship_image()
        {
            return std::move(image);
        }
        std::unique_ptr<engine::image_view> ship_image_view()
        {
            return std::move(image_view_handle);
        }
        
        void clear()
        {
            if (data)
            {
                free(this->data);
				data = nullptr;
            }
        }
    	
        std::vector<glm::vec4> image_cpu;
    	
        std::unique_ptr<engine::image> image;
        std::unique_ptr<engine::image_view> image_view_handle;
        engine::sampler* sampler = nullptr;
        void* data = nullptr;
        glm::vec2 image_size{};
        vk::DeviceSize size = 0;
    };
    
}


#endif //IMAGE_SHIPPER_HPP
