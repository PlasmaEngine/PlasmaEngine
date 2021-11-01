#pragma  once

namespace Plasma
{
    struct VulkanMemoryAllocatorCreationInfo
    {
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VkDevice mDevice = VK_NULL_HANDLE;
        VkInstance mIntance = VK_NULL_HANDLE;
    };

    class VulkanMemoryAllocator
    {
    public:
        VulkanMemoryAllocator(VulkanMemoryAllocatorCreationInfo& creationInfo);
        ~VulkanMemoryAllocator();

    private:
        VulkanMemoryAllocatorCreationInfo mMemoryAllocatorInfo;
        VmaAllocator mAllocator;
    };
}