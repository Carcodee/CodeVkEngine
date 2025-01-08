//
// Created by carlo on 2024-10-08.
//

#ifndef DESCRIPTORS_HPP
#define DESCRIPTORS_HPP

namespace ENGINE
{
    struct DescriptorLayoutBuilder
    {
        void AddBinding(uint32_t binding, vk::DescriptorType type, int count = 1)
        {
            auto newBinding = vk::DescriptorSetLayoutBinding()
            .setBinding(binding)
            .setDescriptorCount(count)
            .setDescriptorType(type);
            bindings.push_back(newBinding);
            uniqueBindings.insert(binding);
        }


        vk::UniqueDescriptorSetLayout BuildBindings(vk::Device device, vk::ShaderStageFlags stageFlags,
                                                    void* pNext = nullptr,
                                                    vk::DescriptorSetLayoutCreateFlags flags =
                                                        vk::DescriptorSetLayoutCreateFlags())
        {
            for (auto& binding : bindings)
            {
                binding.stageFlags |= stageFlags;
            }
            auto dstSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
                                          .setPNext(pNext)
                                          .setPBindings(bindings.data())
                                          .setBindingCount(static_cast<uint32_t>(bindings.size()))
                                          .setFlags(flags);
            clear();
            return device.createDescriptorSetLayoutUnique(dstSetLayoutCreateInfo);
        }

        void clear()
        {
            bindings.clear();
            uniqueBindings.clear();
        }       
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        std::set<int> uniqueBindings;
        
    };

    struct DescriptorWriterBuilder
    {
        void AddImagesArray(int binding, std::vector<ImageView*>& imageViews, std::vector<Sampler*>& samplers, vk::ImageLayout layout,
                           vk::DescriptorType type)
        {

            std::vector<vk::DescriptorImageInfo>& imageInfosArray = imageArrayInfos.emplace_back(std::vector<vk::DescriptorImageInfo>());
            imageInfosArray.resize(imageViews.size());
            for (int i = 0; i < imageViews.size(); ++i)
            {
                if (samplers.size()!= imageViews.size())
                {
                    imageInfosArray[i].sampler = *samplers[0]->samplerHandle;
                }else
                {
                    imageInfosArray[i].sampler = *samplers[i]->samplerHandle;
                }
                imageInfosArray[i].imageView = *imageViews[i]->imageView;
                imageInfosArray[i].imageLayout = layout;
            }

            auto dstWrite = vk::WriteDescriptorSet()
                            .setDstBinding(binding)
                            .setDstSet(VK_NULL_HANDLE)
                            .setDescriptorCount((uint32_t)imageInfosArray.size())
                            .setDescriptorType(type)
                            .setPImageInfo(imageInfosArray.data());

            writes.push_back(dstWrite);
        }
        void AddWriteImage(int binding, ImageView* imageView, vk::Sampler sampler,vk::ImageLayout layout, vk::DescriptorType type)
        {

            vk::DescriptorImageInfo& imageInfo = imageInfos.emplace_back(vk::DescriptorImageInfo()
                .setSampler(sampler)
                .setImageLayout(layout)
                .setImageView(imageView->imageView.get()));

            auto dstWrite = vk::WriteDescriptorSet()
            .setDstBinding(binding)
            .setDstSet(VK_NULL_HANDLE)
            .setDescriptorCount(1)
            .setDescriptorType(type)
            .setPImageInfo(&imageInfo);
            
            writes.push_back(dstWrite);
        }
        
         void AddWriteBuffer(int binding, vk::DescriptorBufferInfo& descriptor, vk::DescriptorType type)
        {
            assert(descriptor.range != 0 && "Descriptor is not set");
            vk::DescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back(descriptor);

            auto dstWrite = vk::WriteDescriptorSet()
            .setDstBinding(binding)
            .setDstSet(VK_NULL_HANDLE)
            .setDescriptorCount(1)
            .setDescriptorType(type)
            .setPBufferInfo(&bufferInfo);

            writes.push_back(dstWrite);
        }
        
        void Clear()
        {
            imageInfos.clear();
            writes.clear();
            bufferInfos.clear();
        }
        
        void UpdateSet(vk::Device device, vk::DescriptorSet set)
        {
            for (auto& write : writes)
            {
                write.setDstSet(set);
            }
            device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
            Clear();
        }
        
        std::deque<std::vector<vk::DescriptorImageInfo>> imageArrayInfos;
        std::deque<vk::DescriptorImageInfo> imageInfos;
        std::deque<vk::DescriptorBufferInfo> bufferInfos;
        std::vector<vk::WriteDescriptorSet> writes;
    };
}

#endif //DESCRIPTORS_HPP
