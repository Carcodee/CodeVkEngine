//
// Created by carlo on 2024-09-21.
//


    

#ifndef SWAPCHAIN_HPP

namespace ENGINE
{
    class SwapChain
    {
    public:

        SwapChain(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, WindowDesc windowDesc 
            ,uint32_t imagesCount, QueueFamilyIndices queueFamilyIndices, vk::PresentModeKHR prefferedMode, glm::uvec2 windowSize)
        {
            
            this->logicalDevice =logicalDevice;
            this->surface = CreateWin32Surface(instance, windowDesc);
            bool invalidSurface = queueFamilyIndices.graphicsFamilyIndex == uint32_t(-1) || !physicalDevice.
                getSurfaceSupportKHR(queueFamilyIndices.presentFamilyIndex, surface.get());
            assert(!invalidSurface &&"Window surface is incompatible with device");

            this->surfaceDetails = GetSurfaceDetails(physicalDevice, this->surface.get());
            this->surfaceFormat = FindSwapchainSurfaceFormat(surfaceDetails.formats);
            this->presentMode = FindSwapchainPresentMode(surfaceDetails.presentModes, prefferedMode);
            this->extent = FindSwapChainExtent(surfaceDetails.capabilities, vk::Extent2D(windowSize.x,windowSize.y));
            this->depthFormat = FindDepthFormat(physicalDevice);

            uint32_t imageCount = std::max(surfaceDetails.capabilities.minImageCount, imagesCount);
            if (surfaceDetails.capabilities.maxImageCount > 0 && imageCount > surfaceDetails.capabilities.maxImageCount)
            {
                imageCount = surfaceDetails.capabilities.maxImageCount;
            }

            auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
                                       .setSurface(surface.get())
                                       .setMinImageCount(imageCount)
                                       .setImageFormat(surfaceFormat.format)
                                       .setImageColorSpace(surfaceFormat.colorSpace)
                                       .setImageExtent(extent)
                                       .setImageArrayLayers(1)
                                       .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                                       .setPreTransform(surfaceDetails.capabilities.currentTransform)
                                       .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                                       .setPresentMode(presentMode)
                                       .setClipped(true)
                                       .setOldSwapchain(nullptr);
            uint32_t famililyIndices[] = {queueFamilyIndices.presentFamilyIndex, queueFamilyIndices.presentFamilyIndex};
            if (queueFamilyIndices.graphicsFamilyIndex != queueFamilyIndices.presentFamilyIndex)
            {
                swapchainCreateInfo
                .setImageSharingMode(vk::SharingMode::eConcurrent)
                .setQueueFamilyIndexCount(2)
                .setPQueueFamilyIndices(famililyIndices);
                
            }else
            {
                swapchainCreateInfo
                .setImageSharingMode(vk::SharingMode::eExclusive);
            }
            this->swapchainHandle = logicalDevice.createSwapchainKHRUnique(swapchainCreateInfo);

            std::vector<vk::Image> swapChainImages = logicalDevice.getSwapchainImagesKHR(swapchainHandle.get());

            this->images.clear();

            for (int imageIndex = 0; imageIndex < swapChainImages.size(); ++imageIndex)
            {
                ImageSwapchain imageFull;
                imageFull.imageData = std::make_unique<ImageData>(swapChainImages[imageIndex], vk::ImageType::e2D,
                                                                  glm::vec3(extent.width, extent.height, 1), 1, 1,
                                                                  surfaceFormat.format, vk::ImageLayout::eUndefined);
                imageFull.imageView = std::make_unique<ImageView>(logicalDevice,
                                                                  imageFull.imageData.get(), 0, 1, 0, 1);

                images.emplace_back(std::move(imageFull));
            }
            
            this->depthImages.clear();
            this->depthImagesFull.clear();
            
            for (int i = 0; i < images.size(); ++i)
            {
                auto createInfo = ENGINE::Image::CreateInfo2d(glm::vec2(extent.width, extent.height), 1, 1, depthFormat, ENGINE::depthImageUsage);
                std::unique_ptr<ENGINE::Image> image = std::make_unique<ENGINE::Image>(physicalDevice, logicalDevice, createInfo);
                depthImages.emplace_back(std::move(image));
            }
            for (int imageIndex = 0; imageIndex < images.size(); ++imageIndex)
            {
                ImageSwapchain depthImageFull;
                depthImageFull.imageData = std::make_unique<ENGINE::ImageData>(depthImages[imageIndex]->imageHandle.get(), vk::ImageType::e2D,
                                                                  glm::vec3(extent.width, extent.height, 1), 1, 1,
                                                                  depthFormat, vk::ImageLayout::eUndefined);
                depthImageFull.imageView = std::make_unique<ENGINE::ImageView>(logicalDevice, depthImageFull.imageData.get(), 0, 1, 0, 1);

                depthImagesFull.emplace_back(std::move(depthImageFull));
            }
                        
        }
        vk::ResultValue<uint32_t> AcquireNextImage(vk::Semaphore semaphore)
        {
            return logicalDevice.acquireNextImageKHR(swapchainHandle.get(), std::numeric_limits<uint64_t>::max(), semaphore, nullptr);
        }
        std::vector<ImageView*> GetImageViews()
        {
            std::vector<ImageView*> resImageViews;
            for (auto& image : this->images)
            {
                resImageViews.push_back(image.imageView.get());
            }
            return resImageViews;
        }

