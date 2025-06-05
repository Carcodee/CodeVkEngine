//


// Created by carlo on 2024-09-26.
//


#ifndef PIPELINE_HPP
#define PIPELINE_HPP

namespace ENGINE
{

    static vk::PrimitiveTopology GetTopology(TopologyConfigs topologyConfigs)
    {
        switch (topologyConfigs)
        {
        case T_TRIANGLE:
            return vk::PrimitiveTopology::eTriangleList;
            break;
        case T_POINT_LIST:
            return vk::PrimitiveTopology::ePointList;
            break;
        default:
            assert(false);
            break;
        }
        
    }
    static vk::PipelineRasterizationStateCreateInfo GetRasterizationInfo(RasterizationConfigs configs)
    {
        auto rasterizationInfo = vk::PipelineRasterizationStateCreateInfo();

        switch (configs) {
        case R_FILL:
            rasterizationInfo.setDepthClampEnable(VK_FALSE)
                             .setRasterizerDiscardEnable(VK_FALSE)
                             .setPolygonMode(vk::PolygonMode::eFill)
                             .setCullMode(vk::CullModeFlagBits::eNone)
                             .setFrontFace(vk::FrontFace::eClockwise)
                             .setLineWidth(1.0f);
            break;
        case R_LINE:
            rasterizationInfo.setDepthClampEnable(VK_FALSE)
                             .setRasterizerDiscardEnable(VK_FALSE)
                             .setPolygonMode(vk::PolygonMode::eLine)
                             .setCullMode(vk::CullModeFlagBits::eNone)
                             .setFrontFace(vk::FrontFace::eClockwise)
                             .setLineWidth(1.0f);
            break;
        case R_POINT:
            rasterizationInfo.setDepthClampEnable(VK_FALSE)
                             .setRasterizerDiscardEnable(VK_FALSE)
                             .setPolygonMode(vk::PolygonMode::ePoint)
                             .setCullMode(vk::CullModeFlagBits::eNone)
                             .setFrontFace(vk::FrontFace::eClockwise)
                             .setLineWidth(1.0f);
            break;
        default:
            assert(false && "Invalid rasterization config");
            break;
        }
        return rasterizationInfo;
    }


    static vk::PipelineColorBlendAttachmentState GetBlendAttachmentState(BlendConfigs configs)
    {
        auto blendAttachmentState = vk::PipelineColorBlendAttachmentState();
        switch (configs)
        {
        case B_OPAQUE:
            blendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                    vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                                .setBlendEnable(VK_FALSE);
            break;
        case B_ADD:
            blendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR |
                                    vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
                                .setBlendEnable(VK_TRUE)
                                .setAlphaBlendOp(vk::BlendOp::eAdd)
                                .setColorBlendOp(vk::BlendOp::eAdd)
                                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                                .setDstColorBlendFactor(vk::BlendFactor::eOne);
            break;
        case B_MIX:
            blendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR |
                                    vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
                                .setBlendEnable(VK_TRUE)
                                .setAlphaBlendOp(vk::BlendOp::eAdd)
                                .setColorBlendOp(vk::BlendOp::eAdd)
                                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
            break;
        case B_ALPHA_BLEND:
            blendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR |
                                    vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
                                .setBlendEnable(VK_TRUE)
                                .setAlphaBlendOp(vk::BlendOp::eAdd)
                                .setColorBlendOp(vk::BlendOp::eAdd)
                                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
            break;
        default:
            assert(false&&"Invalid blend config");
            break;
        }


        return blendAttachmentState;
    }

