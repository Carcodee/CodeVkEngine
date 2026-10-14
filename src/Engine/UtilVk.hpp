//
// Created by carlo on 2024-09-24.
//

#ifndef UTIL_VK_HPP
#define UTIL_VK_HPP

namespace engine
{
     static Surface_Details Get_Surface_Details(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface_khr)
     {
         Surface_Details surface_details;
         surface_details.capabilities = physical_device.getSurfaceCapabilitiesKHR(surface_khr);
         surface_details.formats = physical_device.getSurfaceFormatsKHR(surface_khr);
         surface_details.present_modes = physical_device.getSurfacePresentModesKHR(surface_khr);

         return surface_details;
     }   
	static uint32_t Find_Memory_Type_Index(vk::PhysicalDevice physical_device, uint32_t suitable_indices, vk::MemoryPropertyFlags memory_visibility)
	{
		vk::PhysicalDeviceMemoryProperties available_mem_properties = physical_device.getMemoryProperties();

		for (uint32_t i = 0; i < available_mem_properties.memoryTypeCount; i++)
		{
			if ((suitable_indices & (1 << i)) && (available_mem_properties.memoryTypes[i].propertyFlags & memory_visibility)
				== memory_visibility)
			{
				return i;
			}
		}
		return uint32_t(-1);
	}

    static std::vector<uint32_t> Get_Byte_Code(const std::string filepath)
    {
     	std::ifstream file(filepath, std::ios::ate | std::ios::binary);

     	if (!file.is_open())
     		throw std::runtime_error("failed to open file!");

        size_t file_size = (size_t)file.tellg();
        std::vector<uint32_t> bytecode(file_size / sizeof(uint32_t));

     	file.seekg(0);
     	file.read((char*)bytecode.data(), bytecode.size() * sizeof(uint32_t));
     	file.close();
     	
		std::cout << "Read SPIR-V file of size: " << file_size << " bytes." << std::endl;
     	
     	return bytecode;
    }
	static void Copy_Buffer_To_Image(vk::CommandBuffer command_buffer, vk::Buffer src_buffer,
                                  vk::Image* image, glm::uvec2 size)
    {
        auto image_subresource = vk::ImageSubresourceLayers()
                                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                .setMipLevel(0)
                                .setBaseArrayLayer(0)
                                .setLayerCount(1);
        auto image_copy_region = vk::BufferImageCopy()
        .setBufferOffset(0)
        .setImageExtent(0)
        .setBufferImageHeight(0)
        .setBufferRowLength(0)
        .setImageSubresource(image_subresource)
        .setImageOffset(vk::Offset3D(0))
        .setImageExtent(vk::Extent3D(size.x, size.y, 1));
        command_buffer.copyBufferToImage(src_buffer, *image, vk::ImageLayout::eTransferDstOptimal, 1, &image_copy_region);
    }
    

    static void Print_Invalid_Resource(std::string type, std::string resource_name)
    {
	    std::string message = type + " with name \"" + resource_name + "\" does not exist\n";
     	std::cout <<message;
    }
}


#endif //UTIL_VK_HPP
