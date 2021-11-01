#include "Precompiled.hpp"

namespace Plasma
{
    void RendererVK::CreateInstance()
    {
        VkResult result;

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = GetApplicationName().c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(GetMajorVersion(), GetMinorVersion(), GetPatchVersion());
        appInfo.pEngineName = "Plasma Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(GetMajorVersion(), GetMinorVersion(), GetPatchVersion());
        appInfo.apiVersion = VK_API_VERSION_1_2;

        uint32 instanceLayerCount;
        result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        Assert(result == VK_SUCCESS);
        Array<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
        result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.Data());
        Assert(result == VK_SUCCESS);

        uint32 extensionCount = 0;
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        Assert(result == VK_SUCCESS);
        Array<VkExtensionProperties> availableInstanceExtensions(extensionCount);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableInstanceExtensions.Data());
        Assert(result == VK_SUCCESS);

        Array<const char*> instanceLayers;

        bool validationSupported = false;

        // Check if VK_EXT_debug_utils is supported, which supersedes VK_EXT_Debug_Report
        for (auto& availableExtension : availableInstanceExtensions)
        {
            if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                validationSupported = true;
                mVulkanRuntimeData.mDeviceExtensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            else if (strcmp(availableExtension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
            {
                mVulkanRuntimeData.mDeviceExtensions.PushBack(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
        }

        mVulkanRuntimeData.mDeviceExtensions.PushBack(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
        mVulkanRuntimeData.mDeviceExtensions.PushBack(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif SDL2
        {
            SDL_Window* window = (SDL_Window*)windowHandle;
            uint32 extensionCount;
            SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
            Array<const char*> extensionNames_sdl(extensionCount);
            SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames_sdl.Data());
            instanceExtensions.Reserve(mVulkanRuntimeData.mDeviceExtensions.Size() + extensionNames_sdl.Size());
            instanceExtensions.Insert(mVulkanRuntimeData.mDeviceExtensions.begin(),
                extensionNames_sdl.Begin(), extensionNames_sdl.End());
        }
#endif
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32>(mVulkanRuntimeData.mDeviceExtensions.Size());
        createInfo.ppEnabledExtensionNames = mVulkanRuntimeData.mDeviceExtensions.Data();

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

        if (gDebugUtils && validationSupported)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(gValidationLayers.Size());
            createInfo.ppEnabledLayerNames = gValidationLayers.Data();

            debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugUtilsCreateInfo.pfnUserCallback = DebugCallback;
            createInfo.pNext = &debugUtilsCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        result = vkCreateInstance(&createInfo, nullptr, &mVulkanRuntimeData.mInstance);
        Assert(result == VK_SUCCESS, ResultToError(result).c_str());

        volkLoadInstanceOnly(mVulkanRuntimeData.mInstance);

        if (gDebugUtils && validationSupported)
        {
            result = vkCreateDebugUtilsMessengerEXT(mVulkanRuntimeData.mInstance, &debugUtilsCreateInfo, nullptr, &mVulkanRuntimeData.mDebugMessenger);
            AlwaysErrorIf(result != VK_SUCCESS);

            PlasmaPrint("[Vulkan] Validation Enabled");
        }
    }
}