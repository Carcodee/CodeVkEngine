//
// Created by carlo on 2024-09-22.
//

#ifndef CORE_HPP

namespace ENGINE
{

class PresentQueue;
class SwapChain;
class RenderGraph;
class QueueWorkerManager;

class Core
{
  public:
	Core(const char **instanceExtensions, uint8_t instanceExtensionsCount, WindowDesc *compatibleWindowDesc, bool enableDebugging);
	~Core();
	void ClearCaches();

	std::unique_ptr<SwapChain>   CreateSwapchain(vk::PresentModeKHR presentModeKHR, uint32_t imageCount, WindowDesc windowDesc, glm::uvec2 windowSize);
	std::unique_ptr<RenderGraph> CreateRenderGraph();

	std::vector<vk::UniqueCommandBuffer> AllocateCommandBuffers(vk::CommandPool commandPoolIn, size_t count);
	std::vector<vk::UniqueCommandBuffer> AllocateCommandBuffersSecondary(vk::CommandPool commandPool, size_t count);
	vk::UniqueSemaphore                  CreateVulkanSemaphore();
	vk::UniqueSemaphore                  CreateVulkanTimelineSemaphore(uint32_t initialValue);
	vk::UniqueFence                      CreateFence(bool state);
	void                                 WaitForFence(vk::Fence fence);
	void                                 ResetFence(vk::Fence fence);

	void WaitIdle();

	static vk::Queue GetDeviceQueue(vk::Device logicalDevice, uint32_t familyIndex);

	static vk::UniqueCommandPool CreateCommandPool(vk::Device logicalDevice, uint32_t familyIndex);

	vk::UniqueInstance                  instance;
	vk::PhysicalDevice                  physicalDevice;
	vk::UniqueDevice                    logicalDevice;
	bool                                resizeRequested = false;
	RenderGraph                        *renderGraphRef;
	SwapChain                          *swapchainRef;
	std::deque<std::function<void()> *> deletionQueue;
	std::unique_ptr<QueueWorkerManager> queueWorkerManager;
	QueueFamilyIndices                  queueFamilyIndices;
	std::array<uint8_t, VK_UUID_SIZE>   deviceUUID;

  private:
	static vk::UniqueInstance CreateInstance(const std::vector<const char *> &instanceExtensions,
	                                         const std::vector<const char *> &validationLayers);

	static vk::PhysicalDevice FindPhysicalDevice(vk::Instance instance);

	static std::array<uint8_t, VK_UUID_SIZE> FindPhysicalDeviceUUID(vk::PhysicalDevice physicalDeviceIn);

	static QueueFamilyIndices FindQueueFamilyIndices(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

	static vk::UniqueDevice CreateLogicalDevice(vk::PhysicalDevice physicalDevice, QueueFamilyIndices familyIndices, std::vector<const char *> deviceExtensions, std::vector<const char *> validationLayers);

	static vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> CreateDebugUtilsMessenger(vk::Instance instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, vk::DispatchLoaderDynamic &loader);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessageCallback(
	    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	    VkDebugUtilsMessageTypeFlagsEXT             messageType,
	    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	    void                                       *pUserData);

	friend class Swapchain;
	friend class RenderGraph;

	vk::DispatchLoaderDynamic                                               loader;
	vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> debugUtilsMessenger;
};

struct WorkerQueue
{
	Core                                             *coreRef           = nullptr;
	vk::Queue                                         workerQueue       = {};
	int32_t                                           familyIndex       = -1;
	vk::UniqueCommandPool                             workerCommandPool = {};
	SYSTEMS::TaskThread                               taskThreat        = {};
	std::vector<std::vector<vk::UniqueCommandBuffer>> commandBuffers;
	std::string                                       name              = "";
	int                                               perCmdPoolSize    = 0;
	bool                                              isMainThreat      = true;
	int                                               activeCmdIdx      = 0;
	int                                               cmdsPoolSize      = 0;
	int                                               currentPoolCmdIdx = 0;

