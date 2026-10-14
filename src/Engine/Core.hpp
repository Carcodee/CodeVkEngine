//
// Created by carlo on 2024-09-22.
//

#ifndef CORE_HPP

namespace engine
{

class present_queue;
class swap_chain;
class render_graph;
class queue_worker_manager;

class core
{
  public:
	core(const char **instance_extensions, uint8_t instance_extensions_count, window_desc *compatible_window_desc, bool enable_debugging);
	~core();
	void clear_caches();

	std::unique_ptr<engine::swap_chain>   create_swapchain(vk::PresentModeKHR present_mode_khr, uint32_t image_count, window_desc window_desc, glm::uvec2 window_size);
	std::unique_ptr<engine::render_graph> create_render_graph();

	std::vector<vk::UniqueCommandBuffer> allocate_command_buffers(vk::CommandPool command_pool_in, size_t count);
	std::vector<vk::UniqueCommandBuffer> allocate_command_buffers_secondary(vk::CommandPool command_pool, size_t count);
	vk::UniqueSemaphore                  create_vulkan_semaphore();
	vk::UniqueSemaphore                  create_exportable_binary_semaphore(HANDLE& handle_out);
	vk::UniqueSemaphore                  create_vulkan_timeline_semaphore(uint32_t initial_value);
	vk::UniqueSemaphore                  create_exportable_timeline_semaphore(uint32_t initial_value, HANDLE& handle_out);
	vk::UniqueFence                      create_fence(bool state);
	void                                 wait_for_fence(vk::Fence fence);
	void                                 reset_fence(vk::Fence fence);

	void wait_idle();

	static vk::Queue get_device_queue(vk::Device logical_device, uint32_t family_index);

	static vk::UniqueCommandPool create_command_pool(vk::Device logical_device, uint32_t family_index);

	vk::UniqueInstance                  instance;
	vk::PhysicalDevice                  physical_device;
	vk::UniqueDevice                    logical_device;
	bool                                resize_requested = false;
	engine::render_graph                        *render_graph_ref;
	engine::swap_chain                          *swapchain_ref;
	std::deque<std::function<void()> *> deletion_queue;
	std::unique_ptr<engine::queue_worker_manager> queue_worker_manager;
	engine::queue_family_indices                  queue_family_ids;
	std::array<uint8_t, VK_UUID_SIZE>   device_uuid;

  private:
	static vk::UniqueInstance create_instance(const std::vector<const char *> &instance_extensions,
	                                         const std::vector<const char *> &validation_layers);

	static vk::PhysicalDevice find_physical_device(vk::Instance instance);

	static std::array<uint8_t, VK_UUID_SIZE> find_physical_device_uuid(vk::PhysicalDevice physical_device_in);

	static engine::queue_family_indices find_queue_family_indices(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

	static vk::UniqueDevice create_logical_device(vk::PhysicalDevice physical_device, engine::queue_family_indices family_indices, std::vector<const char *> device_extensions, std::vector<const char *> validation_layers);

	static vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vulkan_dispatch_loader_dynamic> create_debug_utils_messenger(vk::Instance instance, PFN_vkDebugUtilsMessengerCallbackEXT debug_callback, vulkan_dispatch_loader_dynamic &loader);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_message_callback(
	    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
	    VkDebugUtilsMessageTypeFlagsEXT             message_type,
	    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
	    void                                       *p_user_data);

	friend class swapchain;
	friend class render_graph;

	vulkan_dispatch_loader_dynamic                                               loader;
	vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vulkan_dispatch_loader_dynamic> debug_utils_messenger;
};

struct worker_queue
{
	engine::core                                             *core_ref           = nullptr;
	vk::Queue                                         queue_handle       = {};
	int32_t                                           family_index       = -1;
	vk::UniqueCommandPool                             worker_command_pool = {};
	systems::task_thread                               task_threat        = {};
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
	
	HANDLE& get_exportable_handle()
	{
		assert(exportable_handle!= nullptr);
		return exportable_handle;
	}

