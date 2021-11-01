#include "Precompiled.hpp"

namespace Plasma
{
    void RendererVK::CreateSyncObjects()
    {
        mVulkanRuntimeData.mSyncObjects.mImageAvailableSemaphores.Resize(mVulkanRuntimeData.mMaxFramesInFlight);
        mVulkanRuntimeData.mSyncObjects.mRenderFinishedSemaphores.Resize(mVulkanRuntimeData.mMaxFramesInFlight);
        mVulkanRuntimeData.mSyncObjects.mInFlightFences.Resize(mVulkanRuntimeData.mMaxFramesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < mVulkanRuntimeData.mMaxFramesInFlight; i++)
        {
            bool success = true;
            success |= vkCreateSemaphore(mVulkanRuntimeData.mDevice, &semaphoreInfo, nullptr, &mVulkanRuntimeData.mSyncObjects.mImageAvailableSemaphores[i]) != VK_SUCCESS;
            success |= vkCreateSemaphore(mVulkanRuntimeData.mDevice, &semaphoreInfo, nullptr, &mVulkanRuntimeData.mSyncObjects.mRenderFinishedSemaphores[i]) != VK_SUCCESS;
            success |= vkCreateFence(mVulkanRuntimeData.mDevice, &fenceInfo, nullptr, &mVulkanRuntimeData.mSyncObjects.mInFlightFences[i]) != VK_SUCCESS;
            if (!success)
                AlwaysError("[Vulkan] Faield to create semaphores");
        }
    }
}