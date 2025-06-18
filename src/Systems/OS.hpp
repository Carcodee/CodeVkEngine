//

// Created by carlo on 2024-11-22.
//

#ifndef OS_HPP
#define OS_HPP

namespace SYSTEMS
{

class OS
{
  public:
	OS()
	{
		workingDir          = std::filesystem::current_path();
		projectPath         = GetProjectPath(workingDir);
		shadersPath         = projectPath / "src" / "Shaders";
		assetsPath          = projectPath / "Resources" / "Assets";
		engineResourcesPath = projectPath / "Resources" / "Engine";

		glslShadersTemplatePath  = shadersPath / "glsl" / "templates";
		slangShadersTemplatePath = shadersPath / "slang" / "templates";
	}

	static OS *GetInstance()
	{
		static OS instance;
		return &instance;
	}

	std::filesystem::path GetProjectPath(std::filesystem::path &workDir)
	{
		std::filesystem::path projectPath      = workDir;
		bool                  solutionPathFind = false;
		while (!solutionPathFind)
		{
			for (auto element : std::filesystem::directory_iterator(projectPath))
			{
				if (element.path().filename() == ".gitignore")
				{
					solutionPathFind = true;
					break;
				}
			}
			if (solutionPathFind)
			{
				break;
			}

			projectPath = projectPath.parent_path();
			if (projectPath == workDir.root_path())
			{
				break;
			}
		}
		assert(solutionPathFind && "The project dir was not find");

		return projectPath;
	}
	bool IsPathAbsolute(std::string path)
	{
		if (!std::filesystem::exists(path))
		{
			return false;
		}
		return true;
	}
	std::string GetEngineResourcesPath()
	{
		return engineResourcesPath.string();
	}

	std::string GetAssetsPath()
	{
		return assetsPath.string();
	}
	std::string GetShadersPath()
	{
		return shadersPath.string();
	}

	std::string GetGlslTemplatePath()
	{
		return glslShadersTemplatePath.string();
	}

	std::string GetSlangTemplatePath()
	{
		return slangShadersTemplatePath.string();
	}
	static std::string GetExtension(std::string name)
	{
		size_t pointPos = name.find_last_of(".");
		if (pointPos > name.size() || pointPos < 0)
		{
			return "";
		}
		std::string extension = name.substr(pointPos, name.back());
		return extension;
	}

	static std::string ReadFile(const std::string &path)
	{
		std::ifstream inFile(path.c_str(), std::ios::in | std::ios::binary);
		if (!inFile)
		{
			assert(false && "impossible to open the path");
		}
		std::ostringstream oss;

		oss << inFile.rdbuf();

		inFile.close();
		return oss.str();
	}

	static nlohmann::json GetJsonFromFile(const std::string &path)
	{
		std::ifstream inFile(path);
		if (!inFile)
		{
			assert(false && "impossible to open the path");
		}
		nlohmann::json json;

		inFile >> json;

		inFile.close();
		return json;
	}
	static void CreateFileAt(const std::string dstFile)
	{
		std::ofstream dst(dstFile, std::ios::binary);
		if (!dst)
		{
			Logger::GetInstance()->Log("File: (" + dstFile + ") could not be created");
		}
	}

	static void WriteFile(const std::string &path, const char *text, size_t size)
	{
		std::ofstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			assert(false && "Impossible to write file");
			Logger::GetInstance()->Log("File: (" + path + ") could not be opened");
			return;
		}
		file.write(text, size);
		file.close();

		Logger::GetInstance()->Log("File: (" + path + ") was writed succesfully");
	}

	static void CopyFileInto(const std::string &srcFile, const std::string &dstFile)
	{
		std::ifstream src(srcFile, std::ios::binary);
		if (!src)
		{
			assert(false && "Impossible to open src file");
		}
		std::ofstream dst(dstFile, std::ios::binary);
		if (!dst)
		{
			assert(false && "Impossible to create dst file");
		}

		dst << src.rdbuf();
		Logger::GetInstance()->Log("File: (" + srcFile + ") was copied to: " + dstFile);
		src.close();
		dst.close();
	}

	static void DeleteExistingFile(const std::string &path)
	{
		if (!std::filesystem::exists(path))
		{
			Logger::GetInstance()->Log("File: (" + path + ") does not exist");
			return;
		}
		if (std::remove(path.c_str()) == 0)
		{
			Logger::GetInstance()->Log("File: (" + path + ") was deleted succesfully");
		}
		else
		{
			Logger::GetInstance()->Log("File: (" + path + ") was not deleted");
		}
	}
	static void AppendDataToFile(const std::string &path, const std::string &data)
	{
		std::ofstream file(path, std::ios::app);
		if (file)
		{
			file << data;
			Logger::GetInstance()->Log("File: (" + path + ") was appended successfully");
		}
		else
		{
			Logger::GetInstance()->Log("File: (" + path + ") was not appended");
		}
	}

	std::filesystem::path        workingDir;
	std::filesystem::path        projectPath;
	std::filesystem::path        engineResourcesPath;
	std::filesystem::path        assetsPath;
	std::filesystem::path        shadersPath;
	std::filesystem::path        glslShadersTemplatePath;
	std::filesystem::path        slangShadersTemplatePath;
	std::queue<std::string_view> tempFilesToDestroy;
	static OS                   *instance;
};

}        // namespace SYSTEMS

#endif        // OS_HPP
