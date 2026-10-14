//
// Created by carlo on 2024-11-25.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

namespace rendering
{
enum texture_type
{
	albedo             = 0,
	normal             = 1,
	emission           = 2,
	transmission       = 3,
	roughness          = 4,
	metallic           = 5,
	metallic_roughness = 6,
	ao                 = 7,
	height             = 8

};
class material
{
  public:
	rendering::material_packed_data material_packed_data;

	std::map<texture_type, int> textures_offsets{
	    {albedo, -1},
	    {normal, -1},
	    {emission, -1},
	    {transmission, -1},
	    {roughness, -1},
	    {metallic, -1},
	    {metallic_roughness, -1},
	    {ao, -1},
	    {height, -1},
	};
	std::map<texture_type, engine::image_view *> textures_ref{
	    {albedo, nullptr},
	    {normal, nullptr},
	    {emission, nullptr},
	    {transmission, nullptr},
	    {roughness, nullptr},
	    {metallic, nullptr},
	    {metallic_roughness, nullptr},
	    {ao, nullptr},
	    {height, nullptr},
	};
	std::map<texture_type, std::string> textures_strings{
	    {albedo, "Albedo"},
	    {normal, "Normal"},
	    {emission, "Emission"},
	    {transmission, "Transmission"},
	    {roughness, "Roughness"},
	    {metallic, "Metallic"},
	    {metallic_roughness, "Metallic_roughness"},
	    {ao, "Ao"},
	    {height, "Height"},
	};
	std::vector<engine::image_view *> image_views_vec;

	void set_texture(texture_type type, int offset)
	{
		assert(textures_offsets.contains(type) && "Texture type dont exist");
		textures_offsets.at(type) = offset;
		switch (type)
		{
			case albedo:
				material_packed_data.albedo_offset = offset;
				break;
			case normal:
				material_packed_data.normal_offset = offset;
				break;
			case emission:
				material_packed_data.albedo_offset = offset;
				break;
			case transmission:
				material_packed_data.trans_offset = offset;
				break;
			case roughness:
				material_packed_data.roughness_offset = offset;
				break;
			case metallic:
				material_packed_data.metallic_offset = offset;
				break;
			case metallic_roughness:
				material_packed_data.met_roughness_offset = offset;
				break;
		}
	}
	void set_texture(texture_type type, engine::image_view *img_view)
	{
		assert(textures_offsets.contains(type) && "Texture type dont exist");
		textures_ref.at(type) = img_view;
	}
	std::vector<engine::image_view *> &convert_textures_to_vec()
	{
		image_views_vec.clear();
		image_views_vec.reserve(textures_ref.size());
		for (auto &image_view_handle : textures_ref)
		{
			if (image_view_handle.second == nullptr)
			{
				image_views_vec.emplace_back(
				    engine::resources_manager::get_instance()->get_shipper_from_name("default_tex")->image_view_handle.get());
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
