//
// Created by carlo on 2024-09-22.
//

#ifndef CORE_HPP

namespace engine
{

class Present_Queue;
class Swap_Chain;
class Render_Graph;
class Queue_Worker_Manager;

class Core
{
  public:
	Core(const char **instance_extensions, uint8_t instance_extensions_count, Window_Desc *compatible_window_desc, bool enable_debugging);
	~Core();
	void Clear_Caches();

	std::unique_ptr<engine::Swap_Chain>   Create_Swapchain(vk::PresentModeKHR present_mode_khr, uint32_t image_count, Window_Desc window_desc, glm::uvec2 window_size);
	std::unique_ptr<engine::Render_Graph> Create_Render_Graph();

	std::vector<vk::UniqueCommandBuffer> Allocate_Command_Buffers(vk::CommandPool command_pool_in, size_t count);
	std::vector<vk::UniqueCommandBuffer> Allocate_Command_Buffers_Secondary(vk::CommandPool command_pool, size_t count);
	vk::UniqueSemaphore                  Create_Vulkan_Semaphore();
	vk::UniqueSemaphore                  Create_Exportable_Binary_Semaphore(HANDLE& handle_out);
	vk::UniqueSemaphore                  Create_Vulkan_Timeline_Semaphore(uint32_t initial_value);
	vk::UniqueSemaphore                  Create_Exportable_Timeline_Semaphore(uint32_t initial_value, HANDLE& handle_out);
	vk::UniqueFence                      Create_Fence(bool state);
	void                                 Wait_For_Fence(vk::Fence fence);
	void                                 Reset_Fence(vk::Fence fence);

	void Wait_Idle();

	static vk::Queue Get_Device_Queue(vk::Device logical_device, uint32_t family_index);

	static vk::UniqueCommandPool Create_Command_Pool(vk::Device logical_device, uint32_t family_index);

	vk::UniqueInstance                  instance;
	vk::PhysicalDevice                  physical_device;
	vk::UniqueDevice                    logical_device;
	bool                                resize_requested = false;
	engine::Render_Graph                        *render_graph_ref;
	engine::Swap_Chain                          *swapchain_ref;
	std::deque<std::function<void()> *> deletion_queue;
	std::unique_ptr<engine::Queue_Worker_Manager> queue_worker_manager;
	engine::Queue_Family_Indices                  queue_family_ids;
	std::array<uint8_t, VK_UUID_SIZE>   device_uuid;

  private:
	static vk::UniqueInstance Create_Instance(const std::vector<const char *> &instance_extensions,
	                                         const std::vector<const char *> &validation_layers);

	static vk::PhysicalDevice Find_Physical_Device(vk::Instance instance);

	static std::array<uint8_t, VK_UUID_SIZE> Find_Physical_Device_Uuid(vk::PhysicalDevice physical_device_in);

	static engine::Queue_Family_Indices Find_Queue_Family_Indices(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

	static vk::UniqueDevice Create_Logical_Device(vk::PhysicalDevice physical_device, engine::Queue_Family_Indices family_indices, std::vector<const char *> device_extensions, std::vector<const char *> validation_layers);

	static vk::UniqueHandle<vk::DebugUtilsMessengerEXT, Vulkan_Dispatch_Loader_Dynamic> Create_Debug_Utils_Messenger(vk::Instance instance, PFN_vkDebugUtilsMessengerCallbackEXT debug_callback, Vulkan_Dispatch_Loader_Dynamic &loader);

	static VKAPI_ATTR VkBool32 VKAPI_CALL Debug_Message_Callback(
	    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
	    VkDebugUtilsMessageTypeFlagsEXT             message_type,
	    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
	    void                                       *p_user_data);

	friend class swapchain;
	friend class Render_Graph;

	Vulkan_Dispatch_Loader_Dynamic                                               loader;
	vk::UniqueHandle<vk::DebugUtilsMessengerEXT, Vulkan_Dispatch_Loader_Dynamic> debug_utils_messenger;
};

struct Worker_Queue
{
	engine::Core                                             *core_ref           = nullptr;
	vk::Queue                                         queue_handle       = {};
	int32_t                                           family_index       = -1;
	vk::UniqueCommandPool                             worker_command_pool = {};
	systems::Task_Thread                               task_threat        = {};
	std::vector<std::vector<vk::UniqueCommandBuffer>> command_buffers;
	vk::UniqueSemaphore                               timeline_sem;
	HANDLE                                            exportable_handle   = nullptr;
	int64_t                                           timeline_value      = -1;
	std::string                                       name               = "";
	int                                               per_cmd_pool_size     = 0;
	bool                                              is_main_threat       = true;
	bool                                              is_for_external_usage = false;
	int                                               active_cmd_idx       = 0;
	int                                               cmds_pool_size       = 0;
	int                                               current_pool_cmd_idx  = 0;
	
