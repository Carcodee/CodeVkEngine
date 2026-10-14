//

// Created by carlo on 2025-01-01.
//

#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP

namespace rendering
{
struct animator_info
{
	glm::uvec2 sprite_size_px = glm::uvec2(0);
	int        rows         = -1;
	int        cols         = -1;
	int        current_frame = -1;
	int        frame_count   = -1;
	float      interp_val    = -1.0f;
	bool       is_atlas      = false;
};
struct animator_2d
{
	std::vector<engine::image_view *>    images_frames;
	std::vector<std::vector<glm::vec4>> images_frames_cpu;
	rendering::animator_info                        animation_info;
	int                                 current_frame_spacing = 0;
	int                                 frame_spacing        = 0;
	bool                                stop                = false;

	void load_frames(std::string &file_path, int frame_spacing)
	{
		std::filesystem::path path(file_path);
		int                   i = 0;
		std::vector<engine::image_shipper*> shippers;
		images_frames.clear();
		images_frames_cpu.clear();
		this->frame_spacing        = std::max(1, frame_spacing);
		this->current_frame_spacing = 0;
		
		shippers.reserve(1000);
		for (auto &file : std::filesystem::directory_iterator(path))
		{
			if (!is_regular_file(file))
			{
				continue;
			}
			// if (path.extension() != "png" || )
			engine::image_shipper *image_shipper = engine::resources_manager::get_instance()->get_shipper(
			    "Frame_" + std::to_string(i), file.path().string(), 1, 1, engine::g_shipper_format, engine::graphics_read);
			shippers.push_back(image_shipper);
			images_frames.push_back(image_shipper->image_view_handle.get());
			i++;
		}
		this->animation_info.frame_count   = static_cast<int>(images_frames.size());
		this->animation_info.current_frame = 0;
		this->animation_info.interp_val    = 0.0f;
	}
	void load_frames_cpu(std::string &folder_path, bool flip_y)
	{
		std::filesystem::path path(folder_path);
		std::vector<std::filesystem::path> frame_paths;
		images_frames_cpu.clear();
		current_frame_spacing = 0;
		animation_info        = {};

		std::error_code directory_error;
		for (auto &file : std::filesystem::directory_iterator(path, directory_error))
		{
			if (directory_error)
			{
				break;
			}
			if (file.is_regular_file())
			{
				frame_paths.push_back(file.path());
			}
		}
		std::sort(frame_paths.begin(), frame_paths.end());

		for (const auto &frame_path : frame_paths)
		{
			engine::image_shipper image_shipper;
			if (!image_shipper.set_data_from_path(frame_path.string()))
			{
				continue;
			}

			const glm::uvec2 frame_size(
			    static_cast<uint32_t>(image_shipper.image_size.x),
			    static_cast<uint32_t>(image_shipper.image_size.y));
			if (!images_frames_cpu.empty() && frame_size != animation_info.sprite_size_px)
			{
				systems::logger::get_instance()->log(
				    "Skipping animation frame with a different size: " + frame_path.string(),
				    systems::log_level::l_warn);
				image_shipper.clear();
				continue;
			}
			if (images_frames_cpu.empty())
			{
				animation_info.sprite_size_px = frame_size;
			}
			image_shipper.build_cpu_image(".", 0, flip_y);
			images_frames_cpu.emplace_back(image_shipper.ship_image_cpu());
		}
		animation_info.frame_count   = static_cast<int>(images_frames_cpu.size());
		animation_info.current_frame = 0;
		animation_info.interp_val    = 0.0f;
	}
	
	void clear_cpu()
	{
		images_frames_cpu.clear();
		animation_info        = {};
		current_frame_spacing = 0;
		stop                = true;
	}

	void load_atlas(std::string path, int frame_spacing, rendering::animator_info animation_info)
	{
		if (!std::filesystem::is_regular_file(path))
		{
			systems::logger::get_instance()->log("Invalid file type when loading atlas");
			return;
		}
		this->frame_spacing = frame_spacing;
		this->animation_info = animation_info;

		engine::image_view *image_view_handle = engine::resources_manager::get_instance()->get_shipper(
		                                                                          "Atlas_", path, 1, 1, engine::g_shipper_format,
		                                                                          engine::graphics_read)
		                                   ->image_view_handle.get();
		images_frames.push_back(image_view_handle);
		this->animation_info.frame_count   = animation_info.cols * animation_info.rows;
		this->animation_info.current_frame = 0;
		this->animation_info.interp_val    = 0.0f;
	}

	
	std::vector<glm::vec4> *use_cpu_frame()
	{
		if (images_frames_cpu.empty())
		{
			return nullptr;
		}

		const int frame_count = static_cast<int>(images_frames_cpu.size());
		animation_info.frame_count = frame_count;
		animation_info.current_frame = std::max(0, animation_info.current_frame) % frame_count;
		const int index = animation_info.current_frame;
		if (stop)
		{
			return &images_frames_cpu.at(index);
		}
		const int safe_frame_spacing   = std::max(1, frame_spacing);
		this->animation_info.interp_val = static_cast<float>(current_frame_spacing) / static_cast<float>(safe_frame_spacing);
		current_frame_spacing          = (current_frame_spacing + 1) % safe_frame_spacing;
		if (current_frame_spacing == 0)
		{
			animation_info.current_frame = (animation_info.current_frame + 1) % frame_count;
		}
		return &images_frames_cpu.at(index);
	}
	engine::image_view *use_frame()
	{
		if (images_frames.empty())
		{
			return nullptr;
		}

		const int frame_count = static_cast<int>(images_frames.size());
		animation_info.frame_count = frame_count;
		animation_info.current_frame = std::max(0, animation_info.current_frame) % frame_count;
		const int index = animation_info.current_frame;
		if (stop)
		{
			return images_frames.at(index);
		}
		const int safe_frame_spacing   = std::max(1, frame_spacing);
		this->animation_info.interp_val = static_cast<float>(current_frame_spacing) / static_cast<float>(safe_frame_spacing);
		current_frame_spacing          = (current_frame_spacing + 1) % safe_frame_spacing;
		if (current_frame_spacing == 0)
		{
			animation_info.current_frame = (animation_info.current_frame + 1) % frame_count;
		}
		return images_frames.at(index);
	}
};
}        // namespace rendering
#endif        // ANIMATOR_HPP
