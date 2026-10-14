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
    enum shader_stage
    {
        s_vert,
        s_frag,
        s_tess_control,
        s_tess_eval,
        s_geom,
        s_comp,
        s_unknown
    };

    enum shader_compiler
    {
        c_glsl,
        c_slang
    };
    static bool is_glsl_valid_extension(const std::string extension)
    {
        if (extension == ".comp" || extension == ".vert" || extension == ".frag" || extension == ".tesc" || extension ==
            ".tese" || extension == ".geom" )
        {
            return true;
        }
        return false;
    }
    
    static std::string convert_shader_path_to_spirv(const std::filesystem::path& file_path, shader_stage stage)
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
            systems::logger::get_instance()->log_message("Shader is already spv no conversion needed");
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
                case s_vert:
                    extension += "_VS.spv";
                    break;
                case s_frag:
                    extension += "_FS.spv";
                    break;
                case s_tess_control:
                    assert(false && "Unsupported slang stage");
                    extension += "_TESC.spv";
                    break;
                case s_tess_eval:
                    assert(false && "Unsupported slang stage");
                    extension += "_TESE.spv";
                    break;
                case s_geom:
                    extension += "_GEOM.spv";
                    break;
                case s_comp:
                    extension += "_CS.spv";
                    break;
                case s_unknown:
                    assert(false && "invalid stage");
                    break;
                }
                spirv_path /= file_name_no_ext + extension;
            }
            else if (dirs_parts.string() == filename && is_glsl_valid_extension(dirs_parts.extension().string()))
            {
                int first_dot_pos = dirs_parts.string().find_first_of('.');
                std::string file_name_no_ext = dirs_parts.string().substr(0, first_dot_pos);
                std::string extension = dirs_parts.extension().string();
                switch (stage)
                {
                case s_vert:
                    extension += ".spv";
                    break;
                case s_frag:
                    extension += ".spv";
                    break;
                case s_tess_control:
                    extension += ".spv";
                    break;
                case s_tess_eval:
                    extension += ".spv";
                    break;
                case s_geom:
                    extension += ".spv";
                    break;
                case s_comp:
                    extension += ".spv";
                    break;
                case s_unknown:
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
        
    static std::string convert_spirv_to_shader_path(const std::filesystem::path& file_path, shader_stage stage)
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
                        case s_vert:
                            extension = ".vert";
                            break;
                        case s_frag:
                            extension = ".frag";
                            break;
                        case s_tess_control:
                            extension = ".tesc";
                            break;
                        case s_tess_eval:
                            extension = ".tese";
                            break;
                        case s_geom:
                            extension = ".geom";
                            break;
                        case s_comp:
                            extension = ".comp";
                            break;
                        case s_unknown:
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

    static std::vector<uint32_t> compile_slang_into_spirv(const std::string& out_file, const std::string& code,
                                                  const std::string& entry_point, shader_stage stage)
    {
        SlangStage slang_stage = SLANG_STAGE_NONE;
        switch (stage)
        {
        case s_vert:
            slang_stage = SLANG_STAGE_VERTEX;
            break;
        case s_frag:
            slang_stage = SLANG_STAGE_FRAGMENT;
            break;
        case s_tess_control:
            assert(false&& "unsuported stage");
        case s_tess_eval:
            assert(false&& "unsuported stage");
        case s_geom:
            slang_stage = SLANG_STAGE_GEOMETRY;
        case s_comp:
            slang_stage = SLANG_STAGE_COMPUTE;
            break;
        case s_unknown:
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
            systems::os::write_file(out_file,spirv_code_data , spirv_size);
            const uint32_t* spirv_words = reinterpret_cast<const uint32_t*>(spirv_code);
            size_t word_count = spirv_size / sizeof(uint32_t);;
            return std::vector<uint32_t>(spirv_words, spirv_words + word_count);
        }

        spDestroyCompileRequest(request);
        spDestroySession(session);
        
        assert(false && "error when compiling into spirv at runtime");
        return std::vector<uint32_t>();
    }

    static std::string get_slang_entry_point(shader_stage stage)
    {
        std::string entry_point = "";
        switch (stage)
        {
        case s_vert:
            entry_point = "main_vs";
            break;
        case s_frag:
            entry_point = "main_fs";
            break;
        case s_comp:
            entry_point = "main_cs";
            break;
        case s_tess_control:
            assert(false && "uknown stage");
            break;
        case s_tess_eval:
            assert(false && "uknown stage");
            break;
        case s_geom:
            entry_point = "main_gs";
            break;
        case s_unknown:
            assert(false && "uknown stage");
            break;
        }
        return entry_point;
    }

    static void compile_glsl_into_spirv(const std::string& out_file, const std::string& path)
    {
        std::string temp_file_path = systems::os::get_instance()->get_shaders_path() + "\\tempFile.bat";
        systems::os::create_file_at(temp_file_path);

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
        systems::os::write_file(temp_file_path, command.c_str(), command.size());

        int result = std::system(temp_file_path.c_str());
        if (result != 0)
        {
            assert(false && "invalid bat file");
        }

        std::remove(temp_file_path.c_str());
    }
    class shader_parser
    {

    public:

        shader_parser(std::vector<uint32_t>& byte_code, bool slang_resource_names = false, std::string shader_name = "")
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
                    stage = s_vert;
                    break;
                case spv::ExecutionModelFragment:
                    stage = s_frag;
                    break;
                case spv::ExecutionModelTessellationControl:
                    stage = s_tess_control;
                    break;
                case spv::ExecutionModelTessellationEvaluation:
                    stage = s_tess_eval;
                    break;
                case spv::ExecutionModelGeometry:
                    stage = s_geom;
                    break;
                case spv::ExecutionModelGLCompute:
                    stage = s_comp;
                    break;
                default:
                    std::cout << "Unknown shader stage\n";
                    stage = s_unknown;
                    break;
                }
            }


            for (auto& resource : resources.uniform_buffers)
            {
                std::string name = get_buffer_resource_name(glsl, resource, slang_resource_names);
                uint32_t binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                bool array = false;
                spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
                if (!type.array.empty())
                {
                    array = true;
                }
                uniform_buffers.emplace_back(shader_resource{name, binding, set, array, vk::DescriptorType::eUniformBuffer});


            }
             for (auto& resource : resources.storage_buffers)
            {
                std::string name = get_buffer_resource_name(glsl, resource, slang_resource_names);
                uint32_t binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                bool array = false;
                spirv_cross::SPIRType type = glsl.get_type(resource.type_id);
                if (!type.array.empty())
                {
                    array = true;
                }
                storage_buffers.emplace_back(shader_resource{name, binding, set, array, vk::DescriptorType::eStorageBuffer});
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
                
                sampled_images.emplace_back(shader_resource{name, binding, set, array, vk::DescriptorType::eCombinedImageSampler});
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
                
                storage_images.emplace_back(shader_resource{name, binding, set, array, vk::DescriptorType::eStorageImage});
            }
            
        }


        void get_binding(std::vector<shader_resource>& resources, descriptor_layout_builder& builder)
        {
            for (auto& resource : resources)
            {
                if (builder.unique_bindings.contains(resource.binding))
                {
                    continue;
                }
                builder.add_binding(resource.binding, resource.type);
            }
        }

        void get_binding(std::vector<shader_resource>& resources, std::vector<shader_resource>& unique_resources, std::set<uint32_t> unique_bindings)
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
        void get_layout(descriptor_layout_builder& builder)
        {
            std::set<uint32_t> repeated_bindings;
            get_binding(sampled_images, builder);
            get_binding(storage_images, builder);
            get_binding(uniform_buffers, builder);
            get_binding(storage_buffers, builder);
        }

        void get_layout(std::vector<shader_resource>& unique_resources)
        {
            std::set<uint32_t> unique_bindings;
            get_binding(sampled_images, unique_resources, unique_bindings);
            get_binding(storage_images, unique_resources, unique_bindings);
            get_binding(uniform_buffers,unique_resources, unique_bindings);
            get_binding(storage_buffers,unique_resources, unique_bindings);
        }

        vk::ShaderStageFlags get_vk_stage()
        {
            switch (stage)
            {
            case s_vert:
                return vk::ShaderStageFlagBits::eVertex;
            case s_frag:
                return vk::ShaderStageFlagBits::eFragment;
            case s_tess_control:
                return vk::ShaderStageFlagBits::eTessellationControl;
            case s_tess_eval:
                return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case s_geom:
                return vk::ShaderStageFlagBits::eGeometry;
            case s_comp:
                return vk::ShaderStageFlagBits::eCompute;
            case s_unknown:
                assert(false&& "not valid stage");
                break;
            }
        }

        std::vector<shader_resource> uniform_buffers;
        std::vector<shader_resource> storage_buffers;
        std::vector<shader_resource> sampled_images;
        std::vector<shader_resource> storage_images;
        shader_stage stage;
        std::string shader_name;

    private:
        static std::string get_buffer_resource_name(spirv_cross::CompilerGLSL& glsl,
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

    class shader
    {
    public:
        shader(vk::Device logical_device, const std::string& path, shader_stage stage)
        {
            assert(std::filesystem::exists(path) && "Path does not exist");
            this->stage = stage;
            handle_path_received(path);
            assert(std::filesystem::path(this->spirv_path).extension() == ".spv" && "spv must be .spv filetypes");
            this->logical_device = logical_device;
            std::vector<uint32_t> byte_code = get_byte_code(spirv_path);
            s_parser = std::make_unique<shader_parser>(byte_code, is_slang_shader(), this->path);
            s_module = std::make_unique<shader_module>(logical_device, byte_code);
            shader_file_info = std::make_unique<systems::file_info>(this->path);
        }

        bool reload()
        {
            shader_file_info->check_last_time_write();
            if (!shader_file_info->modified)
            {
                return false;
            }
            std::string code = systems::os::read_file(path);
            std::vector<uint32_t> byte_code;
            if (std::filesystem::path(path).extension() == ".slang")
            {
                std::string entry_point = get_slang_entry_point(stage);
                byte_code = compile_slang_into_spirv(spirv_path, code, entry_point, stage);
                if (byte_code.empty())
                {
                    systems::logger::get_instance()->log_message("Byte code compilation is empty in: ("+path+")");
                    byte_code = get_byte_code(spirv_path);
                }
            }
            else if (std::filesystem::path(path).extension() == ".frag"
                || std::filesystem::path(path).extension() == ".vert"
                || std::filesystem::path(path).extension() == ".tesc"
                || std::filesystem::path(path).extension() == ".tese"
                || std::filesystem::path(path).extension() == ".geom"
                || std::filesystem::path(path).extension() == ".comp")
            {
                
                compile_glsl_into_spirv(spirv_path, path);
                byte_code = get_byte_code(spirv_path);
            }else
            {
                assert(false && "invalid shader file extension");
            }
            
            s_parser.reset();
            s_module.reset();
            s_parser = std::make_unique<shader_parser>(byte_code, is_slang_shader(), path);
            s_module = std::make_unique<shader_module>(logical_device, byte_code);
            systems::logger::get_instance()->log_message("Shader compiled: "+ path);
            systems::logger::get_instance()->log_message("SPIRV: "+ spirv_path);
            shader_file_info->invalidate();
        	return true;
        }



        void handle_path_received(const std::string& path)
        {
            std::filesystem::path file_path(path);
            bool glsl = false;
            if (file_path.extension() == ".spv")
            {
                std::string shader_path= "";
            
                shader_path = convert_spirv_to_shader_path(file_path, stage);
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
                spirv_path = convert_shader_path_to_spirv(file_path, stage);

                if (std::filesystem::exists(file_path) && !std::filesystem::exists(spirv_path))
                {
                    if (file_path.extension() == ".slang")
                    {
                        std::string entry_point = get_slang_entry_point(stage);
                        std::string code = systems::os::read_file(file_path.string());
                        compile_slang_into_spirv(spirv_path, code, entry_point, stage);
                    }else
                    {
                        compile_glsl_into_spirv(spirv_path, path);
                    }
                }
                assert((std::filesystem::exists(spirv_path) && (spirv_path != file_path) && "invalid shader path"));
                this->spirv_path = spirv_path;
                this->path = path;
            }
            
        }
        
        std::unique_ptr<shader_parser> s_parser;
        std::unique_ptr<shader_module> s_module;
        vk::Device logical_device;
        shader_stage stage;
        //spirv path
        std::string path;
        std::string spirv_path;
        std::unique_ptr<systems::file_info> shader_file_info;

    private:
        bool is_slang_shader() const
        {
            return std::filesystem::path(path).extension() == ".slang";
        }
        
    };
}

#endif //SHADER_PARSER_HPP
