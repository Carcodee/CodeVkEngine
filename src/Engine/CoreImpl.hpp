//

// Created by carlo on 2024-09-22.
//

#ifndef CORE_IMPL_HPP
#define CORE_IMPL_HPP

#include <set>

namespace engine
{
engine::Core::Core(const char **instance_extensions, uint8_t instance_extensions_count, Window_Desc *compatible_window_desc,
           bool enable_debugging)
{
	std::vector<const char *> res_instance_extensions(instance_extensions, instance_extensions + instance_extensions_count);
	std::vector<const char *> validation_layers;
	res_instance_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
	res_instance_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);

	if (enable_debugging)
	{
		validation_layers.push_back("VK_LAYER_KHRONOS_validation");
		// validationLayers.push_back("VK_LAYER_LUNARG_api_dump");
		res_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	this->instance = Create_Instance(res_instance_extensions, validation_layers);

	loader = Vulkan_Dispatch_Loader_Dynamic(instance.get(), vkGetInstanceProcAddr);
	loader.init();

	auto properties = vk::enumerateInstanceLayerProperties();

	if (enable_debugging)
	{
		this->debug_utils_messenger = Create_Debug_Utils_Messenger(instance.get(), Debug_Message_Callback, loader);
	}
	this->physical_device = Find_Physical_Device(instance.get());
	this->device_uuid     = Find_Physical_Device_Uuid(this->physical_device);

	if (compatible_window_desc)
	{
		vk::UniqueSurfaceKHR compatible_surface;
		compatible_surface        = Create_Win32_Surface(instance.get(), *compatible_window_desc);
		this->queue_family_ids = Find_Queue_Family_Indices(physical_device, compatible_surface.get());
	}
	std::vector<const char *> device_extensions;
	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	device_extensions.push_back(VK_EXT_MULTI_DRAW_EXTENSION_NAME);

	device_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
	device_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);

	device_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
	device_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);

	this->logical_device      = Create_Logical_Device(this->physical_device, this->queue_family_ids, device_extensions,
	                                               validation_layers);
	// this->presentation_queue       = Get_Device_Queue(this->logical_device.get(), queue_family_ids.present_family_index);
	
	this->queue_worker_manager = std::make_unique<engine::Queue_Worker_Manager>(this);
	this->queue_worker_manager->Get_Or_Create_Worker_Queue("Present", queue_family_ids.present_family_index);
	this->queue_worker_manager->Get_Or_Create_Worker_Queue("Graphics", queue_family_ids.graphics_family_index);
	this->queue_worker_manager->Get_Or_Create_Worker_Queue("Compute", queue_family_ids.compute_family_index);
	this->queue_worker_manager->Get_Or_Create_Worker_Queue("Graphics_Test", queue_family_ids.graphics_family_index);
	this->queue_worker_manager->Get_Or_Create_Worker_Queue("UI", queue_family_ids.graphics_family_index);
	this->queue_worker_manager->Get_Or_Create_Worker_Queue("CUDA", queue_family_ids.compute_family_index, true, true);

	for (auto &property : properties)
	{
		std::cout << "Layer Prop: " << property.layerName << "\n";
	}

	// std::cout << "Supported extensions:\n";
	// auto extensions = physical_device.enumerateDeviceExtensionProperties();
	// for (auto extension : extensions)
	// {
	//     std::cout << "  " << extension.extensionName << "\n";
	// }
}

engine::Core::~Core()
{
	for (auto &func : deletion_queue)
	{
		(*func)();
	}
	deletion_queue.clear();
}

void engine::Core::Clear_Caches()
{
}

std::unique_ptr<engine::Swap_Chain> engine::Core::Create_Swapchain(vk::PresentModeKHR present_mode_khr, uint32_t image_count,
                                                 Window_Desc window_desc, glm::uvec2 window_size)
{
	auto swap_chain     = std::unique_ptr<engine::Swap_Chain>(new engine::Swap_Chain(this->instance.get(), this->physical_device,
	                                                              this->logical_device.get(), window_desc, image_count, this->queue_family_ids,
	                                                              present_mode_khr, window_size));
	this->swapchain_ref = swap_chain.get();
	return swap_chain;
}

std::unique_ptr<engine::Render_Graph> engine::Core::Create_Render_Graph()
{
	auto render_graph = std::make_unique<engine::Render_Graph>(this);

	this->render_graph_ref = render_graph.get();

	return render_graph;
}

