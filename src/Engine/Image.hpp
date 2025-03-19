//
// Created by carlo on 2024-09-23.
//

#ifndef IMAGE_HPP
#define IMAGE_HPP

namespace ENGINE
{
    static bool IsDepthFormat(vk::Format format)
    {
        return (format >= vk::Format::eD16Unorm && format < vk::Format::eD32SfloatS8Uint);
    }

    static vk::ImageUsageFlags GetGeneralUsageFlags(vk::Format format)
    {
        vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled;
        if (IsDepthFormat(format))
        {
            usageFlags |= vk::ImageUsageFlagBits::eDepthStencilAttachment |vk::ImageUsageFlagBits::eSampled;
        }else
        {
            usageFlags |= vk::ImageUsageFlagBits::eColorAttachment |vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        }
        return usageFlags;
    }
    
    static const vk::Format g_32bFormat = vk::Format::eR32G32B32A32Sfloat;
    static const vk::Format g_16bFormat = vk::Format::eR16G16B16A16Sfloat;
    static const vk::Format g_ShipperFormat = vk::Format::eR8G8B8A8Unorm;
    
    
    static const vk::ImageUsageFlags colorImageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    static const vk::ImageUsageFlags depthImageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;

    class SwapChain;
    class Image;

    class ImageData
    {
    public:
        glm::uvec3 GetMipSize(uint32_t mipLevel)
        {
            return mipInfos[mipLevel].size;
        }

        ImageData(vk::Image imageHandle, vk::ImageType imageType, glm::uvec3 size, uint32_t mipCount,
                  uint32_t arrayLayersCount, vk::Format format, vk::ImageLayout layout)
        {
            this->imageHandle = imageHandle;
            this->format = format;
            this->mipsCount = mipCount;
            this->arrayLayersCount = arrayLayersCount;
            this->imageType = imageType;

            currentLayout = EMPTY;
            currentPattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            currentPattern.accessMask = vk::AccessFlagBits::eNone;
            currentPattern.layout = vk::ImageLayout::eUndefined;
            currentPattern.queueFamilyType = QueueFamilyTypes::UNDEFINED;

            glm::vec3 currSize = size;

            for (size_t mipLevel = 0; mipLevel < mipsCount; mipLevel++)
            {
                MipInfo mipInfo;
                mipInfo.size = currSize;
                mipInfo.size.x /= 2;
                if (imageType == vk::ImageType::e2D)
                {
                    mipInfo.size.y /= 2;
                }
                if (imageType == vk::ImageType::e3D)
                {
                    mipInfo.size.z /= 2;
                }

                mipInfos.push_back(mipInfo);
            }
            if (IsDepthFormat(format))
            {
                this->aspectFlags =vk::ImageAspectFlagBits::eDepth;
                
            }else
            {
                this->aspectFlags =vk::ImageAspectFlagBits::eColor;
            }
        }
        void SetDebugName(std::string _debugName) 
        {
            this->debugName = _debugName;
        }

        std::vector<MipInfo> mipInfos;
        vk::ImageAspectFlags aspectFlags;
        vk::Image imageHandle;
        vk::Format format;
        vk::ImageType imageType;
        uint32_t mipsCount;
        uint32_t arrayLayersCount;
        LayoutPatterns currentLayout;
        ImageAccessPattern currentPattern;
        
        std::string debugName;
        friend class Image;
        friend class SwapChain;
    };

    class Image
    {
    public:

        static vk::ImageCreateInfo CreateInfo2d(glm::uvec2 size, uint32_t mipsCount, uint32_t arraysLayerCount,
                                                vk::Format format, vk::ImageUsageFlags usage)
        {
            auto layout = vk::ImageLayout::eUndefined;
            auto imageInfo = vk::ImageCreateInfo()
                             .setImageType(vk::ImageType::e2D)
                             .setExtent(vk::Extent3D(size.x, size.y, 1))
                             .setMipLevels(mipsCount)
                             .setArrayLayers(arraysLayerCount)
                             .setFormat(format)
                             .setInitialLayout(layout)
                             .setUsage(usage)
                             .setSamples(vk::SampleCountFlagBits::e1)
                             .setFlags(vk::ImageCreateFlags());
            return imageInfo;
        }

        static vk::ImageCreateInfo CreateInfoVolume(glm::uvec3 size, uint32_t mipsCount, uint32_t arraysLayerCount,
                                                    vk::Format format, vk::ImageUsageFlags usage)
        {
            auto layout = vk::ImageLayout::eUndefined;
            auto imageInfo = vk::ImageCreateInfo()
                             .setImageType(vk::ImageType::e3D)
                             .setExtent(vk::Extent3D(size.x, size.y, size.z))
                             .setMipLevels(mipsCount)
                             .setArrayLayers(arraysLayerCount)
                             .setFormat(format)
                             .setInitialLayout(layout)
                             .setUsage(usage)
                             .setSamples(vk::SampleCountFlagBits::e1)
                             .setFlags(vk::ImageCreateFlags())
                             .setTiling(vk::ImageTiling::eOptimal);
            return imageInfo;
        }

        Image(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::ImageCreateInfo imageInfo,
              vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal)
        {
            imageHandle = logicalDevice.createImageUnique(imageInfo);
            glm::uvec3 size = {imageInfo.extent.width, imageInfo.extent.height, imageInfo.extent.depth};
            imageData.reset(new ImageData(imageHandle.get(), imageInfo.imageType, size, imageInfo.mipLevels, imageInfo.arrayLayers, imageInfo.format, imageInfo.initialLayout));
            vk::MemoryRequirements imageMemReq= logicalDevice.getImageMemoryRequirements(imageHandle.get());

            auto allocInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(imageMemReq.size)
            .setMemoryTypeIndex(FindMemoryTypeIndex(physicalDevice, imageMemReq.memoryTypeBits, memFlags));

            imageMemory = logicalDevice.allocateMemoryUnique(allocInfo);
            
            logicalDevice.bindImageMemory(imageHandle.get(), imageMemory.get(), 0);
            
        }

        std::unique_ptr<ImageData> imageData;
        vk::UniqueImage imageHandle;
        vk::UniqueDeviceMemory imageMemory;
    };

}

#endif //IMAGE_HPP
