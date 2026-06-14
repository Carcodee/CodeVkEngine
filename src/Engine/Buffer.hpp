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
	void *Map()
	{
		mappedMem = logicalDevice.mapMemory(deviceMemHandle.get(), 0, deviceSize);
		return mappedMem;
	}

	void Unmap()
	{
		logicalDevice.unmapMemory(deviceMemHandle.get());
		mappedMem = nullptr;
	}
	// whole_size for the whole buffer
	void Flush(vk::DeviceSize offset = 0)
	{
		auto mappedMemRange = vk::MappedMemoryRange()
		                          .setMemory(deviceMemHandle.get())
		                          .setOffset(offset)
		                          .setSize(deviceSize);
		logicalDevice.flushMappedMemoryRanges(mappedMemRange);
	}
	void Invlidate(vk::DeviceSize offset = 0)
	{
		auto mappedMemRange = vk::MappedMemoryRange()
		                          .setMemory(deviceMemHandle.get())
		                          .setOffset(offset)
		                          .setSize(deviceSize);
		logicalDevice.invalidateMappedMemoryRanges(mappedMemRange);
	}
	void SetupDescriptor(vk::DeviceSize offset = 0)
	{
		descriptor.buffer = bufferHandle.get();
		descriptor.range  = deviceSize;
		descriptor.offset = offset;
	}
	
	HANDLE GetBufferHandle()
	{
		assert(isExternal && "Buffer is not external so external handle was not created");
		return winMemHandle;
	}

	Buffer(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::BufferUsageFlags bufferUsageFlags,
	       vk::MemoryPropertyFlags memPropertyFlags, vk::DeviceSize deviceSize,
	        void *data = nullptr, bool isExternal = false)
	{
		this->logicalDevice    = logicalDevice;
		this->physicalDevice   = physicalDevice;
		this->deviceSize       = deviceSize;
		this->usageFlags       = bufferUsageFlags;
		this->isExternal = isExternal;
		this->memPropertyFlags = memPropertyFlags;
		auto bufferCreateInfo  = vk::BufferCreateInfo()
		                            .setSize(deviceSize)
		                            .setUsage(bufferUsageFlags)
		                            .setSharingMode(vk::SharingMode::eExclusive);

		vk::ExternalMemoryBufferCreateInfo externalMemCreateInfo{};
		externalMemCreateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
		if (this->isExternal)
		{
			bufferCreateInfo.pNext = &externalMemCreateInfo;
		}

		bufferHandle = logicalDevice.createBufferUnique(bufferCreateInfo);

		vk::MemoryRequirements bufferMemReq = logicalDevice.getBufferMemoryRequirements(bufferHandle.get());

		alignment         = bufferMemReq.alignment;
		auto memAllocInfo = vk::MemoryAllocateInfo()
		                        .setAllocationSize(bufferMemReq.size)
		                        .setMemoryTypeIndex(FindMemoryTypeIndex(physicalDevice, bufferMemReq.memoryTypeBits, memPropertyFlags));

		vk::ExportMemoryAllocateInfo exportAllocateInfo{};
		exportAllocateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
		if (this->isExternal)
		{
			memAllocInfo.pNext = &exportAllocateInfo;
		}

		deviceMemHandle = logicalDevice.allocateMemoryUnique(memAllocInfo);

		if (data != nullptr)
		{
			Map();
			memcpy(mappedMem, data, deviceSize);
			Unmap();
			if (!(memPropertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent))
			{
				Flush();
			}
		}
		if ((bufferUsageFlags & vk::BufferUsageFlagBits::eStorageBuffer) || (bufferUsageFlags &
		                                                                     vk::BufferUsageFlagBits::eUniformBuffer))
		{
			SetupDescriptor();
		}

		logicalDevice.bindBufferMemory(bufferHandle.get(), deviceMemHandle.get(), 0);
		
		if (this->isExternal)
		{
			
			PFN_vkGetMemoryWin32HandleKHR fn = reinterpret_cast<PFN_vkGetMemoryWin32HandleKHR>(
			vkGetDeviceProcAddr(logicalDevice, "vkGetMemoryWin32HandleKHR"));
			
			if (!fn)
			{
				throw std::runtime_error("Failed to load vkGetMemoryWin32HandleKHR");
			}
			
			VkMemoryGetWin32HandleInfoKHR info{};
			info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
			info.memory = static_cast<VkDeviceMemory>(deviceMemHandle.get());
			info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

			VkResult result =
				fn(
					static_cast<VkDevice>(logicalDevice),
					&info,
					&winMemHandle
				);
			
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("vkGetMemoryWin32HandleKHR failed");
			}
		}
	}

	vk::UniqueDeviceMemory   deviceMemHandle;
	vk::DeviceSize           deviceSize;
	vk::DeviceSize           alignment;
	vk::DeviceSize           offset = 0;
	vk::BufferUsageFlags     usageFlags;
	vk::MemoryPropertyFlags  memPropertyFlags;
	vk::Device               logicalDevice;
	vk::PhysicalDevice       physicalDevice;
	vk::UniqueBuffer         bufferHandle;
	vk::DescriptorBufferInfo descriptor;
	BufferUsageTypes         bufferUsage = B_EMPTY;
	void                    *mappedMem;
private:
	HANDLE winMemHandle;
	bool isExternal;
};
}        // namespace ENGINE
#endif        // BUFFER_HPP
