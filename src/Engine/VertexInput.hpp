//
// Created by carlo on 2024-09-26.
//

#ifndef VERTEXINPUT_HPP
#define VERTEXINPUT_HPP

namespace ENGINE
{
    class VertexInput
    {
    public:
        enum Attribs
        {
            INT,
            FLOAT,
            VEC2,
            VEC3,
            VEC4,
            U8VEC3,
            U8VEC4,
            COLOR_32
        };


        vk::Format GetFormatFromAttrib(Attribs attribs)
        {
            switch (attribs)
            {
            case Attribs::INT: return vk::Format::eR32Sint;
                break;
            case Attribs::FLOAT: return vk::Format::eR32Sfloat;
                break;
            case Attribs::VEC2: return vk::Format::eR32G32Sfloat;
                break;
            case Attribs::VEC3: return vk::Format::eR32G32B32Sfloat;
                break;
            case Attribs::VEC4: return vk::Format::eR32G32B32A32Sfloat;
                break;
            case Attribs::U8VEC3: return vk::Format::eR8G8B8Unorm;
                break;
            case Attribs::U8VEC4: return vk::Format::eR8G8B8A8Unorm;
                break;
            case Attribs::COLOR_32: return vk::Format::eR8G8B8A8Snorm;
                break;
            default:
                assert(false && "Invalid attrib type");
                break;
            }
            return vk::Format::eUndefined;
        }

        VertexInput* AddVertexInputBinding(uint32_t bufferBinding, uint32_t stride)
        {
            auto bindDesc = vk::VertexInputBindingDescription()
                            .setBinding(bufferBinding)
                            .setStride(stride)
                            .setInputRate(vk::VertexInputRate::eVertex);
            bindingDescription.push_back(bindDesc);
            return this;
        }

        VertexInput* AddVertexAttrib(Attribs attribs, uint32_t binding, uint32_t offset, uint32_t location)
        {
            auto vertexAttrib = vk::VertexInputAttributeDescription()
                                .setBinding(binding)
                                .setLocation(location)
                                .setFormat(GetFormatFromAttrib(attribs))
                                .setOffset(offset);
            inputDescription.push_back(vertexAttrib);
            return this;
        }

        std::vector<size_t> vertexOffsets;
        int attribSize = 0;
        std::vector<vk::VertexInputAttributeDescription> inputDescription;
        std::vector<vk::VertexInputBindingDescription> bindingDescription;
    };
}

#endif //VERTEXINPUT_HPP
