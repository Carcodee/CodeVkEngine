//
// Created by carlo on 2024-09-23.
//

#ifndef IMAGE_HPP
#define IMAGE_HPP

namespace engine
{
    static bool is_depth_format(vk::Format format)
    {
        return (format >= vk::Format::eD16Unorm && format < vk::Format::eD32SfloatS8Uint);
    }

    static vk::ImageUsageFlags get_general_usage_flags(vk::Format format)
    {
        vk::ImageUsageFlags usage_flags = vk::ImageUsageFlagBits::eSampled;
        if (is_depth_format(format))
        {
            usage_flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment |vk::ImageUsageFlagBits::eSampled;
        }else
        {
            usage_flags |= vk::ImageUsageFlagBits::eColorAttachment |vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        }
        return usage_flags;
    }

    static const vk::Format g_32b_format = vk::Format::eR32G32B32A32Sfloat;
    static const vk::Format g_16b_format = vk::Format::eR16G16B16A16Sfloat;
    static const vk::Format g_shipper_format = vk::Format::eR8G8B8A8Unorm;
    
    
    static const vk::ImageUsageFlags color_image_usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    static const vk::ImageUsageFlags depth_image_usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;

    class swap_chain;
    class image;

    class image_data
    {
    public:
        glm::uvec3 get_mip_size(uint32_t mip_level)
        {
            return mip_infos[mip_level].size;
        }
    	
        glm::uvec2 get_image_size()
        {
            return glm::uvec2(base_size.x, base_size.y);
        }

        image_data(vk::Image image_handle, vk::ImageType image_type, glm::uvec3 size, uint32_t mip_count,
                  uint32_t array_layers_count, vk::Format format, vk::ImageLayout layout)
        {
            this->image_handle = image_handle;
            this->format = format;
            this->mips_count = mip_count;
            this->array_layers_count = array_layers_count;
            this->image_type = image_type;

            current_layout = empty;
            current_pattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            current_pattern.access_mask = vk::AccessFlagBits::eNone;
            current_pattern.layout = vk::ImageLayout::eUndefined;
            current_pattern.queue_family_type = queue_family_types::undefined;

            this->base_size= size;

            for (size_t mip_level = 0; mip_level < mips_count; mip_level++)
            {
                mip_info mip_info;
                mip_info.size = this->base_size;
                mip_info.size.x /= 2;
                if (image_type == vk::ImageType::e2D)
                {
                    mip_info.size.y /= 2;
                }
                if (image_type == vk::ImageType::e3D)
                {
                    mip_info.size.z /= 2;
                }

                mip_infos.push_back(mip_info);
            }
            if (is_depth_format(format))
            {
                this->aspect_flags =vk::ImageAspectFlagBits::eDepth;
                
            }else
            {
                this->aspect_flags =vk::ImageAspectFlagBits::eColor;
            }
        }
        void set_debug_name(std::string debug_name_param)
        {
            this->debug_name = debug_name_param;
        }

        std::vector<mip_info> mip_infos;
        vk::ImageAspectFlags aspect_flags;
        vk::Image image_handle;
        vk::Format format;
        glm::uvec3 base_size;
        vk::ImageType image_type;
        uint32_t mips_count;
        uint32_t array_layers_count;
        layout_patterns current_layout;
        image_access_pattern current_pattern;
        
        std::string debug_name;
        friend class image;
        friend class swap_chain;
    };

    class image
    {
    public:

        static vk::ImageCreateInfo create_info2d(glm::uvec2 size, uint32_t mips_count, uint32_t arrays_layer_count,
                                                vk::Format format, vk::ImageUsageFlags usage)
        {
            auto layout = vk::ImageLayout::eUndefined;
            auto image_info = vk::ImageCreateInfo()
                             .setImageType(vk::ImageType::e2D)
                             .setExtent(vk::Extent3D(size.x, size.y, 1))
                             .setMipLevels(mips_count)
                             .setArrayLayers(arrays_layer_count)
                             .setFormat(format)
                             .setInitialLayout(layout)
                             .setUsage(usage)
                             .setSamples(vk::SampleCountFlagBits::e1)
                             .setFlags(vk::ImageCreateFlags());
            return image_info;
        }

        static vk::ImageCreateInfo create_info_volume(glm::uvec3 size, uint32_t mips_count, uint32_t arrays_layer_count,
                                                    vk::Format format, vk::ImageUsageFlags usage)
        {
            auto layout = vk::ImageLayout::eUndefined;
            auto image_info = vk::ImageCreateInfo()
                             .setImageType(vk::ImageType::e3D)
                             .setExtent(vk::Extent3D(size.x, size.y, size.z))
                             .setMipLevels(mips_count)
                             .setArrayLayers(arrays_layer_count)
                             .setFormat(format)
                             .setInitialLayout(layout)
                             .setUsage(usage)
                             .setSamples(vk::SampleCountFlagBits::e1)
                             .setFlags(vk::ImageCreateFlags())
                             .setTiling(vk::ImageTiling::eOptimal);
            return image_info;
        }

        image(vk::PhysicalDevice physical_device, vk::Device logical_device, vk::ImageCreateInfo image_info,
              vk::MemoryPropertyFlags mem_flags = vk::MemoryPropertyFlagBits::eDeviceLocal)
        {
            image_handle = logical_device.createImageUnique(image_info);
            glm::uvec3 size = {image_info.extent.width, image_info.extent.height, image_info.extent.depth};
            image_metadata.reset(new engine::image_data(image_handle.get(), image_info.imageType, size, image_info.mipLevels, image_info.arrayLayers, image_info.format, image_info.initialLayout));
            vk::MemoryRequirements image_mem_req= logical_device.getImageMemoryRequirements(image_handle.get());

            auto alloc_info = vk::MemoryAllocateInfo()
            .setAllocationSize(image_mem_req.size)
            .setMemoryTypeIndex(find_memory_type_index(physical_device, image_mem_req.memoryTypeBits, mem_flags));

            image_memory = logical_device.allocateMemoryUnique(alloc_info);
            
            logical_device.bindImageMemory(image_handle.get(), image_memory.get(), 0);
            
        }

        std::unique_ptr<engine::image_data> image_metadata;
        vk::UniqueImage image_handle;
        vk::UniqueDeviceMemory image_memory;
    };

}

#endif //IMAGE_HPP
