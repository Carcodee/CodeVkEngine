//




// Created by carlo on 2024-09-25.
//



#ifndef SYNCRONIZATION_PATTERNS_HPP
#define SYNCRONIZATION_PATTERNS_HPP


namespace engine
{


    //maybe is worst to only use the dst instead of also getting the source
    static Image_Access_Pattern Get_Image_Dst_Pattern(Layout_Patterns pattern)
    {
        Image_Access_Pattern access_pattern;
        switch (pattern)
        {
        case Graphics_Read:
            access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader
                | vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader 
                 | vk::PipelineStageFlagBits::eGeometryShader;
            access_pattern.access_mask = vk::AccessFlagBits::eShaderRead;
            access_pattern.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            access_pattern.queue_family_type = Queue_Family_Types::Graphics;
            break;
        case Graphics_Write:
            access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader
                | vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader 
                 | vk::PipelineStageFlagBits::eGeometryShader;
            access_pattern.access_mask = vk::AccessFlagBits::eShaderWrite;
            access_pattern.layout = vk::ImageLayout::eGeneral;
            access_pattern.queue_family_type = Queue_Family_Types::Graphics;
            break;
        case Compute:
            access_pattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            access_pattern.access_mask = vk::AccessFlagBits::eShaderRead;
            access_pattern.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            access_pattern.queue_family_type = Queue_Family_Types::Compute;
            break;
        case Compute_Write:
            access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader;
            access_pattern.access_mask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            access_pattern.layout = vk::ImageLayout::eGeneral;
            access_pattern.queue_family_type = Queue_Family_Types::Compute;
            break;
        case Transfer_Src:
            access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            access_pattern.access_mask = vk::AccessFlagBits::eTransferRead;
            access_pattern.layout = vk::ImageLayout::eTransferSrcOptimal;
            access_pattern.queue_family_type = Queue_Family_Types::Transfer;
            break;
        case Transfer_Dst:
            access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            access_pattern.access_mask = vk::AccessFlagBits::eTransferWrite;
            access_pattern.layout = vk::ImageLayout::eTransferDstOptimal;
            access_pattern.queue_family_type = Queue_Family_Types::Transfer;
            break;
        case Color_Attachment:
            access_pattern.stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            access_pattern.access_mask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
            access_pattern.layout = vk::ImageLayout::eColorAttachmentOptimal;
            access_pattern.queue_family_type = Queue_Family_Types::Graphics;
            break;
        case Depth_Attachment:
            access_pattern.stage = vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eEarlyFragmentTests;
            access_pattern.access_mask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
            access_pattern.layout = vk::ImageLayout::eDepthAttachmentOptimal;
            access_pattern.queue_family_type = Queue_Family_Types::Graphics;
            break;
        case Present:
            access_pattern.stage = vk::PipelineStageFlagBits::eBottomOfPipe;
            access_pattern.access_mask = vk::AccessFlags();
            access_pattern.layout = vk::ImageLayout::ePresentSrcKHR;
            access_pattern.queue_family_type = Queue_Family_Types::Present;
            break;
        case Empty:
            access_pattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            access_pattern.access_mask = vk::AccessFlagBits::eNone;
            access_pattern.layout = vk::ImageLayout::eUndefined;
            access_pattern.queue_family_type = Queue_Family_Types::Undefined;
            break;
        default:
            assert(false && "Access pattern not known");
            break;
            
            
        }
        return access_pattern;
        
    }
    static bool Is_Image_Transition_Needed(Layout_Patterns src_pattern, Layout_Patterns dst_pattern)
    {
        if ((src_pattern == Graphics_Read) && (dst_pattern == Graphics_Read))
        {
            return false;
        }
        return true;
        
    }

    static void Transition_Image(engine::Image_Data* image_metadata, Layout_Patterns dst_layout, vk::ImageSubresourceRange range, vk::CommandBuffer& command_buffer)
    {
        Image_Access_Pattern image_pattern = Get_Image_Dst_Pattern(image_metadata->current_layout);
        Image_Access_Pattern dst_pattern = Get_Image_Dst_Pattern(dst_layout);

        assert(dst_layout != Layout_Patterns::Empty && "Right now the engine does not support transitions to empty states");
        
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
    static Buffer_Access_Pattern Get_Src_Buffer_Access_Pattern(Buffer_Usage_Types usage_type)
    {
        Buffer_Access_Pattern buffer_access_pattern;

        switch (usage_type)
        {
        case B_Vertex_Buffer:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eVertexInput;
            buffer_access_pattern.access_mask = vk::AccessFlags();
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Graphics;
            break;
        case B_Graphics_Write:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader
                | vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader 
                 | vk::PipelineStageFlagBits::eGeometryShader;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eShaderWrite;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Graphics;
            break;
        case B_Compute_Write:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eShaderWrite;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Compute;
            break;
        case B_Transfer_Src:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eTransferWrite;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Transfer;
            break;
        case B_Transfer_Dst:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            buffer_access_pattern.access_mask = vk::AccessFlags();
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Transfer;
            break;
        case B_Draw_Indirect:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eDrawIndirect;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eIndirectCommandRead;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Undefined;
            break;
        case B_Empty:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            buffer_access_pattern.access_mask = vk::AccessFlags();
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Undefined;
            break;
        default:
            assert(false && "Buffer Access pattern not known");
            break;
        }

        return buffer_access_pattern;
        
    }
        static Buffer_Access_Pattern Get_Dst_Buffer_Access_Pattern(Buffer_Usage_Types usage_type)
    {
        Buffer_Access_Pattern buffer_access_pattern;

        switch (usage_type)
        {
        case B_Vertex_Buffer:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eVertexInput;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eVertexAttributeRead;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Graphics;
            break;
        case B_Graphics_Write:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader
                | vk::PipelineStageFlagBits::eTessellationControlShader | vk::PipelineStageFlagBits::eTessellationEvaluationShader 
                 | vk::PipelineStageFlagBits::eGeometryShader;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Graphics;
            break;
        case B_Compute_Write:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Compute;
            break;
        case B_Transfer_Src:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eTransferWrite;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Transfer;
            break;
        case B_Transfer_Dst:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTransfer;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eTransferRead;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Transfer;
            break;
        case B_Draw_Indirect:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eDrawIndirect;
            buffer_access_pattern.access_mask = vk::AccessFlagBits::eIndirectCommandRead;
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Undefined;
            break;
        case B_Empty:
            buffer_access_pattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            buffer_access_pattern.access_mask = vk::AccessFlags();
            buffer_access_pattern.queue_family_type = Queue_Family_Types::Undefined;
            break;
        default:
            assert(false && "Buffer Access pattern not known");
            break;
        }

        return buffer_access_pattern;
        
    }

    static void Create_Mem_Barrier(Buffer_Access_Pattern src_barrier, Buffer_Access_Pattern dst_barrier,
                                vk::CommandBuffer& command_buffer)
    {

        auto mem_barrier = vk::MemoryBarrier()
                            .setSrcAccessMask(src_barrier.access_mask)
                            .setDstAccessMask(dst_barrier.access_mask);

        command_buffer.pipelineBarrier(src_barrier.stage, dst_barrier.stage, {}, mem_barrier,
                                      nullptr, nullptr);
    }
        
    static void Create_Buffer_Barrier(Buffer_Access_Pattern src_barrier, Buffer_Access_Pattern dst_barrier,
                                engine::Buffer* buffer, vk::CommandBuffer& command_buffer)
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
