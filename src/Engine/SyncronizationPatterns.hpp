//




// Created by carlo on 2024-09-25.
//



#ifndef SYNCRONIZATION_PATTERNS_HPP
#define SYNCRONIZATION_PATTERNS_HPP


namespace engine
{


    //maybe is worst to only use the dst instead of also getting the source
    static image_access_pattern get_image_dst_pattern(layout_patterns pattern)
    {
        image_access_pattern access_pattern;
        switch (pattern)
        {
        case graphics_read:
            access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader
                | vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader 
                 | vk::PipelineStageFlagBits::eGeometryShader;
            access_pattern.access_mask = vk::AccessFlagBits::eShaderRead;
            access_pattern.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            access_pattern.queue_family_type = queue_family_types::graphics;
            break;
        case graphics_write:
            access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader
                | vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader 
                 | vk::PipelineStageFlagBits::eGeometryShader;
            access_pattern.access_mask = vk::AccessFlagBits::eShaderWrite;
            access_pattern.layout = vk::ImageLayout::eGeneral;
            access_pattern.queue_family_type = queue_family_types::graphics;
            break;
        case compute:
            access_pattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            access_pattern.access_mask = vk::AccessFlagBits::eShaderRead;
            access_pattern.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            access_pattern.queue_family_type = queue_family_types::compute;
            break;
        case compute_write:
            access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader;
            access_pattern.access_mask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            access_pattern.layout = vk::ImageLayout::eGeneral;
            access_pattern.queue_family_type = queue_family_types::compute;
            break;
        case transfer_src:
            access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            access_pattern.access_mask = vk::AccessFlagBits::eTransferRead;
            access_pattern.layout = vk::ImageLayout::eTransferSrcOptimal;
            access_pattern.queue_family_type = queue_family_types::transfer;
            break;
        case transfer_dst:
            access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            access_pattern.access_mask = vk::AccessFlagBits::eTransferWrite;
            access_pattern.layout = vk::ImageLayout::eTransferDstOptimal;
            access_pattern.queue_family_type = queue_family_types::transfer;
            break;
        case color_attachment:
            access_pattern.stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            access_pattern.access_mask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
            access_pattern.layout = vk::ImageLayout::eColorAttachmentOptimal;
            access_pattern.queue_family_type = queue_family_types::graphics;
            break;
        case depth_attachment:
            access_pattern.stage = vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eEarlyFragmentTests;
            access_pattern.access_mask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
            access_pattern.layout = vk::ImageLayout::eDepthAttachmentOptimal;
            access_pattern.queue_family_type = queue_family_types::graphics;
            break;
        case present:
            access_pattern.stage = vk::PipelineStageFlagBits::eBottomOfPipe;
            access_pattern.access_mask = vk::AccessFlags();
            access_pattern.layout = vk::ImageLayout::ePresentSrcKHR;
            access_pattern.queue_family_type = queue_family_types::present;
            break;
        case empty:
            access_pattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            access_pattern.access_mask = vk::AccessFlagBits::eNone;
            access_pattern.layout = vk::ImageLayout::eUndefined;
            access_pattern.queue_family_type = queue_family_types::undefined;
            break;
        default:
            assert(false && "Access pattern not known");
            break;
            
            
        }
        return access_pattern;
        
    }
    static bool is_image_transition_needed(layout_patterns src_pattern, layout_patterns dst_pattern)
    {
        if ((src_pattern == graphics_read) && (dst_pattern == graphics_read))
        {
            return false;
        }
        return true;
        
    }

