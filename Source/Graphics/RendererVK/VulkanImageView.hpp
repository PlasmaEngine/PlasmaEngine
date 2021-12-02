#pragma once

namespace Plasma
{
	class VulkanImage;

	//-------------------------------------------------------------------VulkanImageViewInfo
	struct VulkanImageViewInfo
	{
		VkImageViewType mViewType = VK_IMAGE_VIEW_TYPE_2D;
		VkFormat mFormat = VK_FORMAT_UNDEFINED;
		VkComponentSwizzle mComponents[4] = { VK_COMPONENT_SWIZZLE_IDENTITY };
		VkImageAspectFlags mAspectFlags = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
		uint32_t mBaseMipLevel = 0;
		uint32_t mMipLevels = 1;
		uint32_t mBaseArrayLayer = 0;
		uint32_t mLayerCount = 1;
	};

	//-------------------------------------------------------------------VulkanImageView
	class VulkanImageView
	{
	public:
		VulkanImageView(VkDevice device, VulkanImage* image, const VulkanImageViewInfo& info);
		~VulkanImageView();

		void Free();

		VulkanImage* GetImage() const;
		VkFormat GetImageFormat() const;
		VkImageView GetVulkanImageView() const;
		VulkanImageViewInfo GetCreationInfo() const;

	private:
		VkDevice mDevice;
		VulkanImageViewInfo mInfo;
		VulkanImage* mImage = nullptr;
		VkImageView mImageView = VK_NULL_HANDLE;
	};

}