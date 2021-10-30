#include "Precompiled.hpp"

#define VOLK_IMPLEMENTATION
#include "Volk/volk.h"

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

    RendererVK::RendererVK(OsHandle windowHandle, String& error)
    {
        PlasmaPrint("-------- Vulkan Initialize --------\n");

        VkResult result;
        result = volkInitialize();
        Assert(result == VK_SUCCESS);
        PlasmaPrint("[Vulkan] Vok Initialized \n");

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
        PlasmaPrint("[Vulkan] Vulkan Instance Created \n");

        CreateDevice(result);
        PlasmaPrint("[Vulkan] Vulkan Device Created \n");

        PlasmaPrint("-------- Vulkan Initialize End --------\n");
    }

    void RendererVK::CreateDevice(VkResult& result)
    {
        uint32 deviceCount = 0;
        result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
        Assert(result == VK_SUCCESS);

        Assert(deviceCount == 0, "ERROR: No GPU found with Vulkan Support!!");

        Array<VkPhysicalDevice> devices(deviceCount);
        result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.Data());
        Assert(result == VK_SUCCESS);

        const Array<const char*> requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        Array<const char*> enabledDeviceExtensions;

        for each (const VkPhysicalDevice& physicalDevice in devices)
        {
            bool suitableDevice = true;

            uint32 extensoinCount;
            result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensoinCount, nullptr);
            Assert(result == VK_SUCCESS);

            Array<VkExtensionProperties> availableDeviceExtensions(extensoinCount);
            result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensoinCount, availableDeviceExtensions.Data());
            Assert(result == VK_SUCCESS);

            for each (auto & requiredExtension in requiredDeviceExtensions)
            {
                if (!CheckExtensionSupported(requiredExtension, availableDeviceExtensions))
                {
                    suitableDevice = false;
                }

                if (!suitableDevice)
                    continue;

                mFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                mFeatures_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                mFeatures_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                mFeatures2.pNext = &mFeatures_1_1;
                mFeatures_1_1.pNext = &mFeatures_1_2;
                void** featureChain = &mFeatures_1_2.pNext;
                mRayTracingFeatures = {};
                mRayQueryFeatures = {};
                mFragmentShadingRateFeatures = {};
                mMeshShaderFeatures = {};

                mProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                mProperties_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
                mProperties_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
                mProperties2.pNext = &mProperties_1_1;
                mProperties_1_1.pNext = &mProperties_1_2;
                void** propertiesChain = &mProperties_1_2.pNext;
                mSamplerMinMaxProperties = {};
                mAccelerationStructureProperties = {};
                mRayTracingProperties = {};
                mFragmentShadingRateProperties = {};
                mMeshShaderProperties = {};

                mSamplerMinMaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
                *propertiesChain = &mSamplerMinMaxProperties;
                propertiesChain = &mSamplerMinMaxProperties.pNext;

                enabledDeviceExtensions = requiredDeviceExtensions;

                if (CheckExtensionSupported(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME, availableDeviceExtensions))
                {
                    enabledDeviceExtensions.PushBack(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
                    mDepthClipEnableFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
                    *featureChain = &mDepthClipEnableFeatures;
                    featureChain = &mDepthClipEnableFeatures.pNext;
                }

                if (CheckExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, availableDeviceExtensions))
                {
                    enabledDeviceExtensions.PushBack(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                    mAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
                    *featureChain = &mAccelerationStructureFeatures;
                    featureChain = &mAccelerationStructureFeatures.pNext;
                    mAccelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
                    *propertiesChain = &mAccelerationStructureProperties;
                    propertiesChain = &mAccelerationStructureProperties.pNext;

                    if (CheckExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, availableDeviceExtensions))
                    {
                        enabledDeviceExtensions.PushBack(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                        enabledDeviceExtensions.PushBack(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
                        mRayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                        *featureChain = &mRayTracingFeatures;
                        featureChain = &mRayTracingFeatures.pNext;
                        mRayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                        *propertiesChain = &mRayTracingProperties;
                        propertiesChain = &mRayTracingProperties.pNext;

                    }

                    if (CheckExtensionSupported(VK_KHR_RAY_QUERY_EXTENSION_NAME, availableDeviceExtensions))
                    {
                        enabledDeviceExtensions.PushBack(VK_KHR_RAY_QUERY_EXTENSION_NAME);
                        mRayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                        *featureChain = &mRayQueryFeatures;
                        featureChain = &mRayQueryFeatures.pNext;
                    }
                }

                if (CheckExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, availableDeviceExtensions))
                {
                    Assert(CheckExtensionSupported(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, availableDeviceExtensions));
                    enabledDeviceExtensions.PushBack(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
                    enabledDeviceExtensions.PushBack(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
                    mFragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
                    *featureChain = &mFragmentShadingRateFeatures;
                    featureChain = &mFragmentShadingRateFeatures.pNext;
                    mFragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
                    *propertiesChain = &mFragmentShadingRateProperties;
                    propertiesChain = &mFragmentShadingRateProperties.pNext;
                }

                if (CheckExtensionSupported(VK_NV_MESH_SHADER_EXTENSION_NAME, availableDeviceExtensions))
                {
                    enabledDeviceExtensions.PushBack(VK_NV_MESH_SHADER_EXTENSION_NAME);
                    mMeshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
                    *featureChain = &mMeshShaderFeatures;
                    featureChain = &mMeshShaderFeatures.pNext;
                    mMeshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV;
                    *propertiesChain = &mMeshShaderProperties;
                    propertiesChain = &mMeshShaderProperties.pNext;
                }

                if (CheckExtensionSupported(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME, availableDeviceExtensions))
                {
                    enabledDeviceExtensions.PushBack(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
                    mConditionalRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT;
                    *featureChain = &mConditionalRenderingFeatures;
                    featureChain = &mConditionalRenderingFeatures.pNext;
                }

                vkGetPhysicalDeviceProperties2(physicalDevice, &mProperties2);

                bool discrete = mProperties2.properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                if (discrete || physicalDevice == VK_NULL_HANDLE)
                {
                    mPhysicalDevice = physicalDevice;
                 
                    PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] New GPU Found\n");
                    PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] GPU Name : %s \n", mProperties2.properties.deviceName);

                    uint32 driverMajor = VK_VERSION_MAJOR(mProperties2.properties.driverVersion);
                    uint32 driverMinor = VK_VERSION_MINOR(mProperties2.properties.driverVersion);
                    uint32 driverPatch = VK_VERSION_PATCH(mProperties2.properties.driverVersion);

                    if (mProperties2.properties.vendorID == VendorID_NVIDIA)
                    {
                        driverMajor = ((uint32)(mProperties2.properties.driverVersion) >> 22) & 0x3ff;
                        driverMinor = ((uint32)(mProperties2.properties.driverVersion) >> 14) & 0x0ff;

                        uint32 secondary = ((uint32)(mProperties2.properties.driverVersion) >> 6) & 0x0ff;
                        uint32 tertiary = mProperties2.properties.driverVersion & 0x03f;

                        driverPatch = (secondary << 8) | tertiary;
                    }

#ifndef PlasmaTargetOsLinux
                    if (mProperties2.properties.vendorID == VendorID_Intel)
                    {
                        PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] Driver Versoin : %d (0x%X) \n",
                            driverMajor, driverMinor, driverPatch,
                            mProperties2.properties.driverVersion);
                    }
                    else
#endif
                    {
                        PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] Driver Versoin : %d.%d.%d (0x%X) \n",
                            driverMajor, driverMinor, driverPatch,
                            mProperties2.properties.driverVersion);
                    }
                    PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] API Supported: %d.%d \n", 
                        VK_VERSION_MAJOR(mProperties2.properties.apiVersion),
                        VK_VERSION_MINOR(mProperties2.properties.apiVersion));
                    PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] Dedicated GPU : %s \n", discrete ? "true" : "false");

                    if (discrete)
                    {
                        break;
                    }
                }
            }


            PlasmaPrintFilter(Filter::RenderingFilter, "[Vulkan] Slected GPU : %s \n", mProperties2.properties.deviceName);

            Assert(mPhysicalDevice == VK_NULL_HANDLE, "[Vulkan] No suitable GPU found!!");

            vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &mFeatures2);

            Assert(mFeatures2.features.imageCubeArray == VK_TRUE);
            Assert(mFeatures2.features.independentBlend == VK_TRUE);
            Assert(mFeatures2.features.geometryShader == VK_TRUE);
            Assert(mFeatures2.features.samplerAnisotropy == VK_TRUE);
            Assert(mFeatures2.features.shaderClipDistance == VK_TRUE);
            Assert(mFeatures2.features.textureCompressionBC == VK_TRUE);
            Assert(mFeatures2.features.occlusionQueryPrecise == VK_TRUE);
            Assert(mFeatures_1_2.descriptorIndexing == VK_TRUE);

            if (mFeatures2.features.tessellationShader == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::Tessellation;
            }
            if (mFeatures2.features.shaderStorageImageExtendedFormats == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::UavLoadFormatCommon;
            }
            if (mFeatures_1_2.shaderOutputLayer == VK_TRUE && mFeatures_1_2.shaderOutputViewportIndex == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::RenderTargetAndViewportArrayIndexWithoutGS;
            }

            if (mRayTracingFeatures.rayTracingPipeline == VK_TRUE &&
                mRayQueryFeatures.rayQuery == VK_TRUE &&
                mAccelerationStructureFeatures.accelerationStructure == VK_TRUE &&
                mFeatures_1_2.bufferDeviceAddress == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::RayTracing;
                mShaderIdentifierSize = mRayTracingProperties.shaderGroupHandleSize;
            }

            if (mMeshShaderFeatures.meshShader == VK_TRUE && mMeshShaderFeatures.taskShader == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::MeshShading;
            }

            if (mFragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::VariableRateShading;
            }

            if (mFragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::VariableRateShadingTier2;
                mVariableRateShadingTileSize = Math::Min(mFragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize.width,
                    mFragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize.height);
            }

            VkFormatProperties formatProperties = {};
            PlasmaTodo("Convert foarmat to use internal definition");
            vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, VK_FORMAT_B10G11R11_UFLOAT_PACK32, &formatProperties);

            if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            {
                mCapabilities |= GraphicsCapability::UAVLoadFormatR11G11B10F;
            }

            if (mConditionalRenderingFeatures.conditionalRendering == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::Prediction;
            }

            if (mFeatures_1_2.samplerFilterMinmax == VK_TRUE)
            {
                mCapabilities |= GraphicsCapability::SamplerMinMax;
            }

            uint32 queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);

            mQueueFamilies.Resize(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, mQueueFamilies.Data());

            uint32 familyIndex = 0;
            for (const VkQueueFamilyProperties& queueFamily : mQueueFamilies)
            {
                if (mGraphicsFamily == VK_QUEUE_FAMILY_IGNORED && queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    mGraphicsFamily = familyIndex;
                }

                if (mCopyFamily == VK_QUEUE_FAMILY_IGNORED && queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    mCopyFamily = familyIndex;
                }

                if (mComputeFamily == VK_QUEUE_FAMILY_IGNORED && queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    mComputeFamily = familyIndex;
                }

                familyIndex++;
            }

            familyIndex = 0;
            for (const VkQueueFamilyProperties& queueFamily : mQueueFamilies)
            {
                if (queueFamily.queueCount > 0 &&
                    queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT &&
                    !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                    !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    )
                {
                    mCopyFamily = familyIndex;
                }

                if (queueFamily.queueCount > 0 &&
                    queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT &&
                    !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    )
                {
                    mComputeFamily = familyIndex;
                }

                familyIndex++;
            }

            Array<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32> uniqueQueueFamilies = { mGraphicsFamily, mCopyFamily, mComputeFamily };

            float queuePriority = 1.0f;
            for (uint32 queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.PushBack(queueCreateInfo);
                mFamilies.PushBack(queueFamily);
            }

            VkDeviceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = (uint32)queueCreateInfos.Size();
            createInfo.pQueueCreateInfos = queueCreateInfos.Data();
            createInfo.pEnabledFeatures = nullptr;
            createInfo.pNext = &mFeatures2;
            createInfo.enabledExtensionCount = static_cast<uint32>(enabledDeviceExtensions.Size());
            createInfo.ppEnabledExtensionNames = enabledDeviceExtensions.Data();

            result = vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice);
            Assert(result == VK_SUCCESS);

            volkLoadDevice(mDevice);

            vkGetDeviceQueue(mDevice, mGraphicsFamily, 0, &mGraphicsQueue);
            vkGetDeviceQueue(mDevice, mComputeFamily, 0, &mComputeQueue);
            vkGetDeviceQueue(mDevice, mCopyFamily, 0, &mCopyQueue);
        }
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
        Assert(result == VK_SUCCESS);

        volkLoadInstanceOnly(mInstance);

        //if (mDebugUtils)
        //{
        //    result = vkCreateDebugUtilsMessengerEXT(mInstance, &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger);
        //    AlwaysErrorIf(result != VK_SUCCESS);
        //}
    }

    bool RendererVK::CheckExtensionSupported(const char* extension, const Array<VkExtensionProperties>& avalibleExtensions)
    {
        for (const VkExtensionProperties& avalible : avalibleExtensions)
        {
            if (strcmp(avalible.extensionName, extension) == 0)
            {
                return true;
            }
        }
        return false;
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