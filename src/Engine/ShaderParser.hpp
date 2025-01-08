//

// Created by carlo on 2024-10-09.
//


#ifndef SHADERPARSER_HPP
#define SHADERPARSER_HPP

namespace ENGINE
{
    class ShaderParser
    {

    public:
        enum ShaderStage
        {
            S_VERT,
            S_FRAG,
            S_COMP,
            S_UNKNOWN
        };

        ShaderParser(std::vector<uint32_t>& byteCode)
        {
            spirv_cross::CompilerGLSL glsl((byteCode));

            spirv_cross::ShaderResources resources = glsl.get_shader_resources();

            auto entryPoint = glsl.get_entry_points_and_stages();
            for (auto& entry : entryPoint)
            {
                spv::ExecutionModel model = entry.execution_model;

                switch (model)
                {
                case spv::ExecutionModelVertex:
                    stage = S_VERT;
                    break;
                case spv::ExecutionModelFragment:
                    stage = S_FRAG;
                    break;
                case spv::ExecutionModelGLCompute:
                    stage = S_COMP;
                    break;
                default:
                    std::cout << "Unknown shader stage\n";
                    stage = S_UNKNOWN;
                    break;
                }
            }


            for (auto& resource : resources.uniform_buffers)
            {
                std::string name = resource.name;
                uint32_t binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                bool array = false;
                spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
                if (!type.array.empty())
                {
                    array = true;
                }
                uniformBuffers.emplace_back(ShaderResource{name, binding, set, array, vk::DescriptorType::eUniformBuffer});


            }
             for (auto& resource : resources.storage_buffers)
            {
                
                std::string name = resource.name;
                uint32_t binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                bool array = false;
                spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
                if (!type.array.empty())
                {
                    array = true;
                }
                storageBuffers.emplace_back(ShaderResource{name, binding, set, array, vk::DescriptorType::eStorageBuffer});
            }
            for (auto& resource : resources.sampled_images)
            {

                std::string name = resource.name;
                uint32_t binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                bool array = false;
                spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
                if (!type.array.empty())
                {
                    array = true;
                }
                
                sampledImages.emplace_back(ShaderResource{name, binding, set, array, vk::DescriptorType::eCombinedImageSampler});
            }
            
            for (auto& resource : resources.storage_images)
            {
                std::string name = resource.name;
                uint32_t binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                bool array = false;
                spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
                if (!type.array.empty())
                {
                    array = true;
                }
                
                storageImages.emplace_back(ShaderResource{name, binding, set, array, vk::DescriptorType::eStorageImage});
            }
            
        }
        void CompileIntoSpirv(const std::string& path)
        {
            std::filesystem::path shaderPath(path);

            if (shaderPath.extension() != ".slang")
            {
                assert(false && "Unsuported extension for compiling into spirv at runtime");
            }
            
        }

        void GetBinding(std::vector<ShaderResource>& resources, DescriptorLayoutBuilder& builder)
        {
            for (auto& resource : resources)
            {
                if (builder.uniqueBindings.contains(resource.binding))
                {
                    continue;
                }
                builder.AddBinding(resource.binding, resource.type);
            }
        }

        void GetBinding(std::vector<ShaderResource>& resources, std::vector<ShaderResource>& uniqueResources, std::set<uint32_t> uniqueBindings)
        {
            for (auto& resource : resources)
            {
                if (uniqueBindings.contains(resource.binding))
                {
                    continue;
                }
                
                uniqueResources.emplace_back(resource);
            }
        }
        void GetLayout(DescriptorLayoutBuilder& builder)
        {
            std::set<uint32_t> repeatedBindings;
            GetBinding(sampledImages, builder);
            GetBinding(storageImages, builder);
            GetBinding(uniformBuffers, builder);
            GetBinding(storageBuffers, builder);
        }

        void GetLayout(std::vector<ShaderResource>& uniqueResources)
        {
            std::set<uint32_t> uniqueBindings;
            GetBinding(sampledImages, uniqueResources, uniqueBindings);
            GetBinding(storageImages, uniqueResources, uniqueBindings);
            GetBinding(uniformBuffers,uniqueResources, uniqueBindings);
            GetBinding(storageBuffers,uniqueResources, uniqueBindings);
        }

        vk::ShaderStageFlags GetVkStage()
        {
            switch (stage)
            {
            case S_VERT:
                return vk::ShaderStageFlagBits::eVertex;
                break;
            case S_FRAG:
                return vk::ShaderStageFlagBits::eFragment;
                break;
            case S_COMP:
                return vk::ShaderStageFlagBits::eCompute;
                break;
            case S_UNKNOWN:
                assert(false&& "not valid stage");
                break;
            }
        }

        std::vector<ShaderResource> uniformBuffers;
        std::vector<ShaderResource> storageBuffers;
        std::vector<ShaderResource> sampledImages;
        std::vector<ShaderResource> storageImages;
        ShaderStage stage;
    };

    class Shader 
    {
    public:
        Shader(vk::Device logicalDevice, std::string path)
        {
            assert(std::filesystem::exists(path) && "Path does not exist");
            this->path = path;
            this->logicalDevice = logicalDevice;
            std::vector<uint32_t> byteCode = GetByteCode(path);
            sParser = std::make_unique<ShaderParser>(byteCode);
            sModule = std::make_unique<ShaderModule>(logicalDevice, byteCode);
        }
        void Reload()
        {
            std::vector<uint32_t> byteCode = GetByteCode(path);
            sParser.reset();
            sModule.reset();
            sParser = std::make_unique<ShaderParser>(byteCode);
            sModule = std::make_unique<ShaderModule>(logicalDevice, byteCode);
        }
        std::unique_ptr<ShaderParser> sParser;
        std::unique_ptr<ShaderModule> sModule;
        vk::Device logicalDevice;
        std::string path;
        
        
    };
}

#endif //SHADERPARSER_HPP
