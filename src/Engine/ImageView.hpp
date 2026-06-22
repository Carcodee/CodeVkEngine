//
// Created by carlo on 2024-09-23.
//

#ifndef IMAGEVIEW_HPP
#define IMAGEVIEW_HPP

namespace ENGINE
{
    class SwapChain;

    class ImageView
    {
    public:

        ImageView(vk::Device logicalDevice, ImageData* imageData,
                  uint32_t baseMipLevel, uint32_t mipLevelCount, uint32_t baseArrayLayer, uint32_t arrayLayersCount)
        {
            this->imageData = imageData;

            this->baseMipLevel = baseMipLevel;
            this->mipLevelCount = mipLevelCount;
            this->baseArrayLayer = baseArrayLayer;
            this->arrayLayersCount = arrayLayersCount;
        	this->logicalDevice = logicalDevice;

            auto subResourceRange = vk::ImageSubresourceRange()
                                    .setLayerCount(arrayLayersCount)
                                    .setLevelCount(mipLevelCount)
                                    .setBaseArrayLayer(baseArrayLayer)
                                    .setBaseMipLevel(baseMipLevel)
                                    .setAspectMask(imageData->aspectFlags);


            vk::ImageViewType imageViewType;

            switch (imageData->imageType)
            {
            case vk::ImageType::e1D:
                imageViewType = vk::ImageViewType::e1D;
                break;
            case vk::ImageType::e2D:
                imageViewType = vk::ImageViewType::e2D;
                break;
            case vk::ImageType::e3D:
                imageViewType = vk::ImageViewType::e3D;
                break;
            }

            auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                                       .setSubresourceRange(subResourceRange)
                                       .setFormat(imageData->format)
                                       .setImage(imageData->imageHandle)
                                       .setViewType(imageViewType);

            this->imageView = logicalDevice.createImageViewUnique(imageViewCreateInfo);
        }

        ImageView(vk::Device logicalDevice, ImageData* imageData,
                  uint32_t baseMipLevel, uint32_t mipLevelCount, uint32_t baseArrayLayer, uint32_t arrayLayersCount,
                  std::string name, int32_t id)
        {
            this->imageData = imageData;

            this->baseMipLevel = baseMipLevel;
            this->mipLevelCount = mipLevelCount;
            this->baseArrayLayer = baseArrayLayer;
            this->arrayLayersCount = arrayLayersCount;
            this->name = name;
            this->id = id;
        	this->logicalDevice = logicalDevice;

            auto subResourceRange = vk::ImageSubresourceRange()
                                    .setLayerCount(arrayLayersCount)
                                    .setLevelCount(mipLevelCount)
                                    .setBaseArrayLayer(baseArrayLayer)
                                    .setBaseMipLevel(baseMipLevel)
                                    .setAspectMask(imageData->aspectFlags);


            vk::ImageViewType imageViewType;

            switch (imageData->imageType)
            {
            case vk::ImageType::e1D:
                imageViewType = vk::ImageViewType::e1D;
                break;
            case vk::ImageType::e2D:
                imageViewType = vk::ImageViewType::e2D;
                break;
            case vk::ImageType::e3D:
                imageViewType = vk::ImageViewType::e3D;
                break;
            }

            auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                                       .setSubresourceRange(subResourceRange)
                                       .setFormat(imageData->format)
                                       .setImage(imageData->imageHandle)
                                       .setViewType(imageViewType);

            this->imageView = logicalDevice.createImageViewUnique(imageViewCreateInfo);
        }


        //cubemaps
        ImageView(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, ImageData* imageData,
                  uint32_t mipLevelCount, uint32_t baseMipLevel)
        {
            this->imageData = imageData;
            this->mipLevelCount = mipLevelCount;
            this->baseMipLevel = baseMipLevel;
            this->baseArrayLayer = 0;
            this->arrayLayersCount = 0;
        	this->logicalDevice = logicalDevice;

            auto subResourceRange = vk::ImageSubresourceRange()
                                    .setLayerCount(this->arrayLayersCount)
                                    .setLevelCount(this->mipLevelCount)
                                    .setBaseArrayLayer(this->baseArrayLayer)
                                    .setBaseMipLevel(this->baseMipLevel)
                                    .setAspectMask(imageData->aspectFlags);


            vk::ImageViewType imageViewType = vk::ImageViewType::eCube;

            assert(imageData->imageType == vk::ImageType::e2D && "ImageType must be 2d for cubemap image view");
            assert(imageData->arrayLayersCount == 6 && "array layer count must be 6 for cubemaps");

            auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                                       .setSubresourceRange(subResourceRange)
                                       .setFormat(imageData->format)
                                       .setImage(imageData->imageHandle)
                                       .setViewType(imageViewType);

            this->imageView = logicalDevice.createImageViewUnique(imageViewCreateInfo);
        }
    	void Recreate(ImageData* newImageData)
        {
        	imageData = newImageData;
        	Recreate();
        }
    	vk::ImageSubresourceRange GetSubresourceRange()
        {
        	auto subResourceRange = vk::ImageSubresourceRange()
									.setLayerCount(this->arrayLayersCount)
									.setLevelCount(this->mipLevelCount)
									.setBaseArrayLayer(this->baseArrayLayer)
									.setBaseMipLevel(this->baseMipLevel)
									.setAspectMask(imageData->aspectFlags);
        	return subResourceRange;

        }
    	
    private:
    	vk::ImageViewType GetViewType() const
    	{
    		switch (imageData->imageType)
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
    	void Recreate()
        {
        	vk::ImageViewCreateInfo createInfo{};
        	createInfo.setImage(imageData->imageHandle)
					  .setFormat(imageData->format)
					  .setViewType(GetViewType())
					  .setSubresourceRange(GetSubresourceRange());

        	imageView = logicalDevice.createImageViewUnique(createInfo);
        }
    	
    public:
    	vk::Device logicalDevice;
        uint32_t mipLevelCount;
        uint32_t baseMipLevel;
        uint32_t baseArrayLayer;
        uint32_t arrayLayersCount;
        std::string name;
        int32_t id;

        vk::UniqueImageView imageView;
        ImageData* imageData;
        
        friend class SwapChain;
    };
}

#endif //IMAGEVIEW_HPP
