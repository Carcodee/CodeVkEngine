//
// Created by carlo on 2024-10-08.
//

#ifndef DESCRIPTOR_ALLOCATOR_HPP
#define DESCRIPTOR_ALLOCATOR_HPP

namespace engine
{
    class Descriptor_Allocator
    {
    public:
        struct Pool_Size_Ratio
        {
            vk::DescriptorType type;
            float ratio;
        };

        Descriptor_Allocator()
        {
            
        }
        
        void Begin_Pool(vk::Device logical_device, uint32_t max_sets, std::span<Pool_Size_Ratio> pool_ratios)
        {
            std::vector<vk::DescriptorPoolSize> pool_sizes;
            for (auto& ratio : pool_ratios)
            {
                auto pool_size= vk::DescriptorPoolSize()
                .setType(ratio.type)
                .setDescriptorCount(static_cast<uint32_t>(max_sets * ratio.ratio));
                pool_sizes.push_back(pool_size);
            }

            auto pool_info = vk::DescriptorPoolCreateInfo()
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
            .setMaxSets(max_sets)
            .setPPoolSizes(pool_sizes.data())
            .setPoolSizeCount(static_cast<uint32_t>(pool_sizes.size()));
                
             pool =  logical_device.createDescriptorPoolUnique(pool_info);


        }
        vk::UniqueDescriptorSet Allocate(vk::Device logical_device, vk::DescriptorSetLayout dst_set_layout)
        {
            auto dst_set_create_info = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(pool.get())
            .setPNext(nullptr)
            .setDescriptorSetCount(1)
            .setPSetLayouts(&dst_set_layout);
            auto sets = logical_device.allocateDescriptorSetsUnique(dst_set_create_info);
            assert(!sets.empty()&&"Descriptor sets are empty");
            return std::move(sets.front());
        }

        void Clear_Pool(vk::Device device)
        {
            device.resetDescriptorPool(this->pool.get()); 
        }
        void Destroy_Pool()
        {
            pool.release();
        }
        
        
        vk::UniqueDescriptorPool pool;
    };
}


#endif //DESCRIPTOR_ALLOCATOR_HPP
