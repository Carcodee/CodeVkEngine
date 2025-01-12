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
			workingDir = std::filesystem::current_path();
			projectPath = GetProjectPath(workingDir);
			shadersPath = projectPath / "src" / "Shaders";
			assetsPath = projectPath / "Resources"/"Assets";
			engineResourcesPath = projectPath / "Resources"/"Engine";
			
		}


		static OS* GetInstance()
		{
			if (instance == nullptr)
			{
				instance = new OS;
			}
			return instance;
		}


		std::filesystem::path GetProjectPath(std::filesystem::path& workDir)
		{
			std::filesystem::path projectPath = workDir;
			bool solutionPathFind = false;
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
		
		static std::string ReadFile(const std::string& path)
		{
			std::ifstream inFile(path, std::ios::in || std::ios::binary);
			if (!inFile)
			{
				assert(false && "impossible to open the path");
			}
			std::ostringstream oss;

			oss << inFile.rdbuf();
			
			return oss.str();
		}

		static void CreateFileAt(const std::string dstFile)
		{
			std::ofstream dst(dstFile, std::ios::binary);
			if (!dst)
			{
				assert(false &&"Impossible to create dst file");
			}
		}

		static void WriteFile(const std::string& path, const char* text, size_t size)
		{
			std::ofstream file(path, std::ios::binary);
			if (!file.is_open())
			{
				assert(false &&"Impossible to write file");
			}
			file.write(text, size);

			Logger::GetInstance()->Log("File: " + path + " was writed succesfully");
			
		}

		static void CopyFileInto(const std::string& srcFile, const std::string& dstFile)
		{
			std::ifstream src(srcFile, std::ios::binary);
			if (!src)
			{
				assert(false &&"Impossible to open src file");
			}
			std::ofstream dst(dstFile, std::ios::binary);
			if (!dst)
			{
				assert(false &&"Impossible to create dst file");
			}

			dst << src.rdbuf();
			Logger::GetInstance()->Log("File: "+dstFile + " was created");
			
		}
		
		std::filesystem::path workingDir;
		std::filesystem::path projectPath;
        std::filesystem::path engineResourcesPath;
        std::filesystem::path assetsPath;
		std::filesystem::path shadersPath;
        static OS* instance;
	};
	
    OS* OS::instance = nullptr;
}

#endif //OS_HPP