	vk::CommandBuffer &request_queue_cmd(int &pool_idx_out)
	{
		assert(current_pool_cmd_idx < 10 && "never should be that many pools");
		assert(cmds_pool_size < 10 && "never should be that many pools");
		if (current_pool_cmd_idx >= cmds_pool_size)
		{
			increase_pool();
			allocate_pool_cmds(per_cmd_pool_size, current_pool_cmd_idx);
		}
		pool_idx_out            = current_pool_cmd_idx;
		vk::CommandBuffer cmd = get_current_cmd(current_pool_cmd_idx);
		current_pool_cmd_idx++;

		return cmd;
	}

	void init_timeline_semaphore(int timeline_value_in, bool external_usage_in)
	{
		assert(core_ref != nullptr);

		is_for_external_usage = external_usage_in;
		timeline_value      = timeline_value_in;
		if (is_for_external_usage)
		{
			timeline_sem = core_ref->create_exportable_timeline_semaphore(timeline_value, exportable_handle);
		}
		else
		{
			timeline_sem = core_ref->create_vulkan_timeline_semaphore(timeline_value);
		}
	}

	void reset_pool_usage()
	{
		current_pool_cmd_idx = 0;
	}
	void increase_pool()
	{
		assert(cmds_pool_size < 10 && "never should be that many pools");
		cmds_pool_size++;
		command_buffers.emplace_back(std::vector<vk::UniqueCommandBuffer>());
	}

	void init_cmds_to_use_pools(int count)
	{
		cmds_pool_size = count;
		command_buffers.reserve(10);
		for (int i = 0; i < count; ++i)
		{
			command_buffers.emplace_back(std::vector<vk::UniqueCommandBuffer>());
		}
	}

	void set_cmd_idx(int idx)
	{
		assert(core_ref != nullptr && "Core is null");
		if (command_buffers.empty())
		{
			return;
		}
		active_cmd_idx = idx;
	}
	vk::CommandBuffer &get_current_cmd(int cmd_pool_idx)
	{
		assert(core_ref != nullptr && "Core is null");
		assert(command_buffers.size() != 0 && "There is no cmds allocated");
		return command_buffers[cmd_pool_idx][active_cmd_idx].get();
	};

	void allocate_pool_cmds(int count, int cmd_pool_idx)
	{
		assert(core_ref != nullptr && "Core is null");
		assert(!command_buffers.empty() && "command buffers must be non empty");
		assert(cmd_pool_idx < command_buffers.size() && "pool idx is bigger than the command buffer pools");
		assert(count > 0 && "there must be more than 0 cmds to allocate");
		command_buffers[cmd_pool_idx] = std::move(core_ref->allocate_command_buffers(worker_command_pool.get(), count));
	}

	void allocate_all_cmds(int count)
	{
		assert(core_ref != nullptr && "Core is null");
		assert(!command_buffers.empty() && "command buffers must be non empty");
		per_cmd_pool_size = count;
		for (int i = 0; i < command_buffers.size(); ++i)
		{
			allocate_pool_cmds(per_cmd_pool_size, i);
		}
	}

