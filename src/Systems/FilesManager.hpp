//

// Created by carlo on 2025-02-27.
//

#ifndef FILES_MANAGER_HPP
#define FILES_MANAGER_HPP

namespace systems
{
class Files_Manager
{
  public:
	Files_Manager()
	{
	}

	~Files_Manager()
	{
		for (auto &path : paths_to_delete)
		{
			Os::Get_Instance()->Delete_Existing_File(path);
		}
		paths_to_delete.clear();
	}

	void Add_Path_To_Delete(std::string path)
	{
		assert(false && "You are about to delete paths from generated folder, be sure is intended");
		paths_to_delete.push_back(path);
	}
	void Check_All_Changes()
	{
		for (auto &file : render_nodes_files)
		{
			// calling some callbacks
			file->Check_Last_Time_Write();
		}
	}
	void Add_File_To_Watch(std::string path, std::function<void()> function)
	{
		assert(!path.empty() && "Path is empty");
		render_nodes_files.emplace_back(std::make_unique<File_Info>(path, function));
	}
	void Collect_Render_Nodes_Metadata(std::function<void()> on_change_callback)
	{
		std::filesystem::path render_passes_dir = Os::Get_Instance()->engine_resources_path / "RenderNodes";
		for (auto pass_file_data : std::filesystem::directory_iterator(render_passes_dir))
		{
			render_nodes_files.emplace_back(std::make_unique<File_Info>(pass_file_data.path().string(), on_change_callback));
		}
	}

	std::deque<std::string>                paths_to_delete;
	std::vector<std::unique_ptr<File_Info>> render_nodes_files;
};
}        // namespace systems

#endif        // FILES_MANAGER_HPP
