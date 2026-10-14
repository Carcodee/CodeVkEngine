//
// Created by carlo on 2024-10-08.
//

#ifndef DESCRIPTOR_ALLOCATOR_HPP
#define DESCRIPTOR_ALLOCATOR_HPP

namespace engine
{
    class descriptor_allocator
    {
    public:
        struct pool_size_ratio
        {
            vk::DescriptorType type;
            float ratio;
        };

        descriptor_allocator()
        {
            
        }
        
        void begin_pool(vk::Device logical_device, uint32_t max_sets, std::span<pool_size_ratio> pool_ratios)
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
        vk::UniqueDescriptorSet allocate(vk::Device logical_device, vk::DescriptorSetLayout dst_set_layout)
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

        void clear_pool(vk::Device device)
        {
            device.resetDescriptorPool(this->pool.get()); 
        }
        void destroy_pool()
        {
            pool.release();
        }
        
        
        vk::UniqueDescriptorPool pool;
    };
}


#endif //DESCRIPTOR_ALLOCATOR_HPP