        std::vector<ImageView*> GetDepthImageViews()
        {
            std::vector<ImageView*> resImageViews;
            for (auto& image : this->depthImagesFull)
            {
                resImageViews.push_back(image.imageView.get());
            }
            return resImageViews;
        }

        vk::Format GetFormat()
        {
            return surfaceFormat.format;
        }
        
        

        static vk::SurfaceFormatKHR FindSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& avalibleFormats)
        {
            vk::SurfaceFormatKHR bestFormat = {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};

            if (avalibleFormats.size() == 1 && avalibleFormats[0].format == vk::Format::eUndefined)
            {
                return bestFormat;
            }

            for (auto availableFormat : avalibleFormats)
            {
                if (availableFormat.format == bestFormat.format && availableFormat.colorSpace == bestFormat.colorSpace)
                {
                    return  availableFormat;
                }
            }
            std::cout << "No suitable format\n"; 
            return bestFormat;
            
        }

        static vk::PresentModeKHR FindSwapchainPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes,
                                                           vk::PresentModeKHR preferredMode)
        {
            for (const auto& availablePresentMode : availablePresentModes)
            {
                //if (availablePresentMode == vk::PresentModeKHR::eMailbox)
                if (availablePresentMode == preferredMode)
                    return availablePresentMode;
            }

            return vk::PresentModeKHR::eFifo;
        }

        static vk::Format FindDepthFormat(vk::PhysicalDevice physicalDevice)
        {
            std::vector<vk::Format> depthFormats = {
                vk::Format::eD32Sfloat,           
                vk::Format::eD32SfloatS8Uint,      
                vk::Format::eD24UnormS8Uint        
            };

            for (vk::Format format : depthFormats)
            {
                if (IsDepthFormat(format))
                {
                    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);

                    if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
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
        static vk::Extent2D FindSwapChainExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilitiesKHR,
                                                           vk::Extent2D windowSize)
        {
            if (surfaceCapabilitiesKHR.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return surfaceCapabilitiesKHR.currentExtent;
            }
            else
            {
                vk::Extent2D actualExtent = windowSize;
                actualExtent.width = std::max(surfaceCapabilitiesKHR.minImageExtent.width, std::min(surfaceCapabilitiesKHR.maxImageExtent.width, actualExtent.width));
                actualExtent.height = std::max(surfaceCapabilitiesKHR.minImageExtent.height, std::min(surfaceCapabilitiesKHR.maxImageExtent.height, actualExtent.height));
                
                return actualExtent;
            }            
        }
        
        



        SurfaceDetails surfaceDetails;
        vk::Device logicalDevice;
        vk::SurfaceFormatKHR surfaceFormat;
        vk::Format depthFormat; 
        vk::PresentModeKHR presentMode;
        vk::Extent2D extent;
        struct ImageSwapchain
        {
            std::unique_ptr<ImageData> imageData;
            std::unique_ptr<ImageView> imageView;
        };

        std::vector<ImageSwapchain> images;
        
        std::vector<std::unique_ptr<Image>> depthImages;
        std::vector<ImageSwapchain> depthImagesFull;

        vk::UniqueSurfaceKHR surface;
        vk::UniqueSwapchainKHR swapchainHandle;
        

    };


}
#define SWAPCHAIN_HPP

#endif //SWAPCHAIN_HPP
    