    static vk::PipelineDepthStencilStateCreateInfo GetDepthStencil(DepthConfigs configs)
    {
        auto depthStencilCreateInfo = vk::PipelineDepthStencilStateCreateInfo()
                                      .setDepthTestEnable(VK_TRUE)
                                      .setDepthWriteEnable(VK_TRUE)
                                      .setDepthCompareOp(vk::CompareOp::eLess)
                                      .setDepthBoundsTestEnable(VK_FALSE)
                                      .setStencilTestEnable(VK_FALSE);
        switch (configs)
        {
        case D_ENABLE:
            depthStencilCreateInfo
                .setDepthTestEnable(VK_TRUE)
                .setDepthWriteEnable(VK_TRUE)
                .setDepthCompareOp(vk::CompareOp::eLess)
                .setDepthBoundsTestEnable(VK_FALSE);
            break;
        case D_DISABLE:
            depthStencilCreateInfo
                .setDepthTestEnable(VK_FALSE)
                .setDepthWriteEnable(VK_TRUE)
                .setDepthCompareOp(vk::CompareOp::eAlways)
                .setDepthBoundsTestEnable(VK_FALSE);
            break;
        default:
            assert(false && "Invalid depth config");
            break;
        }

        return depthStencilCreateInfo;
    }

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(vk::Device& logicalDevice, std::map<vk::ShaderStageFlagBits,vk::ShaderModule>& shaders,
                         vk::PipelineLayout pipelineLayout,
                         vk::PipelineRenderingCreateInfo dynamicRenderPass, GraphicsPipelineConfigs pipelineDisplayInfo,
                         std::vector<BlendConfigs>& blendConfigs, DepthConfigs depthConfigs, VertexInput& vertexInput,
                         vk::PipelineCache pipelineCache = nullptr)
        {
            assert(!vertexInput.inputDescription.empty()&&"vertexInput is empty");
            assert((shaders.size() >= 2 ) &&"vertex shader module is empty");
            this->pipelineLayout = pipelineLayout;
            std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
            vk::PipelineShaderStageCreateInfo vertShaderStage;
            vk::PipelineShaderStageCreateInfo fragShaderStage;
            vk::PipelineShaderStageCreateInfo tescShaderStage;
            vk::PipelineShaderStageCreateInfo teseShaderStage;
            vk::PipelineShaderStageCreateInfo geomShaderStage;
            int stageIndex = 0;
            for (auto& shader : shaders)
            {
                if (shader.first == vk::ShaderStageFlagBits::eVertex)
                {
                    vertShaderStage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second)
                                      .setStage(vk::ShaderStageFlagBits::eVertex)
                                      .setPName("main");
                    shaderStages[stageIndex] = vertShaderStage;
                }
                if (shader.first == vk::ShaderStageFlagBits::eFragment)
                {
                    fragShaderStage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second)
                                      .setStage(vk::ShaderStageFlagBits::eFragment)
                                      .setPName("main");
                    shaderStages[stageIndex] = fragShaderStage;
                }
                if (shader.first == vk::ShaderStageFlagBits::eTessellationControl)
                {
                    tescShaderStage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second)
                                      .setStage(vk::ShaderStageFlagBits::eTessellationControl)
                                      .setPName("main");
                    shaderStages[stageIndex] = tescShaderStage;
                }
                if (shader.first == vk::ShaderStageFlagBits::eTessellationEvaluation)
                {
                    teseShaderStage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second)
                                      .setStage(vk::ShaderStageFlagBits::eTessellationEvaluation)
                                      .setPName("main");
                    shaderStages[stageIndex] = teseShaderStage;
                }
                if (shader.first == vk::ShaderStageFlagBits::eGeometry)
                {
                    geomShaderStage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second)
                                      .setStage(vk::ShaderStageFlagBits::eGeometry)
                                      .setPName("main");
                    shaderStages[stageIndex] = geomShaderStage;
                }
            }

            auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
                                 .setTopology(GetTopology(pipelineDisplayInfo.topologyConfigs))
                                 .setPrimitiveRestartEnable(VK_FALSE);

            auto rasterization = GetRasterizationInfo(pipelineDisplayInfo.rasterizationConfigs);

            vk::DynamicState dynamicStates[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

            auto dynamicStateInfo = vk::PipelineDynamicStateCreateInfo()
                                    .setDynamicStateCount(2)
                                    .setPDynamicStates(dynamicStates);

            auto pipelineViewport = vk::PipelineViewportStateCreateInfo()
                                    .setViewportCount(1)
                                    .setScissorCount(1);

            auto multiSample = vk::PipelineMultisampleStateCreateInfo()
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);

            auto _vertexInput = vk::PipelineVertexInputStateCreateInfo()
                                .setVertexBindingDescriptionCount(1)
                                .setPVertexBindingDescriptions(vertexInput.bindingDescription.data())
                                .setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexInput.inputDescription.size()))
                                .setPVertexAttributeDescriptions(vertexInput.inputDescription.data());

            vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
            if (depthConfigs != D_NONE)
            {
                 depthStencilStateCreateInfo = GetDepthStencil(depthConfigs);
            }

            std::vector<vk::PipelineColorBlendAttachmentState> blendStates;
            for (auto blendConfig : blendConfigs)
            {
                vk::PipelineColorBlendAttachmentState blendState = GetBlendAttachmentState(blendConfig);
                blendStates.push_back(blendState);
                
            }
            auto pipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
                                              .setLogicOpEnable(VK_FALSE)
                                              .setAttachmentCount(static_cast<uint32_t>(blendStates.size()))
                                              .setPAttachments(blendStates.data());
            
            auto graphicsPipeline = vk::GraphicsPipelineCreateInfo()
                                    .setStageCount(2)
                                    .setPStages(shaderStages.data())
                                    .setPVertexInputState(&_vertexInput)
                                    .setPInputAssemblyState(&inputAssembly)
                                    .setPDynamicState(&dynamicStateInfo)
                                    .setPViewportState(&pipelineViewport)
                                    .setPRasterizationState(&rasterization)
                                    .setPMultisampleState(&multiSample)
                                    .setPColorBlendState(&pipelineColorBlendStateCreateInfo)
                                    .setPDepthStencilState(&depthStencilStateCreateInfo)
                                    .setLayout(pipelineLayout)
                                    .setRenderPass(VK_NULL_HANDLE)
                                    .setPNext(&dynamicRenderPass)
                                    .setBasePipelineHandle(VK_NULL_HANDLE)
                                    .setBasePipelineIndex(-1);

            pipelineHandle = logicalDevice.createGraphicsPipelineUnique(pipelineCache, graphicsPipeline).value;
        }
        GraphicsPipeline(vk::Device& logicalDevice, vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader,
                         vk::PipelineLayout pipelineLayout,
                         vk::PipelineRenderingCreateInfo dynamicRenderPass, GraphicsPipelineConfigs pipelineDisplayInfo,
                         std::vector<BlendConfigs>& blendConfigs, DepthConfigs depthConfigs, VertexInput& vertexInput,
                         vk::PipelineCache pipelineCache = nullptr)
        {
            assert(!vertexInput.inputDescription.empty()&&"vertexInput is empty");
            assert((vertexShader != nullptr) &&"vertex shader module is empty");
            assert((fragmentShader != nullptr) &&"fragment shader module is empty");
            this->pipelineLayout = pipelineLayout;
            std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);

            auto vertShaderStage = vk::PipelineShaderStageCreateInfo()
                                   .setModule(vertexShader)
                                   .setStage(vk::ShaderStageFlagBits::eVertex)
                                   .setPName("main");
            auto fragShaderStage = vk::PipelineShaderStageCreateInfo()
                                   .setModule(fragmentShader)
                                   .setStage(vk::ShaderStageFlagBits::eFragment)
                                   .setPName("main");

            shaderStages[0] = vertShaderStage;
            shaderStages[1] = fragShaderStage;


            auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
                                 .setTopology(GetTopology(pipelineDisplayInfo.topologyConfigs))
                                 .setPrimitiveRestartEnable(VK_FALSE);

            auto rasterization = GetRasterizationInfo(pipelineDisplayInfo.rasterizationConfigs);

            vk::DynamicState dynamicStates[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

            auto dynamicStateInfo = vk::PipelineDynamicStateCreateInfo()
                                    .setDynamicStateCount(2)
                                    .setPDynamicStates(dynamicStates);

            auto pipelineViewport = vk::PipelineViewportStateCreateInfo()
                                    .setViewportCount(1)
                                    .setScissorCount(1);

            auto multiSample = vk::PipelineMultisampleStateCreateInfo()
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);

            auto _vertexInput = vk::PipelineVertexInputStateCreateInfo()
                                .setVertexBindingDescriptionCount(1)
                                .setPVertexBindingDescriptions(vertexInput.bindingDescription.data())
                                .setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexInput.inputDescription.size()))
                                .setPVertexAttributeDescriptions(vertexInput.inputDescription.data());

            vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
            if (depthConfigs != D_NONE)
            {
                 depthStencilStateCreateInfo = GetDepthStencil(depthConfigs);
            }

            std::vector<vk::PipelineColorBlendAttachmentState> blendStates;
            for (auto blendConfig : blendConfigs)
            {
                vk::PipelineColorBlendAttachmentState blendState = GetBlendAttachmentState(blendConfig);
                blendStates.push_back(blendState);
                
            }
            auto pipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
                                              .setLogicOpEnable(VK_FALSE)
                                              .setAttachmentCount(static_cast<uint32_t>(blendStates.size()))
                                              .setPAttachments(blendStates.data());
            
            auto graphicsPipeline = vk::GraphicsPipelineCreateInfo()
                                    .setStageCount(2)
                                    .setPStages(shaderStages.data())
                                    .setPVertexInputState(&_vertexInput)
                                    .setPInputAssemblyState(&inputAssembly)
                                    .setPDynamicState(&dynamicStateInfo)
                                    .setPViewportState(&pipelineViewport)
                                    .setPRasterizationState(&rasterization)
                                    .setPMultisampleState(&multiSample)
                                    .setPColorBlendState(&pipelineColorBlendStateCreateInfo)
                                    .setPDepthStencilState(&depthStencilStateCreateInfo)
                                    .setLayout(pipelineLayout)
                                    .setRenderPass(VK_NULL_HANDLE)
                                    .setPNext(&dynamicRenderPass)
                                    .setBasePipelineHandle(VK_NULL_HANDLE)
                                    .setBasePipelineIndex(-1);

            pipelineHandle = logicalDevice.createGraphicsPipelineUnique(pipelineCache, graphicsPipeline).value;
        }


        vk::UniquePipeline pipelineHandle;
        vk::PipelineLayout pipelineLayout;
    };

    class ComputePipeline
    {
    public:
        ComputePipeline(vk::Device logicalDevice, vk::ShaderModule computeModule, vk::PipelineLayout pipelineLayout, vk::PipelineCache pipelineCache = nullptr)
        {
            assert(computeModule != nullptr&& "Compute shader module is empty");
            this->pipelineLayout = pipelineLayout;
            auto computeStage = vk::PipelineShaderStageCreateInfo()
                                .setStage(vk::ShaderStageFlagBits::eCompute)
                                .setModule(computeModule)
                                .setPName("main");

            auto viewportSate = vk::PipelineViewportStateCreateInfo();

            auto computePipeline = vk::ComputePipelineCreateInfo()
                                   .setFlags(vk::PipelineCreateFlags())
                                   .setStage(computeStage)
                                   .setLayout(pipelineLayout)
                                   .setBasePipelineHandle(nullptr)
                                   .setBasePipelineIndex(-1);

            pipelineHandle = logicalDevice.createComputePipelineUnique(pipelineCache, computePipeline).value;
        }

        vk::PipelineLayout pipelineLayout;
        vk::UniquePipeline pipelineHandle;
    };
}


#endif //PIPELINE_HPP