	HANDLE& Get_Exportable_Handle()
	{
		assert(exportable_handle!= nullptr);
		return exportable_handle;
	}

	vk::CommandBuffer &Request_Queue_Cmd(int &pool_idx_out)
	{
		assert(current_pool_cmd_idx < 10 && "never should be that many pools");
		assert(cmds_pool_size < 10 && "never should be that many pools");
		if (current_pool_cmd_idx >= cmds_pool_size)
		{
			Increase_Pool();
			Allocate_Pool_Cmds(per_cmd_pool_size, current_pool_cmd_idx);
		}
		pool_idx_out            = current_pool_cmd_idx;
		vk::CommandBuffer cmd = Get_Current_Cmd(current_pool_cmd_idx);
		current_pool_cmd_idx++;

		return cmd;
	}

	void Init_Timeline_Semaphore(int timeline_value_in, bool external_usage_in)
	{
		assert(core_ref != nullptr);

		is_for_external_usage = external_usage_in;
		timeline_value      = timeline_value_in;
		if (is_for_external_usage)
		{
			timeline_sem = core_ref->Create_Exportable_Timeline_Semaphore(timeline_value, exportable_handle);
		}
		else
		{
			timeline_sem = core_ref->Create_Vulkan_Timeline_Semaphore(timeline_value);
		}
	}

	void Reset_Pool_Usage()
	{
		current_pool_cmd_idx = 0;
	}
	void Increase_Pool()
	{
		assert(cmds_pool_size < 10 && "never should be that many pools");
		cmds_pool_size++;
		command_buffers.emplace_back(std::vector<vk::UniqueCommandBuffer>());
	}

	void Init_Cmds_To_Use_Pools(int count)
	{
		cmds_pool_size = count;
		command_buffers.reserve(10);
		for (int i = 0; i < count; ++i)
		{
			command_buffers.emplace_back(std::vector<vk::UniqueCommandBuffer>());
		}
	}

	void Set_Cmd_Idx(int idx)
	{
		assert(core_ref != nullptr && "Core is null");
		if (command_buffers.empty())
		{
			return;
		}
		active_cmd_idx = idx;
	}
	vk::CommandBuffer &Get_Current_Cmd(int cmd_pool_idx)
	{
		assert(core_ref != nullptr && "Core is null");
		assert(command_buffers.size() != 0 && "There is no cmds allocated");
		return command_buffers[cmd_pool_idx][active_cmd_idx].get();
	};

	void Allocate_Pool_Cmds(int count, int cmd_pool_idx)
	{
		assert(core_ref != nullptr && "Core is null");
		assert(!command_buffers.empty() && "command buffers must be non empty");
		assert(cmd_pool_idx < command_buffers.size() && "pool idx is bigger than the command buffer pools");
		assert(count > 0 && "there must be more than 0 cmds to allocate");
		command_buffers[cmd_pool_idx] = std::move(core_ref->Allocate_Command_Buffers(worker_command_pool.get(), count));
	}

	void Allocate_All_Cmds(int count)
	{
		assert(core_ref != nullptr && "Core is null");
		assert(!command_buffers.empty() && "command buffers must be non empty");
		per_cmd_pool_size = count;
		for (int i = 0; i < command_buffers.size(); ++i)
		{
			Allocate_Pool_Cmds(per_cmd_pool_size, i);
		}
	}

