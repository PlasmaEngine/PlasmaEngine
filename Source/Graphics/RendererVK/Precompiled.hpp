#pragma once

#include "CommonStandard.hpp"
#include "SupportStandard.hpp"
#include "RendererVKStandard.hpp"

#if defined(PLASMA_PLATFORM_WINDOWS)
    #define VK_USE_PLATFORM_WIN32_KHR
#else PLASMA_PLATFORM_WINDOWS
    #include "SDL.h"
#endif

#define VK_NO_PROTOTYPES
#include "VulkanHeaders/vulkan.h"
#include "Volk/volk.h"
#include "VKMemory/vk_mem_alloc.h"

#include "VulkanRenderer.hpp"