//
// Created by carlo on 2024-11-25.
//


#ifndef MATERIAL_HPP
#define MATERIAL_HPP

namespace Rendering
{
    enum TextureType
    {
        ALBEDO = 0,
        NORMAL = 1,
        EMISSION = 2,
        TRANSMISSION = 3,
        ROUGHNESS = 4,
        METALLIC = 5,
        METALLIC_ROUGHNESS = 6,
        AO = 7,
        HEIGHT = 8
        
    };
    class Material 
    {
    public:
        MaterialPackedData materialPackedData;

        std::map<TextureType, int> texturesOffsets{
            {ALBEDO, -1},
            {NORMAL, -1},
            {EMISSION, -1},
            {TRANSMISSION, -1},
            {ROUGHNESS, -1},
            {METALLIC, -1},
            {METALLIC_ROUGHNESS, -1},
            {AO, -1},
            {HEIGHT, -1},
        };
        std::map<TextureType, ENGINE::ImageView*> texturesRef{
            {ALBEDO, nullptr},
            {NORMAL, nullptr},
            {EMISSION, nullptr},
            {TRANSMISSION, nullptr},
            {ROUGHNESS, nullptr},
            {METALLIC, nullptr},
            {METALLIC_ROUGHNESS, nullptr},
            {AO, nullptr},
            {HEIGHT, nullptr},
        };
        std::map<TextureType, std::string> texturesStrings{
            {ALBEDO,"Albedo"},
            {NORMAL,"Normal"},
            {EMISSION,"Emission"},
            {TRANSMISSION,"Transmission"},
            {ROUGHNESS,"Roughness"},
            {METALLIC,"Metallic"},
            {METALLIC_ROUGHNESS,"Metallic_roughness"},
            {AO,"Ao"},
            {HEIGHT,"Height"},
        };
        std::vector<ENGINE::ImageView*> imageViewsVec;

        void SetTexture(TextureType type, int offset)
        {
            assert(texturesOffsets.contains(type) && "Texture type dont exist");
            texturesOffsets.at(type) = offset;
            switch (type) {
            case ALBEDO:
                materialPackedData.albedoOffset = offset;
                break;
            case NORMAL:
                materialPackedData.normalOffset = offset;
                break;
            case EMISSION:
                materialPackedData.albedoOffset = offset;
                break;
            case TRANSMISSION:
                materialPackedData.transOffset = offset;
                break;
            case ROUGHNESS:
                materialPackedData.roughnessOffset = offset;
                break;
            case METALLIC:
                materialPackedData.metallicOffset = offset;
                break;
            case METALLIC_ROUGHNESS:
                materialPackedData.metRoughnessOffset = offset;
                break;
            }
        }
        void SetTexture(TextureType type, ENGINE::ImageView* imgView)
        {
            assert(texturesOffsets.contains(type) && "Texture type dont exist");
            texturesRef.at(type) = imgView;
        }
        std::vector<ENGINE::ImageView*>& ConvertTexturesToVec()
        {
            imageViewsVec.clear();
            imageViewsVec.reserve(texturesRef.size());
            for (auto& imageView : texturesRef)
            {
                if (imageView.second == nullptr)
                {
                    imageViewsVec.emplace_back(
                        ENGINE::ResourcesManager::GetInstance()->GetShipperFromName("default_tex")->imageView.get());
                }else
                {
                    imageViewsVec.emplace_back(imageView.second);
                }
            }
            return imageViewsVec;
        }
 
        
        
    };
}

#endif //MATERIAL_HPP
