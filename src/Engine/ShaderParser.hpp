//

// Created by carlo on 2024-10-09.
//


#ifndef SHADERPARSER_HPP
#define SHADERPARSER_HPP

#include <spirv_glsl.hpp>
#include <glslang/Include/glslang_c_interface.h>
#include <slang.h>

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
        std::vector<uint32_t>& CompileIntoSpirv(const std::string& path ,const std::string& code, const std::string& entryPoint, ShaderStage stage)
        {
            SlangStage slangStage = SLANG_STAGE_NONE;
            switch (stage)
            {
            case S_VERT:
                slangStage = SLANG_STAGE_VERTEX;
                break;
            case S_FRAG:
                
                slangStage = SLANG_STAGE_FRAGMENT;
                break;
            case S_COMP:
                slangStage = SLANG_STAGE_COMPUTE;
                break;
            case S_UNKNOWN:
                assert(false&& "unsuported stage");
                break;
            }
            SlangSession* session = spCreateSession(nullptr);
            if (!session)
            {
                assert(false&& "failed to create session");
            }
            SlangCompileRequest* request = spCreateCompileRequest(session);
            if (!session)
            {
                spDestroySession(session);
                assert(false&& "failed to create compilation request");
            }

            spSetCodeGenTarget(request, SLANG_SPIRV);
            int translationUnitIndex= spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

            spAddEntryPoint(request, translationUnitIndex, entryPoint.c_str(), slangStage);
            
            spAddTranslationUnitSourceString(request, translationUnitIndex, entryPoint.c_str(), code.c_str());

            if (spCompile(request) != SLANG_OK)
            {
                const char* diagnostics = spGetDiagnosticOutput(request);
                printf("Compilation failed: %s\n", diagnostics);
            }
            size_t spirvSize = 0;
            const void* spirvCode = spGetEntryPointCode(request, 0, &spirvSize);
            


            // assert(false && "Unsuported extension for compiling into spirv at runtime");
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
