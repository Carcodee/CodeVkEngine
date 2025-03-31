//




// Created by carlo on 2024-09-25.
//



#ifndef SYNCRONIZATIONPATTERNS_HPP
#define SYNCRONIZATIONPATTERNS_HPP


namespace ENGINE
{


    //maybe is worst to only use the dst instead of also getting the source
    static ImageAccessPattern GetImageDstPattern(LayoutPatterns pattern)
    {
        ImageAccessPattern accessPattern;
        switch (pattern)
        {
        case GRAPHICS_READ:
            accessPattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader ;
            accessPattern.accessMask = vk::AccessFlagBits::eShaderRead;
            accessPattern.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            accessPattern.queueFamilyType = QueueFamilyTypes::GRAPHICS;
            break;
        case GRAPHICS_WRITE:
            accessPattern.stage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eVertexShader ;
            accessPattern.accessMask = vk::AccessFlagBits::eShaderWrite;
            accessPattern.layout = vk::ImageLayout::eGeneral;
            accessPattern.queueFamilyType = QueueFamilyTypes::GRAPHICS;
            break;
        case COMPUTE:
            accessPattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            accessPattern.accessMask = vk::AccessFlagBits::eShaderRead;
            accessPattern.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            accessPattern.queueFamilyType = QueueFamilyTypes::COMPUTE;
            break;
        case COMPUTE_WRITE:
            accessPattern.stage = vk::PipelineStageFlagBits::eFragmentShader;
            accessPattern.accessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            accessPattern.layout = vk::ImageLayout::eGeneral;
            accessPattern.queueFamilyType = QueueFamilyTypes::COMPUTE;
            break;
        case TRANSFER_SRC:
            accessPattern.stage = vk::PipelineStageFlagBits::eTransfer;
            accessPattern.accessMask = vk::AccessFlagBits::eTransferRead;
            accessPattern.layout = vk::ImageLayout::eTransferSrcOptimal;
            accessPattern.queueFamilyType = QueueFamilyTypes::TRANSFER;
            break;
        case TRANSFER_DST:
            accessPattern.stage = vk::PipelineStageFlagBits::eTransfer;
            accessPattern.accessMask = vk::AccessFlagBits::eTransferWrite;
            accessPattern.layout = vk::ImageLayout::eTransferDstOptimal;
            accessPattern.queueFamilyType = QueueFamilyTypes::TRANSFER;
            break;
        case COLOR_ATTACHMENT:
            accessPattern.stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            accessPattern.accessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
            accessPattern.layout = vk::ImageLayout::eColorAttachmentOptimal;
            accessPattern.queueFamilyType = QueueFamilyTypes::GRAPHICS;
            break;
        case DEPTH_ATTACHMENT:
            accessPattern.stage = vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eEarlyFragmentTests;
            accessPattern.accessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
            accessPattern.layout = vk::ImageLayout::eDepthAttachmentOptimal;
            accessPattern.queueFamilyType = QueueFamilyTypes::GRAPHICS;
            break;
        case PRESENT:
            accessPattern.stage = vk::PipelineStageFlagBits::eBottomOfPipe;
            accessPattern.accessMask = vk::AccessFlags();
            accessPattern.layout = vk::ImageLayout::ePresentSrcKHR;
            accessPattern.queueFamilyType = QueueFamilyTypes::PRESENT;
            break;
        case EMPTY:
            accessPattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            accessPattern.accessMask = vk::AccessFlagBits::eNone;
            accessPattern.layout = vk::ImageLayout::eUndefined;
            accessPattern.queueFamilyType = QueueFamilyTypes::UNDEFINED;
            break;
        default:
            assert(false && "Access pattern not known");
            break;
            
            
        }
        return accessPattern;
        
    }
    static bool IsImageTransitionNeeded(LayoutPatterns srcPattern, LayoutPatterns dstPattern)
    {
        if ((srcPattern == GRAPHICS_READ) && (dstPattern == GRAPHICS_READ))
        {
            return false;
        }
        return true;
        
    }

