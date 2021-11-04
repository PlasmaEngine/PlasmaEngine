#pragma once

namespace Plasma
{
    struct VulkanBufferData : BufferRenderData
    {
        VkDeviceSize mSize;
        VkBufferUsageFlags mUsage = 0;
        VkBufferCreateFlags mCreateFlags = 0;
        VkSharingMode mSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    };

    struct VulkanBufferCreationInfo
    {
        VkDevice mDevice = VK_NULL_HANDLE;
        VulkanBufferData mBufferData;
    };

    //-------------------------------------------------------------------VulkanBuffer
    class VulkanBuffer
    {
    public:
        VulkanBuffer(const VulkanBufferCreationInfo& creationInfo);
        ~VulkanBuffer();

        void Free();

        VkBuffer GetVulkanBuffer() const;
        VulkanBufferCreationInfo GetInfo() const;

    private:
        VulkanBufferCreationInfo mInfo;
        VkBuffer mBuffer = VK_NULL_HANDLE;
    };

}