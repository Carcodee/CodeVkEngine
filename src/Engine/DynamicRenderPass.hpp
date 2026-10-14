//
// Created by carlo on 2024-09-30.
//

#ifndef DYNAMIC_RENDER_PASS_HPP
#define DYNAMIC_RENDER_PASS_HPP

namespace engine
{

// TODO: set base configs
static engine::Attachment_Info Get_Color_Attachment_Info(Blend_Configs blend_configs, glm::vec4 clear_col = glm::vec4(0.0f, 0.1f, 0.1f, 1.0f),
                                             vk::Format            format  = g_32b_format,
                                             vk::AttachmentLoadOp  load_op  = vk::AttachmentLoadOp::eClear,
                                             vk::AttachmentStoreOp store_op = vk::AttachmentStoreOp::eStore)
{
	engine::Attachment_Info vk_attachment_info;
	vk_attachment_info.blend_configs = blend_configs;
	vk_attachment_info.vk_attachment_info = vk::RenderingAttachmentInfo()
	                                    .setImageView(nullptr)
	                                    .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
	                                    .setLoadOp(load_op)
	                                    .setStoreOp(store_op)
	                                    .setClearValue(
	                                        vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{
	                                            clear_col.x, clear_col.y, clear_col.z, clear_col.w})));

	vk_attachment_info.format = format;

	return vk_attachment_info;
}

static engine::Attachment_Info Get_Depth_Attachment_Info(vk::Format format = vk::Format::eD32Sfloat)
{
	engine::Attachment_Info vk_attachment_info;
	vk_attachment_info.vk_attachment_info = vk::RenderingAttachmentInfo()
	                                    .setImageView(nullptr)
	                                    .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
	                                    .setLoadOp(vk::AttachmentLoadOp::eClear)
	                                    .setStoreOp(vk::AttachmentStoreOp::eStore)
	                                    .setClearValue(vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)));
	vk_attachment_info.format = format;

	return vk_attachment_info;
}

struct Dynamic_Render_Pass
{
  public:
	void Set_Pipeline_Rendering_Info(uint32_t                color_attachment_count,
	                              std::vector<vk::Format> color_formats,
	                              vk::Format              depth_format = vk::Format::eUndefined)
	{
		if (color_formats.empty())
		{
			std::cout << "Warning (Dynamic Render Pass): no color formats were set\n";
			color_formats.push_back(vk::Format::eUndefined);
		}
		this->color_formats                = color_formats;
		this->depth_format                 = depth_format;
		this->expected_color_attachment_size = color_attachment_count;
		pipeline_rendering_create_info       = vk::PipelineRenderingCreateInfo()
		                                  .setColorAttachmentCount(color_attachment_count)
		                                  .setPColorAttachmentFormats(this->color_formats.data())
		                                  .setDepthAttachmentFormat(this->depth_format)
		                                  .setStencilAttachmentFormat(vk::Format::eUndefined);
	}
	void Set_Render_Info(std::vector<vk::RenderingAttachmentInfo> &color_attachments,
	                   glm::uvec2 framebuffer_size, vk::RenderingAttachmentInfo *depth_attachment)
	{
		for (auto &element : color_attachments)
		{
			assert(element.imageView != nullptr && "Image view was not set before setting the render info");
		}
		assert(
		    color_attachments.size() == expected_color_attachment_size &&
		    "Color attachment must be the same as the one indicated in the pipeline creation");
		render_info = vk::RenderingInfo()
		                 .setRenderArea({{0, 0}, {framebuffer_size.x, framebuffer_size.y}})
		                 .setLayerCount(1)
		                 .setColorAttachmentCount(color_attachments.size())
		                 .setPColorAttachments(color_attachments.data());
		if (depth_attachment->imageView != nullptr)
		{
			render_info.setPDepthAttachment(depth_attachment);
		}
	}

	void Set_Render_Info_Unsafe(std::vector<vk::RenderingAttachmentInfo> &color_attachments,
	                         glm::uvec2 framebuffer_size, vk::RenderingAttachmentInfo *depth_attachment)
	{
		for (auto &element : color_attachments)
		{
			assert(element.imageView != nullptr && "Image view was not set before setting the render info");
		}
		render_info = vk::RenderingInfo()
		                 .setRenderArea({{0, 0}, {framebuffer_size.x, framebuffer_size.y}})
		                 .setLayerCount(1)
		                 .setColorAttachmentCount(color_attachments.size())
		                 .setPColorAttachments(color_attachments.data());
		if (depth_attachment->imageView != nullptr)
		{
			render_info.setPDepthAttachment(depth_attachment);
		}
	}
	void Set_Viewport(glm::uvec2 viewport_size, glm::uvec2 scissor_size)
	{
		viewport
		    .setX(0.0f)
		    .setY(0.0f)
		    .setWidth(static_cast<float>(viewport_size.x))
		    .setHeight(static_cast<float>(viewport_size.y))
		    .setMinDepth(0.0f)
		    .setMaxDepth(1.0f);
		scissor
		    .setOffset({0, 0})
		    .setExtent({scissor_size.x, scissor_size.y});
	}
	void Reset()
	{
		color_formats.clear();
		depth_format                 = vk::Format::eUndefined;
		expected_color_attachment_size = 0;
		render_info                  = vk::RenderingInfo();
		pipeline_rendering_create_info = vk::PipelineRenderingCreateInfo();
		viewport                    = vk::Viewport();
		scissor                     = vk::Rect2D();
	}
	std::vector<vk::Format>         color_formats;
	vk::Format                      depth_format;
	uint32_t                        expected_color_attachment_size = 0;
	vk::RenderingInfo               render_info;
	vk::PipelineRenderingCreateInfo pipeline_rendering_create_info;
	vk::Viewport                    viewport;
	vk::Rect2D                      scissor;
};
}        // namespace engine
#endif        // DYNAMIC_RENDER_PASS_HPP
