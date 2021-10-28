#pragma once

namespace Plasma
{
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
        void CreateInstance(VkApplicationInfo& appInfo, Plasma::Array<const char*>& instanceLayers, Plasma::Array<const char*>& instanceExtensions);

        bool mDebugUtils = false;
        VkInstance mInstance = VK_NULL_HANDLE;
    };

    Renderer* CreateRenderer(OsHandle windowHandle, String& error)
    {
        return new RendererVK(windowHandle, error);
    }

}