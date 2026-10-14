//

// Created by carlo on 2024-10-02.
//

#ifndef STAGED_BUFFER_HPP
#define STAGED_BUFFER_HPP

namespace engine
{
    class Staged_Buffer
    {
    public:
        Staged_Buffer(vk::PhysicalDevice physical_device, vk::Device logical_device, vk::BufferUsageFlags usage,
                     vk::DeviceSize size, vk::DeviceSize stride)
        {
            staging_buffer = std::make_unique<engine::Buffer>(physical_device, logical_device,
                                                             vk::BufferUsageFlagBits::eTransferSrc,
                                                             vk::MemoryPropertyFlagBits::eHostVisible |
                                                             vk::MemoryPropertyFlagBits::eHostCoherent,
                                                             size, stride);
            device_buffer = std::make_unique<engine::Buffer>(physical_device, logical_device,
                                                            usage | vk::BufferUsageFlagBits::eTransferDst,
                                                            vk::MemoryPropertyFlagBits::eDeviceLocal, size, stride);
            this->size = size;
        }

        void* Map()
        {
            return staging_buffer->Map();
        }

        void Unmap(vk::CommandBuffer command_buffer)
        {
            staging_buffer->Unmap();

            auto copy_region = vk::BufferCopy()
                              .setSrcOffset(0)
                              .setDstOffset(0)
                              .setSize(size);

            command_buffer.copyBuffer(staging_buffer->buffer_handle.get(), device_buffer->buffer_handle.get(), 1 ,&copy_region);
        }
        

        vk::Buffer Get_Buffer()
        {
            return device_buffer->buffer_handle.get();
        }

        std::unique_ptr<engine::Buffer> staging_buffer;
        std::unique_ptr<engine::Buffer> device_buffer;
        vk::DeviceSize size = 0;
    };
    
    
}
#endif //STAGED_BUFFER_HPP
