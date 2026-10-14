//

// Created by carlo on 2024-11-22.
//

#ifndef OS_HPP
#define OS_HPP

namespace systems
{

class os
{
  public:
	os()
	{
		working_dir          = std::filesystem::current_path();
		project_path         = get_project_path(working_dir);
		shaders_path         = project_path / "src" / "Shaders";
		assets_path          = project_path / "Resources" / "Assets";
		engine_resources_path = project_path / "Resources" / "Engine";

		glsl_shaders_template_path  = shaders_path / "glsl" / "templates";
		slang_shaders_template_path = shaders_path / "slang" / "templates";
	}

	static os *get_instance()
	{
		static os instance;
		return &instance;
	}

	std::filesystem::path get_project_path(std::filesystem::path &work_dir)
	{
		std::filesystem::path project_path      = work_dir;
		bool                  solution_path_find = false;
		while (!solution_path_find)
		{
			for (auto element : std::filesystem::directory_iterator(project_path))
			{
				if (element.path().filename() == ".gitignore")
				{
					solution_path_find = true;
					break;
				}
			}
			if (solution_path_find)
			{
				break;
			}

			project_path = project_path.parent_path();
			if (project_path == work_dir.root_path())
			{
				break;
			}
		}
		assert(solution_path_find && "The project dir was not find");

		return project_path;
	}
	bool is_path_absolute(std::string path)
	{
		if (!std::filesystem::exists(path))
		{
			return false;
		}
		return true;
	}
	std::string get_engine_resources_path()
	{
		return engine_resources_path.string();
	}

	std::string get_assets_path()
	{
		return assets_path.string();
	}
	std::string get_shaders_path()
	{
		return shaders_path.string();
	}

	std::string get_glsl_template_path()
	{
		return glsl_shaders_template_path.string();
	}

	std::string get_slang_template_path()
	{
		return slang_shaders_template_path.string();
	}
	static std::string get_extension(std::string name)
	{
		size_t point_pos = name.find_last_of(".");
		if (point_pos > name.size() || point_pos < 0)
		{
			return "";
		}
		std::string extension = name.substr(point_pos, name.back());
		return extension;
	}

	static std::string read_file(const std::string &path)
	{
		std::ifstream in_file(path.c_str(), std::ios::in | std::ios::binary);
		if (!in_file)
		{
			assert(false && "impossible to open the path");
		}
		std::ostringstream oss;

		oss << in_file.rdbuf();

		in_file.close();
		return oss.str();
	}

	static nlohmann::json get_json_from_file(const std::string &path)
	{
		std::ifstream in_file(path);
		if (!in_file)
		{
			assert(false && "impossible to open the path");
		}
		nlohmann::json json;

		in_file >> json;

		in_file.close();
		return json;
	}
	static void create_file_at(const std::string dst_file)
	{
		std::ofstream dst(dst_file, std::ios::binary);
		if (!dst)
		{
			logger::get_instance()->log("File: (" + dst_file + ") could not be created");
		}
	}

	static void write_file(const std::string &path, const char *text, size_t size)
	{
		std::ofstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			assert(false && "Impossible to write file");
			logger::get_instance()->log("File: (" + path + ") could not be opened");
			return;
		}
		file.write(text, size);
		file.close();

		logger::get_instance()->log("File: (" + path + ") was writed succesfully");
	}

	static void copy_file_into(const std::string &src_file, const std::string &dst_file)
	{
		std::ifstream src(src_file, std::ios::binary);
		if (!src)
		{
			assert(false && "Impossible to open src file");
		}
		std::ofstream dst(dst_file, std::ios::binary);
		if (!dst)
		{
			assert(false && "Impossible to create dst file");
		}

		dst << src.rdbuf();
		logger::get_instance()->log("File: (" + src_file + ") was copied to: " + dst_file);
		src.close();
		dst.close();
	}

	static void delete_existing_file(const std::string &path)
	{
		if (!std::filesystem::exists(path))
		{
			logger::get_instance()->log("File: (" + path + ") does not exist");
			return;
		}
		if (std::remove(path.c_str()) == 0)
		{
			logger::get_instance()->log("File: (" + path + ") was deleted succesfully");
		}
		else
		{
			logger::get_instance()->log("File: (" + path + ") was not deleted");
		}
	}
	static void append_data_to_file(const std::string &path, const std::string &data)
	{
		std::ofstream file(path, std::ios::app);
		if (file)
		{
			file << data;
			logger::get_instance()->log("File: (" + path + ") was appended successfully");
		}
		else
		{
			logger::get_instance()->log("File: (" + path + ") was not appended");
		}
	}

	std::filesystem::path        working_dir;
	std::filesystem::path        project_path;
	std::filesystem::path        engine_resources_path;
	std::filesystem::path        assets_path;
	std::filesystem::path        shaders_path;
	std::filesystem::path        glsl_shaders_template_path;
	std::filesystem::path        slang_shaders_template_path;
	std::queue<std::string_view> temp_files_to_destroy;
	static os                   *instance;
};

}        // namespace systems

#endif        // OS_HPP
