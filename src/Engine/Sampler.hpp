//
// Created by carlo on 2024-09-23.
//

#ifndef SAMPLER_HPP
#define SAMPLER_HPP

namespace ENGINE
{
    class Sampler
    {
    public:

        Sampler(vk::Device logicalDevice, vk::SamplerAddressMode addressMode, vk::Filter minMagFilterType,
                vk::SamplerMipmapMode mipFilterType, bool useComparison = false,
                vk::BorderColor borderColor = vk::BorderColor())
        {
            this->addressMode = addressMode;
            this->minMagFilterType = minMagFilterType;
            this->mipFilterType = mipFilterType;
            this->useComparison = useComparison;
            this->borderColor = borderColor;
            auto samplerCreateInfo = vk::SamplerCreateInfo()
                                     .setAddressModeU(addressMode)
                                     .setAddressModeV(addressMode)
                                     .setAddressModeW(addressMode)
                                     .setAnisotropyEnable(false)
                                     .setCompareEnable(useComparison)
                                     .setCompareOp(useComparison ? vk::CompareOp::eLessOrEqual : vk::CompareOp::eAlways)
                                     .setMagFilter(minMagFilterType)
                                     .setMinFilter(minMagFilterType)
                                     .setMaxLod(1e7f)
                                     .setMinLod(0.0f)
                                     .setMipmapMode(mipFilterType)
                                     .setUnnormalizedCoordinates(false)
                                     .setBorderColor(borderColor);

            samplerHandle = logicalDevice.createSamplerUnique(samplerCreateInfo);
        }
        vk::SamplerAddressMode addressMode;
        vk::Filter minMagFilterType;
        vk::SamplerMipmapMode mipFilterType;
        bool useComparison;
        vk::BorderColor borderColor;
        vk::UniqueSampler samplerHandle;
    };
    class SamplerPool
    {
    public:
        Sampler* AddSampler(vk::Device logicalDevice, vk::SamplerAddressMode addressMode, vk::Filter minMagFilterType,
                vk::SamplerMipmapMode mipFilterType, bool useComparison = false,
                vk::BorderColor borderColor = vk::BorderColor())
        {
            for (auto& sampler : samplerPool)
            {
                if (sampler->addressMode == addressMode && sampler->minMagFilterType == minMagFilterType &&sampler-> 
                mipFilterType == mipFilterType && sampler->useComparison == useComparison && sampler->borderColor
                == borderColor)
                {
                      
                    return sampler.get();
                }
            }
            std::unique_ptr<Sampler> sampler = std::make_unique<Sampler>(logicalDevice ,addressMode, minMagFilterType, mipFilterType, useComparison, borderColor);
            samplerPool.emplace_back(std::move(sampler));
            return samplerPool.front().get();
        }
        Sampler* GetSampler(vk::SamplerAddressMode addressMode, vk::Filter minMagFilterType,
                vk::SamplerMipmapMode mipFilterType, bool useComparison = false,
                vk::BorderColor borderColor = vk::BorderColor())
        {
             for (auto& sampler : samplerPool)
            {
                if (sampler->addressMode == addressMode && sampler->minMagFilterType == minMagFilterType &&sampler-> 
                mipFilterType == mipFilterType && sampler->useComparison == useComparison && sampler->borderColor
                == borderColor)
                {
                    return sampler.get();
                }
            }
            assert(false && "Sampler does not exist");
            return nullptr;
        }

        
        std::vector<std::unique_ptr<Sampler>> samplerPool;
        
    };
}

#endif //SAMPLER_HPP