    static void TransitionImage(ImageData* imageData, LayoutPatterns dstLayout, vk::ImageSubresourceRange range, vk::CommandBuffer& commandBuffer)
    {
        ImageAccessPattern imagePattern = GetImageDstPattern(imageData->currentLayout);
        ImageAccessPattern dstPattern = GetImageDstPattern(dstLayout);

        assert(dstLayout != LayoutPatterns::EMPTY && "Right now the engine does not support transitions to empty states");
        
        auto imageBarrier = vk::ImageMemoryBarrier()
                            .setSrcAccessMask(imagePattern.accessMask)
                            .setOldLayout(imagePattern.layout)
                            .setDstAccessMask(dstPattern.accessMask)
                            .setNewLayout(dstPattern.layout)
                            .setSubresourceRange(range)
                            .setImage(imageData->imageHandle);

        commandBuffer.pipelineBarrier(imagePattern.stage, dstPattern.stage, vk::DependencyFlags(), {}, nullptr, imageBarrier);
        imageData->currentLayout = dstLayout;
        imageData->currentPattern = dstPattern;
    }
    static BufferAccessPattern GetSrcBufferAccessPattern(BufferUsageTypes usageType)
    {
        BufferAccessPattern bufferAccessPattern;

        switch (usageType)
        {
        case B_VERTEX_BUFFER:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eVertexInput;
            bufferAccessPattern.accessMask = vk::AccessFlags();
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::GRAPHICS;
            break;
        case B_GRAPHICS_WRITE:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eShaderWrite;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::GRAPHICS;
            break;
        case B_COMPUTE_WRITE:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eShaderWrite;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::COMPUTE;
            break;
        case B_TRANSFER_SRC:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eTransfer;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eTransferWrite;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::TRANSFER;
            break;
        case B_TRANSFER_DST:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eTransfer;
            bufferAccessPattern.accessMask = vk::AccessFlags();
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::TRANSFER;
            break;
        case B_DRAW_INDIRECT:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eDrawIndirect;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eIndirectCommandRead;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::UNDEFINED;
            break;
        case B_EMPTY:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            bufferAccessPattern.accessMask = vk::AccessFlags();
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::UNDEFINED;
            break;
        default:
            assert(false && "Buffer Access pattern not known");
            break;
        }

        return bufferAccessPattern;
        
    }
        static BufferAccessPattern GetDstBufferAccessPattern(BufferUsageTypes usageType)
    {
        BufferAccessPattern bufferAccessPattern;

        switch (usageType)
        {
        case B_VERTEX_BUFFER:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eVertexInput;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eVertexAttributeRead;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::GRAPHICS;
            break;
        case B_GRAPHICS_WRITE:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::GRAPHICS;
            break;
        case B_COMPUTE_WRITE:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eComputeShader;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::COMPUTE;
            break;
        case B_TRANSFER_SRC:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eTransfer;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eTransferWrite;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::TRANSFER;
            break;
        case B_TRANSFER_DST:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eTransfer;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eTransferRead;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::TRANSFER;
            break;
        case B_DRAW_INDIRECT:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eDrawIndirect;
            bufferAccessPattern.accessMask = vk::AccessFlagBits::eIndirectCommandRead;
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::UNDEFINED;
            break;
        case B_EMPTY:
            bufferAccessPattern.stage = vk::PipelineStageFlagBits::eTopOfPipe;
            bufferAccessPattern.accessMask = vk::AccessFlags();
            bufferAccessPattern.queueFamilyType = QueueFamilyTypes::UNDEFINED;
            break;
        default:
            assert(false && "Buffer Access pattern not known");
            break;
        }

        return bufferAccessPattern;
        
    }

    static void CreateMemBarrier(BufferAccessPattern srcBarrier, BufferAccessPattern dstBarrier,
                                vk::CommandBuffer& commandBuffer)
    {

        auto memBarrier = vk::MemoryBarrier()
                            .setSrcAccessMask(srcBarrier.accessMask)
                            .setDstAccessMask(dstBarrier.accessMask);

        commandBuffer.pipelineBarrier(srcBarrier.stage, dstBarrier.stage, {}, memBarrier,
                                      nullptr, nullptr);
    }
        
    static void CreateBufferBarrier(BufferAccessPattern srcBarrier, BufferAccessPattern dstBarrier,
                                Buffer* buffer, vk::CommandBuffer& commandBuffer)
    {
        vk::BufferMemoryBarrier bufferMemoryBarrier{};
        bufferMemoryBarrier.srcAccessMask = srcBarrier.accessMask;
        bufferMemoryBarrier.dstAccessMask = dstBarrier.accessMask; 
        bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufferMemoryBarrier.buffer = buffer->bufferHandle.get();
        bufferMemoryBarrier.offset = 0; 
        bufferMemoryBarrier.size = VK_WHOLE_SIZE;
        commandBuffer.pipelineBarrier(srcBarrier.stage, dstBarrier.stage, {},{},
                                      bufferMemoryBarrier, nullptr);
    }
   
   
}


#endif //SYNCRONIZATIONPATTERNS_HPP