std::vector<vk::UniqueCommandBuffer> engine::Core::Allocate_Command_Buffers(vk::CommandPool command_pool_in, size_t count)
{
	auto command_buffer_alloc_info = vk::CommandBufferAllocateInfo()
	                                  .setCommandPool(command_pool_in)
	                                  .setLevel(vk::CommandBufferLevel::ePrimary)
	                                  .setCommandBufferCount(uint32_t(count));

	return logical_device->allocateCommandBuffersUnique(command_buffer_alloc_info);
}

std::vector<vk::UniqueCommandBuffer> engine::Core::Allocate_Command_Buffers_Secondary(vk::CommandPool command_pool_in, size_t count)
{
	auto command_buffer_alloc_info = vk::CommandBufferAllocateInfo()
	                                  .setCommandPool(command_pool_in)
	                                  .setLevel(vk::CommandBufferLevel::eSecondary)
	                                  .setCommandBufferCount(uint32_t(count));

	return logical_device->allocateCommandBuffersUnique(command_buffer_alloc_info);
}
vk::UniqueSemaphore engine::Core::Create_Vulkan_Semaphore()
{
	auto semaphore_info = vk::SemaphoreCreateInfo();
	return logical_device->createSemaphoreUnique(semaphore_info);
}

vk::UniqueSemaphore engine::Core::Create_Exportable_Binary_Semaphore(HANDLE& handle_out)
{
	auto semaphore_type_info          = vk::SemaphoreTypeCreateInfo();
	semaphore_type_info.semaphoreType = vk::SemaphoreType::eBinary;
	
	vk::ExportSemaphoreCreateInfo export_info{};
	export_info.handleTypes =
		vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
	
	semaphore_type_info.pNext = &export_info;

	auto semaphore_info  = vk::SemaphoreCreateInfo();
	semaphore_info.pNext = &semaphore_type_info;
	auto binary_sem = logical_device->createSemaphoreUnique(semaphore_info);
	
	vk::SemaphoreGetWin32HandleInfoKHR handle_info{};
	handle_info.semaphore = binary_sem.get();
	handle_info.handleType =
		vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;

	handle_out = logical_device->getSemaphoreWin32HandleKHR(handle_info, loader);
	return binary_sem;
}

vk::UniqueSemaphore engine::Core::Create_Vulkan_Timeline_Semaphore(uint32_t initial_value)
{
	auto semaphore_type_info          = vk::SemaphoreTypeCreateInfo();
	semaphore_type_info.semaphoreType = vk::SemaphoreType::eTimeline;
	semaphore_type_info.initialValue  = initial_value;

	auto semaphore_info  = vk::SemaphoreCreateInfo();
	semaphore_info.pNext = &semaphore_type_info;
	return logical_device->createSemaphoreUnique(semaphore_info);
}

vk::UniqueSemaphore engine::Core::Create_Exportable_Timeline_Semaphore(uint32_t initial_value, HANDLE& handle_out)
{
	auto semaphore_type_info          = vk::SemaphoreTypeCreateInfo();
	semaphore_type_info.semaphoreType = vk::SemaphoreType::eTimeline;
	semaphore_type_info.initialValue  = initial_value;
	
	vk::ExportSemaphoreCreateInfo export_info{};
	export_info.handleTypes =
		vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
	
	semaphore_type_info.pNext = &export_info;

	auto semaphore_info  = vk::SemaphoreCreateInfo();
	semaphore_info.pNext = &semaphore_type_info;
	auto timeline = logical_device->createSemaphoreUnique(semaphore_info);
	
	vk::SemaphoreGetWin32HandleInfoKHR handle_info{};
	handle_info.semaphore = timeline.get();
	handle_info.handleType =
		vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;

	handle_out = logical_device->getSemaphoreWin32HandleKHR(handle_info, loader);

	return timeline;
}
vk::UniqueFence engine::Core::Create_Fence(bool state)
{
	auto fence_info = vk::FenceCreateInfo();
	if (state)
	{
		fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled);
	}
	return logical_device->createFenceUnique(fence_info);
}

void engine::Core::Wait_For_Fence(vk::Fence fence)
{
	auto res = logical_device->waitForFences({fence}, true, std::numeric_limits<uint64_t>::max());
}
void engine::Core::Reset_Fence(vk::Fence fence)
{
	logical_device->resetFences({fence});
}

void engine::Core::Wait_Idle()
{
	if (this->logical_device)
	{
		this->logical_device->waitIdle();
	}
	else
	{
		std::cout << "Unable to wait idle without a device\n";
	}
}

