//

// Created by carlo on 2024-10-09.
//








#ifndef SHADER_PARSER_HPP
#define SHADER_PARSER_HPP

#include <slang.h>

#include <spirv_glsl.hpp>
#include <shaderc/shaderc.hpp>

namespace engine
{
    enum Shader_Stage
    {
        S_Vert,
        S_Frag,
        S_Tess_Control,
        S_Tess_Eval,
        S_Geom,
        S_Comp,
        S_Unknown
    };

    enum Shader_Compiler
    {
        C_Glsl,
        C_Slang
    };
    static bool Is_Glsl_Valid_Extension(const std::string extension)
    {
        if (extension == ".comp" || extension == ".vert" || extension == ".frag" || extension == ".tesc" || extension ==
            ".tese" || extension == ".geom" )
        {
            return true;
        }
        return false;
    }
    
    static std::string Convert_Shader_Path_To_Spirv(const std::filesystem::path& file_path, Shader_Stage stage)
    {
        assert(
            (file_path.extension() == ".slang" ||
                file_path.extension() == ".vert" ||
                file_path.extension() == ".frag" ||
                file_path.extension() == ".comp" ||
                file_path.extension() == ".tese" ||
                file_path.extension() == ".tesc" ||
                file_path.extension() == ".geom" ||
                file_path.extension() == ".spv") && "Path is not a shader");
        if (file_path.extension().string() == ".spv")
        {
            systems::Logger::Get_Instance()->Log_Message("Shader is already spv no conversion needed");
            return file_path.string();
        }
        std::filesystem::path shader_path(file_path);
        std::string filename = shader_path.filename().string();
       
        std::filesystem::path spirv_path;
        for (auto& dirs_parts : file_path)
        {
            if (dirs_parts.string() == "slang")
            {
                spirv_path /= "spirvSlang";
            }
            else if (dirs_parts.string() == "glsl")
            {
                spirv_path /= "spirvGlsl";
            }
            else if ( dirs_parts.string() == filename && dirs_parts.extension() == ".slang")
            {
                int first_dot_pos = dirs_parts.string().find_first_of('.');
                std::string file_name_no_ext = dirs_parts.string().substr(0, first_dot_pos);
                std::string extension = ".slang";
                switch (stage)
                {
                case S_Vert:
                    extension += "_VS.spv";
                    break;
                case S_Frag:
                    extension += "_FS.spv";
                    break;
                case S_Tess_Control:
                    assert(false && "Unsupported slang stage");
                    extension += "_TESC.spv";
                    break;
                case S_Tess_Eval:
                    assert(false && "Unsupported slang stage");
                    extension += "_TESE.spv";
                    break;
                case S_Geom:
                    extension += "_GEOM.spv";
                    break;
                case S_Comp:
                    extension += "_CS.spv";
                    break;
                case S_Unknown:
                    assert(false && "invalid stage");
                    break;
                }
                spirv_path /= file_name_no_ext + extension;
            }
            else if (dirs_parts.string() == filename && Is_Glsl_Valid_Extension(dirs_parts.extension().string()))
            {
                int first_dot_pos = dirs_parts.string().find_first_of('.');
                std::string file_name_no_ext = dirs_parts.string().substr(0, first_dot_pos);
                std::string extension = dirs_parts.extension().string();
                switch (stage)
                {
                case S_Vert:
                    extension += ".spv";
                    break;
                case S_Frag:
                    extension += ".spv";
                    break;
                case S_Tess_Control:
                    extension += ".spv";
                    break;
                case S_Tess_Eval:
                    extension += ".spv";
                    break;
                case S_Geom:
                    extension += ".spv";
                    break;
                case S_Comp:
                    extension += ".spv";
                    break;
                case S_Unknown:
                    assert(false && "invalid stage");
                    break;
                }
                spirv_path /= file_name_no_ext + extension;
            }
            else
            {
                spirv_path /= dirs_parts;
            }
        }
        return spirv_path.string();
    }
        
    static std::string Convert_Spirv_To_Shader_Path(const std::filesystem::path& file_path, Shader_Stage stage)
    {
        assert(file_path.extension() == ".spv" && "Path is not spirv");
        bool glsl = false;
        std::filesystem::path shader_path;
        for (auto& dirs_parts : file_path)
        {
            if (dirs_parts.string() == "spirvSlang")
            {
                shader_path /= "slang";
            }
            else if (dirs_parts.string() == "spirvGlsl")
            {
                glsl = true;
                shader_path /= "glsl";
            }
            else
            {
                if (dirs_parts.extension() == ".spv")
                {
                    int first_dot_pos = dirs_parts.string().find_first_of('.');
                    std::string file_name_no_ext = dirs_parts.string().substr(0, first_dot_pos);
                    std::string extension = ".slang";
                    if (glsl)
                    {
                        switch (stage)
                        {
                        case S_Vert:
                            extension = ".vert";
                            break;
                        case S_Frag:
                            extension = ".frag";
                            break;
                        case S_Tess_Control:
                            extension = ".tesc";
                            break;
                        case S_Tess_Eval:
                            extension = ".tese";
                            break;
                        case S_Geom:
                            extension = ".geom";
                            break;
                        case S_Comp:
                            extension = ".comp";
                            break;
                        case S_Unknown:
                            extension = ".invalid";
                            break;
                        }
                    }
                    std::string path_name = file_name_no_ext + extension;
                    shader_path /= path_name;
                }
                else
                {
                    shader_path /= dirs_parts;
                }
            }
        }
        return shader_path.string();
    }

