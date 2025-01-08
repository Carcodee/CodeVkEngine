//
// Created by carlo on 2024-09-24.
//

#ifndef SHADERMODULE_HPP
#define SHADERMODULE_HPP

namespace ENGINE
{
    
    class ShaderModule 
    {
        
    public:
        ShaderModule(vk::Device device, std::vector<uint32_t>& byteCode)
        {
            
            auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(sizeof(uint32_t) * byteCode.size())
            .setPCode(byteCode.data());
            this->shaderModuleHandle = device.createShaderModuleUnique(shaderModuleCreateInfo);
        }
        vk::UniqueShaderModule shaderModuleHandle;
    };
    
}

#endif //SHADERMODULE_HPP
