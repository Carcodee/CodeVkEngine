//

// Created by carlo on 2024-09-22.
//

#ifndef STRUCTS_HPP
namespace engine
{
// core
struct Queue_Family_Indices
{
	uint32_t transfer_family_index;
	uint32_t compute_family_index;
	uint32_t graphics_family_index;
	uint32_t present_family_index;
};
struct Window_Desc
{
	HINSTANCE h_instance;
	HWND      h_wnd;
};

// image

struct Sub_Image_Info
{
	vk::ImageLayout curr_layout;
};
struct Mip_Info
{
	std::vector<Sub_Image_Info> layer_infos;
	glm::uvec3                size;
};

// swapChain
struct Surface_Details
{
	vk::SurfaceCapabilitiesKHR        capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR>   present_modes;
};

// presentQueue
struct Frame_Resources
{
	vk::UniqueSemaphore image_acquired_semaphore;
	vk::UniqueSemaphore rendering_finished_semaphore;
	vk::UniqueFence     inflight_fence;
};

struct Attachment_Info : systems::I_Serializable<engine::Attachment_Info>
{
	
	Blend_Configs blend_configs = {};
	vk::RenderingAttachmentInfo vk_attachment_info = {};
	vk::Format                  format         = vk::Format::eUndefined;

	nlohmann::json Serialize() override
	{
		nlohmann::json j = nlohmann::json{
		    {"format", static_cast<int>(this->format)},        // Store format as an integer
		    {
		        "attachmentInfo", {// Store as uint64_t for safety
		                           // Convert Vulkan enum to int
		                           {"resolveMode", static_cast<int>(this->vk_attachment_info.resolveMode)},
		                           {"loadOp", static_cast<int>(this->vk_attachment_info.loadOp)},
		                           {"storeOp", static_cast<int>(this->vk_attachment_info.storeOp)},
		                           {"clearValue", {this->vk_attachment_info.clearValue.color.float32[0], this->vk_attachment_info.clearValue.color.float32[1], this->vk_attachment_info.clearValue.color.float32[2], this->vk_attachment_info.clearValue.color.float32[3]}}}}};
		return j;
	}
	// Deserialization function for `AttachmentInfo`
	engine::Attachment_Info *Deserialize(std::string filename) override
	{
		nlohmann::json json = systems::Os::Get_Json_From_File(filename);

		this->format = static_cast<vk::Format>(json.at("format").get<int>());

		const auto &ai               = json.at("attachmentInfo");
		this->vk_attachment_info.loadOp  = static_cast<vk::AttachmentLoadOp>(ai.at("loadOp").get<int>());
		this->vk_attachment_info.storeOp = static_cast<vk::AttachmentStoreOp>(ai.at("storeOp").get<int>());
		auto clear_values             = ai.at("clearValue").get<std::vector<float>>();
		for (int i = 0; i < 4; i++)
		{
			this->vk_attachment_info.clearValue.color.float32[i] = clear_values[i];
		}
		return this;
	}
	engine::Attachment_Info *Get_From_Json(nlohmann::json json)
	{
		this->format = static_cast<vk::Format>(json.at("format").get<int>());

		const auto &ai               = json.at("attachmentInfo");
		this->vk_attachment_info.loadOp  = static_cast<vk::AttachmentLoadOp>(ai.at("loadOp").get<int>());
		this->vk_attachment_info.storeOp = static_cast<vk::AttachmentStoreOp>(ai.at("storeOp").get<int>());
		auto clear_values             = ai.at("clearValue").get<std::vector<float>>();
		for (int i = 0; i < 4; i++)
		{
			this->vk_attachment_info.clearValue.color.float32[i] = clear_values[i];
		}
		return this;
	}
};

struct Image_Access_Pattern
{
	vk::PipelineStageFlags stage;
	vk::AccessFlags        access_mask;
	vk::ImageLayout        layout;
	Queue_Family_Types       queue_family_type;

	Image_Access_Pattern &operator=(const Image_Access_Pattern &other)
	{
		layout          = other.layout;
		stage           = other.stage;
		access_mask      = other.access_mask;
		queue_family_type = other.queue_family_type;
		return *this;
	}
};
struct Buffer_Access_Pattern
{
	vk::PipelineStageFlags stage;
	vk::AccessFlags        access_mask;
	Queue_Family_Types       queue_family_type;
};

struct Shader_Resource
{
	std::string        name;
	uint32_t           binding;
	uint32_t           set;
	bool               array = false;
	vk::DescriptorType type;
};

struct Draw_Indirect_Indexed_Cmd
{
	uint32_t index_count;
	uint32_t instance_count;
	uint32_t first_index;
	uint32_t vertex_offset;
	uint32_t first_instance;
};

struct Draw_Indirect_Cmd
{
	uint32_t vertex_count;
	uint32_t instance_count;
	uint32_t first_index;
	uint32_t first_instance;
};
struct Graphics_Pipeline_Configs
{
	Rasterization_Configs rasterization_configs = Rasterization_Configs::R_Fill;
	Topology_Configs      topology_configs      = Topology_Configs::T_Triangle;
};


}        // namespace engine
#	define STRUCTS_HPP

#endif        // STRUCTS_HPP
