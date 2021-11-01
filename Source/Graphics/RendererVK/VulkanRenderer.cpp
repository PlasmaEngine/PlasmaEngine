#include "Precompiled.hpp"

#define VOLK_IMPLEMENTATION
#include "Volk/volk.h"

namespace Plasma
{
    RendererVK::RendererVK(OsHandle windowHandle, String& error)
    {
        PlasmaPrint("-------- Vulkan Initialize --------\n");

        VkResult result;
        result = volkInitialize();
        Assert(result == VK_SUCCESS);
        PlasmaPrint("[Vulkan] Vok Initialized \n");

       

        PlasmaTodo("Add Vulkan debug layer");

        CreateInstance();
        PlasmaPrint("[Vulkan] Vulkan Instance Created \n");
        CreateSurface(windowHandle);
        PlasmaPrint("[Vulkan] Vulkan Surface Created \n");
        SelectPhysicalDevice();
        PlasmaPrint("[Vulkan] Vulkan Physcial Device Selected\n");
        CreateLogicalDevice();
        PlasmaPrint("[Vulkan] Vulkan Logcal Device Created\n");

        

        PlasmaPrint("-------- Vulkan Initialize End --------\n");
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