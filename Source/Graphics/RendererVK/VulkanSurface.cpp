#include "Precompiled.hpp"

namespace Plasma
{
    void RendererVK::CreateSurface(OsHandle windowHandle)
    {
#if defined(PLASMA_PLATFORM_WINDOWS)
        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = (HWND)windowHandle;
        createInfo.hinstance = GetModuleHandle(nullptr);

        VkResult result = vkCreateWin32SurfaceKHR(mVulkanRuntimeData.mInstance, &createInfo, nullptr, &mVulkanRuntimeData.mSurface);
        Assert(result == VK_SUCCESS);
#else
        if (!SDL_Vulkan_CreateSurface((SDL_Window*)window, instance, &internal_state->surface))
        {
            throw sdl2::SDLError("Error creating a vulkan surface");
        }
#endif
    }
}