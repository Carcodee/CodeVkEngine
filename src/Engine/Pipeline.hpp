//


// Created by carlo on 2024-09-26.
//



#ifndef PIPELINE_HPP
#define PIPELINE_HPP

namespace engine
{

    static vk::PrimitiveTopology get_topology(topology_configs topology_configs)
    {
        switch (topology_configs)
        {
        case t_triangle:
            return vk::PrimitiveTopology::eTriangleList;
        case t_point_list:
            return vk::PrimitiveTopology::ePointList;
        case t_patch_list:
            return vk::PrimitiveTopology::ePatchList;
        default:
            assert(false);
            break;
        }
        
    }
    static vk::PipelineRasterizationStateCreateInfo get_rasterization_info(rasterization_configs configs)
    {
        auto rasterization_info = vk::PipelineRasterizationStateCreateInfo();

        switch (configs) {
        case r_fill:
            rasterization_info.setDepthClampEnable(VK_FALSE)
                             .setRasterizerDiscardEnable(VK_FALSE)
                             .setPolygonMode(vk::PolygonMode::eFill)
                             .setCullMode(vk::CullModeFlagBits::eNone)
                             .setFrontFace(vk::FrontFace::eClockwise)
                             .setLineWidth(1.0f);
            break;
        case r_line:
            rasterization_info.setDepthClampEnable(VK_FALSE)
                             .setRasterizerDiscardEnable(VK_FALSE)
                             .setPolygonMode(vk::PolygonMode::eLine)
                             .setCullMode(vk::CullModeFlagBits::eNone)
                             .setFrontFace(vk::FrontFace::eClockwise)
                             .setLineWidth(1.0f);
            break;
        case r_point:
            rasterization_info.setDepthClampEnable(VK_FALSE)
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
        return rasterization_info;
    }


    static vk::PipelineColorBlendAttachmentState get_blend_attachment_state(blend_configs configs)
    {
        auto blend_attachment_state = vk::PipelineColorBlendAttachmentState();
        switch (configs)
        {
        case b_none:
            blend_attachment_state.setColorWriteMask(vk::ColorComponentFlags{})
                                .setBlendEnable(VK_FALSE);
            break;
        case b_opaque:
            blend_attachment_state.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                    vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                                .setBlendEnable(VK_FALSE);
            break;
        case b_add:
            blend_attachment_state.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR |
                                    vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
                                .setBlendEnable(VK_TRUE)
                                .setAlphaBlendOp(vk::BlendOp::eAdd)
                                .setColorBlendOp(vk::BlendOp::eAdd)
                                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                                .setDstColorBlendFactor(vk::BlendFactor::eOne);
            break;
        case b_mix:
            blend_attachment_state.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR |
                                    vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
                                .setBlendEnable(VK_TRUE)
                                .setAlphaBlendOp(vk::BlendOp::eAdd)
                                .setColorBlendOp(vk::BlendOp::eAdd)
                                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
            break;
        case b_alpha_blend:
            blend_attachment_state.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR |
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


        return blend_attachment_state;
    }

    static vk::PipelineDepthStencilStateCreateInfo get_depth_stencil(depth_configs configs)
    {
        auto depth_stencil_create_info = vk::PipelineDepthStencilStateCreateInfo()
                                      .setDepthTestEnable(VK_TRUE)
                                      .setDepthWriteEnable(VK_TRUE)
                                      .setDepthCompareOp(vk::CompareOp::eLess)
                                      .setDepthBoundsTestEnable(VK_FALSE)
                                      .setStencilTestEnable(VK_FALSE);
        switch (configs)
        {
        case d_enable:
            depth_stencil_create_info
                .setDepthTestEnable(VK_TRUE)
                .setDepthWriteEnable(VK_TRUE)
                .setDepthCompareOp(vk::CompareOp::eLess)
                .setDepthBoundsTestEnable(VK_FALSE);
            break;
        case d_disable:
            depth_stencil_create_info
                .setDepthTestEnable(VK_FALSE)
                .setDepthWriteEnable(VK_TRUE)
                .setDepthCompareOp(vk::CompareOp::eAlways)
                .setDepthBoundsTestEnable(VK_FALSE);
            break;
        default:
            assert(false && "Invalid depth config");
            break;
        }

        return depth_stencil_create_info;
    }

    class graphics_pipeline
    {
    public:
        graphics_pipeline(vk::Device& logical_device,
                         std::map<shader_stage, shader*>& shaders,
                         vk::PipelineLayout pipeline_layout,
                         vk::PipelineRenderingCreateInfo dynamic_render_pass,
                         engine::graphics_pipeline_configs pipeline_display_info,
                         std::vector<blend_configs>& blend_configs,
                         depth_configs depth_configs,
                         engine::vertex_input& vertex_layout,
                         vk::PipelineCache pipeline_cache = nullptr)
        {
            assert(!vertex_layout.input_description.empty()&&"vertexInput is empty");
            assert((shaders.size() >= 2 ) &&"vertex shader module is empty");
            this->pipeline_layout = pipeline_layout;
            std::vector<vk::PipelineShaderStageCreateInfo> shader_stages (shaders.size());
            vk::PipelineShaderStageCreateInfo vert_shader_stage;
            vk::PipelineShaderStageCreateInfo frag_shader_stage;
            vk::PipelineShaderStageCreateInfo tesc_shader_stage;
            vk::PipelineShaderStageCreateInfo tese_shader_stage;
            vk::PipelineShaderStageCreateInfo geom_shader_stage;
            int stage_index = 0;
            for (auto& shader : shaders)
            {
                if (shader.first == s_vert)
                {
                    vert_shader_stage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second->s_module.get()->shader_module_handle.get())
                                      .setStage(vk::ShaderStageFlagBits::eVertex)
                                      .setPName("main");
                    shader_stages[stage_index] = vert_shader_stage;
                }
                if (shader.first == s_frag)
                {
                    frag_shader_stage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second->s_module.get()->shader_module_handle.get())
                                      .setStage(vk::ShaderStageFlagBits::eFragment)
                                      .setPName("main");
                    shader_stages[stage_index] = frag_shader_stage;
                }
                if (shader.first == s_tess_control)
                {
                    tesc_shader_stage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second->s_module.get()->shader_module_handle.get())
                                      .setStage(vk::ShaderStageFlagBits::eTessellationControl)
                                      .setPName("main");
                    shader_stages[stage_index] = tesc_shader_stage;
                }
                if (shader.first == s_tess_eval)
                {
                    tese_shader_stage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second->s_module.get()->shader_module_handle.get())
                                      .setStage(vk::ShaderStageFlagBits::eTessellationEvaluation)
                                      .setPName("main");
                    shader_stages[stage_index] = tese_shader_stage;
                }
                if (shader.first == s_geom)
                {
                    geom_shader_stage = vk::PipelineShaderStageCreateInfo()
                                      .setModule(shader.second->s_module.get()->shader_module_handle.get())
                                      .setStage(vk::ShaderStageFlagBits::eGeometry)
                                      .setPName("main");
                    shader_stages[stage_index] = geom_shader_stage;
                }
                stage_index++;
            }

