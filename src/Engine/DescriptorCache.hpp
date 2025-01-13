//





// Created by carlo on 2024-11-19.
//

#ifndef DESCRIPTORCACHE_HPP
#define DESCRIPTORCACHE_HPP
namespace ENGINE
{
#define DEFAULT_VAR_DESCRIPTOR_COUNT 1000
    class DescriptorCache : SYSTEMS::Watcher
    {
        struct SamplerBinding 
        {
            ImageView* imageView;
            Sampler* sampler;
        };
        struct StorageBinding
        {
            ImageView* imageView;
            Sampler* sampler;
        };
        struct SamplerArray
        {
            std::vector<ImageView*> imageViewsArray{};
            std::vector<Sampler*> samplers;
        };
        struct StorageArray
        {
            std::vector<ImageView*> imageViewsArray{};
            std::vector<Sampler*> samplers;
        };
    public:
        DescriptorCache(Core* core)
        {
            this->core = core;
            resourcesManagerRef = ResourcesManager::GetInstance();
            resourcesManagerRef->Attach(this);
            ImageShipper* defaultShipper = resourcesManagerRef->GetShipperFromName("default_tex");
            this->defaultImageView = defaultShipper->imageView.get();
            this->defaultSampler = resourcesManagerRef->shipperSampler;
            this->defaultStorageImageView = resourcesManagerRef->GetStorageFromName("default_storage");
            this->defaultStorageSampler = resourcesManagerRef->shipperSampler;
            
        }
        void UpdateWatcher() override
        {
            FlushBuffers();
        }

        void AddShaderInfo(ShaderParser* parser)
        {
            std::vector<ShaderResource> uniqueResources;
            parser->GetLayout(uniqueResources);
            for (auto& resource : uniqueResources)
            {
                if (dstSetBuilder.uniqueBindings.contains(resource.binding))
                {
                    SYSTEMS::Logger::GetInstance()->Log("Binding: " + std::to_string(resource.binding) + " already exist with name: " + resource.name, SYSTEMS::LogLevel::L_ERROR);
                    continue;
                }
                Buffer* ubo;
                Buffer* ssbo;
                std::unique_ptr<Image> storageImage;
                std::unique_ptr<ImageData> storageImageData;
                std::unique_ptr<ImageView> storageImageView;
                switch (resource.type)
                {
                case vk::DescriptorType::eCombinedImageSampler:
                    assert(defaultSampler != nullptr && "Default sampler is needed");
                    assert(defaultImageView != nullptr && "Default Image view is needed");
                    imageBindingsKeys.try_emplace(resource.name, resource);
                    if (resource.array){
                        samplerArrayResources.try_emplace(resource.binding, SamplerArray({defaultImageView}, {defaultSampler}));
                    }else
                    {
                        imageSamplers.try_emplace(resource.binding, SamplerBinding(defaultImageView, defaultSampler));
                    }
                    break;
                case vk::DescriptorType::eStorageImage:
                    assert(defaultStorageSampler != nullptr && "Default sampler is needed");
                    assert(defaultStorageImageView != nullptr && "Default Image view is needed");
                    imageBindingsKeys.try_emplace(resource.name, resource);
                    if (resource.array)
                    {
                        storageArrayResources.try_emplace(resource.binding,StorageArray({defaultStorageImageView}, {defaultStorageSampler}));
                    }
                    else
                    {
                        storageImages.try_emplace(resource.binding, StorageBinding(defaultStorageImageView, defaultStorageSampler));
                    }
                    
                    break;
                case vk::DescriptorType::eUniformBuffer:
                    bufferBindingsKeys.try_emplace(resource.name, resource);
                    // ubo = resourcesManagerRef->GetStageBuffer(resource.name, vk::BufferUsageFlagBits::eUniformBuffer, 1)->deviceBuffer.get();
                    ubo = resourcesManagerRef->GetBuffer(resource.name, vk::BufferUsageFlagBits::eUniformBuffer,
                                                         vk::MemoryPropertyFlagBits::eHostVisible |
                                                         vk::MemoryPropertyFlagBits::eHostCoherent, 1);
                    buffersResources.try_emplace(resource.binding, ubo);
                    break;
                case vk::DescriptorType::eStorageBuffer:
                    bufferBindingsKeys.try_emplace(resource.name, resource);
                    // ssbo = resourcesManagerRef->GetStageBuffer(resource.name ,vk::BufferUsageFlagBits::eStorageBuffer, 1)->deviceBuffer.get();
                    ssbo = resourcesManagerRef->GetBuffer(resource.name, vk::BufferUsageFlagBits::eStorageBuffer,
                                                         vk::MemoryPropertyFlagBits::eHostVisible |
                                                         vk::MemoryPropertyFlagBits::eHostCoherent, 1);
                    buffersResources.try_emplace(resource.binding,ssbo);
                    break;
                }
                if (resource.array)
                {
                    dstSetBuilder.AddBinding(resource.binding, resource.type, DEFAULT_VAR_DESCRIPTOR_COUNT);
                }else
                {
                    dstSetBuilder.AddBinding(resource.binding, resource.type);
                }
            }
            
        }

