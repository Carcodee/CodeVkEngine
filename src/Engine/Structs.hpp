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

    struct AttachmentInfo : SYSTEMS::ISerializable<AttachmentInfo>
    {
        vk::RenderingAttachmentInfo attachmentInfo = {};
        vk::Format format = vk::Format::eUndefined;

        nlohmann::json Serialize() override
        {
	        nlohmann::json j = nlohmann::json{
		        {"format", static_cast<int>(this->format)}, // Store format as an integer
		        {
			        "attachmentInfo", {
				        // Store as uint64_t for safety
				        // Convert Vulkan enum to int
				        {"resolveMode", static_cast<int>(this->attachmentInfo.resolveMode)},
				        {"loadOp", static_cast<int>(this->attachmentInfo.loadOp)},
				        {"storeOp", static_cast<int>(this->attachmentInfo.storeOp)},
				        {
					        "clearValue", {
						        this->attachmentInfo.clearValue.color.float32[0],
						        this->attachmentInfo.clearValue.color.float32[1],
						        this->attachmentInfo.clearValue.color.float32[2],
						        this->attachmentInfo.clearValue.color.float32[3]
					        }
				        }
			        }
		        }
	        };
        	return j;
        }
        // Deserialization function for `AttachmentInfo`
        AttachmentInfo* Deserialize(std::string filename) override
        {
        	nlohmann::json json = SYSTEMS::OS::GetJsonFromFile(filename);
        	
	        this->format = static_cast<vk::Format>(json.at("format").get<int>());

	        const auto& ai = json.at("attachmentInfo");
	        this->attachmentInfo.loadOp = static_cast<vk::AttachmentLoadOp>(ai.at("loadOp").get<int>());
	        this->attachmentInfo.storeOp = static_cast<vk::AttachmentStoreOp>(ai.at("storeOp").get<int>());
	        auto clearValues = ai.at("clearValue").get<std::vector<float>>();
	        for (int i = 0; i < 4; i++)
	        {
		        this->attachmentInfo.clearValue.color.float32[i] = clearValues[i];
	        }
	        return this;
        }
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
