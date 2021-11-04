#include "Precompiled.hpp"

namespace Plasma
{
    void RendererVK::CreateMemoryAllocator()
    {
        VulkanMemoryAllocatorCreationInfo memoryCreationInfo;
        memoryCreationInfo.mPhysicalDevice = mVulkanRuntimeData.mPhysicalDevice;
        memoryCreationInfo.mDevice = mVulkanRuntimeData.mDevice;
        memoryCreationInfo.mGraphicsQueue = mVulkanRuntimeData.mQueueData.mGraphicsQueue;
        memoryCreationInfo.mCommandPool = mVulkanRuntimeData.mCommandPool;
        mVulkanRuntimeData.mAllocator = new VulkanMemoryAllocator(memoryCreationInfo);
    }

    VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanMemoryAllocatorCreationInfo& creationInfo)
    {
        mPhysicalDevice = creationInfo.mPhysicalDevice;
        mDevice = creationInfo.mDevice;
        mGraphicsQueue = creationInfo.mGraphicsQueue;
        mCommandPool = creationInfo.mCommandPool;
    }

    VulkanMemoryAllocator::~VulkanMemoryAllocator()
    {
        FreeAllAllocations();
    }

    VkDeviceMemory VulkanMemoryAllocator::AllocateImageMemory(const VulkanImage* image, bool transient)
    {
        VulkanImageCreationInfo imageInfo = image->GetCreationInfo();

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(mDevice, image->GetVulkanImage(), &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        FindMemoryType(mPhysicalDevice, memRequirements.memoryTypeBits, imageInfo.mImageData.mProperties, allocInfo.memoryTypeIndex);

        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
        if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            AlwaysError("failed to allocate image memory!");

        if (!transient)
            mAllocations[(void*)image] = imageMemory;

        return imageMemory;
    }

    VkDeviceMemory VulkanMemoryAllocator::AllocateBufferMemory(const VulkanBuffer* buffer, VkMemoryPropertyFlags properties, bool transient)
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(mDevice, buffer->GetVulkanBuffer(), &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        FindMemoryType(mPhysicalDevice, memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex);

        VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
        if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            AlwaysError("failed to allocate vertex buffer memory!");

        if (!transient)
            mAllocations[(void*)buffer] = bufferMemory;

        return bufferMemory;
    }

    void VulkanMemoryAllocator::FreeAllocation(void* key)
    {
        VkDeviceMemory* allocation = mAllocations.FindPointer(key);
        if (allocation == nullptr)
            return;

        FreeAllocation(*allocation);
        mAllocations.Erase(key);
    }

    void VulkanMemoryAllocator::FreeAllocation(VkDeviceMemory memory)
    {
        vkFreeMemory(mDevice, memory, nullptr);
    }

    void VulkanMemoryAllocator::FreeAllAllocations()
    {
        for (VkDeviceMemory& allocation : mAllocations.Values())
            FreeAllocation(allocation);
        mAllocations.Clear();
    }
    Status VulkanMemoryAllocator::FindMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties, uint32& outMemoryType)

    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32 i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                outMemoryType = i;
                return Status();
            }
        }

        return Status(StatusState::Failure, "failed to find suitable memory type!");
    }
}