	void begin_current_cmds(vk::CommandBufferBeginInfo begin_info)
	{
		for (int i = 0; i < command_buffers.size(); ++i)
		{
			command_buffers[i][active_cmd_idx]->begin(begin_info);
		}
	}
	void end_current_cmds()
	{
		for (int i = 0; i < command_buffers.size(); ++i)
		{
			command_buffers[i][active_cmd_idx]->end();
		}
	}
	worker_queue()                               = default;
	~worker_queue()                              = default;
	worker_queue(const worker_queue &)            = delete;
	worker_queue &operator=(const worker_queue &) = delete;
	worker_queue(worker_queue &&)                 = default;
	worker_queue &operator=(worker_queue &&)      = default;
};

class queue_worker_manager
{
  public:
	queue_worker_manager(engine::core *core)
	{
		assert(core != nullptr);
		this->core_ref = core;
	}
	~queue_worker_manager() = default;
	worker_queue *get_or_create_worker_queue(std::string name, uint32_t family_index, bool main_thread = true, bool external_usage = false)
	{
		assert(core_ref != nullptr);
		if (workers_queues.contains(name))
		{
			return get_worker_queue(name);
		}
		workers_queues.try_emplace(name);
		auto &queue_ref             = workers_queues.at(name);
		queue_ref.core_ref           = core_ref;
		queue_ref.name              = name;
		queue_ref.queue_handle       = core_ref->get_device_queue(core_ref->logical_device.get(), family_index);
		queue_ref.family_index       = static_cast<int32_t>(family_index);
		queue_ref.is_main_threat      = main_thread;
		queue_ref.cmds_pool_size      = 1;
		queue_ref.is_for_external_usage = external_usage;
		queue_ref.worker_command_pool = core_ref->create_command_pool(core_ref->logical_device.get(), family_index);
		queue_ref.init_cmds_to_use_pools(queue_ref.cmds_pool_size);
		queue_ref.init_timeline_semaphore(0, external_usage);
		if (main_thread == false)
		{
			queue_ref.task_threat.start();
		}
		return &queue_ref;
	}
	worker_queue *get_worker_queue(std::string name)
	{
		if (!workers_queues.contains(name))
		{
			return nullptr;
		}
		return &workers_queues.at(name);
	}

	vk::CommandBuffer request_worker_queue_cmd(std::string name, int &cmd_pool_idx_used_out)
	{
		if (!workers_queues.contains(name))
		{
			return nullptr;
		};
		return workers_queues.at(name).request_queue_cmd(cmd_pool_idx_used_out);
	}

	worker_queue *use_queue(std::string name)
	{
		if (!workers_queues.contains(name))
		{
			return nullptr;
		}
		return &workers_queues.at(name);
	}

	void set_frame_index(int frame_idx)
	{
		for (auto &queue : workers_queues)
		{
			queue.second.set_cmd_idx(frame_idx);
		}
	}
	void allocate_cmds(int count)
	{
		for (auto &queue : workers_queues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			queue.second.allocate_all_cmds(count);
		}
	}
	void begin_cmds()
	{
		for (auto &queue : workers_queues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			auto buffer_begin_info = vk::CommandBufferBeginInfo()
			                           .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			queue.second.begin_current_cmds(buffer_begin_info);
		}
	}

	void end_cmds()
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
			queue.second.end_current_cmds();
		}
	}
	void reset_pool_usage()
	{
		for (auto &queue : workers_queues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			queue.second.reset_pool_usage();
		}
	}
	engine::core                                        *core_ref;
	std::unordered_map<std::string, worker_queue> workers_queues = {};
};

class execute_once_command
{
  public:
	execute_once_command(engine::core *core, std::string queue_name = "Graphics")
	{
		this->core          = core;
		this->queue_name     = queue_name;
		command_buffer_handle = std::move(core->allocate_command_buffers(core->queue_worker_manager->get_worker_queue(queue_name)->worker_command_pool.get(), 1)[0]);
	}

	vk::CommandBuffer begin_command_buffer()
	{
		auto buffer_begin_info = vk::CommandBufferBeginInfo()
		                           .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		command_buffer_handle->begin(buffer_begin_info);
		return command_buffer_handle.get();
	}

	void end_command_buffer()
	{
		command_buffer_handle->end();
		vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eAllCommands};

		auto submit_info = vk::SubmitInfo()
		                      .setWaitSemaphoreCount(0)
		                      .setPWaitDstStageMask(wait_stages)
		                      .setCommandBufferCount(1)
		                      .setPCommandBuffers(&command_buffer_handle.get())
		                      .setSignalSemaphoreCount(0);

		core->queue_worker_manager->get_worker_queue(this->queue_name)->queue_handle.submit({submit_info}, nullptr);
		core->queue_worker_manager->get_worker_queue(this->queue_name)->queue_handle.waitIdle();
	}

	engine::core                   *core;
	vk::UniqueCommandBuffer command_buffer_handle;
	std::string             queue_name = "Graphics";
};

}        // namespace engine

#	define CORE_HPP

#endif        // CORE_HPP
