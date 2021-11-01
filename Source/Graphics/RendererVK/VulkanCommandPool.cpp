#include "Precompiled.hpp"

namespace Plasma
{
    void RendererVK::CreateCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mVulkanRuntimeData.mPhysicalDevice, mVulkanRuntimeData.mSurface);

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.mGraphicsFamily;
        poolInfo.flags = 0; // Optional
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VkResult result = vkCreateCommandPool(mVulkanRuntimeData.mDevice, &poolInfo, nullptr, &mVulkanRuntimeData.mCommandPool);
        Assert(result == VK_SUCCESS, ResultToError(result).c_str());
    }
}