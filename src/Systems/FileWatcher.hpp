//
// Created by carlo on 2025-01-12.
//

#ifndef FILE_WATCHER_HPP
#define FILE_WATCHER_HPP

namespace systems
{
struct file_info
{
	// lastTime write it
	file_info(const std::string &path, std::function<void()> on_change_callback)
	{
		assert(std::filesystem::exists(path) && "invalid path");

		this->on_change_callback = on_change_callback;
		this->path             = path;
		this->name             = this->path.filename().string();
		this->extension        = this->path.extension().string();
		this->modified         = false;
		last_time_check          = std::filesystem::last_write_time(this->path);
	}
	file_info(const std::string &path)
	{
		assert(std::filesystem::exists(path) && "invalid path");

		this->path      = path;
		this->name      = this->path.filename().string();
		this->extension = this->path.extension().string();
		this->modified  = false;
		last_time_check   = std::filesystem::last_write_time(this->path);
	}
	void check_last_time_write()
	{
		auto f_time = std::filesystem::last_write_time(this->path);
		if (f_time != last_time_check)
		{
			modified      = true;
			last_time_check = f_time;
			// if (on_change_callback)
			// {
			// onChangeCallback();
			// }
		}
	}
	void invalidate()
	{
		modified = false;
	}
	std::filesystem::path           path;
	std::string                     name;
	std::string                     extension;
	std::atomic<bool>               modified;
	std::filesystem::file_time_type last_time_check;
	std::function<void()>           on_change_callback = {};
};
// use background threat for this eventually
//  class FileWatcher
//  {
//      void CheckChanges()
//      {
//          for (auto& file : filesToWatch)
//          {
//              file.CheckLastTimeWrite();
//          }
//      }
//      // file_info& GetFile(const std::string& path)
//      // {
//      //     filesToWatch.emplace_back(FileInfo(path));
//      //     return filesToWatch.back();
//      // }
//
//      std::vector<file_info> filesToWatch;
//      std::thread watcherThreat;
//  };

}        // namespace systems

#endif        // FILE_WATCHER_HPP