vk::UniqueInstance engine::Core::Create_Instance(const std::vector<const char *> &instance_extensions,
                                        const std::vector<const char *> &validation_layers)
{
	auto app_info = vk::ApplicationInfo()
	                   .setPApplicationName("Vulkan Template App")
	                   .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
	                   .setPEngineName("Vulkan Template Engine")
	                   .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
	                   .setApiVersion(VK_API_VERSION_1_3);

	// std::vector validationFeaturesEnable = {vk::ValidationFeatureEnableEXT::eDebugPrintf};
	// auto valFeaturesPNext = vk::ValidationFeaturesEXT()
	// .setEnabledValidationFeatureCount((uint32_t)validationFeaturesEnable.size())
	// .setPEnabledValidationFeatures(validationFeaturesEnable.data());
	//
	auto instance_create_info = vk::InstanceCreateInfo()
	                              .setPApplicationInfo(&app_info)
	                              .setEnabledExtensionCount(uint32_t(instance_extensions.size()))
	                              .setPpEnabledExtensionNames(instance_extensions.data())
	                              .setEnabledLayerCount(uint32_t(validation_layers.size()))
	                              .setPpEnabledLayerNames(validation_layers.data());
	// .setPNext(&valFeaturesPNext);

	return vk::createInstanceUnique(instance_create_info);
}

std::array<uint8_t, VK_UUID_SIZE> engine::Core::Find_Physical_Device_Uuid(vk::PhysicalDevice physical_device_in)
{
	vk::PhysicalDeviceIDProperties physical_device_id_properties{};

	vk::PhysicalDeviceProperties2 physical_device_properties2 = {};

	physical_device_properties2.pNext = &physical_device_id_properties;

	physical_device_in.getProperties2(&physical_device_properties2);
	
	std::array<uint8_t, VK_UUID_SIZE> uuid {};

	std::memcpy(uuid.data(), physical_device_id_properties.deviceUUID.data(), VK_UUID_SIZE);
	return uuid;
}
vk::PhysicalDevice engine::Core::Find_Physical_Device(vk::Instance instance)
{
	std::vector<vk::PhysicalDevice> physical_devices = instance.enumeratePhysicalDevices();
	std::cout << "Found " << physical_devices.size() << " physical device(s)\n";
	vk::PhysicalDevice physical_device = nullptr;
	for (const auto &device : physical_devices)
	{
		vk::PhysicalDeviceProperties device_properties = device.getProperties();
		std::cout << "Physical device found: " << device_properties.deviceName;
		vk::PhysicalDeviceFeatures physical_device_features = device.getFeatures();
		if (device_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
		    VK_VERSION_MAJOR(device_properties.apiVersion) >= 1 &&
		    VK_VERSION_MINOR(device_properties.apiVersion) >= 3)
		{
			physical_device = device;
			std::cout << "<--- Using this device";
		}
		std::cout << "\n";
		std::cout << "API Version: " << VK_VERSION_MAJOR(device_properties.apiVersion) << "."
		          << VK_VERSION_MINOR(device_properties.apiVersion) << "."
		          << VK_VERSION_PATCH(device_properties.apiVersion) << std::endl;
	}
	assert((physical_device != nullptr) && "Failed to find physical device");
	return physical_device;
}