    static std::vector<uint32_t> Compile_Slang_Into_Spirv(const std::string& out_file, const std::string& code,
                                                  const std::string& entry_point, Shader_Stage stage)
    {
        SlangStage slang_stage = SLANG_STAGE_NONE;
        switch (stage)
        {
        case S_Vert:
            slang_stage = SLANG_STAGE_VERTEX;
            break;
        case S_Frag:
            slang_stage = SLANG_STAGE_FRAGMENT;
            break;
        case S_Tess_Control:
            assert(false&& "unsuported stage");
        case S_Tess_Eval:
            assert(false&& "unsuported stage");
        case S_Geom:
            slang_stage = SLANG_STAGE_GEOMETRY;
        case S_Comp:
            slang_stage = SLANG_STAGE_COMPUTE;
            break;
        case S_Unknown:
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
        const char* preserve_params_args[] = {
            "-preserve-params",
        };

        if (spProcessCommandLineArguments(request, preserve_params_args, 1) != SLANG_OK)
        {
            assert(false && "Failed to set Slang compiler options");
        }
        int translation_unit_index = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

        spAddEntryPoint(request, translation_unit_index, entry_point.c_str(), slang_stage);

        spAddTranslationUnitSourceString(request, translation_unit_index, entry_point.c_str(), code.c_str());

        if (spCompile(request) != SLANG_OK)
        {
            const char* diagnostics = spGetDiagnosticOutput(request);
            printf("Compilation failed using last shader version: %s\n", diagnostics);
            return std::vector<uint32_t>();
        }
        size_t spirv_size = 0;
        const void* spirv_code = spGetEntryPointCode(request, 0, &spirv_size);

        if (spirv_code)
        {
            const char* spirv_code_data =  reinterpret_cast<const char*>(spirv_code);
            systems::Os::Write_File(out_file,spirv_code_data , spirv_size);
            const uint32_t* spirv_words = reinterpret_cast<const uint32_t*>(spirv_code);
            size_t word_count = spirv_size / sizeof(uint32_t);;
            return std::vector<uint32_t>(spirv_words, spirv_words + word_count);
        }

        spDestroyCompileRequest(request);
        spDestroySession(session);
        
        assert(false && "error when compiling into spirv at runtime");
        return std::vector<uint32_t>();
    }

    static std::string Get_Slang_Entry_Point(Shader_Stage stage)
    {
        std::string entry_point = "";
        switch (stage)
        {
        case S_Vert:
            entry_point = "Main_Vs";
            break;
        case S_Frag:
            entry_point = "Main_Fs";
            break;
        case S_Comp:
            entry_point = "Main_Cs";
            break;
        case S_Tess_Control:
            assert(false && "uknown stage");
            break;
        case S_Tess_Eval:
            assert(false && "uknown stage");
            break;
        case S_Geom:
            entry_point = "main_gs";
            break;
        case S_Unknown:
            assert(false && "uknown stage");
            break;
        }
        return entry_point;
    }

    static void Compile_Glsl_Into_Spirv(const std::string& out_file, const std::string& path)
    {
        std::string temp_file_path = systems::Os::Get_Instance()->Get_Shaders_Path() + "\\tempFile.bat";
        systems::Os::Create_File_At(temp_file_path);

        // Start constructing the batch file content
        std::string command = R"(@echo off
setlocal EnableExtensions EnableDelayedExpansion
set compiler_exe="%VULKAN_SDK%\Bin\glslangValidator.exe"
set optimizer_config="OptimizerConfig.cfg"
set "error_found="
        )";

        command += "%compiler_exe% -V \"" + path + "\" -l --target-env vulkan1.2 -o \"" + out_file +
                   "\" || set \"error_found=1\"\n";

        command += R"(if defined error_found (
        echo.
        echo Errors were found during compilation.
        pause
)
)";
        systems::Os::Write_File(temp_file_path, command.c_str(), command.size());

        int result = std::system(temp_file_path.c_str());
        if (result != 0)
        {
            assert(false && "invalid bat file");
        }

        std::remove(temp_file_path.c_str());
    }
    class Shader_Parser
    {

    public:

        Shader_Parser(std::vector<uint32_t>& byte_code, bool slang_resource_names = false, std::string shader_name = "")
        {
            this->shader_name = shader_name;
            
            spirv_cross::CompilerGLSL glsl((byte_code));

            spirv_cross::ShaderResources resources = glsl.get_shader_resources();

            auto entry_point = glsl.get_entry_points_and_stages();
            for (auto& entry : entry_point)
            {
                spv::ExecutionModel model = entry.execution_model;

                switch (model)
                {
                case spv::ExecutionModelVertex:
                    stage = S_Vert;
                    break;
                case spv::ExecutionModelFragment:
                    stage = S_Frag;
                    break;
                case spv::ExecutionModelTessellationControl:
                    stage = S_Tess_Control;
                    break;
                case spv::ExecutionModelTessellationEvaluation:
                    stage = S_Tess_Eval;
                    break;
                case spv::ExecutionModelGeometry:
                    stage = S_Geom;
                    break;
                case spv::ExecutionModelGLCompute:
                    stage = S_Comp;
                    break;
                default:
                    std::cout << "Unknown shader stage\n";
                    stage = S_Unknown;
                    break;
                }
            }


            for (auto& resource : resources.uniform_buffers)
            {
                std::string name = Get_Buffer_Resource_Name(glsl, resource, slang_resource_names);
                uint32_t binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                bool array = false;
                spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
                if (!type.array.empty())
                {
                    array = true;
                }
                uniform_buffers.emplace_back(Shader_Resource{name, binding, set, array, vk::DescriptorType::eUniformBuffer});


            }
             for (auto& resource : resources.storage_buffers)
            {
                std::string name = Get_Buffer_Resource_Name(glsl, resource, slang_resource_names);
                uint32_t binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                bool array = false;
                spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
                if (!type.array.empty())
                {
                    array = true;
                }
                storage_buffers.emplace_back(Shader_Resource{name, binding, set, array, vk::DescriptorType::eStorageBuffer});
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
                
                sampled_images.emplace_back(Shader_Resource{name, binding, set, array, vk::DescriptorType::eCombinedImageSampler});
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
                
                storage_images.emplace_back(Shader_Resource{name, binding, set, array, vk::DescriptorType::eStorageImage});
            }
            
        }


        void Get_Binding(std::vector<Shader_Resource>& resources, Descriptor_Layout_Builder& builder)
        {
            for (auto& resource : resources)
            {
                if (builder.unique_bindings.contains(resource.binding))
                {
                    continue;
                }
                builder.Add_Binding(resource.binding, resource.type);
            }
        }

        void Get_Binding(std::vector<Shader_Resource>& resources, std::vector<Shader_Resource>& unique_resources, std::set<uint32_t> unique_bindings)
        {
            for (auto& resource : resources)
            {
                if (unique_bindings.contains(resource.binding))
                {
                    continue;
                }
                
                unique_resources.emplace_back(resource);
            }
        }
        void Get_Layout(Descriptor_Layout_Builder& builder)
        {
            std::set<uint32_t> repeated_bindings;
            Get_Binding(sampled_images, builder);
            Get_Binding(storage_images, builder);
            Get_Binding(uniform_buffers, builder);
            Get_Binding(storage_buffers, builder);
        }

        void Get_Layout(std::vector<Shader_Resource>& unique_resources)
        {
            std::set<uint32_t> unique_bindings;
            Get_Binding(sampled_images, unique_resources, unique_bindings);
            Get_Binding(storage_images, unique_resources, unique_bindings);
            Get_Binding(uniform_buffers,unique_resources, unique_bindings);
            Get_Binding(storage_buffers,unique_resources, unique_bindings);
        }

        vk::ShaderStageFlags Get_Vk_Stage()
        {
            switch (stage)
            {
            case S_Vert:
                return vk::ShaderStageFlagBits::eVertex;
            case S_Frag:
                return vk::ShaderStageFlagBits::eFragment;
            case S_Tess_Control:
                return vk::ShaderStageFlagBits::eTessellationControl;
            case S_Tess_Eval:
                return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case S_Geom:
                return vk::ShaderStageFlagBits::eGeometry;
            case S_Comp:
                return vk::ShaderStageFlagBits::eCompute;
            case S_Unknown:
                assert(false&& "not valid stage");
                break;
            }
        }

        std::vector<Shader_Resource> uniform_buffers;
        std::vector<Shader_Resource> storage_buffers;
        std::vector<Shader_Resource> sampled_images;
        std::vector<Shader_Resource> storage_images;
        Shader_Stage stage;
        std::string shader_name;

    private:
        static std::string Get_Buffer_Resource_Name(spirv_cross::CompilerGLSL& glsl,
                                                 const spirv_cross::Resource& resource,
                                                 bool slang_resource_names)
        {
            if (slang_resource_names)
            {
                std::string variable_name = glsl.get_name(resource.id);
                if (!variable_name.empty())
                {
                    return variable_name;
                }
            }
            return resource.name;
        }
    };

    class Shader
    {
    public:
        Shader(vk::Device logical_device, const std::string& path, Shader_Stage stage)
        {
            assert(std::filesystem::exists(path) && "Path does not exist");
            this->stage = stage;
            Handle_Path_Received(path);
            assert(std::filesystem::path(this->spirv_path).extension() == ".spv" && "spv must be .spv filetypes");
            this->logical_device = logical_device;
            std::vector<uint32_t> byte_code = Get_Byte_Code(spirv_path);
            s_parser = std::make_unique<Shader_Parser>(byte_code, Is_Slang_Shader(), this->path);
            s_module = std::make_unique<Shader_Module>(logical_device, byte_code);
            shader_file_info = std::make_unique<systems::File_Info>(this->path);
        }

        bool Reload()
        {
            shader_file_info->Check_Last_Time_Write();
            if (!shader_file_info->modified)
            {
                return false;
            }
            std::string code = systems::Os::Read_File(path);
            std::vector<uint32_t> byte_code;
            if (std::filesystem::path(path).extension() == ".slang")
            {
                std::string entry_point = Get_Slang_Entry_Point(stage);
                byte_code = Compile_Slang_Into_Spirv(spirv_path, code, entry_point, stage);
                if (byte_code.empty())
                {
                    systems::Logger::Get_Instance()->Log_Message("Byte code compilation is empty in: ("+path+")");
                    byte_code = Get_Byte_Code(spirv_path);
                }
            }
            else if (std::filesystem::path(path).extension() == ".frag"
                || std::filesystem::path(path).extension() == ".vert"
                || std::filesystem::path(path).extension() == ".tesc"
                || std::filesystem::path(path).extension() == ".tese"
                || std::filesystem::path(path).extension() == ".geom"
                || std::filesystem::path(path).extension() == ".comp")
            {
                
                Compile_Glsl_Into_Spirv(spirv_path, path);
                byte_code = Get_Byte_Code(spirv_path);
            }else
            {
                assert(false && "invalid shader file extension");
            }
            
            s_parser.reset();
            s_module.reset();
            s_parser = std::make_unique<Shader_Parser>(byte_code, Is_Slang_Shader(), path);
            s_module = std::make_unique<Shader_Module>(logical_device, byte_code);
            systems::Logger::Get_Instance()->Log_Message("Shader compiled: "+ path);
            systems::Logger::Get_Instance()->Log_Message("SPIRV: "+ spirv_path);
            shader_file_info->Invalidate();
        	return true;
        }



        void Handle_Path_Received(const std::string& path)
        {
            std::filesystem::path file_path(path);
            bool glsl = false;
            if (file_path.extension() == ".spv")
            {
                std::string shader_path= "";
            
                shader_path = Convert_Spirv_To_Shader_Path(file_path, stage);
                assert((std::filesystem::exists(shader_path) && shader_path != file_path) && "invalid shader path");
                this->path = shader_path;
                this->spirv_path = path;
            }
            else if (file_path.extension() == ".slang" ||
                file_path.extension() == ".frag" ||
                file_path.extension() ==".vert" ||
                file_path.extension() ==".tesc" ||
                file_path.extension() ==".tese" ||
                file_path.extension() ==".geom" ||
                file_path.extension() == ".comp")
            {

                std::string spirv_path ="";
                spirv_path = Convert_Shader_Path_To_Spirv(file_path, stage);

                if (std::filesystem::exists(file_path) && !std::filesystem::exists(spirv_path))
                {
                    if (file_path.extension() == ".slang")
                    {
                        std::string entry_point = Get_Slang_Entry_Point(stage);
                        std::string code = systems::Os::Read_File(file_path.string());
                        Compile_Slang_Into_Spirv(spirv_path, code, entry_point, stage);
                    }else
                    {
                        Compile_Glsl_Into_Spirv(spirv_path, path);
                    }
                }
                assert((std::filesystem::exists(spirv_path) && (spirv_path != file_path) && "invalid shader path"));
                this->spirv_path = spirv_path;
                this->path = path;
            }
            
        }
        
        std::unique_ptr<Shader_Parser> s_parser;
        std::unique_ptr<Shader_Module> s_module;
        vk::Device logical_device;
        Shader_Stage stage;
        //spirv path
        std::string path;
        std::string spirv_path;
        std::unique_ptr<systems::File_Info> shader_file_info;

    private:
        bool Is_Slang_Shader() const
        {
            return std::filesystem::path(path).extension() == ".slang";
        }
        
    };
}

#endif //SHADER_PARSER_HPP
