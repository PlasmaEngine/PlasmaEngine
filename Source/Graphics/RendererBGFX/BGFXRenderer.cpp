#include "Precompiled.hpp"

namespace Plasma
{
	static void DumpCaps()
	{
		PlasmaPrint("\n");

		const bgfx::Caps* caps = bgfx::getCaps();

		if (0 < caps->numGPUs)
		{
			PlasmaPrint("Detected GPUs (%d): \n", caps->numGPUs);
			PlasmaPrint("\t +----------------   Index \n");
			PlasmaPrint("\t |  +-------------   Device ID \n");
			PlasmaPrint("\t |  |    +--------   Vendor ID \n");
			for (uint32_t ii = 0; ii < caps->numGPUs; ++ii)
			{
				const bgfx::Caps::GPU& gpu = caps->gpu[ii];
				UnusedParameter(gpu);

				PlasmaPrint("\t %d: %04x %04x \n"
					, ii
					, gpu.deviceId
					, gpu.vendorId
				);
			}

			PlasmaPrint("\n");
		}


		PlasmaPrint("\n");
    
		PlasmaPrint("Capabilities (renderer %s, vendor 0x%04x, device 0x%04x): \n"
			, bgfx::getRendererName(caps->rendererType)
			, caps->vendorId
			, caps->deviceId
		);

        PlasmaPrint("\n");
	}


    RendererBGFX::RendererBGFX(OsHandle windowHandle, IntVec2 resolution, RenderAPI::Enum api, String& error) : mResolution(resolution)
    {
        bgfx::Init bgfxInit;
#if defined(PLASMA_PLATFORM_WINDOWS)
        bgfxInit.platformData.nwh = (void*)windowHandle;
#endif
        bgfxInit.resolution.width = mResolution.x;
        bgfxInit.resolution.height = mResolution.y;
        bgfxInit.resolution.reset = BGFX_RESET_VSYNC;
        bgfxInit.type = PlasmaApiToBGFX(api);

        bgfx::init(bgfxInit);

        bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);

