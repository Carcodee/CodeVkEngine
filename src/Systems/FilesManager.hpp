//

// Created by carlo on 2025-02-27.
//

#ifndef FILESMANAGER_HPP
#define FILESMANAGER_HPP

namespace SYSTEMS
{
class FilesManager
{
  public:
	FilesManager()
	{
	}

	~FilesManager()
	{
		for (auto &path : pathsToDelete)
		{
			OS::GetInstance()->DeleteExistingFile(path);
		}
		pathsToDelete.clear();
	}

	void AddPathToDelete(std::string path)
	{
		assert(false && "You are about to delete paths from generated folder, be sure is intended");
		pathsToDelete.push_back(path);
	}
	void CheckAllChanges()
	{
		for (auto &file : renderNodesFiles)
		{
			// calling some callbacks
			file->CheckLastTimeWrite();
		}
	}
	void AddFileToWatch(std::string path, std::function<void()> function)
	{
		assert(!path.empty() && "Path is empty");
		renderNodesFiles.emplace_back(std::make_unique<FileInfo>(path, function));
	}
	void CollectRenderNodesMetadata(std::function<void()> onChangeCallback)
	{
		std::filesystem::path renderPassesDir = OS::GetInstance()->engineResourcesPath / "RenderNodes";
		for (auto passFileData : std::filesystem::directory_iterator(renderPassesDir))
		{
			renderNodesFiles.emplace_back(std::make_unique<FileInfo>(passFileData.path().string(), onChangeCallback));
		}
	}

	std::deque<std::string>                pathsToDelete;
	std::vector<std::unique_ptr<FileInfo>> renderNodesFiles;
};
}        // namespace SYSTEMS

#endif        // FILESMANAGER_HPP