	vk::CommandBuffer &RequestQueueCmd(int &poolIdxOut)
	{
		assert(currentPoolCmdIdx < 10 && "never should be that many pools");
		assert(cmdsPoolSize < 10 && "never should be that many pools");
		if (currentPoolCmdIdx >= cmdsPoolSize)
		{
			IncreasePool();
			AllocatePoolCmds(perCmdPoolSize, currentPoolCmdIdx);
		}
		poolIdxOut = currentPoolCmdIdx;
		vk::CommandBuffer cmd = GetCurrentCmd(currentPoolCmdIdx);
		currentPoolCmdIdx++;

		return cmd;
	}
	
	void ResetPoolUsage()
	{
		currentPoolCmdIdx = 0;
	}
	void IncreasePool()
	{
		assert(cmdsPoolSize < 10 && "never should be that many pools");
		cmdsPoolSize++;
		commandBuffers.emplace_back(std::vector<vk::UniqueCommandBuffer>());
	}
	
	void InitPools(int count)
	{
		cmdsPoolSize = count;
		commandBuffers.reserve(10);
		for (int i = 0; i < count; ++i)
		{
			commandBuffers.emplace_back(std::vector<vk::UniqueCommandBuffer>());
		}
	}
	
	void SetCmdIdx(int idx)
	{
		assert(coreRef != nullptr && "Core is null");
		if (commandBuffers.empty())
		{
			return;
		}
		activeCmdIdx = idx;
	}
	vk::CommandBuffer &GetCurrentCmd(int cmdPoolIdx)
	{
		assert(coreRef != nullptr && "Core is null");
		assert(commandBuffers.size() != 0 && "There is no cmds allocated");
		return commandBuffers[cmdPoolIdx][activeCmdIdx].get();
	};

	void AllocatePoolCmds(int count, int cmdPoolIdx)
	{
		assert(coreRef != nullptr && "Core is null");
		assert(!commandBuffers.empty() && "command buffers must be non empty");
		assert(cmdPoolIdx < commandBuffers.size() && "pool idx is bigger than the command buffer pools");
		assert(count > 0 && "there must be more than 0 cmds to allocate");
		commandBuffers[cmdPoolIdx] = std::move(coreRef->AllocateCommandBuffers(workerCommandPool.get(), count));
	}
	
	void AllocateAllCmds(int count)
	{
		assert(coreRef != nullptr && "Core is null");
		assert(!commandBuffers.empty() && "command buffers must be non empty");
		perCmdPoolSize = count;
		for (int i = 0; i < commandBuffers.size(); ++i)
		{
			AllocatePoolCmds(perCmdPoolSize, i);
		}
	}
	
	void BeginCurrentCmds(vk::CommandBufferBeginInfo beginInfo)
	{
		for (int i = 0; i < commandBuffers.size(); ++i)
		{
			commandBuffers[i][activeCmdIdx]->begin(beginInfo);
		}
	}
	void EndCurrentCmds()
	{
		for (int i = 0; i < commandBuffers.size(); ++i)
		{
			commandBuffers[i][activeCmdIdx]->end();
		}
	}
	WorkerQueue()                               = default;
	~WorkerQueue()                              = default;
	WorkerQueue(const WorkerQueue &)            = delete;
	WorkerQueue &operator=(const WorkerQueue &) = delete;
	WorkerQueue(WorkerQueue &&)                 = default;
	WorkerQueue &operator=(WorkerQueue &&)      = default;
};

class QueueWorkerManager
{
  public:
	QueueWorkerManager(Core *core)
	{
		assert(core != nullptr);
		this->coreRef = core;
	}
	~QueueWorkerManager() = default;
	WorkerQueue *GetOrCreateWorkerQueue(std::string name, uint32_t familyIndex, bool mainThread = true)
	{
		assert(coreRef != nullptr);
		if (workersQueues.contains(name))
		{
			return GetWorkerQueue(name);
		}
		workersQueues.try_emplace(name);
		workersQueues.at(name).coreRef           = coreRef;
		workersQueues.at(name).name              = name;
		workersQueues.at(name).workerQueue       = coreRef->GetDeviceQueue(coreRef->logicalDevice.get(), familyIndex);
		workersQueues.at(name).familyIndex       = static_cast<int32_t>(familyIndex);
		workersQueues.at(name).workerCommandPool = coreRef->CreateCommandPool(coreRef->logicalDevice.get(), familyIndex);
		workersQueues.at(name).isMainThreat      = mainThread;
		workersQueues.at(name).cmdsPoolSize      = 1;
		workersQueues.at(name).InitPools(workersQueues.at(name).cmdsPoolSize);

		if (mainThread == false)
		{
			workersQueues.at(name).taskThreat.Start();
		}
		return &workersQueues.at(name);
	}
	WorkerQueue *GetWorkerQueue(std::string name)
	{
		if (!workersQueues.contains(name))
		{
			return nullptr;
		}
		return &workersQueues.at(name);
	}

