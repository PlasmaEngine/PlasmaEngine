#pragma once

namespace Plasma
{
	constexpr uint32 VendorID_AMD = 0x1002;
	constexpr uint32 VendorID_ARM = 0x13B5;
	constexpr uint32 VendorID_Broadcom = 0x14E4;
	constexpr uint32 VendorID_GOOGLE = 0x1AE0;
	constexpr uint32 VendorID_ImgTec = 0x1010;
	constexpr uint32 VendorID_Intel = 0x8086;
	constexpr uint32 VendorID_NVIDIA = 0x10DE;
	constexpr uint32 VendorID_Qualcomm = 0x5143;
	constexpr uint32 VendorID_VMWare = 0x15ad;
	constexpr uint32 VendorID_Vivante = 0x10001;
	constexpr uint32 VendorID_VeriSilicon = 0x10002;
	constexpr uint32 VendorID_Kazan = 0x10003;

	struct DeviceSuitabilityData
	{
		VkSurfaceKHR mSurface;
		void* mUserData;

		DeviceSuitabilityData() : mUserData(nullptr)
		{

		}
	};

	struct QueueFamilyIndices
	{
		uint32 mGraphicsFamily = VK_QUEUE_FAMILY_IGNORED;
		uint32 mPresentFamily = VK_QUEUE_FAMILY_IGNORED;

		bool IsComplete() { return mGraphicsFamily != VK_QUEUE_FAMILY_IGNORED && mPresentFamily != VK_QUEUE_FAMILY_IGNORED; }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		Array<VkSurfaceFormatKHR> formats;
		Array<VkPresentModeKHR> presentModes;
	};

	struct PhyscialDeviceLimits
	{
		uint32 mMaxUniformBufferRange;
		VkDeviceSize mMinUniformBufferOffsetAlignment;
	};
}