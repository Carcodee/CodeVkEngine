//
// Created by carlo on 2024-10-08.
//




#ifndef IMAGESHIPPER_HPP
#define IMAGESHIPPER_HPP

namespace ENGINE
{
    struct ImageShipper 
    {
        bool SetDataFromPath(const std::string &path)
        {
            int width = 0, height = 0, channels = 0;
            stbi_uc* pixelsData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			if (!pixelsData)
			{
				data      = nullptr;
				size      = 0;
				imageSize = {};
				return false;
			}
        	//4 - 1 byte per channel - R G B A
            size = width * height * 4;
            data = static_cast<void*>(pixelsData);
            imageSize = {width, height};
			return true;
        }

        void SetDataRaw(void* data, int width, int height, vk::DeviceSize size)
        {
            this->data = data;
            imageSize = {width, height};
            this->size =size;
        }

        void BuildImage(Core* core, Sampler* sampler, uint32_t arrayLayersCount, uint32_t mipsCount, vk::Format format, LayoutPatterns dstPattern, std::string name, int32_t id, std::string queueName = "Graphics")
        {
            assert(this->data && "variable \"data\" is not set or is invalid");
            vk::ImageUsageFlags usage = GetGeneralUsageFlags(format);
            vk::ImageCreateInfo createInfo = Image::CreateInfo2d(imageSize, mipsCount, arrayLayersCount, format, usage);
            

            image = std::make_unique<Image>(core->physicalDevice, core->logicalDevice.get(), createInfo);

            imageView = std::make_unique<ImageView>(core->logicalDevice.get(), image->imageData.get(),
                                                    0, mipsCount, 0, arrayLayersCount, name, id);

            
            auto commandExecutor = std::make_unique<ExecuteOnceCommand>(core, queueName);
            auto commandBuffer = commandExecutor->BeginCommandBuffer();
            std::unique_ptr<Buffer> stagedBuffer = std::make_unique<Buffer>(core->physicalDevice, core->logicalDevice.get(), vk::BufferUsageFlagBits::eTransferSrc,
                                                    vk::MemoryPropertyFlagBits::eHostVisible |
                                                    vk::MemoryPropertyFlagBits::eHostCoherent, size, 0);
            
            void* bufferMemBlock = stagedBuffer->Map();
            memcpy(bufferMemBlock, this->data, size);
            stagedBuffer->Unmap();
            
            TransitionImage(image->imageData.get(), TRANSFER_DST, imageView->GetSubresourceRange(), commandBuffer);
            CopyBufferToImage(commandBuffer, stagedBuffer->bufferHandle.get(), &imageView->imageData->imageHandle,
                              imageSize);
            
            TransitionImage(imageView->imageData, dstPattern, imageView->GetSubresourceRange(), commandBuffer);
            commandExecutor->EndCommandBuffer();

            if (data)
            {
                free(this->data);
                data = nullptr;
            }
            this->sampler = sampler;
        }
    	void BuildCPUImage(std::string name, int32_t id, bool flipY = false)
        {
            assert(this->data && "variable \"data\" is not set or is invalid");
        	imageCPU.resize(static_cast<size_t>(imageSize.x) * imageSize.y);
	        	const auto *dataAsBytes = static_cast<const stbi_uc *>(data);
        	for (int y = 0; y < imageSize.y; ++y)
        	{
        		for (int x = 0; x < imageSize.x; ++x)
        		{
        			size_t yTrue = flipY ? imageSize.y - 1 - y : y;
        			const size_t pixelIndex =yTrue * imageSize.x + x;
        			const size_t byteIndex  = pixelIndex * 4;

	        			imageCPU[static_cast<size_t>(y) * imageSize.x + x] = glm::vec4(
						static_cast<float>(dataAsBytes[byteIndex + 0]) / 255.0f,
						static_cast<float>(dataAsBytes[byteIndex + 1]) / 255.0f,
						static_cast<float>(dataAsBytes[byteIndex + 2]) / 255.0f,
						static_cast<float>(dataAsBytes[byteIndex + 3]) / 255.0f);
        		}
        	}
        	
        	if (data)
        	{
        		free(this->data);
        		data = nullptr;
        	}
        }
    	
    	std::vector<glm::vec4> ShipImageCPU()
        {
        	return std::move(imageCPU);
        }

        std::unique_ptr<Image> ShipImage()
        {
            return std::move(image);
        }
        std::unique_ptr<ImageView> ShipImageView()
        {
            return std::move(imageView);
        }
        
        void Clear()
        {
            if (data)
            {
                free(this->data);
				data = nullptr;
            }
        }
    	
    	std::vector<glm::vec4> imageCPU;
    	
        std::unique_ptr<Image> image;
        std::unique_ptr<ImageView> imageView;
        Sampler* sampler = nullptr;
        void* data = nullptr;
        glm::vec2 imageSize{};
        vk::DeviceSize size = 0;
    };
    
}


#endif //IMAGESHIPPER_HPP
