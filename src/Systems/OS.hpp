//

// Created by carlo on 2024-11-22.
//

#ifndef OS_HPP
#define OS_HPP

namespace systems
{

class Os
{
  public:
	Os()
	{
		working_dir          = std::filesystem::current_path();
		project_path         = Get_Project_Path(working_dir);
		shaders_path         = project_path / "src" / "Shaders";
		assets_path          = project_path / "Resources" / "Assets";
		engine_resources_path = project_path / "Resources" / "Engine";

		glsl_shaders_template_path  = shaders_path / "glsl" / "templates";
		slang_shaders_template_path = shaders_path / "slang" / "templates";
	}

	static Os *Get_Instance()
	{
		static Os instance;
		return &instance;
	}

	std::filesystem::path Get_Project_Path(std::filesystem::path &work_dir)
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
	bool Is_Path_Absolute(std::string path)
	{
		if (!std::filesystem::exists(path))
		{
			return false;
		}
		return true;
	}
	std::string Get_Engine_Resources_Path()
	{
		return engine_resources_path.string();
	}

	std::string Get_Assets_Path()
	{
		return assets_path.string();
	}
	std::string Get_Shaders_Path()
	{
		return shaders_path.string();
	}

	std::string Get_Glsl_Template_Path()
	{
		return glsl_shaders_template_path.string();
	}

	std::string Get_Slang_Template_Path()
	{
		return slang_shaders_template_path.string();
	}
	static std::string Get_Extension(std::string name)
	{
		size_t point_pos = name.find_last_of(".");
		if (point_pos > name.size() || point_pos < 0)
		{
			return "";
		}
		std::string extension = name.substr(point_pos, name.back());
		return extension;
	}

	static std::string Read_File(const std::string &path)
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

	static nlohmann::json Get_Json_From_File(const std::string &path)
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
	static void Create_File_At(const std::string dst_file)
	{
		std::ofstream dst(dst_file, std::ios::binary);
		if (!dst)
		{
			Logger::Get_Instance()->Log("File: (" + dst_file + ") could not be created");
		}
	}

	static void Write_File(const std::string &path, const char *text, size_t size)
	{
		std::ofstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			assert(false && "Impossible to write file");
			Logger::Get_Instance()->Log("File: (" + path + ") could not be opened");
			return;
		}
		file.write(text, size);
		file.close();

		Logger::Get_Instance()->Log("File: (" + path + ") was writed succesfully");
	}

	static void Copy_File_Into(const std::string &src_file, const std::string &dst_file)
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
		Logger::Get_Instance()->Log("File: (" + src_file + ") was copied to: " + dst_file);
		src.close();
		dst.close();
	}

	static void Delete_Existing_File(const std::string &path)
	{
		if (!std::filesystem::exists(path))
		{
			Logger::Get_Instance()->Log("File: (" + path + ") does not exist");
			return;
		}
		if (std::remove(path.c_str()) == 0)
		{
			Logger::Get_Instance()->Log("File: (" + path + ") was deleted succesfully");
		}
		else
		{
			Logger::Get_Instance()->Log("File: (" + path + ") was not deleted");
		}
	}
	static void Append_Data_To_File(const std::string &path, const std::string &data)
	{
		std::ofstream file(path, std::ios::app);
		if (file)
		{
			file << data;
			Logger::Get_Instance()->Log("File: (" + path + ") was appended successfully");
		}
		else
		{
			Logger::Get_Instance()->Log("File: (" + path + ") was not appended");
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
	static Os                   *instance;
};

}        // namespace systems

#endif        // OS_HPP
