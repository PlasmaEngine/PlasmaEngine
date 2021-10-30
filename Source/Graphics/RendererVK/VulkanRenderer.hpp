#pragma once

namespace Plasma
{
    class RendererVK : public Renderer
    {
    public:
        RendererVK(OsHandle windowHandle, String& error);
        void CreateDevice(VkResult& result);
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
        void CreateInstance(VkApplicationInfo& appInfo, Plasma::Array<const char*>& instanceLayers, Plasma::Array<const char*>& instanceExtensions);

        bool CheckExtensionSupported(const char* extension, const Array<VkExtensionProperties>& avalibleExtensions);

        bool mDebugUtils = false;
        VkInstance mInstance = VK_NULL_HANDLE;
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VkDevice mDevice = VK_NULL_HANDLE;

        Array<VkQueueFamilyProperties> mQueueFamilies;
        uint32 mGraphicsFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32 mComputeFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32 mCopyFamily = VK_QUEUE_FAMILY_IGNORED;
        Array<uint32> mFamilies;
        VkQueue mGraphicsQueue = VK_NULL_HANDLE;
        VkQueue mComputeQueue = VK_NULL_HANDLE;
        VkQueue mCopyQueue = VK_NULL_HANDLE;

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

    Renderer* CreateRenderer(OsHandle windowHandle, String& error)
    {
        return new RendererVK(windowHandle, error);
    }

}