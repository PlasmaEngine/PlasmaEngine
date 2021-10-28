#include "Precompiled.hpp"

#define VOLK_IMPLEMENTATION
#include "Volk/volk.h"

namespace Plasma
{
    RendererVK::RendererVK(OsHandle windowHandle, String& error)
    {
        VkResult result;
        result = volkInitialize();
        AlwaysErrorIf(result != VK_SUCCESS);

        VkApplicationInfo appInfo = {}; 
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = GetApplicationName().c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(GetMajorVersion(), GetMinorVersion(), GetPatchVersion());
        appInfo.pEngineName = "Plasma Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(GetMajorVersion(), GetMinorVersion(), GetPatchVersion());
        appInfo.apiVersion = VK_API_VERSION_1_2;

        uint32 instanceLayerCount;
        result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        AlwaysErrorIf(result != VK_SUCCESS);
        Array<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
        result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.Data());
        AlwaysErrorIf(result != VK_SUCCESS);

        uint32 extensionCount = 0;
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        AlwaysErrorIf(result != VK_SUCCESS);
        Array<VkExtensionProperties> availableInstanceExtensions(extensionCount);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableInstanceExtensions.Data());
        AlwaysErrorIf(result != VK_SUCCESS);

        Array<const char*> instanceLayers;
        Array<const char*> instanceExtensions;

        // Check if VK_EXT_debug_utils is supported, which supersedes VK_EXT_Debug_Report
        for (auto& availableExtension : availableInstanceExtensions)
        {
            if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                mDebugUtils = true;
                instanceExtensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            else if (strcmp(availableExtension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
            {
                instanceExtensions.PushBack(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
        }

        instanceExtensions.PushBack(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
        instanceExtensions.PushBack(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif SDL2
        {
            SDL_Window* window = (SDL_Window*)windowHandle;
            uint32 extensionCount;
            SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
            Array<const char*> extensionNames_sdl(extensionCount);
            SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames_sdl.Data());
            instanceExtensions.Reserve(instanceExtensions.Size() + extensionNames_sdl.Size());
            instanceExtensions.Insert(instanceExtensions.begin(),
                extensionNames_sdl.Begin(), extensionNames_sdl.End());
        }
#endif

        PlasmaTodo("Add Vulkan debug layer");

        CreateInstance(appInfo, instanceLayers, instanceExtensions);

    }

    void RendererVK::CreateInstance(VkApplicationInfo& appInfo, Array<const char*>& instanceLayers, Array<const char*>& instanceExtensions)
    {
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.Size());
        createInfo.ppEnabledLayerNames = instanceLayers.Data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.Size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.Data();

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

        //if (debugUtils)
        //{
        //    debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        //    debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        //    debugUtilsCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;
        //    createInfo.pNext = &debugUtilsCreateInfo;
        //}

        VkResult result;
        result = vkCreateInstance(&createInfo, nullptr, &mInstance);
        AlwaysErrorIf(result != VK_SUCCESS);

        volkLoadInstanceOnly(mInstance);

        //if (mDebugUtils)
        //{
        //    result = vkCreateDebugUtilsMessengerEXT(mInstance, &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger);
        //    AlwaysErrorIf(result != VK_SUCCESS);
        //}
    }

    RendererVK::~RendererVK()
    {
    }
    
    void RendererVK::BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane)
    {
    }
    
    void RendererVK::BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane)
    {
    }
    
    MaterialRenderData* RendererVK::CreateMaterialRenderData()
    {
        return new MaterialRenderData();
    }
    
    MeshRenderData* RendererVK::CreateMeshRenderData()
    {
        return new MeshRenderData();
    }
    
    TextureRenderData* RendererVK::CreateTextureRenderData()
    {
        return new TextureRenderData();
    }
    
    void RendererVK::AddMaterial(AddMaterialInfo* info)
    {
    }
    
    void RendererVK::AddMesh(AddMeshInfo* info)
    {
    }
    
    void RendererVK::AddTexture(AddTextureInfo* info)
    {
    }
    
    void RendererVK::RemoveMaterial(MaterialRenderData* data)
    {
    }
    
    void RendererVK::RemoveMesh(MeshRenderData* data)
    {
    }
    
    void RendererVK::RemoveTexture(TextureRenderData* data)
    {
    }
    
    bool RendererVK::GetLazyShaderCompilation()
    {
        return false;
    }
    
    void RendererVK::SetLazyShaderCompilation(bool isLazy)
    {
    }
    
    void RendererVK::AddShaders(Array<ShaderEntry>& entries, uint forceCompileBatchCount)
    {
    }
    
    void RendererVK::RemoveShaders(Array<ShaderEntry>& entries)
    {
    }
    
    void RendererVK::SetVSync(bool vsync)
    {
    }
    
    void RendererVK::GetTextureData(GetTextureDataInfo* info)
    {
    }
    
    void RendererVK::ShowProgress(ShowProgressInfo* info)
    {
    }
   
    void RendererVK::DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues)
    {
    }
}