            vk::PipelineTessellationStateCreateInfo tessellation_state_create_info;
            
            
            auto input_assembly = vk::PipelineInputAssemblyStateCreateInfo()
                                 .setTopology(get_topology(pipeline_display_info.topology_configs))
                                 .setPrimitiveRestartEnable(VK_FALSE);

            auto rasterization = get_rasterization_info(pipeline_display_info.rasterization_configs);

            vk::DynamicState dynamic_states[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

            auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo()
                                    .setDynamicStateCount(2)
                                    .setPDynamicStates(dynamic_states);

            auto pipeline_viewport = vk::PipelineViewportStateCreateInfo()
                                    .setViewportCount(1)
                                    .setScissorCount(1);

            auto multi_sample = vk::PipelineMultisampleStateCreateInfo()
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);

            auto vertex_input_state = vk::PipelineVertexInputStateCreateInfo()
                                .setVertexBindingDescriptionCount(1)
                                .setPVertexBindingDescriptions(vertex_layout.binding_description.data())
                                .setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertex_layout.input_description.size()))
                                .setPVertexAttributeDescriptions(vertex_layout.input_description.data());

            vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_create_info;
            if (depth_configs != d_none)
            {
                 depth_stencil_state_create_info = get_depth_stencil(depth_configs);
            }

            std::vector<vk::PipelineColorBlendAttachmentState> blend_states;
            for (auto blend_config : blend_configs)
            {
                vk::PipelineColorBlendAttachmentState blend_state = get_blend_attachment_state(blend_config);
                blend_states.push_back(blend_state);
                
            }
            auto pipeline_color_blend_state_create_info = vk::PipelineColorBlendStateCreateInfo()
                                              .setLogicOpEnable(VK_FALSE)
                                              .setAttachmentCount(static_cast<uint32_t>(blend_states.size()))
                                              .setPAttachments(blend_states.data());
            
            auto graphics_pipeline = vk::GraphicsPipelineCreateInfo()
                                    .setStageCount(shader_stages.size())
                                    .setPStages(shader_stages.data())
                                    .setPVertexInputState(&vertex_input_state)
                                    .setPInputAssemblyState(&input_assembly)
                                    .setPDynamicState(&dynamic_state_info)
                                    .setPViewportState(&pipeline_viewport)
                                    .setPRasterizationState(&rasterization)
                                    .setPMultisampleState(&multi_sample)
                                    .setPColorBlendState(&pipeline_color_blend_state_create_info)
                                    .setPDepthStencilState(&depth_stencil_state_create_info)
                                    .setLayout(pipeline_layout)
                                    .setRenderPass(VK_NULL_HANDLE)
                                    .setPNext(&dynamic_render_pass)
                                    .setBasePipelineHandle(VK_NULL_HANDLE)
                                    .setBasePipelineIndex(-1);
            
            if (shaders.contains(s_tess_control))
            {
                if (pipeline_display_info.topology_configs != topology_configs::t_patch_list)
                {
                    assert(false && "cant create tesselation shader without triangle patch list");
                }

                tessellation_state_create_info = vk::PipelineTessellationStateCreateInfo()
                .setPatchControlPoints(3);
                graphics_pipeline.setPTessellationState(&tessellation_state_create_info);
                
            }

            pipeline_handle = logical_device.createGraphicsPipelineUnique(pipeline_cache, graphics_pipeline).value;
        }


        vk::UniquePipeline pipeline_handle;
        vk::PipelineLayout pipeline_layout;
    };

    class compute_pipeline
    {
    public:
        compute_pipeline(vk::Device logical_device, vk::ShaderModule compute_module, vk::PipelineLayout pipeline_layout, vk::PipelineCache pipeline_cache = nullptr)
        {
            assert(compute_module != nullptr&& "Compute shader module is empty");
            this->pipeline_layout = pipeline_layout;
            auto compute_stage = vk::PipelineShaderStageCreateInfo()
                                .setStage(vk::ShaderStageFlagBits::eCompute)
                                .setModule(compute_module)
                                .setPName("main");

            auto viewport_sate = vk::PipelineViewportStateCreateInfo();

            auto compute_pipeline = vk::ComputePipelineCreateInfo()
                                   .setFlags(vk::PipelineCreateFlags())
                                   .setStage(compute_stage)
                                   .setLayout(pipeline_layout)
                                   .setBasePipelineHandle(nullptr)
                                   .setBasePipelineIndex(-1);

            pipeline_handle = logical_device.createComputePipelineUnique(pipeline_cache, compute_pipeline).value;
        }

        vk::PipelineLayout pipeline_layout;
        vk::UniquePipeline pipeline_handle;
    };
}


#endif //PIPELINE_HPP