        DumpCaps();

        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x303030ff
            , 1.0f
            , 0
        );
    }

    bgfx::RendererType::Enum RendererBGFX::PlasmaApiToBGFX(RenderAPI::Enum api)
    {
        switch (api)
        {
        case Plasma::RenderAPI::OpenGL:
            return bgfx::RendererType::OpenGL;
        case Plasma::RenderAPI::Vulkan:
            return bgfx::RendererType::Vulkan;
        case Plasma::RenderAPI::DX11:
            return bgfx::RendererType::Direct3D11;
        case Plasma::RenderAPI::DX12:
            return bgfx::RendererType::Direct3D12;
        case Plasma::RenderAPI::GLes:
            return bgfx::RendererType::OpenGLES;
        case Plasma::RenderAPI::Metal:
            return bgfx::RendererType::Metal;
        case Plasma::RenderAPI::Web:
            return bgfx::RendererType::WebGPU;
        default:
            break;
        }

        return bgfx::RendererType::OpenGL;
    }

    bgfx::TextureFormat::Enum RendererBGFX::PlasmaFormatToBGFX(TextureFormat::Enum format)
    {
        switch (format)
        {
        case Plasma::TextureFormat::None:
            return bgfx::TextureFormat::Unknown;
        case Plasma::TextureFormat::R8:
            return bgfx::TextureFormat::R8;
        case Plasma::TextureFormat::RG8:
            return bgfx::TextureFormat::RG8;
        case Plasma::TextureFormat::RGB8:
            return bgfx::TextureFormat::RGB8;
        case Plasma::TextureFormat::RGBA8:
            return bgfx::TextureFormat::RGBA8;
        case Plasma::TextureFormat::R16:
            return bgfx::TextureFormat::R16;
        case Plasma::TextureFormat::RG16:
            return bgfx::TextureFormat::RG16;
        case Plasma::TextureFormat::RGB16:
            return bgfx::TextureFormat::RGBA16;
        case Plasma::TextureFormat::RGBA16:
            return bgfx::TextureFormat::RGBA16;
        case Plasma::TextureFormat::R16f:
            return bgfx::TextureFormat::R16F;
        case Plasma::TextureFormat::RG16f:
            return bgfx::TextureFormat::RG16F;
        case Plasma::TextureFormat::RGB16f:
            return bgfx::TextureFormat::RGBA16F;
        case Plasma::TextureFormat::RGBA16f:
            return bgfx::TextureFormat::RGBA16F;
        case Plasma::TextureFormat::R32f:
            return bgfx::TextureFormat::R32F;
        case Plasma::TextureFormat::RG32f:
            return bgfx::TextureFormat::RG32F;
        case Plasma::TextureFormat::RGB32f:
            return bgfx::TextureFormat::RGBA32F;
        case Plasma::TextureFormat::RGBA32f:
            return bgfx::TextureFormat::RGBA32F;
        case Plasma::TextureFormat::SRGB8:
            return bgfx::TextureFormat::RGB8S;
        case Plasma::TextureFormat::SRGB8A8:
            return bgfx::TextureFormat::RGBA8S;
        case Plasma::TextureFormat::Depth16:
            return bgfx::TextureFormat::D16;
        case Plasma::TextureFormat::Depth24:
            return bgfx::TextureFormat::D24;
        case Plasma::TextureFormat::Depth32:
            return bgfx::TextureFormat::D32;
        case Plasma::TextureFormat::Depth32f:
            return bgfx::TextureFormat::D32F;
        case Plasma::TextureFormat::Depth24Stencil8:
            return bgfx::TextureFormat::D24S8;
        case Plasma::TextureFormat::Depth32fStencil8Pad24:
            return bgfx::TextureFormat::D24S8;
        default:
            return bgfx::TextureFormat::Unknown;
        }
    }

    RendererBGFX::~RendererBGFX()
    {
    }

    void RendererBGFX::BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane)
    {
    }

    void RendererBGFX::BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane)
    {
    }

    MaterialRenderData* RendererBGFX::CreateMaterialRenderData()
    {
        return new MaterialRenderData();
    }

    MeshRenderData* RendererBGFX::CreateMeshRenderData()
    {
        return new MeshRenderData();
    }

    TextureRenderData* RendererBGFX::CreateTextureRenderData()
    {
        BGFXTextureData* textureData = new BGFXTextureData();
        textureData->mTextureHandle = BGFX_INVALID_HANDLE;
        return textureData;
    }

    void RendererBGFX::AddMaterial(AddMaterialInfo* info)
    {
    }

    void RendererBGFX::AddMesh(AddMeshInfo* info)
    {
    }

    void RendererBGFX::AddTexture(AddTextureInfo* info)
    {
        BGFXTextureData* textureData = static_cast<BGFXTextureData*>(info->mRenderData);

        if (info->mFormat == TextureFormat::None)
        {
            bgfx::destroy(textureData->mTextureHandle);
        }
        switch (info->mType)
        {
        case TextureType::Texture2D:
        {
            uint64_t flags = 0;
            textureData->mTextureHandle = bgfx::createTexture2D(info->mWidth, info->mHeight, info->mMipCount > 0, 1, PlasmaFormatToBGFX(info->mFormat), flags);
        }
        break;
        case TextureType::Texture3D:
        {

        }
        break;
        case TextureType::TextureCube:
        {

        }
        break;
        default:
            break;
        }
    }

    void RendererBGFX::RemoveMaterial(MaterialRenderData* data)
    {
    }

    void RendererBGFX::RemoveMesh(MeshRenderData* data)
    {
    }

    void RendererBGFX::RemoveTexture(TextureRenderData* data)
    {
    }

    bool RendererBGFX::GetLazyShaderCompilation()
    {
        return false;
    }

    void RendererBGFX::SetLazyShaderCompilation(bool isLazy)
    {
    }

    void RendererBGFX::AddShaders(Array<ShaderEntry>& entries, uint forceCompileBatchCount)
    {
    }

    void RendererBGFX::RemoveShaders(Array<ShaderEntry>& entries)
    {
    }

    void RendererBGFX::SetVSync(bool vsync)
    {
    }

    void RendererBGFX::GetTextureData(GetTextureDataInfo* info)
    {
    }

    void RendererBGFX::ShowProgress(ShowProgressInfo* info)
    {
        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, uint16_t(mResolution.x), uint16_t(mResolution.y));

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        // Use debug font to print information about this example.
        bgfx::dbgTextClear();

        bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

        bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
        bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

        const bgfx::Stats* stats = bgfx::getStats();
        bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
            , stats->width
            , stats->height
            , stats->textWidth
            , stats->textHeight
        );

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();
    }

    void RendererBGFX::DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues)
    {
    }
}