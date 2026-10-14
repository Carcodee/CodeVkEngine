//
// Created by carlo on 2024-09-23.
//

#ifndef IMAGE_VIEW_HPP
#define IMAGE_VIEW_HPP

namespace engine
{
    class swap_chain;

    class image_view
    {
    public:

        image_view(vk::Device logical_device, engine::image_data* image_metadata,
                  uint32_t base_mip_level, uint32_t mip_level_count, uint32_t base_array_layer, uint32_t array_layers_count)
        {
            this->image_metadata = image_metadata;

            this->base_mip_level = base_mip_level;
            this->mip_level_count = mip_level_count;
            this->base_array_layer = base_array_layer;
            this->array_layers_count = array_layers_count;
            this->logical_device = logical_device;

            auto sub_resource_range = vk::ImageSubresourceRange()
                                    .setLayerCount(array_layers_count)
                                    .setLevelCount(mip_level_count)
                                    .setBaseArrayLayer(base_array_layer)
                                    .setBaseMipLevel(base_mip_level)
                                    .setAspectMask(image_metadata->aspect_flags);


            vk::ImageViewType image_view_type;

            switch (image_metadata->image_type)
            {
            case vk::ImageType::e1D:
                image_view_type = vk::ImageViewType::e1D;
                break;
            case vk::ImageType::e2D:
                image_view_type = vk::ImageViewType::e2D;
                break;
            case vk::ImageType::e3D:
                image_view_type = vk::ImageViewType::e3D;
                break;
            }

            auto image_view_create_info = vk::ImageViewCreateInfo()
                                       .setSubresourceRange(sub_resource_range)
                                       .setFormat(image_metadata->format)
                                       .setImage(image_metadata->image_handle)
                                       .setViewType(image_view_type);

            this->image_view_handle = logical_device.createImageViewUnique(image_view_create_info);
        }

        image_view(vk::Device logical_device, engine::image_data* image_metadata,
                  uint32_t base_mip_level, uint32_t mip_level_count, uint32_t base_array_layer, uint32_t array_layers_count,
                  std::string name, int32_t id)
        {
            this->image_metadata = image_metadata;

            this->base_mip_level = base_mip_level;
            this->mip_level_count = mip_level_count;
            this->base_array_layer = base_array_layer;
            this->array_layers_count = array_layers_count;
            this->name = name;
            this->id = id;
            this->logical_device = logical_device;

            auto sub_resource_range = vk::ImageSubresourceRange()
                                    .setLayerCount(array_layers_count)
                                    .setLevelCount(mip_level_count)
                                    .setBaseArrayLayer(base_array_layer)
                                    .setBaseMipLevel(base_mip_level)
                                    .setAspectMask(image_metadata->aspect_flags);


            vk::ImageViewType image_view_type;

            switch (image_metadata->image_type)
            {
            case vk::ImageType::e1D:
                image_view_type = vk::ImageViewType::e1D;
                break;
            case vk::ImageType::e2D:
                image_view_type = vk::ImageViewType::e2D;
                break;
            case vk::ImageType::e3D:
                image_view_type = vk::ImageViewType::e3D;
                break;
            }

            auto image_view_create_info = vk::ImageViewCreateInfo()
                                       .setSubresourceRange(sub_resource_range)
                                       .setFormat(image_metadata->format)
                                       .setImage(image_metadata->image_handle)
                                       .setViewType(image_view_type);

            this->image_view_handle = logical_device.createImageViewUnique(image_view_create_info);
        }


        //cubemaps
        image_view(vk::PhysicalDevice physical_device, vk::Device logical_device, engine::image_data* image_metadata,
                  uint32_t mip_level_count, uint32_t base_mip_level)
        {
            this->image_metadata = image_metadata;
            this->mip_level_count = mip_level_count;
            this->base_mip_level = base_mip_level;
            this->base_array_layer = 0;
            this->array_layers_count = 0;
            this->logical_device = logical_device;

            auto sub_resource_range = vk::ImageSubresourceRange()
                                    .setLayerCount(this->array_layers_count)
                                    .setLevelCount(this->mip_level_count)
                                    .setBaseArrayLayer(this->base_array_layer)
                                    .setBaseMipLevel(this->base_mip_level)
                                    .setAspectMask(image_metadata->aspect_flags);


            vk::ImageViewType image_view_type = vk::ImageViewType::eCube;

            assert(image_metadata->image_type == vk::ImageType::e2D && "ImageType must be 2d for cubemap image view");
            assert(image_metadata->array_layers_count == 6 && "array layer count must be 6 for cubemaps");

            auto image_view_create_info = vk::ImageViewCreateInfo()
                                       .setSubresourceRange(sub_resource_range)
                                       .setFormat(image_metadata->format)
                                       .setImage(image_metadata->image_handle)
                                       .setViewType(image_view_type);

            this->image_view_handle = logical_device.createImageViewUnique(image_view_create_info);
        }
        void recreate(engine::image_data* new_image_data)
        {
            image_metadata = new_image_data;
            recreate();
        }
        vk::ImageSubresourceRange get_subresource_range()
        {
            auto sub_resource_range = vk::ImageSubresourceRange()
									.setLayerCount(this->array_layers_count)
									.setLevelCount(this->mip_level_count)
									.setBaseArrayLayer(this->base_array_layer)
									.setBaseMipLevel(this->base_mip_level)
									.setAspectMask(image_metadata->aspect_flags);
            return sub_resource_range;

        }
    	
    private:
        vk::ImageViewType get_view_type() const
    	{
            switch (image_metadata->image_type)
    		{
    			case vk::ImageType::e1D:
    				return vk::ImageViewType::e1D;

    			case vk::ImageType::e2D:
    				return vk::ImageViewType::e2D;

    			case vk::ImageType::e3D:
    				return vk::ImageViewType::e3D;

    			default:
    				assert(false && "Unsupported image type");
    				return vk::ImageViewType::e2D;
    		}
    	}
        void recreate()
        {
            vk::ImageViewCreateInfo create_info{};
            create_info.setImage(image_metadata->image_handle)
					  .setFormat(image_metadata->format)
					  .setViewType(get_view_type())
					  .setSubresourceRange(get_subresource_range());

            image_view_handle = logical_device.createImageViewUnique(create_info);
        }
    	
    public:
        vk::Device logical_device;
        uint32_t mip_level_count;
        uint32_t base_mip_level;
        uint32_t base_array_layer;
        uint32_t array_layers_count;
        std::string name;
        int32_t id;

        vk::UniqueImageView image_view_handle;
        engine::image_data* image_metadata;
        
        friend class swap_chain;
    };
}

#endif //IMAGE_VIEW_HPP
