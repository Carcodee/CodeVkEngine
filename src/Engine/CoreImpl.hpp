//
// Created by carlo on 2024-09-22.
//





#ifndef COREIMPL_HPP
#define COREIMPL_HPP

#include <set>

namespace ENGINE
{
    Core::Core(const char** instanceExtensions, uint8_t instanceExtensionsCount, WindowDesc* compatibleWindowDesc,
               bool enableDebugging)
    {
        std::vector<const char*>resInstanceExtensions(instanceExtensions, instanceExtensions + instanceExtensionsCount);
        std::vector<const char*> validationLayers;
        if (enableDebugging)
        {
            validationLayers.push_back("VK_LAYER_KHRONOS_validation");
            // validationLayers.push_back("VK_LAYER_LUNARG_api_dump");
            resInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        this->instance = CreateInstance(resInstanceExtensions, validationLayers);
        
        loader = vk::DispatchLoaderDynamic(instance.get(), vkGetInstanceProcAddr);
        loader.init();
        
        auto properties = vk::enumerateInstanceLayerProperties();

        if (enableDebugging)
        {
            this->debugUtilsMessenger = CreateDebugUtilsMessenger(instance.get(), DebugMessageCallback, loader);
        }
        this->physicalDevice = FindPhysicalDevice(instance.get());

        if (compatibleWindowDesc)
        {
            vk::UniqueSurfaceKHR compatibleSurface;
            compatibleSurface = CreateWin32Surface(instance.get(), *compatibleWindowDesc);
            this->queueFamilyIndices = FindQueueFamilyIndices(physicalDevice, compatibleSurface.get());
        }
        std::vector<const char*> deviceExtensions;
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        deviceExtensions.push_back(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
        this->logicalDevice = CreateLogicalDevice(this->physicalDevice, this->queueFamilyIndices, deviceExtensions,
                                                  validationLayers);
        this->graphicsQueue = GetDeviceQueue(this->logicalDevice.get(), queueFamilyIndices.graphicsFamilyIndex);
        this->presentQueue = GetDeviceQueue(this->logicalDevice.get(), queueFamilyIndices.presentFamilyIndex);
        this->commandPool = CreateCommandPool(this->logicalDevice.get(), queueFamilyIndices.graphicsFamilyIndex);
        
        for (auto& property : properties)
        {
            std::cout << "Layer Prop: " << property.layerName << "\n";
        }

        // std::cout << "Supported extensions:\n";
        // auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
        // for (auto extension : extensions)
        // {
        //     std::cout << "  " << extension.extensionName << "\n";
        // }
        
    }

    Core::~Core()
    {
        for (auto& func : deletionQueue)
        {
            (*func)();
        }
        deletionQueue.clear();
        
    }

    void Core::ClearCaches()
    {
    }

    std::unique_ptr<SwapChain> Core::CreateSwapchain(vk::PresentModeKHR presentModeKHR, uint32_t imageCount,
                                                     WindowDesc windowDesc, glm::uvec2 windowSize)
    {
        auto swapChain = std::unique_ptr<SwapChain>(new SwapChain(this->instance.get(), this->physicalDevice,
                                                                  this->logicalDevice.get(), windowDesc
                                                                  , imageCount, this->queueFamilyIndices,
                                                                  presentModeKHR, windowSize));
        this->swapchainRef = swapChain.get();
        return swapChain;
    }

    std::unique_ptr<RenderGraph> Core::CreateRenderGraph()
    {
        auto renderGraph = std::make_unique<RenderGraph>(this);

        this->renderGraphRef = renderGraph.get();

        return renderGraph;
    }


    std::vector<vk::UniqueCommandBuffer> Core::AllocateCommandBuffers(size_t count)
    {
            auto commandBufferAllocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(commandPool.get())
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(uint32_t(count));

            return logicalDevice->allocateCommandBuffersUnique(commandBufferAllocInfo); 
    }

    vk::UniqueSemaphore Core::CreateVulkanSemaphore()
    {
        auto semaphoreInfo = vk::SemaphoreCreateInfo();
        return logicalDevice->createSemaphoreUnique(semaphoreInfo);
        
    }
    
    vk::UniqueFence Core::CreateFence(bool state)
    {
        auto fenceInfo = vk::FenceCreateInfo();
        if (state)
        {
            fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        }
        return logicalDevice->createFenceUnique(fenceInfo);
    }

    void Core::WaitForFence(vk::Fence fence)
    {
       auto res= logicalDevice->waitForFences({fence} , true, std::numeric_limits<uint64_t>::max());
    }
    void Core::ResetFence(vk::Fence fence)
    {
        logicalDevice->resetFences({fence});
    }

    void Core::WaitIdle()
    {
        if (this->logicalDevice)
        {
            this->logicalDevice->waitIdle();
        }else
        {
            std::cout << "Unable to wait idle without a device\n";
        }
    }


    vk::UniqueInstance Core::CreateInstance(const std::vector<const char*>& instanceExtensions,
                                            const std::vector<const char*>& validationLayers)
    {
        auto appInfo = vk::ApplicationInfo()
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
        auto instanceCreateInfo = vk::InstanceCreateInfo()
                                  .setPApplicationInfo(&appInfo)
                                  .setEnabledExtensionCount(uint32_t(instanceExtensions.size()))
                                  .setPpEnabledExtensionNames(instanceExtensions.data())
                                  .setEnabledLayerCount(uint32_t(validationLayers.size()))
                                  .setPpEnabledLayerNames(validationLayers.data());
                                  // .setPNext(&valFeaturesPNext);
        

        return vk::createInstanceUnique(instanceCreateInfo);
    }

    vk::PhysicalDevice Core::FindPhysicalDevice(vk::Instance instance)
    {
        std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
        std::cout << "Found " << physicalDevices.size() << " physical device(s)\n";
        vk::PhysicalDevice physicalDevice = nullptr;
        for (const auto& device : physicalDevices)
        {
            vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
            std::cout << "Physical device found: " << deviceProperties.deviceName;
            vk::PhysicalDeviceFeatures physicalDeviceFeatures = device.getFeatures();
            if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
                VK_VERSION_MAJOR(deviceProperties.apiVersion)>= 1 &&
                VK_VERSION_MINOR(deviceProperties.apiVersion)>= 3)
            {
                physicalDevice = device;
                std::cout << "<--- Using this device";
            }
            std::cout << "\n";
            std::cout << "API Version: " << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
                << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
                << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
        }
        assert((physicalDevice != nullptr) && "Failed to find physical device");
        return physicalDevice;
    }

    QueueFamilyIndices Core::FindQueueFamilyIndices(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
    {
        std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
        QueueFamilyIndices queueFamilyIndices;
        queueFamilyIndices.graphicsFamilyIndex = uint32_t(-1);
        queueFamilyIndices.presentFamilyIndex = uint32_t(-1);
        for (int familyIndex = 0; familyIndex < queueFamilies.size(); ++familyIndex)
        {
            if (queueFamilies[familyIndex].queueFlags & vk::QueueFlagBits::eGraphics && queueFamilies[familyIndex].
                queueCount > 0 && queueFamilyIndices.graphicsFamilyIndex == uint32_t(-1))
            {
                queueFamilyIndices.graphicsFamilyIndex = familyIndex;
            }
            if (physicalDevice.getSurfaceSupportKHR(familyIndex, surface) && queueFamilies[familyIndex].queueCount > 0
                && queueFamilyIndices.presentFamilyIndex == uint32_t(-1))
            {
                queueFamilyIndices.presentFamilyIndex = familyIndex;
            }
        }
        assert(queueFamilyIndices.graphicsFamilyIndex != uint32_t(-1) && "Failed to find appropiate queue families");
        assert(queueFamilyIndices.presentFamilyIndex != uint32_t(-1) && "Failed to find appropiate queue families");

        return queueFamilyIndices;
    }


    vk::UniqueDevice Core::CreateLogicalDevice(vk::PhysicalDevice physicalDevice, QueueFamilyIndices familyIndices,
                                               std::vector<const char*> deviceExtensions,
                                               std::vector<const char*> validationLayers)
    {
        std::pmr::set<uint32_t> uniqueQueueFamilyIndices = {
            familyIndices.graphicsFamilyIndex, familyIndices.presentFamilyIndex
        };
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilyIndices)
        {
            auto queueCreateInfo = vk::DeviceQueueCreateInfo()
                                   .setQueueFamilyIndex(queueFamily)
                                   .setQueueCount(1)
                                   .setPQueuePriorities(&queuePriority);

            queueCreateInfos.push_back(queueCreateInfo);
        }
        auto deviceFeatures = vk::PhysicalDeviceFeatures()
                              .setFragmentStoresAndAtomics(true)
                              .setVertexPipelineStoresAndAtomics(true)
                              .setMultiDrawIndirect(true)
                              .setWideLines(true)
                              .setFillModeNonSolid(true);

        auto deviceCreateInfo = vk::DeviceCreateInfo()
                                .setQueueCreateInfoCount(uint32_t(queueCreateInfos.size()))
                                .setPQueueCreateInfos(queueCreateInfos.data())
                                .setPEnabledFeatures(&deviceFeatures)
                                .setEnabledExtensionCount(uint32_t(deviceExtensions.size()))
                                .setPpEnabledExtensionNames(deviceExtensions.data())
                                .setEnabledLayerCount(uint32_t(validationLayers.size()))
                                .setPpEnabledLayerNames(validationLayers.data());

        auto deviceFeatures12 = vk::PhysicalDeviceVulkan12Features()
            .setScalarBlockLayout(true)
            .setDescriptorIndexing(true)
            .setRuntimeDescriptorArray(true)
            .setDescriptorBindingPartiallyBound(true);

        auto deviceFeatures13 = vk::PhysicalDeviceVulkan13Features()
        .setDynamicRendering(true);

        vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features> chain = {
            deviceCreateInfo, deviceFeatures12, deviceFeatures13
        };
        return physicalDevice.createDeviceUnique(chain.get<vk::DeviceCreateInfo>());
    }


    vk::UniqueCommandPool Core::CreateCommandPool(vk::Device logicalDevice, uint32_t familyIndex)
    {
        auto commandPoolInfo = vk::CommandPoolCreateInfo()
                               .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                               .setQueueFamilyIndex(familyIndex);

        return logicalDevice.createCommandPoolUnique(commandPoolInfo);
    }

    vk::Queue Core::GetDeviceQueue(vk::Device logicalDevice, uint32_t familyIndex)
    {
        return logicalDevice.getQueue(familyIndex, 0);
    }

    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> Core::CreateDebugUtilsMessenger(vk::Instance instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, vk::DispatchLoaderDynamic& loader)
    {
        auto messengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
        .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning| vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
        .setPfnUserCallback(debugCallback)
        .setPUserData(nullptr);

        return instance.createDebugUtilsMessengerEXTUnique(messengerCreateInfo, nullptr, loader);
        
    }


    VKAPI_ATTR VkBool32 VKAPI_CALL Core::DebugMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
}

#endif //COREIMPL_HPP
