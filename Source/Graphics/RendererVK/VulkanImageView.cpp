#include "Precompiled.hpp"

namespace Plasma
{
    //-------------------------------------------------------------------VulkanImageView
    VulkanImageView::VulkanImageView(VkDevice device, VulkanImage* image, const VulkanImageViewInfo& info)
    {
        mDevice = device;

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image->GetVulkanImage();
        viewInfo.viewType = info.mViewType;
        viewInfo.format = info.mFormat;
        viewInfo.components.r = info.mComponents[0];
        viewInfo.components.g = info.mComponents[1];
        viewInfo.components.b = info.mComponents[2];
        viewInfo.components.a = info.mComponents[3];
        viewInfo.subresourceRange.aspectMask = info.mAspectFlags;
        viewInfo.subresourceRange.baseMipLevel = info.mBaseMipLevel;
        viewInfo.subresourceRange.levelCount = info.mMipLevels;
        viewInfo.subresourceRange.baseArrayLayer = info.mBaseArrayLayer;
        viewInfo.subresourceRange.layerCount = info.mLayerCount;

        if (vkCreateImageView(mDevice, &viewInfo, nullptr, &mImageView) != VK_SUCCESS)
            AlwaysError("failed to create texture image view!");

        mImage = image;
        mInfo = info;
    }

    VulkanImageView::~VulkanImageView()
    {
        Free();
    }

    void VulkanImageView::Free()
    {
        vkDestroyImageView(mDevice, mImageView, nullptr);
        mInfo = VulkanImageViewInfo();
        mImageView = VK_NULL_HANDLE;
        mImage = nullptr;
    }

    VulkanImage* VulkanImageView::GetImage() const
    {
        return mImage;
    }

    VkFormat VulkanImageView::GetImageFormat() const
    {
        return mImage->GetInitialFormat();
    }

    VkImageView VulkanImageView::GetVulkanImageView() const
    {
        return mImageView;
    }

    VulkanImageViewInfo VulkanImageView::GetCreationInfo() const
    {
        return mInfo;
    }

}