#include "Precompiled.hpp"

namespace Plasma
{
    QueueFamilyIndices RendererVK::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices;

        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        Array<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.Data());

        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
        {
            if (indices.mGraphicsFamily == VK_QUEUE_FAMILY_IGNORED && queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.mGraphicsFamily = i;

            VkBool32 presentSupported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported);
            if (presentSupported)
                indices.mPresentFamily = i;

            if (indices.IsComplete())
                break;

            i++;

        }

        return indices;
    }

    bool RendererVK::CheckExtensionSupported(VkPhysicalDevice device, const Array<const char*>& deviceExtensions)
    {
        uint32 extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        Array<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.Data());

        HashSet<String> requiredExtensions;

        for (const char* extension : deviceExtensions)
        {
            requiredExtensions.Insert(String(extension));
        }
        for (const VkExtensionProperties& extension : availableExtensions)
        {
            requiredExtensions.Erase(extension.extensionName);
        }

        return requiredExtensions.Empty();
    }


    SwapChainSupportDetails RendererVK::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32 formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.Resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.Data());
        }

        uint32 presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.presentModes.Resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.Data());
        }

        return details;
    }

    bool RendererVK::IsDeviceSuitable(VkPhysicalDevice physicalDevice, DeviceSuitabilityData* data)
    {
        VulkanRuntimeData* runtimeData = (VulkanRuntimeData*)data->mUserData;
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, data->mSurface);

        bool extensionSupported = CheckExtensionSupported(physicalDevice, runtimeData->mRequiredDeviceExtensions);

        bool swapChainAdequate = false;
        if (extensionSupported)
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, data->mSurface);
            swapChainAdequate = !swapChainSupport.formats.Empty() && !swapChainSupport.presentModes.Empty();
        }

        return indices.IsComplete() && extensionSupported && swapChainAdequate;
    }

    void RendererVK::QueryPhysicalDeviceLimits(VkPhysicalDevice device, PhyscialDeviceLimits& results)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        results.mMaxUniformBufferRange = properties.limits.maxUniformBufferRange;
        results.mMinUniformBufferOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment;
    }

    void RendererVK::LogDeviceInfo(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        bool discrete = properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

        PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] GPU Name : %s \n", properties.deviceName);

        uint32 driverMajor = VK_VERSION_MAJOR(properties.driverVersion);
        uint32 driverMinor = VK_VERSION_MINOR(properties.driverVersion);
        uint32 driverPatch = VK_VERSION_PATCH(properties.driverVersion);

        if (properties.vendorID == VendorID_NVIDIA)
        {
            driverMajor = ((uint32)(properties.driverVersion) >> 22) & 0x3ff;
            driverMinor = ((uint32)(properties.driverVersion) >> 14) & 0x0ff;

            uint32 secondary = ((uint32)(properties.driverVersion) >> 6) & 0x0ff;
            uint32 tertiary = properties.driverVersion & 0x03f;

            driverPatch = (secondary << 8) | tertiary;
        }

#ifndef PlasmaTargetOsLinux
        if (properties.vendorID == VendorID_Intel)
        {
            PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] Driver Versoin : %d (0x%X) \n",
                driverMajor, driverMinor, driverPatch,
                properties.driverVersion);
        }
        else
#endif
        {
            PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] Driver Versoin : %d.%d.%d (0x%X) \n",
                driverMajor, driverMinor, driverPatch,
                properties.driverVersion);
        }
        PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] API Supported: %d.%d \n",
            VK_VERSION_MAJOR(properties.apiVersion),
            VK_VERSION_MINOR(properties.apiVersion));
        PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] Dedicated GPU : %s \n", discrete ? "true" : "false");
    }

    void RendererVK::SelectPhysicalDevice()
    {
        DeviceSuitabilityData suitabilityData;
        suitabilityData.mUserData = &mVulkanRuntimeData;
        suitabilityData.mSurface = mVulkanRuntimeData.mSurface;

        mVulkanRuntimeData.mPhysicalDevice = VK_NULL_HANDLE;

        uint32 deviceCount = 0;
        vkEnumeratePhysicalDevices(mVulkanRuntimeData.mInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
            AlwaysError("Failed to find GPU with Vulkan support!");

        Array<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(mVulkanRuntimeData.mInstance, &deviceCount, devices.Data());

        for (const auto& device : devices)
        {
            bool isSuitable = IsDeviceSuitable(device, &suitabilityData);
            if (isSuitable)
            {
                mVulkanRuntimeData.mPhysicalDevice = device;
                break;
            }
        }

        if (mVulkanRuntimeData.mPhysicalDevice == VK_NULL_HANDLE)
            AlwaysError("Failed to find suitable GPU!");

        QueryPhysicalDeviceLimits(mVulkanRuntimeData.mPhysicalDevice, mVulkanRuntimeData.mDeviceLimits);
        LogDeviceInfo(mVulkanRuntimeData.mPhysicalDevice);
    }

    void RendererVK::CreateLogicalDevice()
    {
        QueueFamilyIndices indices = FindQueueFamilies(mVulkanRuntimeData.mPhysicalDevice, mVulkanRuntimeData.mSurface);

        Array<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32> uniqueQueueFamilies = { indices.mGraphicsFamily, indices.mPresentFamily };

        float queuePriority = 1.0f;
        for (uint32 queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.PushBack(queueCreateInfo);
        }

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.mGraphicsFamily;
        queueCreateInfo.queueCount = 1;

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        const Array<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.Size());
        createInfo.pQueueCreateInfos = queueCreateInfos.Data();

        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32>(deviceExtensions.Size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.Data();

        VkResult result = vkCreateDevice(mVulkanRuntimeData.mPhysicalDevice, &createInfo, nullptr, &mVulkanRuntimeData.mDevice);
        Assert(result == VK_SUCCESS, ResultToError(result).c_str());

        volkLoadDevice(mVulkanRuntimeData.mDevice);

        vkGetDeviceQueue(mVulkanRuntimeData.mDevice, indices.mGraphicsFamily, 0, &mVulkanRuntimeData.mQueueData.mGraphicsQueue);
        vkGetDeviceQueue(mVulkanRuntimeData.mDevice, indices.mPresentFamily, 0, &mVulkanRuntimeData.mQueueData.mPresentQueue);
    }
}