//
// Created by carlo on 2024-09-24.
//

#ifndef SHADER_MODULE_HPP
#define SHADER_MODULE_HPP

namespace engine
{
    
    class Shader_Module
    {
        
    public:
        Shader_Module(vk::Device device, std::vector<uint32_t>& byte_code)
        {
            
            auto shader_module_create_info = vk::ShaderModuleCreateInfo()
            .setCodeSize(sizeof(uint32_t) * byte_code.size())
            .setPCode(byte_code.data());
            this->shader_module_handle = device.createShaderModuleUnique(shader_module_create_info);
        }
        vk::UniqueShaderModule shader_module_handle;
    };
    
}

#endif //SHADER_MODULE_HPP
