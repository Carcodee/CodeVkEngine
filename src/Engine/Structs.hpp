//

// Created by carlo on 2024-09-22.
//


#ifndef STRUCTS_HPP
namespace ENGINE
{
    //core
    struct QueueFamilyIndices
    {
        uint32_t graphicsFamilyIndex;
        uint32_t presentFamilyIndex;
    };
    struct WindowDesc
    {
        HINSTANCE hInstance;
        HWND hWnd;
    };

    //image

    struct SubImageInfo
    {
      vk::ImageLayout currLayout;
    };
    struct MipInfo
    {
      std::vector<SubImageInfo> layerInfos;
      glm::uvec3 size;
    };

    //swapChain
    struct SurfaceDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    //presentQueue
    struct FrameResources
    {
        vk::UniqueSemaphore imageAcquiredSemaphore;
        vk::UniqueSemaphore renderingFinishedSemaphore;
        vk::UniqueFence inflightFence;

        vk::UniqueCommandBuffer commandBuffer;
    };

    struct AttachmentInfo
    {
        vk::RenderingAttachmentInfo attachmentInfo;
        vk::Format format = vk::Format::eUndefined;
    };

    struct ImageAccessPattern
    {
        vk::PipelineStageFlags stage;
        vk::AccessFlags accessMask;
        vk::ImageLayout layout;
        QueueFamilyTypes queueFamilyType;

        ImageAccessPattern& operator =(const ImageAccessPattern& other)
        {
            layout = other.layout;
            stage = other.stage;
            accessMask = other.accessMask;
            queueFamilyType = other.queueFamilyType;
            return *this;
            
        }
    };
    struct BufferAccessPattern
    {
        vk::PipelineStageFlags stage;
        vk::AccessFlags accessMask;
        QueueFamilyTypes queueFamilyType;
    };

    struct ShaderResource
    {
        std::string name;
        uint32_t binding;
        uint32_t set;
        bool array = false;
        vk::DescriptorType type;
    };

    struct DrawIndirectIndexedCmd
    {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
        uint32_t firstInstance;
    };

    struct DrawIndirectCmd
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t firstInstance;
    };

}
#define STRUCTS_HPP

#endif //STRUCTS_HPP