	void Begin_Current_Cmds(vk::CommandBufferBeginInfo begin_info)
	{
		for (int i = 0; i < command_buffers.size(); ++i)
		{
			command_buffers[i][active_cmd_idx]->begin(begin_info);
		}
	}
	void End_Current_Cmds()
	{
		for (int i = 0; i < command_buffers.size(); ++i)
		{
			command_buffers[i][active_cmd_idx]->end();
		}
	}
	Worker_Queue()                               = default;
	~Worker_Queue()                              = default;
	Worker_Queue(const Worker_Queue &)            = delete;
	Worker_Queue &operator=(const Worker_Queue &) = delete;
	Worker_Queue(Worker_Queue &&)                 = default;
	Worker_Queue &operator=(Worker_Queue &&)      = default;
};

class Queue_Worker_Manager
{
  public:
	Queue_Worker_Manager(engine::Core *core)
	{
		assert(core != nullptr);
		this->core_ref = core;
	}
	~Queue_Worker_Manager() = default;
	Worker_Queue *Get_Or_Create_Worker_Queue(std::string name, uint32_t family_index, bool main_thread = true, bool external_usage = false)
	{
		assert(core_ref != nullptr);
		if (workers_queues.contains(name))
		{
			return Get_Worker_Queue(name);
		}
		workers_queues.try_emplace(name);
		auto &queue_ref             = workers_queues.at(name);
		queue_ref.core_ref           = core_ref;
		queue_ref.name              = name;
		queue_ref.queue_handle       = core_ref->Get_Device_Queue(core_ref->logical_device.get(), family_index);
		queue_ref.family_index       = static_cast<int32_t>(family_index);
		queue_ref.is_main_threat      = main_thread;
		queue_ref.cmds_pool_size      = 1;
		queue_ref.is_for_external_usage = external_usage;
		queue_ref.worker_command_pool = core_ref->Create_Command_Pool(core_ref->logical_device.get(), family_index);
		queue_ref.Init_Cmds_To_Use_Pools(queue_ref.cmds_pool_size);
		queue_ref.Init_Timeline_Semaphore(0, external_usage);
		if (main_thread == false)
		{
			queue_ref.task_threat.Start();
		}
		return &queue_ref;
	}
	Worker_Queue *Get_Worker_Queue(std::string name)
	{
		if (!workers_queues.contains(name))
		{
			return nullptr;
		}
		return &workers_queues.at(name);
	}

	vk::CommandBuffer Request_Worker_Queue_Cmd(std::string name, int &cmd_pool_idx_used_out)
	{
		if (!workers_queues.contains(name))
		{
			return nullptr;
		};
		return workers_queues.at(name).Request_Queue_Cmd(cmd_pool_idx_used_out);
	}

	Worker_Queue *Use_Queue(std::string name)
	{
		if (!workers_queues.contains(name))
		{
			return nullptr;
		}
		return &workers_queues.at(name);
	}

	void Set_Frame_Index(int frame_idx)
	{
		for (auto &queue : workers_queues)
		{
			queue.second.Set_Cmd_Idx(frame_idx);
		}
	}
	void Allocate_Cmds(int count)
	{
		for (auto &queue : workers_queues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			queue.second.Allocate_All_Cmds(count);
		}
	}
	void Begin_Cmds()
	{
		for (auto &queue : workers_queues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			auto buffer_begin_info = vk::CommandBufferBeginInfo()
			                           .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			queue.second.Begin_Current_Cmds(buffer_begin_info);
		}
	}

	void End_Cmds()
	{
		for (auto &queue : workers_queues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			// for (auto cmdsPool : queue.second.command_buffers)
			// {
			// }
			queue.second.End_Current_Cmds();
		}
	}
	void Reset_Pool_Usage()
	{
		for (auto &queue : workers_queues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			queue.second.Reset_Pool_Usage();
		}
	}
	engine::Core                                        *core_ref;
	std::unordered_map<std::string, Worker_Queue> workers_queues = {};
};

class Execute_Once_Command
{
  public:
	Execute_Once_Command(engine::Core *core, std::string queue_name = "Graphics")
	{
		this->core          = core;
		this->queue_name     = queue_name;
		command_buffer_handle = std::move(core->Allocate_Command_Buffers(core->queue_worker_manager->Get_Worker_Queue(queue_name)->worker_command_pool.get(), 1)[0]);
	}

	vk::CommandBuffer Begin_Command_Buffer()
	{
		auto buffer_begin_info = vk::CommandBufferBeginInfo()
		                           .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		command_buffer_handle->begin(buffer_begin_info);
		return command_buffer_handle.get();
	}

	void End_Command_Buffer()
	{
		command_buffer_handle->end();
		vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eAllCommands};

		auto submit_info = vk::SubmitInfo()
		                      .setWaitSemaphoreCount(0)
		                      .setPWaitDstStageMask(wait_stages)
		                      .setCommandBufferCount(1)
		                      .setPCommandBuffers(&command_buffer_handle.get())
		                      .setSignalSemaphoreCount(0);

		core->queue_worker_manager->Get_Worker_Queue(this->queue_name)->queue_handle.submit({submit_info}, nullptr);
		core->queue_worker_manager->Get_Worker_Queue(this->queue_name)->queue_handle.waitIdle();
	}

	engine::Core                   *core;
	vk::UniqueCommandBuffer command_buffer_handle;
	std::string             queue_name = "Graphics";
};

}        // namespace engine

#	define CORE_HPP

#endif        // CORE_HPP
