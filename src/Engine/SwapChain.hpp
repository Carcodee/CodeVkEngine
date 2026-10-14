//
// Created by carlo on 2024-09-21.
//


    

#ifndef SWAP_CHAIN_HPP

namespace engine
{
    class swap_chain
    {
    public:

        swap_chain(vk::Instance instance, vk::PhysicalDevice physical_device, vk::Device logical_device, window_desc window_desc
            ,uint32_t images_count, engine::queue_family_indices queue_family_ids, vk::PresentModeKHR preffered_mode, glm::uvec2 window_size)
        {
            
            this->logical_device =logical_device;
            this->surface = create_win32_surface(instance, window_desc);
            bool invalid_surface = queue_family_ids.graphics_family_index == uint32_t(-1) || !physical_device.
                getSurfaceSupportKHR(queue_family_ids.present_family_index, surface.get());
            assert(!invalid_surface &&"Window surface is incompatible with device");

            this->surface_details = get_surface_details(physical_device, this->surface.get());
            this->surface_format = find_swapchain_surface_format(surface_details.formats);
            this->present_mode = find_swapchain_present_mode(surface_details.present_modes, preffered_mode);
            this->extent = find_swap_chain_extent(surface_details.capabilities, vk::Extent2D(window_size.x,window_size.y));
            this->depth_format = find_depth_format(physical_device);

            uint32_t image_count = std::max(surface_details.capabilities.minImageCount, images_count);
            if (surface_details.capabilities.maxImageCount > 0 && image_count > surface_details.capabilities.maxImageCount)
            {
                image_count = surface_details.capabilities.maxImageCount;
            }

            auto swapchain_create_info = vk::SwapchainCreateInfoKHR()
                                       .setSurface(surface.get())
                                       .setMinImageCount(image_count)
                                       .setImageFormat(surface_format.format)
                                       .setImageColorSpace(surface_format.colorSpace)
                                       .setImageExtent(extent)
                                       .setImageArrayLayers(1)
                                       .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                                       .setPreTransform(surface_details.capabilities.currentTransform)
                                       .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                                       .setPresentMode(present_mode)
                                       .setClipped(true)
                                       .setOldSwapchain(nullptr);
            uint32_t familily_indices[] = {queue_family_ids.present_family_index, queue_family_ids.present_family_index};
            if (queue_family_ids.graphics_family_index != queue_family_ids.present_family_index)
            {
                swapchain_create_info
                .setImageSharingMode(vk::SharingMode::eConcurrent)
                .setQueueFamilyIndexCount(2)
                .setPQueueFamilyIndices(familily_indices);
                
            }else
            {
                swapchain_create_info
                .setImageSharingMode(vk::SharingMode::eExclusive);
            }
            this->swapchain_handle = logical_device.createSwapchainKHRUnique(swapchain_create_info);

            std::vector<vk::Image> swap_chain_images = logical_device.getSwapchainImagesKHR(swapchain_handle.get());

            this->images.clear();

            for (int image_index = 0; image_index < swap_chain_images.size(); ++image_index)
            {
                image_swapchain image_full;
                image_full.image_metadata = std::make_unique<engine::image_data>(swap_chain_images[image_index], vk::ImageType::e2D,
                                                                  glm::vec3(extent.width, extent.height, 1), 1, 1,
                                                                  surface_format.format, vk::ImageLayout::eUndefined);
                image_full.image_view_handle = std::make_unique<engine::image_view>(logical_device,
                                                                  image_full.image_metadata.get(), 0, 1, 0, 1);

                images.emplace_back(std::move(image_full));
            }
            
            this->depth_images.clear();
            this->depth_images_full.clear();
            
            for (int i = 0; i < images.size(); ++i)
            {
                auto create_info = engine::image::create_info2d(glm::vec2(extent.width, extent.height), 1, 1, depth_format, engine::depth_image_usage);
                std::unique_ptr<engine::image> image = std::make_unique<engine::image>(physical_device, logical_device, create_info);
                depth_images.emplace_back(std::move(image));
            }
            for (int image_index = 0; image_index < images.size(); ++image_index)
            {
                image_swapchain depth_image_full;
                depth_image_full.image_metadata = std::make_unique<engine::image_data>(depth_images[image_index]->image_handle.get(), vk::ImageType::e2D,
                                                                  glm::vec3(extent.width, extent.height, 1), 1, 1,
                                                                  depth_format, vk::ImageLayout::eUndefined);
                depth_image_full.image_view_handle = std::make_unique<engine::image_view>(logical_device, depth_image_full.image_metadata.get(), 0, 1, 0, 1);

                depth_images_full.emplace_back(std::move(depth_image_full));
            }
                        
        }
        vk::ResultValue<uint32_t> acquire_next_image(vk::Semaphore semaphore)
        {
            return logical_device.acquireNextImageKHR(swapchain_handle.get(), std::numeric_limits<uint64_t>::max(), semaphore, nullptr);
        }
        std::vector<engine::image_view*> get_image_views()
        {
            std::vector<engine::image_view*> res_image_views;
            for (auto& image : this->images)
            {
                res_image_views.push_back(image.image_view_handle.get());
            }
            return res_image_views;
        }

