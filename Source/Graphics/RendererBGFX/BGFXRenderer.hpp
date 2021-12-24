#pragma once

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

    struct BGFXMaterialData : public MaterialRenderData
    {
    public:
    };

    struct BGFXMeshData : public MeshRenderData
    {
        bgfx::VertexBufferHandle mVertexBuffer;
        bgfx::IndexBufferHandle mIndexBuffer;
        PrimitiveType::Enum mPrimitiveType;
        Array<MeshBone> mBones;
    };

    struct BGFXTextureData : public TextureRenderData
    {
        bgfx::TextureHandle mTextureHandle;
        TextureType::Enum mType;
        TextureFormat::Enum mFormat;
        uint mWidth;
        uint mHeight;
        uint mDepth;
        u32 mSamplerSettings;
    };

    class RendererBGFX : public Renderer
    {
    public:
        RendererBGFX(OsHandle windowHandle, IntVec2 resolution, RenderAPI::Enum api, String& error);
        ~RendererBGFX() override;

        virtual void BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane);
        virtual void BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane);

        virtual bool YInvertImageData(TextureType::Enum type);

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

        void DelayedRenderDataDestruction();
        void DestroyRenderData(BGFXMaterialData* renderData);
        void DestroyRenderData(BGFXMeshData* renderData);
        void DestroyRenderData(BGFXTextureData* renderData);

    private:

        void DumpCaps();
        String GetVendorName(uint16_t id);

        IntVec2 mResolution;

        bgfx::VertexLayout m_vertexLayout;

        bgfx::RendererType::Enum PlasmaApiToBGFX(RenderAPI::Enum api);
        bgfx::TextureFormat::Enum PlasmaFormatToBGFX(TextureFormat::Enum);

        bgfx::UniformHandle m_texColor;

        Array<BGFXMaterialData*> mMaterialRenderDataToDestroy;
        Array<BGFXMeshData*> mMeshRenderDataToDestroy;
        Array<BGFXTextureData*> mTextureRenderDataToDestroy;
    };

    Renderer* CreateRenderer(OsHandle windowHandle, IntVec2 resolution, RenderAPI::Enum api, String& error)
    {
        return new RendererBGFX(windowHandle, resolution, api, error);
    }
}