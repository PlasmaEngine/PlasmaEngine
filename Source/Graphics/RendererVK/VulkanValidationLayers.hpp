#pragma once

namespace Plasma
{
    const bool gDebugUtils = true;
    const Array<const char*> gValidationLayers = { "VK_LAYER_KHRONOS_validation" };

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        constexpr bool assertOnError = true;
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT && assertOnError)
        {
            Assert(false, pCallbackData->pMessage);
        }

        PlasmaPrint("[Vulkan Validation] %s \n", pCallbackData->pMessage);

        return VK_FALSE;
    }
}