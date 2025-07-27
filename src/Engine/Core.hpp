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
	vk::UniqueCommandPool               commandPool;
	vk::Queue                           graphicsQueue;
	vk::Queue                           presentQueue;
	bool                                resizeRequested = false;
	RenderGraph                        *renderGraphRef;
	SwapChain                          *swapchainRef;
	std::deque<std::function<void()> *> deletionQueue;
	std::unique_ptr<QueueWorkerManager> queueWorkerManager;
	QueueFamilyIndices                  queueFamilyIndices;

  private:
	static vk::UniqueInstance CreateInstance(const std::vector<const char *> &instanceExtensions,
	                                         const std::vector<const char *> &validationLayers);

	static vk::PhysicalDevice FindPhysicalDevice(vk::Instance instance);

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

class ExecuteOnceCommand
{
  public:
	ExecuteOnceCommand(Core *core)
	{
		this->core          = core;
		commandBufferHandle = std::move(core->AllocateCommandBuffers(core->commandPool.get(), 1)[0]);
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

		core->graphicsQueue.submit({submitInfo}, nullptr);
		core->graphicsQueue.waitIdle();
	}

	Core                   *core;
	vk::UniqueCommandBuffer commandBufferHandle;
};

}        // namespace ENGINE

#	define CORE_HPP

#endif        // CORE_HPP
