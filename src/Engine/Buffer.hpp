//
// Created by carlo on 2024-09-23.
//

#ifndef BUFFER_HPP
#define BUFFER_HPP

namespace ENGINE
{
    class Buffer
    {
    public:

        void* Map()
        {
            mappedMem = logicalDevice.mapMemory(deviceMemHandle.get(), 0, deviceSize);
            return mappedMem;
        }

        void Unmap()
        {
            logicalDevice.unmapMemory(deviceMemHandle.get());
            mappedMem = nullptr;
        }
        //whole_size for the whole buffer
        void Flush(vk::DeviceSize offset = 0)
        {
            auto mappedMemRange= vk::MappedMemoryRange()
            .setMemory(deviceMemHandle.get())
            .setOffset(offset)
            .setSize(deviceSize);
            logicalDevice.flushMappedMemoryRanges(mappedMemRange);
        }
        void Invlidate(vk::DeviceSize offset = 0)
        {
             auto mappedMemRange= vk::MappedMemoryRange()
            .setMemory(deviceMemHandle.get())
            .setOffset(offset)
            .setSize(deviceSize);
            logicalDevice.invalidateMappedMemoryRanges(mappedMemRange);           
        }
        void SetupDescriptor(vk::DeviceSize offset = 0)
        {
            descriptor.buffer = bufferHandle.get();
            descriptor.range = deviceSize;
            descriptor.offset = offset;
        }

        Buffer(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::BufferUsageFlags bufferUsageFlags,
               vk::MemoryPropertyFlags memPropertyFlags, vk::DeviceSize deviceSize
               , void* data = nullptr)
        {
            this->logicalDevice = logicalDevice;
            this->physicalDevice = physicalDevice;
            this->deviceSize = deviceSize;
            this->usageFlags = bufferUsageFlags;
            this->memPropertyFlags = memPropertyFlags;
            auto bufferCreateInfo = vk::BufferCreateInfo()
            .setSize(deviceSize)
            .setUsage(bufferUsageFlags)
            .setSharingMode(vk::SharingMode::eExclusive);

            bufferHandle = logicalDevice.createBufferUnique(bufferCreateInfo);

            vk::MemoryRequirements bufferMemReq= logicalDevice.getBufferMemoryRequirements(bufferHandle.get());

            alignment = bufferMemReq.alignment; 
            auto memAllocInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(bufferMemReq.size)
            .setMemoryTypeIndex(FindMemoryTypeIndex(physicalDevice, bufferMemReq.memoryTypeBits, memPropertyFlags));

            deviceMemHandle = logicalDevice.allocateMemoryUnique(memAllocInfo);

            if (data!= nullptr)
            {
                Map();
                memcpy(mappedMem, data, deviceSize);
                Unmap();
                if (!(memPropertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent)){
                    Flush();
                }
            }
            if ((bufferUsageFlags & vk::BufferUsageFlagBits::eStorageBuffer) || (bufferUsageFlags &
                vk::BufferUsageFlagBits::eUniformBuffer))
            {
                SetupDescriptor();
            }
            
            logicalDevice.bindBufferMemory(bufferHandle.get() ,deviceMemHandle.get(), 0);
        }
        


        vk::UniqueDeviceMemory deviceMemHandle;
        vk::DeviceSize deviceSize;
        vk::DeviceSize alignment;
        vk::DeviceSize offset = 0;
        vk::BufferUsageFlags usageFlags;
        vk::MemoryPropertyFlags memPropertyFlags;
        vk::Device logicalDevice;
        vk::PhysicalDevice physicalDevice;
        vk::UniqueBuffer bufferHandle;
        vk::DescriptorBufferInfo descriptor;
        BufferUsageTypes bufferUsage = B_EMPTY;
        void* mappedMem;
    };
}
#endif //BUFFER_HPP
