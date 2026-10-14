//
// Created by carlo on 2024-10-08.
//

#ifndef DESCRIPTORS_HPP
#define DESCRIPTORS_HPP

namespace engine
{
    struct descriptor_layout_builder
    {
        void add_binding(uint32_t binding, vk::DescriptorType type, int count = 1)
        {
            auto new_binding = vk::DescriptorSetLayoutBinding()
            .setBinding(binding)
            .setDescriptorCount(count)
            .setDescriptorType(type);
            bindings.push_back(new_binding);
            unique_bindings.insert(binding);
        }


        vk::UniqueDescriptorSetLayout build_bindings(vk::Device device, vk::ShaderStageFlags stage_flags,
                                                    void* p_next = nullptr,
                                                    vk::DescriptorSetLayoutCreateFlags flags =
                                                        vk::DescriptorSetLayoutCreateFlags())
        {
            for (auto& binding : bindings)
            {
                binding.stageFlags |= stage_flags;
            }
            auto dst_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo()
                                          .setPNext(p_next)
                                          .setPBindings(bindings.data())
                                          .setBindingCount(static_cast<uint32_t>(bindings.size()))
                                          .setFlags(flags);
            clear();
            return device.createDescriptorSetLayoutUnique(dst_set_layout_create_info);
        }

        void clear()
        {
            bindings.clear();
            unique_bindings.clear();
        }       
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        std::set<int> unique_bindings;
        
    };

    struct descriptor_writer_builder
    {
        void add_images_array(int binding, std::vector<engine::image_view*>& image_views, std::vector<engine::sampler*>& samplers, vk::ImageLayout layout,
                           vk::DescriptorType type)
        {
            // Partially bound arrays may be empty, but Vulkan writes may not be.
            if (image_views.empty()) return;

            std::vector<vk::DescriptorImageInfo>& image_infos_array = image_array_infos.emplace_back(std::vector<vk::DescriptorImageInfo>());
            image_infos_array.resize(image_views.size());
            for (int i = 0; i < image_views.size(); ++i)
            {
                if (samplers.size()!= image_views.size())
                {
                    image_infos_array[i].sampler = *samplers[0]->sampler_handle;
                }else
                {
                    image_infos_array[i].sampler = *samplers[i]->sampler_handle;
                }
                image_infos_array[i].imageView = *image_views[i]->image_view_handle;
                image_infos_array[i].imageLayout = layout;
            }

            auto dst_write = vk::WriteDescriptorSet()
                            .setDstBinding(binding)
                            .setDstSet(VK_NULL_HANDLE)
                            .setDescriptorCount((uint32_t)image_infos_array.size())
                            .setDescriptorType(type)
                            .setPImageInfo(image_infos_array.data());

            writes.push_back(dst_write);
        }
        void add_write_image(int binding, engine::image_view* image_view_handle, vk::Sampler sampler,vk::ImageLayout layout, vk::DescriptorType type)
        {

            vk::DescriptorImageInfo& image_info = image_infos.emplace_back(vk::DescriptorImageInfo()
                .setSampler(sampler)
                .setImageLayout(layout)
                .setImageView(image_view_handle->image_view_handle.get()));

            auto dst_write = vk::WriteDescriptorSet()
            .setDstBinding(binding)
            .setDstSet(VK_NULL_HANDLE)
            .setDescriptorCount(1)
            .setDescriptorType(type)
            .setPImageInfo(&image_info);
            
            writes.push_back(dst_write);
        }
        
         void add_write_buffer(int binding, vk::DescriptorBufferInfo& descriptor, vk::DescriptorType type)
        {
            assert(descriptor.range != 0 && "Descriptor is not set");
            vk::DescriptorBufferInfo& buffer_info = buffer_infos.emplace_back(descriptor);

            auto dst_write = vk::WriteDescriptorSet()
            .setDstBinding(binding)
            .setDstSet(VK_NULL_HANDLE)
            .setDescriptorCount(1)
            .setDescriptorType(type)
            .setPBufferInfo(&buffer_info);

            writes.push_back(dst_write);
        }
        
        void clear()
        {
            image_array_infos.clear();
            image_infos.clear();
            writes.clear();
            buffer_infos.clear();
        }
        
        void update_set(vk::Device device, vk::DescriptorSet set)
        {
            for (auto& write : writes)
            {
                write.setDstSet(set);
            }
            device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
            clear();
        }
        
        std::deque<std::vector<vk::DescriptorImageInfo>> image_array_infos;
        std::deque<vk::DescriptorImageInfo> image_infos;
        std::deque<vk::DescriptorBufferInfo> buffer_infos;
        std::vector<vk::WriteDescriptorSet> writes;
    };
}

#endif //DESCRIPTORS_HPP
