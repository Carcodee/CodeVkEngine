//

// Created by carlo on 2025-02-27.
//

#ifndef FILES_MANAGER_HPP
#define FILES_MANAGER_HPP

namespace systems
{
class files_manager
{
  public:
	files_manager()
	{
	}

	~files_manager()
	{
		for (auto &path : paths_to_delete)
		{
			os::get_instance()->delete_existing_file(path);
		}
		paths_to_delete.clear();
	}

	void add_path_to_delete(std::string path)
	{
		assert(false && "You are about to delete paths from generated folder, be sure is intended");
		paths_to_delete.push_back(path);
	}
	void check_all_changes()
	{
		for (auto &file : render_nodes_files)
		{
			// calling some callbacks
			file->check_last_time_write();
		}
	}
	void add_file_to_watch(std::string path, std::function<void()> function)
	{
		assert(!path.empty() && "Path is empty");
		render_nodes_files.emplace_back(std::make_unique<file_info>(path, function));
	}
	void collect_render_nodes_metadata(std::function<void()> on_change_callback)
	{
		std::filesystem::path render_passes_dir = os::get_instance()->engine_resources_path / "RenderNodes";
		for (auto pass_file_data : std::filesystem::directory_iterator(render_passes_dir))
		{
			render_nodes_files.emplace_back(std::make_unique<file_info>(pass_file_data.path().string(), on_change_callback));
		}
	}

	std::deque<std::string>                paths_to_delete;
	std::vector<std::unique_ptr<file_info>> render_nodes_files;
};
}        // namespace systems

#endif        // FILES_MANAGER_HPP