        std::vector<engine::image_view*> get_depth_image_views()
        {
            std::vector<engine::image_view*> res_image_views;
            for (auto& image : this->depth_images_full)
            {
                res_image_views.push_back(image.image_view_handle.get());
            }
            return res_image_views;
        }

        vk::Format get_format()
        {
            return surface_format.format;
        }
        
        

        static vk::SurfaceFormatKHR find_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR>& avalible_formats)
        {
            vk::SurfaceFormatKHR best_format = {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};

            if (avalible_formats.size() == 1 && avalible_formats[0].format == vk::Format::eUndefined)
            {
                return best_format;
            }

            for (auto available_format : avalible_formats)
            {
                if (available_format.format == best_format.format && available_format.colorSpace == best_format.colorSpace)
                {
                    return  available_format;
                }
            }
            std::cout << "No suitable format\n"; 
            return best_format;
            
        }

        static vk::PresentModeKHR find_swapchain_present_mode(const std::vector<vk::PresentModeKHR> available_present_modes,
                                                           vk::PresentModeKHR preferred_mode)
        {
            for (const auto& available_present_mode : available_present_modes)
            {
                //if (available_present_mode == vk::PresentModeKHR::eMailbox)
                if (available_present_mode == preferred_mode)
                    return available_present_mode;
            }

            return vk::PresentModeKHR::eFifo;
        }

        static vk::Format find_depth_format(vk::PhysicalDevice physical_device)
        {
            std::vector<vk::Format> depth_formats = {
                vk::Format::eD32Sfloat,           
                vk::Format::eD32SfloatS8Uint,      
                vk::Format::eD24UnormS8Uint        
            };

            for (vk::Format format : depth_formats)
            {
                if (is_depth_format(format))
                {
                    vk::FormatProperties format_properties = physical_device.getFormatProperties(format);

                    if (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                    {
                        return format;
                    }                   
                }else
                {
                    std::cout<<"Invalid depth format\n";
                }

            }

            throw std::runtime_error("Failed to find a supported depth format!");
        }
        static vk::Extent2D find_swap_chain_extent(const vk::SurfaceCapabilitiesKHR& surface_capabilities_khr,
                                                           vk::Extent2D window_size)
        {
            if (surface_capabilities_khr.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return surface_capabilities_khr.currentExtent;
            }
            else
            {
                vk::Extent2D actual_extent = window_size;
                actual_extent.width = std::max(surface_capabilities_khr.minImageExtent.width, std::min(surface_capabilities_khr.maxImageExtent.width, actual_extent.width));
                actual_extent.height = std::max(surface_capabilities_khr.minImageExtent.height, std::min(surface_capabilities_khr.maxImageExtent.height, actual_extent.height));
                
                return actual_extent;
            }            
        }
        
        



        surface_details surface_details;
        vk::Device logical_device;
        vk::SurfaceFormatKHR surface_format;
        vk::Format depth_format;
        vk::PresentModeKHR present_mode;
        vk::Extent2D extent;
        struct image_swapchain
        {
            std::unique_ptr<engine::image_data> image_metadata;
            std::unique_ptr<engine::image_view> image_view_handle;
        };

        std::vector<image_swapchain> images;
        
        std::vector<std::unique_ptr<engine::image>> depth_images;
        std::vector<image_swapchain> depth_images_full;

        vk::UniqueSurfaceKHR surface;
        vk::UniqueSwapchainKHR swapchain_handle;
        

    };


}
#define SWAP_CHAIN_HPP

#endif //SWAP_CHAIN_HPP
    
