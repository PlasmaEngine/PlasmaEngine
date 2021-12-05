#pragma once

namespace Plasma
{
    struct BGFXTextureData : public TextureRenderData
    {
        bgfx::TextureHandle mTextureHandle;
    };

    class RendererBGFX : public Renderer
    {
    public:
        RendererBGFX(OsHandle windowHandle, IntVec2 resolution, RenderAPI::Enum api, String& error);
        ~RendererBGFX() override;

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

        IntVec2 mResolution;

        bgfx::RendererType::Enum PlasmaApiToBGFX(RenderAPI::Enum api);
        bgfx::TextureFormat::Enum PlasmaFormatToBGFX(TextureFormat::Enum);
    };

    Renderer* CreateRenderer(OsHandle windowHandle, IntVec2 resolution, RenderAPI::Enum api, String& error)
    {
        return new RendererBGFX(windowHandle, resolution, api, error);
    }
}