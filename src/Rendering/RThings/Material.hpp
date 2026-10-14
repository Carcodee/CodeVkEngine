//
// Created by carlo on 2024-11-25.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

namespace rendering
{
enum Texture_Type
{
	Albedo             = 0,
	Normal             = 1,
	Emission           = 2,
	Transmission       = 3,
	Roughness          = 4,
	Metallic           = 5,
	Metallic_Roughness = 6,
	Ao                 = 7,
	Height             = 8

};
class Material
{
  public:
	rendering::Material_Packed_Data material_packed_data;

	std::map<Texture_Type, int> textures_offsets{
	    {Albedo, -1},
	    {Normal, -1},
	    {Emission, -1},
	    {Transmission, -1},
	    {Roughness, -1},
	    {Metallic, -1},
	    {Metallic_Roughness, -1},
	    {Ao, -1},
	    {Height, -1},
	};
	std::map<Texture_Type, engine::Image_View *> textures_ref{
	    {Albedo, nullptr},
	    {Normal, nullptr},
	    {Emission, nullptr},
	    {Transmission, nullptr},
	    {Roughness, nullptr},
	    {Metallic, nullptr},
	    {Metallic_Roughness, nullptr},
	    {Ao, nullptr},
	    {Height, nullptr},
	};
	std::map<Texture_Type, std::string> textures_strings{
	    {Albedo, "Albedo"},
	    {Normal, "Normal"},
	    {Emission, "Emission"},
	    {Transmission, "Transmission"},
	    {Roughness, "Roughness"},
	    {Metallic, "Metallic"},
	    {Metallic_Roughness, "Metallic_roughness"},
	    {Ao, "Ao"},
	    {Height, "Height"},
	};
	std::vector<engine::Image_View *> image_views_vec;

	void Set_Texture(Texture_Type type, int offset)
	{
		assert(textures_offsets.contains(type) && "Texture type dont exist");
		textures_offsets.at(type) = offset;
		switch (type)
		{
			case Albedo:
				material_packed_data.albedo_offset = offset;
				break;
			case Normal:
				material_packed_data.normal_offset = offset;
				break;
			case Emission:
				material_packed_data.albedo_offset = offset;
				break;
			case Transmission:
				material_packed_data.trans_offset = offset;
				break;
			case Roughness:
				material_packed_data.roughness_offset = offset;
				break;
			case Metallic:
				material_packed_data.metallic_offset = offset;
				break;
			case Metallic_Roughness:
				material_packed_data.met_roughness_offset = offset;
				break;
		}
	}
	void Set_Texture(Texture_Type type, engine::Image_View *img_view)
	{
		assert(textures_offsets.contains(type) && "Texture type dont exist");
		textures_ref.at(type) = img_view;
	}
	std::vector<engine::Image_View *> &Convert_Textures_To_Vec()
	{
		image_views_vec.clear();
		image_views_vec.reserve(textures_ref.size());
		for (auto &image_view_handle : textures_ref)
		{
			if (image_view_handle.second == nullptr)
			{
				image_views_vec.emplace_back(
				    engine::Resources_Manager::Get_Instance()->Get_Shipper_From_Name("default_tex")->image_view_handle.get());
			}
			else
			{
				image_views_vec.emplace_back(image_view_handle.second);
			}
		}
		return image_views_vec;
	}
};
}        // namespace rendering

#endif        // MATERIAL_HPP
