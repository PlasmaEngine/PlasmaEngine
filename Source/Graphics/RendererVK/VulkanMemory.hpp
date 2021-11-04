#pragma once

namespace Plasma
{
    class VulkanImage;
    class VulkanBuffer;

    struct VulkanMemoryAllocatorCreationInfo
    {
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VkDevice mDevice = VK_NULL_HANDLE;
        VkQueue mGraphicsQueue = VK_NULL_HANDLE;
        VkCommandPool mCommandPool = VK_NULL_HANDLE;
    };

    class VulkanMemoryAllocator
    {
    public:
        VulkanMemoryAllocator(VulkanMemoryAllocatorCreationInfo& creationInfo);
        ~VulkanMemoryAllocator();

        VkDeviceMemory AllocateImageMemory(const VulkanImage* image, bool transient);
        VkDeviceMemory AllocateBufferMemory(const VulkanBuffer* buffer, VkMemoryPropertyFlags properties, bool transient);

        void FreeAllocation(void* key);
        void FreeAllocation(VkDeviceMemory memory);
        void FreeAllAllocations();

    private:

        Status FindMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties, uint32& outMemoryType);

        VkPhysicalDevice mPhysicalDevice;
        VkDevice mDevice = VK_NULL_HANDLE;
        VkQueue mGraphicsQueue = VK_NULL_HANDLE;
        VkCommandPool mCommandPool = VK_NULL_HANDLE;
        HashMap<void*, VkDeviceMemory> mAllocations;
    };
}