        void BuildDescriptorsCache(DescriptorAllocator* descriptorAllocatorRef, vk::ShaderStageFlags stageFlags)
        {

            assert((imageBindingsKeys.size() + bufferBindingsKeys.size()) == dstSetBuilder.bindings.size() && "Resources and builder must have the same size");
            std::vector<vk::DescriptorBindingFlags> bindingFlags;
            for (int i = 0; i < dstSetBuilder.bindings.size(); ++i)
            {
                bindingFlags.push_back(
                    vk::DescriptorBindingFlagBits::ePartiallyBound);
            }
             auto bindingsFlagsCreateInfo = vk::DescriptorSetLayoutBindingFlagsCreateInfo()
            .setBindingCount((uint32_t)dstSetBuilder.bindings.size())
            .setPBindingFlags(bindingFlags.data());

            dstLayout = dstSetBuilder.BuildBindings(core->logicalDevice.get(), stageFlags, &bindingsFlagsCreateInfo, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT);

            dstSet = resourcesManagerRef->AllocateDset(dstLayout.get());
            for (auto& buffBinding : bufferBindingsKeys)
            {
                Buffer* buffer = buffersResources.at(buffBinding.second.binding);
                writerBuilder.AddWriteBuffer(buffBinding.second.binding, buffer->descriptor, buffBinding.second.type);
            }

            for (auto imageBinding : imageBindingsKeys)
            {
                if (imageBinding.second.array && imageBinding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    SamplerArray& imageArray = samplerArrayResources.at(imageBinding.second.binding);
                    writerBuilder.AddImagesArray(imageBinding.second.binding, imageArray.imageViewsArray,
                                                 imageArray.samplers, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                 imageBinding.second.type);
                    continue;
                }else if(imageBinding.second.array && imageBinding.second.type == vk::DescriptorType::eStorageImage)
                {
                    StorageArray& imageArray = storageArrayResources.at(imageBinding.second.binding);
                    writerBuilder.AddImagesArray(imageBinding.second.binding, imageArray.imageViewsArray,
                                                 imageArray.samplers, vk::ImageLayout::eGeneral,
                                                 imageBinding.second.type);
                    continue;
                }
                if (imageBinding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    SamplerBinding& samplerBinding = imageSamplers.at(imageBinding.second.binding);
                    writerBuilder.AddWriteImage(imageBinding.second.binding, samplerBinding.imageView,
                                                samplerBinding.sampler->samplerHandle.get(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal, imageBinding.second.type);
                }
                else if (imageBinding.second.type == vk::DescriptorType::eStorageImage)
                {
                    StorageBinding& binding = storageImages.at(imageBinding.second.binding);
                    writerBuilder.AddWriteImage(imageBinding.second.binding, binding.imageView,
                                                binding.sampler->samplerHandle.get(), vk::ImageLayout::eGeneral,
                                                imageBinding.second.type);
                }

            }
            writerBuilder.UpdateSet(core->logicalDevice.get(), dstSet);
        }

        void UpdateDescriptor()
        {
            for (auto& buffBinding : bufferBindingsKeys)
            {
                Buffer* buffer = buffersResources.at(buffBinding.second.binding);
                writerBuilder.AddWriteBuffer(buffBinding.second.binding, buffer->descriptor, buffBinding.second.type);
            }

            for (auto& imageBinding : imageBindingsKeys)
            {
                if (imageBinding.second.array && imageBinding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    SamplerArray& imageArray = samplerArrayResources.at(imageBinding.second.binding);
                    writerBuilder.AddImagesArray(imageBinding.second.binding, imageArray.imageViewsArray,
                                                 imageArray.samplers, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                 imageBinding.second.type);
                    continue;
                }else if(imageBinding.second.array && imageBinding.second.type == vk::DescriptorType::eStorageImage)
                {
                    StorageArray& imageArray = storageArrayResources.at(imageBinding.second.binding);
                    writerBuilder.AddImagesArray(imageBinding.second.binding, imageArray.imageViewsArray,
                                                 imageArray.samplers, vk::ImageLayout::eGeneral,
                                                 imageBinding.second.type);
                    continue;
                }
                if (imageBinding.second.type == vk::DescriptorType::eCombinedImageSampler)
                {
                    SamplerBinding& samplerBinding = imageSamplers.at(imageBinding.second.binding);
                    writerBuilder.AddWriteImage(imageBinding.second.binding, samplerBinding.imageView,
                                                samplerBinding.sampler->samplerHandle.get(),
                                                vk::ImageLayout::eShaderReadOnlyOptimal, imageBinding.second.type);
                }else if (imageBinding.second.type == vk::DescriptorType::eStorageImage)
                {
                    StorageBinding& binding = storageImages.at(imageBinding.second.binding);
                    writerBuilder.AddWriteImage(imageBinding.second.binding, binding.imageView,
                                                binding.sampler->samplerHandle.get(), vk::ImageLayout::eGeneral,
                                                imageBinding.second.type);
                }
                
            }
            writerBuilder.UpdateSet(core->logicalDevice.get(), dstSet);
        }
        template<typename T>
        void SetBuffer(std::string name, std::vector<T>& bufferData)
        {
            ShaderResource& binding = bufferBindingsKeys.at(name);
            Buffer* bufferRef = GetBufferByName(name);
            if (bufferRef==nullptr){return;}
            // resourcesManagerRef->SetStageBuffer(
            //     name,
            //     sizeof(T) * bufferData.size(),
            //     bufferData.data());
            resourcesManagerRef->SetBuffer(
                name,
                sizeof(T) * bufferData.size(),
                bufferData.data());
        }

        template <typename T>
        void SetBuffer(std::string name, T& bufferData)
        {
            ShaderResource& binding = bufferBindingsKeys.at(name);
            Buffer* bufferRef = GetBufferByName(name);
            if (bufferRef==nullptr){return;}
            // resourcesManagerRef->SetStageBuffer(
            //     name,
            //     sizeof(T),
            //     &bufferData);
            resourcesManagerRef->SetBuffer(
                name,
                sizeof(T),
                &bufferData);
        }

        void SetBuffer(std::string name, Buffer* bufferData)
        {
            ShaderResource& binding = bufferBindingsKeys.at(name);
            Buffer* bufferRef = GetBufferByName(name);
            
            assert(bufferRef != nullptr && "Buffer does not exist in shaders");
            //buffer updates is handled outside the descriptor cache
            if (bufferRef != bufferData)
            {
                buffersResources.at(binding.binding) = bufferData;
                UpdateDescriptor();
            }
        }
        void SetSampler(std::string name, ImageView* imageView, Sampler* sampler = nullptr)
        {
            ShaderResource& resource = imageBindingsKeys.at(name);
            assert(resource.type == vk::DescriptorType::eCombinedImageSampler && "The image view is not a sampler image");
            SamplerBinding* samplerBinding = GetSamplerByName(name);

            if (samplerBinding == nullptr){return;}
            if (samplerBinding->imageView == imageView) { return; }

            if (sampler)
            {
                samplerBinding->sampler = sampler;
            }
            samplerBinding->imageView = imageView;
            UpdateDescriptor();    
        }

        void SetStorageImage(std::string name, ImageView* imageView, Sampler* sampler = nullptr)
        {
            ShaderResource& resource = imageBindingsKeys.at(name);
            assert(resource.type == vk::DescriptorType::eStorageImage && "The image view is not a storage image");
             StorageBinding* storageBinding = GetStorageImageByName(name);

            if (storageBinding == nullptr){return;}
            if (storageBinding->imageView == imageView) { return; }

            if (sampler)
            {
                storageBinding->sampler = sampler;
            }
            storageBinding->imageView = imageView;
            UpdateDescriptor();               
        }

        void SetStorageImageArray(std::string name, std::vector<ImageView*>& imageViews,
                           std::vector<Sampler*>* samplers = nullptr)
        {
            StorageArray* imageArrayResource = GetStorageArrayByName(name);
            if (imageArrayResource == nullptr) return;

            bool update = false;
            if (imageViews.size() != imageArrayResource->imageViewsArray.size())
            {
                update = true;
            }else
            {
                for (int i = 0; i < imageArrayResource->imageViewsArray.size(); ++i)
                {
                    if (imageArrayResource->imageViewsArray[i] != imageViews[i])
                    {
                        update = true;
                        break;
                    }
                }
            }
            if (samplers != nullptr && !update)
            {
                if (samplers->size() != imageArrayResource->samplers.size())
                {
                    update = true;
                }
                else
                {
                    for (int i = 0; i < imageArrayResource->samplers.size(); ++i)
                    {
                        std::vector<Sampler*>& samplersRef = *samplers;
                        if (imageArrayResource->samplers[i] != samplersRef[i])
                        {
                            update = true;
                            break;
                        }
                    }
                }               
            }
            if (update)
            {

                imageArrayResource->imageViewsArray.clear();
                imageArrayResource->imageViewsArray.reserve((uint32_t)imageViews.size());
                for (int i = 0; i < imageViews.size(); ++i)
                {
                    imageArrayResource->imageViewsArray.push_back(imageViews[i]);
                }
                if (samplers != nullptr)
                {
                    imageArrayResource->samplers.clear();
                    imageArrayResource->samplers.reserve((uint32_t)samplers->size());
                    for (int i = 0; i < samplers->size(); ++i)
                    {
                        std::vector<Sampler*>& samplersRef = *samplers; 
                        imageArrayResource->samplers.push_back(samplersRef[i]);
                    }
                }
                UpdateDescriptor();               
            }
        }
        void SetSamplerArray(std::string name, std::vector<ImageView*>& imageViews, std::vector<Sampler*>* samplers = nullptr)
        {
            SamplerArray* imageArrayResource = GetSamplerArrayByName(name);
            if (imageArrayResource == nullptr) return;

            bool update = false;
            if (imageViews.size() != imageArrayResource->imageViewsArray.size())
            {
                update = true;
            }else
            {
                for (int i = 0; i < imageArrayResource->imageViewsArray.size(); ++i)
                {
                    if (imageArrayResource->imageViewsArray[i] != imageViews[i])
                    {
                        update = true;
                        break;
                    }
                }
            }
            if (samplers != nullptr && !update)
            {
                if (samplers->size() != imageArrayResource->samplers.size())
                {
                    update = true;
                }
                else
                {
                    for (int i = 0; i < imageArrayResource->samplers.size(); ++i)
                    {
                        std::vector<Sampler*>& samplersRef = *samplers;
                        if (imageArrayResource->samplers[i] != samplersRef[i])
                        {
                            update = true;
                            break;
                        }
                    }
                }               
            }
            if (update)
            {

                imageArrayResource->imageViewsArray.clear();
                imageArrayResource->imageViewsArray.reserve((uint32_t)imageViews.size());
                for (int i = 0; i < imageViews.size(); ++i)
                {
                    imageArrayResource->imageViewsArray.push_back(imageViews[i]);
                }
                if (samplers != nullptr)
                {
                    imageArrayResource->samplers.clear();
                    imageArrayResource->samplers.reserve((uint32_t)samplers->size());
                    for (int i = 0; i < samplers->size(); ++i)
                    {
                        std::vector<Sampler*>& samplersRef = *samplers; 
                        imageArrayResource->samplers.push_back(samplersRef[i]);
                    }
                }
                UpdateDescriptor();               
            }
        }
        Buffer* GetBufferByName(std::string name)
        {
            if (!bufferBindingsKeys.contains(name))
            {
                std::string text = "The buffer with name: " + name + ": does not exist on shaders\n";
                std::cout << text;
                return nullptr;
            }
            ShaderResource& binding = bufferBindingsKeys.at(name);
            if (!buffersResources.contains(binding.binding))
            {
                std::string text = "The buffer with name: " + name + ": is present in the bindings but is not registered as a buffer\n";
                std::cout << text;
                return nullptr;
            }
            return buffersResources.at(binding.binding);
        }
        SamplerBinding* GetSamplerByName(std::string name)
        {
            if (!imageBindingsKeys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders\n";
                std::cout << text;
                return nullptr;
            }
            ShaderResource& binding = imageBindingsKeys.at(name);
            if (!imageSamplers.contains(binding.binding))
            {
                std::string text = "The image with name: " + name + ": is present in the bindings but is not registered as a sampler image \n";
                std::cout << text;
                return nullptr;
            }
            return &imageSamplers.at(binding.binding);
        }

        StorageBinding* GetStorageImageByName(std::string name)
        {
            if (!imageBindingsKeys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders\n";
                std::cout << text;
                return nullptr;
            }
            ShaderResource& binding = imageBindingsKeys.at(name);
            if (!storageImages.contains(binding.binding))
            {
                std::string text = "The image with name: " + name + ": is present in the image bindings but is not registered as a storage image\n";
                std::cout << text;
                return nullptr;
            }
            return &storageImages.at(binding.binding);
        }       
        SamplerArray* GetSamplerArrayByName(std::string name)
        {
            if (!imageBindingsKeys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders\n";
                std::cout << text;
                return nullptr;
            }
            ShaderResource& binding = imageBindingsKeys.at(name);
            if (!samplerArrayResources.contains(binding.binding))
            {
                std::string text = "The image array with name: " + name + ": is present in the bindings but is not registered as a resource this should not happen\n";
                std::cout << text;
                return nullptr;
            }
            return &samplerArrayResources.at(binding.binding);
        }
        StorageArray* GetStorageArrayByName(std::string name)
        {
            if (!imageBindingsKeys.contains(name))
            {
                std::string text = "The image with name: " + name + ": does not exist on shaders\n";
                std::cout << text;
                return nullptr;
            }
            ShaderResource& binding = imageBindingsKeys.at(name);
            if (!storageArrayResources.contains(binding.binding))
            {
                std::string text = "The image array with name: " + name + ": is present in the bindings but is not registered as a resource this should not happen\n";
                std::cout << text;
                return nullptr;
            }
            return &storageArrayResources.at(binding.binding);
        }
        void FlushBuffers()
        {
            for (auto key : bufferBindingsKeys)
            {
                // buffersResources.at(key.second.binding) = resourcesManagerRef->GetStagedBuffFromName(key.first)->
                                                                               // deviceBuffer.get();
                buffersResources.at(key.second.binding) = resourcesManagerRef->GetBuffFromName(key.first);
            }
            UpdateDescriptor();
        }

        ResourcesManager* resourcesManagerRef;
        std::unordered_map<std::string, ShaderResource> bufferBindingsKeys;
        std::unordered_map<std::string, ShaderResource> imageBindingsKeys;
        
        std::map<uint32_t, Buffer*> buffersResources;
        std::map<uint32_t, SamplerBinding> imageSamplers;
        std::map<uint32_t, StorageBinding> storageImages;
        std::map<uint32_t, SamplerArray> samplerArrayResources;
        std::map<uint32_t, StorageArray> storageArrayResources;
        
        DescriptorWriterBuilder writerBuilder;
        vk::UniqueDescriptorSetLayout dstLayout;
        vk::DescriptorSet dstSet;
        
        DescriptorLayoutBuilder dstSetBuilder;
        vk::DescriptorSetLayout* dstSetLayoutHandle;
        
        Core* core;
        ImageView* defaultImageView;
        Sampler* defaultSampler;
        
        ImageView* defaultStorageImageView;
        Sampler* defaultStorageSampler;
        bool useSharedPool;
         
    };
   
}

#endif //DESCRIPTORCACHE_HPP
