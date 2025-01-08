//
// Created by carlo on 2024-10-08.
//




#ifndef IMAGESHIPPER_HPP
#define IMAGESHIPPER_HPP

namespace ENGINE
{
    struct ImageShipper 
    {
        void SetDataFromPath(std::string path)
        {
            int width, height, channels;
            stbi_uc* pixelsData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            size = width * height * 4;
            data = static_cast<void*>(pixelsData);
            imageSize = {width, height};
            
        }

        void SetDataRaw(void* data, int width, int height, vk::DeviceSize size)
        {
            this->data = data;
            imageSize = {width, height};
            this->size =size;
        }

        void BuildImage(Core* core, uint32_t arrayLayersCount, uint32_t mipsCount, vk::Format format, LayoutPatterns dstPattern, std::string name, int32_t id)
        {
            assert(this->data && "variable \"data\" is not set or is invalid");
            vk::ImageUsageFlags usage = GetGeneralUsageFlags(format);
            vk::ImageCreateInfo createInfo = Image::CreateInfo2d(imageSize, mipsCount, arrayLayersCount, format, usage);

            

            image = std::make_unique<Image>(core->physicalDevice, core->logicalDevice.get(), createInfo);

            imageView = std::make_unique<ImageView>(core->logicalDevice.get(), image->imageData.get(),
                                                    0, mipsCount, 0, arrayLayersCount, name, id);

            
            auto commandExecutor = std::make_unique<ExecuteOnceCommand>(core);
            auto commandBuffer = commandExecutor->BeginCommandBuffer();
            std::unique_ptr<Buffer> stagedBuffer = std::make_unique<Buffer>(core->physicalDevice, core->logicalDevice.get(), vk::BufferUsageFlagBits::eTransferSrc,
                                                    vk::MemoryPropertyFlagBits::eHostVisible |
                                                    vk::MemoryPropertyFlagBits::eHostCoherent, size);
            
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
            sampler = core->renderGraphRef->samplerPool.GetSampler(vk::SamplerAddressMode::eRepeat,
                                                vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);
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
            if (imageView){imageView.release();}
            if (image){image.release();}
            if (data)
            {
                free(this->data);
            }
        }

        std::unique_ptr<Image> image;
        std::unique_ptr<ImageView> imageView;
        Sampler* sampler;
        void* data;
        glm::vec2 imageSize;
        vk::DeviceSize size;
    };
    
}


#endif //IMAGESHIPPER_HPP
