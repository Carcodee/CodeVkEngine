//
// Created by carlo on 2024-09-23.
//

#ifndef BUFFER_HPP
#define BUFFER_HPP

namespace engine
{
class Buffer
{
  public:
	void *Map()
	{
		mapped_mem = logical_device.mapMemory(device_mem_handle.get(), 0, device_size);
		return mapped_mem;
	}

	void Unmap()
	{
		logical_device.unmapMemory(device_mem_handle.get());
		mapped_mem = nullptr;
	}
	// whole_size for the whole buffer
	void Flush(vk::DeviceSize offset = 0)
	{
		auto mapped_mem_range = vk::MappedMemoryRange()
		                          .setMemory(device_mem_handle.get())
		                          .setOffset(offset)
		                          .setSize(device_size);
		logical_device.flushMappedMemoryRanges(mapped_mem_range);
	}
	void Invlidate(vk::DeviceSize offset = 0)
	{
		auto mapped_mem_range = vk::MappedMemoryRange()
		                          .setMemory(device_mem_handle.get())
		                          .setOffset(offset)
		                          .setSize(device_size);
		logical_device.invalidateMappedMemoryRanges(mapped_mem_range);
	}
	void Setup_Descriptor(vk::DeviceSize offset = 0)
	{
		descriptor.buffer = buffer_handle.get();
		descriptor.range  = device_size;
		descriptor.offset = offset;
	}
	
	HANDLE Get_Buffer_Handle()
	{
		assert(is_external && "Buffer is not external so external handle was not created");
		return win_mem_handle;
	}

	Buffer(vk::PhysicalDevice physical_device, vk::Device logical_device, vk::BufferUsageFlags buffer_usage_flags,
	       vk::MemoryPropertyFlags mem_property_flags, vk::DeviceSize device_size, vk::DeviceSize stride,
	        void *data = nullptr, bool is_external = false)
	{
		this->logical_device    = logical_device;
		this->physical_device   = physical_device;
		this->device_size       = device_size;
		this->usage_flags       = buffer_usage_flags;
		this->is_external = is_external;
		this->stride = stride;
		this->mem_property_flags = mem_property_flags;
		auto buffer_create_info  = vk::BufferCreateInfo()
		                            .setSize(device_size)
		                            .setUsage(buffer_usage_flags)
		                            .setSharingMode(vk::SharingMode::eExclusive);

		vk::ExternalMemoryBufferCreateInfo external_mem_create_info{};
		external_mem_create_info.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
		if (this->is_external)
		{
			buffer_create_info.pNext = &external_mem_create_info;
		}

		buffer_handle = logical_device.createBufferUnique(buffer_create_info);

		vk::MemoryRequirements buffer_mem_req = logical_device.getBufferMemoryRequirements(buffer_handle.get());

		alignment         = buffer_mem_req.alignment;
		auto mem_alloc_info = vk::MemoryAllocateInfo()
		                        .setAllocationSize(buffer_mem_req.size)
		                        .setMemoryTypeIndex(Find_Memory_Type_Index(physical_device, buffer_mem_req.memoryTypeBits, mem_property_flags));

		vk::ExportMemoryAllocateInfo export_allocate_info{};
		export_allocate_info.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
		if (this->is_external)
		{
			mem_alloc_info.pNext = &export_allocate_info;
		}

		device_mem_handle = logical_device.allocateMemoryUnique(mem_alloc_info);

		if (data != nullptr)
		{
			Map();
			memcpy(mapped_mem, data, device_size);
			Unmap();
			if (!(mem_property_flags & vk::MemoryPropertyFlagBits::eHostCoherent))
			{
				Flush();
			}
		}
		if ((buffer_usage_flags & vk::BufferUsageFlagBits::eStorageBuffer) || (buffer_usage_flags &
		                                                                     vk::BufferUsageFlagBits::eUniformBuffer))
		{
			Setup_Descriptor();
		}

		logical_device.bindBufferMemory(buffer_handle.get(), device_mem_handle.get(), 0);
		
		if (this->is_external)
		{
			
			PFN_vkGetMemoryWin32HandleKHR fn = reinterpret_cast<PFN_vkGetMemoryWin32HandleKHR>(
			vkGetDeviceProcAddr(logical_device, "vkGetMemoryWin32HandleKHR"));
			
			if (!fn)
			{
				throw std::runtime_error("Failed to load vkGetMemoryWin32HandleKHR");
			}
			
			VkMemoryGetWin32HandleInfoKHR info{};
			info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
			info.memory = static_cast<VkDeviceMemory>(device_mem_handle.get());
			info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

			VkResult result =
				fn(
					static_cast<VkDevice>(logical_device),
					&info,
					&win_mem_handle
				);
			
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("vkGetMemoryWin32HandleKHR failed");
			}
		}
	}

	vk::UniqueDeviceMemory   device_mem_handle;
	vk::DeviceSize           device_size;
	vk::DeviceSize           alignment;
	vk::DeviceSize           stride;
	vk::DeviceSize           offset = 0;
	vk::BufferUsageFlags     usage_flags;
	vk::MemoryPropertyFlags  mem_property_flags;
	vk::Device               logical_device;
	vk::PhysicalDevice       physical_device;
	vk::UniqueBuffer         buffer_handle;
	vk::DescriptorBufferInfo descriptor;
	Buffer_Usage_Types         buffer_usage = B_Empty;
	void                    *mapped_mem = nullptr;
private:
	HANDLE win_mem_handle;
	bool is_external;
};
}        // namespace engine
#endif        // BUFFER_HPP