engine::Queue_Family_Indices engine::Core::Find_Queue_Family_Indices(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
{
	std::vector<vk::QueueFamilyProperties> queue_families = physical_device.getQueueFamilyProperties();
	engine::Queue_Family_Indices                     queue_family_ids;
	queue_family_ids.transfer_family_index = uint32_t(-1);
	queue_family_ids.graphics_family_index = uint32_t(-1);
	queue_family_ids.present_family_index  = uint32_t(-1);
	queue_family_ids.compute_family_index  = uint32_t(-1);
	for (int family_index = 0; family_index < queue_families.size(); ++family_index)
	{
		if (queue_families[family_index].queueFlags & vk::QueueFlagBits::eTransfer && queue_families[family_index].queueCount > 0 && queue_family_ids.graphics_family_index == uint32_t(-1))
		{
			queue_family_ids.transfer_family_index = family_index;
		}
		if (queue_families[family_index].queueFlags & vk::QueueFlagBits::eGraphics && queue_families[family_index].queueCount > 0 && queue_family_ids.graphics_family_index == uint32_t(-1))
		{
			queue_family_ids.graphics_family_index = family_index;
		}
		if (queue_families[family_index].queueFlags & vk::QueueFlagBits::eCompute && queue_families[family_index].queueCount > 0 && queue_family_ids.compute_family_index == uint32_t(-1))
		{
			queue_family_ids.compute_family_index = family_index;
		}
		if (physical_device.getSurfaceSupportKHR(family_index, surface) && queue_families[family_index].queueCount > 0 && queue_family_ids.present_family_index == uint32_t(-1))
		{
			queue_family_ids.present_family_index = family_index;
		}
	}
	assert(queue_family_ids.transfer_family_index != uint32_t(-1) && "Failed to find appropiate queue families");
	assert(queue_family_ids.graphics_family_index != uint32_t(-1) && "Failed to find appropiate queue families");
	assert(queue_family_ids.present_family_index != uint32_t(-1) && "Failed to find appropiate queue families");

	return queue_family_ids;
}

vk::UniqueDevice engine::Core::Create_Logical_Device(vk::PhysicalDevice physical_device, engine::Queue_Family_Indices family_indices,
                                           std::vector<const char *> device_extensions,
                                           std::vector<const char *> validation_layers)
{
	std::pmr::set<uint32_t> unique_queue_family_indices = {
	    family_indices.graphics_family_index, family_indices.present_family_index, family_indices.transfer_family_index};
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
	float                                  queue_priority = 1.0f;
	for (uint32_t queue_family : unique_queue_family_indices)
	{
		auto queue_create_info = vk::DeviceQueueCreateInfo()
		                           .setQueueFamilyIndex(queue_family)
		                           .setQueueCount(1)
		                           .setPQueuePriorities(&queue_priority);

		queue_create_infos.push_back(queue_create_info);
	}
	auto device_features = vk::PhysicalDeviceFeatures()
	                          .setFragmentStoresAndAtomics(true)
	                          .setVertexPipelineStoresAndAtomics(true)
	                          .setMultiDrawIndirect(true)
	                          .setWideLines(true)
	                          .setFillModeNonSolid(true)
	                          .setTessellationShader(true)
	                          .setGeometryShader(true);

	auto device_create_info = vk::DeviceCreateInfo()
	                            .setQueueCreateInfoCount(uint32_t(queue_create_infos.size()))
	                            .setPQueueCreateInfos(queue_create_infos.data())
	                            .setPEnabledFeatures(&device_features)
	                            .setEnabledExtensionCount(uint32_t(device_extensions.size()))
	                            .setPpEnabledExtensionNames(device_extensions.data())
	                            .setEnabledLayerCount(uint32_t(validation_layers.size()))
	                            .setPpEnabledLayerNames(validation_layers.data());

	auto device_features12 = vk::PhysicalDeviceVulkan12Features()
	                            .setScalarBlockLayout(true)
	                            .setDescriptorIndexing(true)
	                            .setRuntimeDescriptorArray(true)
	                            .setDescriptorBindingPartiallyBound(true)
	                            .setTimelineSemaphore(true);

	auto device_features13 = vk::PhysicalDeviceVulkan13Features()
	                            .setDynamicRendering(true);

	vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features> chain = {
	    device_create_info, device_features12, device_features13};
	return physical_device.createDeviceUnique(chain.get<vk::DeviceCreateInfo>());
}

vk::UniqueCommandPool engine::Core::Create_Command_Pool(vk::Device logical_device, uint32_t family_index)
{
	auto command_pool_info = vk::CommandPoolCreateInfo()
	                           .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
	                           .setQueueFamilyIndex(family_index);

	return logical_device.createCommandPoolUnique(command_pool_info);
}

vk::Queue engine::Core::Get_Device_Queue(vk::Device logical_device, uint32_t family_index)
{
	return logical_device.getQueue(family_index, 0);
}

vk::UniqueHandle<vk::DebugUtilsMessengerEXT, Vulkan_Dispatch_Loader_Dynamic> engine::Core::Create_Debug_Utils_Messenger(vk::Instance instance, PFN_vkDebugUtilsMessengerCallbackEXT debug_callback, Vulkan_Dispatch_Loader_Dynamic &loader)
{
	auto messenger_create_info = vk::DebugUtilsMessengerCreateInfoEXT()
	                               .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
	                               .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
	                               .setPfnUserCallback(debug_callback)
	                               .setPUserData(nullptr);

	return instance.createDebugUtilsMessengerEXTUnique(messenger_create_info, nullptr, loader);
}

VKAPI_ATTR VkBool32 VKAPI_CALL engine::Core::Debug_Message_Callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
    void                                       *p_user_data)
{
	std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

	return VK_FALSE;
}
}        // namespace engine
#endif        // CORE_IMPL_HPP