	vk::CommandBuffer RequestWorkerQueueCmd(std::string name, int& cmdPoolIdxUsedOut)
	{
		if (!workersQueues.contains(name))
		{
			return nullptr;
		}
		;
		return workersQueues.at(name).RequestQueueCmd(cmdPoolIdxUsedOut);
	}

	WorkerQueue *UseQueue(std::string name)
	{
		if (!workersQueues.contains(name))
		{
			return nullptr;
		}
		return &workersQueues.at(name);
	}

	void SetFrameIndex(int frameIdx)
	{
		for (auto &queue : workersQueues)
		{
			queue.second.SetCmdIdx(frameIdx);
		}
	}
	void AllocateCmds(int count)
	{
		for (auto &queue : workersQueues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			queue.second.AllocateAllCmds(count);
		}
	}
	void BeginCmds()
	{
		for (auto &queue : workersQueues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			auto bufferBeginInfo = vk::CommandBufferBeginInfo()
			                           .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			queue.second.BeginCurrentCmds(bufferBeginInfo);
		}
	}

	void EndCmds()
	{
		for (auto &queue : workersQueues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			// for (auto cmdsPool : queue.second.commandBuffers)
			// {
			// }
			queue.second.EndCurrentCmds();
		}
	}
	void ResetPoolUsage()
	{
		
		for (auto &queue : workersQueues)
		{
			if (queue.first == "Present")
			{
				continue;
			}
			queue.second.ResetPoolUsage();
		}
	}
	Core                                        *coreRef;
	std::unordered_map<std::string, WorkerQueue> workersQueues = {};
};

class ExecuteOnceCommand
{
  public:
	ExecuteOnceCommand(Core *core, std::string queueName = "Graphics")
	{
		this->core          = core;
		this->queueName     = queueName;
		commandBufferHandle = std::move(core->AllocateCommandBuffers(core->queueWorkerManager->GetWorkerQueue(queueName)->workerCommandPool.get(), 1)[0]);
	}

	vk::CommandBuffer BeginCommandBuffer()
	{
		auto bufferBeginInfo = vk::CommandBufferBeginInfo()
		                           .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		commandBufferHandle->begin(bufferBeginInfo);
		return commandBufferHandle.get();
	}

	void EndCommandBuffer()
	{
		commandBufferHandle->end();
		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eAllCommands};

		auto submitInfo = vk::SubmitInfo()
		                      .setWaitSemaphoreCount(0)
		                      .setPWaitDstStageMask(waitStages)
		                      .setCommandBufferCount(1)
		                      .setPCommandBuffers(&commandBufferHandle.get())
		                      .setSignalSemaphoreCount(0);

		core->queueWorkerManager->GetWorkerQueue(this->queueName)->workerQueue.submit({submitInfo}, nullptr);
		core->queueWorkerManager->GetWorkerQueue(this->queueName)->workerQueue.waitIdle();
	}

	Core                   *core;
	vk::UniqueCommandBuffer commandBufferHandle;
	std::string             queueName = "Graphics";
};

}        // namespace ENGINE

#	define CORE_HPP

#endif        // CORE_HPP
