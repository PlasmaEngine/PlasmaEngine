#include "Precompiled.hpp"

namespace Plasma
{
    VulkanImage::VulkanImage(const VulkanImageCreationInfo& creationInfo)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = creationInfo.mImageData.mType;
        imageInfo.extent.width = creationInfo.mImageData.mWidth;
        imageInfo.extent.height = creationInfo.mImageData.mHeight;
        imageInfo.extent.depth = creationInfo.mImageData.mDepth;
        imageInfo.mipLevels = creationInfo.mImageData.mMipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = creationInfo.mImageData.mFormat;
        imageInfo.tiling = creationInfo.mImageData.mTiling;
        imageInfo.initialLayout = creationInfo.mImageData.mInitialLayout;
        imageInfo.usage = creationInfo.mImageData.mUsage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        if (vkCreateImage(creationInfo.mDevice, &imageInfo, nullptr, &mImage) != VK_SUCCESS)
            AlwaysError("failed to create image!");

        mInfo = creationInfo;
    }

    VulkanImage::VulkanImage(VkImage image, const VulkanImageCreationInfo& creationInfo)
    {
        mImage = image;
        mInfo = creationInfo;
    }

    VulkanImage::~VulkanImage()
    {
        Free();
    }

    void VulkanImage::Free()
    {
        if (mInfo.mDevice != VK_NULL_HANDLE)
            vkDestroyImage(mInfo.mDevice, mImage, nullptr);

        Clear();
    }

    void VulkanImage::Clear()
    {
        mInfo = VulkanImageCreationInfo();
        mImage = VK_NULL_HANDLE;
    }

    uint32_t VulkanImage::GetWidth() const
    {
        return mInfo.mImageData.mWidth;
    }

    uint32_t VulkanImage::GetHeight() const
    {
        return mInfo.mImageData.mHeight;
    }

    VkFormat VulkanImage::GetInitialFormat() const
    {
        return mInfo.mImageData.mFormat;
    }

    VkImage VulkanImage::GetVulkanImage() const
    {
        return mImage;
    }

    VulkanImageCreationInfo VulkanImage::GetCreationInfo() const
    {
        return mInfo;
    }

}