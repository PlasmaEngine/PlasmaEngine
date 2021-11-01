#pragma once

namespace Plasma
{
    class RendererVK;

    struct SyncObjects
    {
        Array<VkSemaphore> mImageAvailableSemaphores;
    };

    struct VulkanDeviceData
    {
        VkPhysicalDeviceFeatures2 mFeatures2 = {};
        VkPhysicalDeviceVulkan11Features mFeatures_1_1 = {};
        VkPhysicalDeviceVulkan12Features mFeatures_1_2 = {};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR mAccelerationStructureFeatures = {};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR mRayTracingFeatures = {};
        VkPhysicalDeviceRayQueryFeaturesKHR mRayQueryFeatures = {};
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR mFragmentShadingRateFeatures = {};
        VkPhysicalDeviceMeshShaderFeaturesNV mMeshShaderFeatures = {};
        VkPhysicalDeviceConditionalRenderingFeaturesEXT mConditionalRenderingFeatures = {};
        VkPhysicalDeviceDepthClipEnableFeaturesEXT mDepthClipEnableFeatures = {};

        VkPhysicalDeviceProperties2 mProperties2 = {};
        VkPhysicalDeviceVulkan11Properties mProperties_1_1 = {};
        VkPhysicalDeviceVulkan12Properties mProperties_1_2 = {};
        VkPhysicalDeviceSamplerFilterMinmaxProperties mSamplerMinMaxProperties = {};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR mAccelerationStructureProperties = {};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR mRayTracingProperties = {};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR mFragmentShadingRateProperties = {};
        VkPhysicalDeviceMeshShaderPropertiesNV mMeshShaderProperties = {};
    };

    struct VulkanQueueData
    {
        VkQueue mGraphicsQueue = VK_NULL_HANDLE;
        VkQueue mPresentQueue = VK_NULL_HANDLE;
        VkQueue mComputeQueue = VK_NULL_HANDLE;
        VkQueue mCopyQueue = VK_NULL_HANDLE;
    };

    struct VulkanRuntimeData
    {
        const Array<const char*> mRequiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        Array<const char*> mDeviceExtensions;
        static constexpr size_t mMaxFramesInFlight = 2;

        VkInstance mInstance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR mSurface;
        VulkanDeviceData mDeviceData;
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        PhyscialDeviceLimits mDeviceLimits;
        VkDevice mDevice = VK_NULL_HANDLE;
        VkCommandPool mCommandPool;
        SyncObjects mSyncObjects;

        VulkanQueueData mQueueData;
        VkRenderPass mRenderPass;
        VkPipelineLayout mPipelineLayout;
        VkPipeline mGraphicsPipeline;

        RendererVK* mRenderer = nullptr;
        
    };

    class RendererVK : public Renderer
    {
    public:
        RendererVK(OsHandle windowHandle, String& error);
        ~RendererVK() override;

        virtual void BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane);
        virtual void BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane);

        virtual bool YInvertImageData(TextureType::Enum type)
        {
            return false;
        }

        // Called by main thread
        virtual MaterialRenderData* CreateMaterialRenderData();
        virtual MeshRenderData* CreateMeshRenderData();
        virtual TextureRenderData* CreateTextureRenderData();

        virtual void AddMaterial(AddMaterialInfo* info);
        virtual void AddMesh(AddMeshInfo* info);
        virtual void AddTexture(AddTextureInfo* info);
        virtual void RemoveMaterial(MaterialRenderData* data);
        virtual void RemoveMesh(MeshRenderData* data);
        virtual void RemoveTexture(TextureRenderData* data);

        virtual bool GetLazyShaderCompilation();
        virtual void SetLazyShaderCompilation(bool isLazy);
        virtual void AddShaders(Array<ShaderEntry>& entries, uint forceCompileBatchCount);
        virtual void RemoveShaders(Array<ShaderEntry>& entries);

        virtual void SetVSync(bool vsync);

        virtual void GetTextureData(GetTextureDataInfo* info);

        virtual void ShowProgress(ShowProgressInfo* info);

        virtual void DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues);


    private:
        void CreateInstance();
        void CreateSurface(OsHandle windowHandle);
        void SelectPhysicalDevice();
        void CreateLogicalDevice();

        bool IsDeviceSuitable(VkPhysicalDevice physicalDevice, DeviceSuitabilityData* data);
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

        bool CheckExtensionSupported(VkPhysicalDevice device, const Array<const char*>& deviceExtensions);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
        void QueryPhysicalDeviceLimits(VkPhysicalDevice device, PhyscialDeviceLimits& results);
        void LogDeviceInfo(VkPhysicalDevice device);

        VulkanRuntimeData mVulkanRuntimeData;
    };

    Renderer* CreateRenderer(OsHandle windowHandle, String& error)
    {
        return new RendererVK(windowHandle, error);
    }

}