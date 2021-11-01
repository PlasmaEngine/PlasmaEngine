#include "Precompiled.hpp"

#define VMA_IMPLEMENTATION
#include "VKMemory/vk_mem_alloc.h"

namespace Plasma
{
    VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanMemoryAllocatorCreationInfo& creationInfo) : mMemoryAllocatorInfo(creationInfo)
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
        allocatorInfo.physicalDevice = mMemoryAllocatorInfo.mPhysicalDevice;
        allocatorInfo.device = mMemoryAllocatorInfo.mDevice;
        allocatorInfo.instance = mMemoryAllocatorInfo.mIntance;

        vmaCreateAllocator(&allocatorInfo, &mAllocator);
    }

    VulkanMemoryAllocator::~VulkanMemoryAllocator()
    {
        vmaDestroyAllocator(mAllocator);
    }

}