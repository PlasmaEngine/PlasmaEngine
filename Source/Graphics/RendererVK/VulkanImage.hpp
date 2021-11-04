#pragma once

namespace Plasma
{
    struct VulkanTextureData : TextureRenderData
    {
        VkImageType mType = VK_IMAGE_TYPE_2D;
        VkFormat mFormat = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageUsageFlags mUsage = 0;
        VkMemoryPropertyFlags mProperties = 0;
        VkImageTiling mTiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageLayout mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t mWidth = 1;
        uint32_t mHeight = 1;
        uint32_t mDepth = 1;
        uint32_t mMipLevels = 1;
    };

    struct VulkanImageCreationInfo
    {
        VkDevice mDevice = VK_NULL_HANDLE;
        VulkanTextureData mImageData;
    };

    //-------------------------------------------------------------------VulkanImage
    class VulkanImage
    {
    public:
        VulkanImage() {}
        VulkanImage(const VulkanImageCreationInfo& creationInfo);
        VulkanImage(VkImage image, const VulkanImageCreationInfo& creationInfo);

        VulkanImage(VulkanImage&&) = delete;
        VulkanImage& operator=(VulkanImage&&) = delete;
        ~VulkanImage();

        void Free();
        void Clear();

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        VkFormat GetInitialFormat() const;
        VkImage GetVulkanImage() const;
        VulkanImageCreationInfo GetCreationInfo() const;

    private:
        VkImage mImage = VK_NULL_HANDLE;
        VulkanImageCreationInfo mInfo;
    };

}