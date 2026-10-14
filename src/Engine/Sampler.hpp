//
// Created by carlo on 2024-09-23.
//

#ifndef SAMPLER_HPP
#define SAMPLER_HPP

namespace engine
{
    class Sampler
    {
    public:

        Sampler(vk::Device logical_device, vk::SamplerAddressMode address_mode, vk::Filter min_mag_filter_type,
                vk::SamplerMipmapMode mip_filter_type, bool use_comparison = false,
                vk::BorderColor border_color = vk::BorderColor())
        {
            this->address_mode = address_mode;
            this->min_mag_filter_type = min_mag_filter_type;
            this->mip_filter_type = mip_filter_type;
            this->use_comparison = use_comparison;
            this->border_color = border_color;
            auto sampler_create_info = vk::SamplerCreateInfo()
                                     .setAddressModeU(address_mode)
                                     .setAddressModeV(address_mode)
                                     .setAddressModeW(address_mode)
                                     .setAnisotropyEnable(false)
                                     .setCompareEnable(use_comparison)
                                     .setCompareOp(use_comparison ? vk::CompareOp::eLessOrEqual : vk::CompareOp::eAlways)
                                     .setMagFilter(min_mag_filter_type)
                                     .setMinFilter(min_mag_filter_type)
                                     .setMaxLod(1e7f)
                                     .setMinLod(0.0f)
                                     .setMipmapMode(mip_filter_type)
                                     .setUnnormalizedCoordinates(false)
                                     .setBorderColor(border_color);

            sampler_handle = logical_device.createSamplerUnique(sampler_create_info);
        }
        vk::SamplerAddressMode address_mode;
        vk::Filter min_mag_filter_type;
        vk::SamplerMipmapMode mip_filter_type;
        bool use_comparison;
        vk::BorderColor border_color;
        vk::UniqueSampler sampler_handle;
    };
    class Sampler_Pool
    {
    public:
        engine::Sampler* Add_Sampler(vk::Device logical_device, vk::SamplerAddressMode address_mode, vk::Filter min_mag_filter_type,
                vk::SamplerMipmapMode mip_filter_type, bool use_comparison = false,
                vk::BorderColor border_color = vk::BorderColor())
        {
            for (auto& sampler : sampler_pool)
            {
                if (sampler->address_mode == address_mode && sampler->min_mag_filter_type == min_mag_filter_type &&sampler->
                mip_filter_type == mip_filter_type && sampler->use_comparison == use_comparison && sampler->border_color
                == border_color)
                {
                      
                    return sampler.get();
                }
            }
            std::unique_ptr<engine::Sampler> sampler = std::make_unique<engine::Sampler>(logical_device ,address_mode, min_mag_filter_type, mip_filter_type, use_comparison, border_color);
            sampler_pool.emplace_back(std::move(sampler));
            return sampler_pool.front().get();
        }
        engine::Sampler* Get_Sampler(vk::SamplerAddressMode address_mode, vk::Filter min_mag_filter_type,
                vk::SamplerMipmapMode mip_filter_type, bool use_comparison = false,
                vk::BorderColor border_color = vk::BorderColor())
        {
             for (auto& sampler : sampler_pool)
            {
                if (sampler->address_mode == address_mode && sampler->min_mag_filter_type == min_mag_filter_type &&sampler->
                mip_filter_type == mip_filter_type && sampler->use_comparison == use_comparison && sampler->border_color
                == border_color)
                {
                    return sampler.get();
                }
            }
            assert(false && "Sampler does not exist");
            return nullptr;
        }

        
        std::vector<std::unique_ptr<engine::Sampler>> sampler_pool;
        
    };
}

#endif //SAMPLER_HPP
