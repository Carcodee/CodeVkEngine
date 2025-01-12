//

// Created by carlo on 2024-10-09.
//



#ifndef SHADERPARSER_HPP
#define SHADERPARSER_HPP

#include <slang.h>

#include <spirv_glsl.hpp>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace ENGINE
{
    enum ShaderStage
    {
        S_VERT,
        S_FRAG,
        S_COMP,
        S_UNKNOWN
    };

    static std::vector<uint32_t> CompileSlangIntoSpirv(const std::string& outFile, const std::string& code,
                                                  const std::string& entryPoint, ShaderStage stage)
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
            assert(false&& "failed to create compilation request");
        }

        spSetCodeGenTarget(request, SLANG_SPIRV);
        int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

        spAddEntryPoint(request, translationUnitIndex, entryPoint.c_str(), slangStage);

        spAddTranslationUnitSourceString(request, translationUnitIndex, entryPoint.c_str(), code.c_str());

        if (spCompile(request) != SLANG_OK)
        {
            const char* diagnostics = spGetDiagnosticOutput(request);
            printf("Compilation failed: %s\n", diagnostics);
        }
        size_t spirvSize = 0;
        const void* spirvCode = spGetEntryPointCode(request, 0, &spirvSize);

        spDestroyCompileRequest(request);
        spDestroySession(session);
        
        if (spirvCode)
        {
            SYSTEMS::OS::WriteFile(outFile, reinterpret_cast<const char*>(spirvCode), spirvSize);
            const uint32_t* spirvWords = reinterpret_cast<const uint32_t*>(spirvCode);
            size_t wordCount = spirvSize / sizeof(uint32_t);;
            return std::vector<uint32_t>(spirvWords, spirvWords + wordCount);
        }

        assert(false && "error when compiling into spirv at runtime");
        return std::vector<uint32_t>();
    }

    static std::vector<uint32_t> CompileGlslIntoSpirv(const std::string& outFile,
                                                       const std::string& code,
                                                        ShaderStage stage)
    {
        EShLanguage shaderStage;
        switch (stage)
        {
        case S_VERT:
            shaderStage = EShLangVertex;
            break;
        case S_FRAG:
            shaderStage = EShLangFragment;
            break;
        case S_COMP:
            shaderStage = EShLangCompute;
            break;
        case S_UNKNOWN:
            break;
        }
        glslang::TShader shader(shaderStage); // Vertex shader type
        const char* shaderCode = code.c_str();
        shader.setStrings(&shaderCode, 1);
        shader.setEnvInput(glslang::EShSourceGlsl, shaderStage, glslang::EShClientVulkan, 450);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);


        if (!shader.parse(GetDefaultResources(), 450, false, EShMsgDefault))
        {
            std::cout << "GLSL Syntax Error in file: " << outFile << "\n"
                << shader.getInfoLog() << "\n"
                << shader.getInfoDebugLog() << std::endl;
        }

        glslang::TProgram program;
        program.addShader(&shader);

        if (program.link(EShMsgDefault))
        {
            assert(false && "Unable to link glsl");
        }

        std::vector<uint32_t> spirv;
        glslang::GlslangToSpv(*program.getIntermediate(shaderStage), spirv);

        SYSTEMS::OS::WriteFile(outFile, reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
        
        return spirv;
    
    }
    class ShaderParser
    {

    public:

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
        Shader(vk::Device logicalDevice, std::string path, ShaderStage stage)
        {
            assert(std::filesystem::exists(path) && "Path does not exist");
            this->stage = stage;
            HandlePathReceived(path);
            this->logicalDevice = logicalDevice;
            std::vector<uint32_t> byteCode = GetByteCode(spirvPath);
            sParser = std::make_unique<ShaderParser>(byteCode);
            sModule = std::make_unique<ShaderModule>(logicalDevice, byteCode);
        }
        void Reload()
        {
            std::string code = SYSTEMS::OS::ReadFile(path);
            // std::vector<uint32_t> byteCode = GetByteCode(spirvPath);
            std::vector<uint32_t> byteCode;
            if (std::filesystem::path(path).extension() == ".slang")
            {
                std::string entryPoint = "";
                switch (stage)
                {
                case S_VERT:
                    entryPoint = "mainVS";
                    break;
                case S_FRAG:
                    entryPoint = "mainFS";
                    break;
                case S_COMP:
                    entryPoint = "mainCS";
                    break;
                case S_UNKNOWN:
                    assert(false && "uknown stage");
                    break;
                }
                byteCode = CompileSlangIntoSpirv(spirvPath, code, entryPoint, stage);
            }
            else if (std::filesystem::path(path).extension() == ".frag" || std::filesystem::path(path).extension() == ".vert"
                || std::filesystem::path(path).extension() == ".comp")
            {
                byteCode = CompileGlslIntoSpirv(spirvPath, code, stage);
            }else
            {
                assert(false && "invalid shader file extension");
            }
            
            sParser.reset();
            sModule.reset();
            sParser = std::make_unique<ShaderParser>(byteCode);
            sModule = std::make_unique<ShaderModule>(logicalDevice, byteCode);
        }

        void HandlePathReceived(const std::string& path)
        {
            std::filesystem::path filePath(path);
            bool glsl = false;

            if (filePath.extension() == ".spv")
            {
                std::filesystem::path shaderPath;
                for (auto& dirsParts : filePath)
                {
                    if (dirsParts.string() == "spirvSlang")
                    {
                        shaderPath /= "slang";
                    }
                    else if (dirsParts.string() == "spirvGlsl")
                    {
                        glsl = true;
                        shaderPath /= "glsl";
                    }
                    else
                    {
                        if (dirsParts.extension() == ".spv")
                        {
                            int firstDotPos = dirsParts.string().find_first_of('.');
                            std::string fileNameNoExt = dirsParts.string().substr(0, firstDotPos);
                            std::string extension = ".slang";
                            if (glsl)
                            {
                                switch (stage)
                                {
                                case S_VERT:
                                    extension = ".vert";
                                    break;
                                case S_FRAG:
                                    extension = ".frag";
                                    break;
                                case S_COMP:
                                    extension = ".comp";
                                    break;
                                case S_UNKNOWN:
                                    extension = ".invalid";
                                    break;
                                }
                            }
                            std::string pathName = fileNameNoExt + extension;
                            shaderPath /= pathName;
                        }
                        else
                        {
                            shaderPath /= dirsParts;
                        }
                    }
                }
                assert((std::filesystem::exists(shaderPath) && shaderPath != filePath) && "invalid shader path");
                this->path = shaderPath.string();
                this->spirvPath = path;
            }
                
            // }else
            // {
            //     std::filesystem::path spirvPath;
            //     if (filePath.extension() == ".slang" || filePath.extension() == ".frag" || filePath.extension() == ".vert" || filePath.extension() == ".comp")
            //     {
            //         for (auto& dirsParts : filePath)
            //         {
            //             if (dirsParts.c_str() == L"slang")
            //             {
            //                 spirvPath /= "spirvSlang";
            //             }else if(dirsParts.c_str() == L"glsl")
            //             {
            //                 spirvPath /= "spirvGlsl";
            //             }else 
            //             {
            //                 spirvPath /= dirsParts;
            //             }
            //         }
            //     }
            //     assert(spirvPath.c_str() == filePath && "invalid shader path");
            //     this->spirvPath = spirvPath.string();
            //     this->path = path;
            // }
            
            
            
        }
        
        std::unique_ptr<ShaderParser> sParser;
        std::unique_ptr<ShaderModule> sModule;
        vk::Device logicalDevice;
        ShaderStage stage;
        //spirv path
        std::string path;
        std::string spirvPath;
        
        
    };
}

#endif //SHADERPARSER_HPP
