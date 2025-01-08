//
// Created by carlo on 2024-09-22.
//

#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX

#include <cstdint>
#include <algorithm>
#include <list>
#include <set>
#include<deque>
#include <random>



#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include <spirv_glsl.hpp>

#include <ProfilerTask.h>
#include <vulkan/vulkan.hpp>

#include "Structs.hpp"
#include "UtilVk.hpp"
#include "Buffer.hpp"
#include "StagedBuffer.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Sampler.hpp"
#include "SyncronizationPatterns.hpp"
#include "DescriptorAllocator.hpp"
#include "Descriptors.hpp"
#include "ShaderModule.hpp"
#include "ShaderParser.hpp"
#include "VertexInput.hpp"
#include "Surface.hpp"
#include "SwapChain.hpp"
#include "DynamicRenderPass.hpp"
#include "Pipeline.hpp"

#include "Profiler.hpp"
#include "Core.hpp"
#include "RenderGraph.hpp"
#include "CoreImpl.hpp"

#include "PresentQueue.hpp"
#include "ImageShipper.hpp"
#include "ResourcesManager.hpp"
#include "DescriptorCache.hpp"
