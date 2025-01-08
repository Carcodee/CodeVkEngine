//
// Created by carlo on 2024-10-08.
//

#ifndef DESPRIPTORALLOCATOR_HPP
#define DESPRIPTORALLOCATOR_HPP

namespace ENGINE
{
    class DescriptorAllocator
    {
    public:
        struct PoolSizeRatio
        {
            vk::DescriptorType type;
            float ratio;
        };

        DescriptorAllocator()
        {
            
        }
        
        void BeginPool(vk::Device logicalDevice, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
        {
            std::vector<vk::DescriptorPoolSize> poolSizes;
            for (auto& ratio : poolRatios)
            {
                auto poolSize= vk::DescriptorPoolSize()
                .setType(ratio.type)
                .setDescriptorCount(static_cast<uint32_t>(maxSets * ratio.ratio));
                poolSizes.push_back(poolSize);
            }

            auto poolInfo = vk::DescriptorPoolCreateInfo()
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
            .setMaxSets(maxSets)
            .setPPoolSizes(poolSizes.data())
            .setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()));
                
             pool =  logicalDevice.createDescriptorPoolUnique(poolInfo);


        }
        vk::UniqueDescriptorSet Allocate(vk::Device logicalDevice, vk::DescriptorSetLayout dstSetLayout)
        {
            auto dstSetCreateInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(pool.get())
            .setPNext(nullptr)
            .setDescriptorSetCount(1)
            .setPSetLayouts(&dstSetLayout);
            auto sets = logicalDevice.allocateDescriptorSetsUnique(dstSetCreateInfo);
            assert(!sets.empty()&&"Descriptor sets are empty");
            return std::move(sets.front());
        }

        void ClearPool(vk::Device device)
        {
            device.resetDescriptorPool(this->pool.get()); 
        }
        void DestroyPool()
        {
            pool.release();
        }
        
        
        vk::UniqueDescriptorPool pool;
    };
}


#endif //DESPRIPTORALLOCATOR_HPP
