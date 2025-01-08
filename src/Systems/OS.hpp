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

		std::string ReadFile(std::string path)
		{
			std::ifstream inFile(path);
			std::string data;

			while (std::getline(inFile, data))
			{
			}
			return data;
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