    static void transition_image(engine::image_data* image_metadata, layout_patterns dst_layout, vk::ImageSubresourceRange range, vk::CommandBuffer& command_buffer)
    {
        image_access_pattern image_pattern = get_image_dst_pattern(image_metadata->current_layout);
        image_access_pattern dst_pattern = get_image_dst_pattern(dst_layout);

        assert(dst_layout != layout_patterns::empty && "Right now the engine does not support transitions to empty states");
        
        auto image_barrier = vk::ImageMemoryBarrier()
                            .setSrcAccessMask(image_pattern.access_mask)
                            .setOldLayout(image_pattern.layout)
                            .setDstAccessMask(dst_pattern.access_mask)
                            .setNewLayout(dst_pattern.layout)
                            .setSubresourceRange(range)
                            .setImage(image_metadata->image_handle);

        command_buffer.pipelineBarrier(image_pattern.stage, dst_pattern.stage, vk::DependencyFlags(), {}, nullptr, image_barrier);
        image_metadata->current_layout = dst_layout;
        image_metadata->current_pattern = dst_pattern;
    }
    static buffer_access_pattern get_src_buffer_access_pattern(buffer_usage_types usage_type)
    {
        buffer_access_pattern buffer_access_pattern;

        switch (usage_type)
        {
        case b_vertex_buffer:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eVertexInput;
            buffer_access_pattern.access_mask = vk::AccessFlags();
            buffer_access_pattern.queue_family_type = queue_family_types::graphics;
            break;
        case b_graphics_write:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader
                | vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader 
                 | vk::PipelineStageFlagBits::eGeometryShader;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eShaderWrite;
            buffer_access_pattern.queue_family_type = queue_family_types::graphics;
            break;
        case b_compute_write:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eShaderWrite;
            buffer_access_pattern.queue_family_type = queue_family_types::compute;
            break;
        case b_transfer_src:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eTransferWrite;
            buffer_access_pattern.queue_family_type = queue_family_types::transfer;
            break;
        case b_transfer_dst:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            buffer_access_pattern.access_mask = vk::AccessFlags();
            buffer_access_pattern.queue_family_type = queue_family_types::transfer;
            break;
        case b_draw_indirect:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eDrawIndirect;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eIndirectCommandRead;
            buffer_access_pattern.queue_family_type = queue_family_types::undefined;
            break;
        case b_empty:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            buffer_access_pattern.access_mask = vk::AccessFlags();
            buffer_access_pattern.queue_family_type = queue_family_types::undefined;
            break;
        default:
            assert(false && "Buffer Access pattern not known");
            break;
        }

        return buffer_access_pattern;
        
    }
        static buffer_access_pattern get_dst_buffer_access_pattern(buffer_usage_types usage_type)
    {
        buffer_access_pattern buffer_access_pattern;

        switch (usage_type)
        {
        case b_vertex_buffer:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eVertexInput;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eVertexAttributeRead;
            buffer_access_pattern.queue_family_type = queue_family_types::graphics;
            break;
        case b_graphics_write:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader
                | vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader 
                 | vk::PipelineStageFlagBits::eGeometryShader;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            buffer_access_pattern.queue_family_type = queue_family_types::graphics;
            break;
        case b_compute_write:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            buffer_access_pattern.queue_family_type = queue_family_types::compute;
            break;
        case b_transfer_src:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eTransferWrite;
            buffer_access_pattern.queue_family_type = queue_family_types::transfer;
            break;
        case b_transfer_dst:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eTransferRead;
            buffer_access_pattern.queue_family_type = queue_family_types::transfer;
            break;
        case b_draw_indirect:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eDrawIndirect;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eIndirectCommandRead;
            buffer_access_pattern.queue_family_type = queue_family_types::undefined;
            break;
        case b_empty:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            buffer_access_pattern.access_mask = vk::AccessFlags();
            buffer_access_pattern.queue_family_type = queue_family_types::undefined;
            break;
        default:
            assert(false && "Buffer Access pattern not known");
            break;
        }

        return buffer_access_pattern;
        
    }

    static void create_mem_barrier(buffer_access_pattern src_barrier, buffer_access_pattern dst_barrier,
                                vk::CommandBuffer& command_buffer)
    {

        auto mem_barrier = vk::MemoryBarrier()
                            .setSrcAccessMask(src_barrier.access_mask)
                            .setDstAccessMask(dst_barrier.access_mask);

        command_buffer.pipelineBarrier(src_barrier.stage, dst_barrier.stage, {}, mem_barrier,
                                      nullptr, nullptr);
    }
        
    static void create_buffer_barrier(buffer_access_pattern src_barrier, buffer_access_pattern dst_barrier,
                                engine::buffer* buffer, vk::CommandBuffer& command_buffer)
    {
        vk::BufferMemoryBarrier buffer_memory_barrier{};
        buffer_memory_barrier.srcAccessMask = src_barrier.access_mask;
        buffer_memory_barrier.dstAccessMask = dst_barrier.access_mask;
        buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buffer_memory_barrier.buffer = buffer->buffer_handle.get();
        buffer_memory_barrier.offset = 0;
        buffer_memory_barrier.size = VK_WHOLE_SIZE;
        command_buffer.pipelineBarrier(src_barrier.stage, dst_barrier.stage, {},{},
                                      buffer_memory_barrier, nullptr);
    }
   
   
}


#endif //SYNCRONIZATION_PATTERNS_HPP
