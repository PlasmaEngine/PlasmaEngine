#include "Precompiled.hpp"
#include "BGFXRenderer.hpp"


namespace Plasma
{
	void RendererBGFX::DumpCaps()
	{
		PlasmaPrint("----- Renderer Info Begin -----\n");

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

                PlasmaPrint("\t %d: %04x %04x \n"
                    , ii
                    , gpu.deviceId
                    , gpu.vendorId
				);
			}

			PlasmaPrint("\n");
		}

		PlasmaPrint("Capabilities (renderer %s, vendor %s, device 0x%04x): \n"
            , bgfx::getRendererName(caps->rendererType)
            , GetVendorName(caps->vendorId).c_str()
            , caps->deviceId
		);

        PlasmaPrint("----- Renderer Info End -----\n");
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

        m_texColor = bgfx::createUniform("s_texColor",  bgfx::UniformType::Sampler);

        bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);

        DumpCaps();

        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x303030ff
            , 1.0f
            , 0
        );

        m_vertexLayout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Uint8)
            .add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float);

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
        bgfx::destroy(m_texColor);
        bgfx::destroy(mLoadingShader);
        bgfx::shutdown();
    }

    void RendererBGFX::BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane)
    {
        BuildOrthographicTransformGl(matrix, size, aspect, nearPlane, farPlane);
    }

    void RendererBGFX::BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane)
    {
        BuildPerspectiveTransformGl(matrix, fov, aspect, nearPlane, farPlane);
    }

    bool RendererBGFX::YInvertImageData(TextureType::Enum type)
    {
        return (type != TextureType::TextureCube);
    }

    MaterialRenderData* RendererBGFX::CreateMaterialRenderData()
    {
        BGFXMaterialData* materialData = new BGFXMaterialData();
        materialData->mResourceId = 0;
        return materialData;
    }

    MeshRenderData* RendererBGFX::CreateMeshRenderData()
    {
        BGFXMeshData* meshData = new BGFXMeshData();
        meshData->mVertexBuffer = BGFX_INVALID_HANDLE;
        meshData->mIndexBuffer = BGFX_INVALID_HANDLE;
        return meshData;
    }

    TextureRenderData* RendererBGFX::CreateTextureRenderData()
    {
        BGFXTextureData* textureData = new BGFXTextureData();
        textureData->mTextureHandle = BGFX_INVALID_HANDLE;
        textureData->mFormat = TextureFormat::None;
        return textureData;
    }

    void RendererBGFX::AddMaterial(AddMaterialInfo* info)
    {
        BGFXMaterialData* materialData = static_cast<BGFXMaterialData*>(info->mRenderData);

        materialData->mCompositeName = info->mCompositeName;
        materialData->mResourceId = info->mMaterialId;
    }

    void RendererBGFX::AddMesh(AddMeshInfo* info)
    {
        BGFXMeshData* meshData = static_cast<BGFXMeshData*>(info->mRenderData);
        if (bgfx::isValid(meshData->mVertexBuffer))
        {
            bgfx::destroy(meshData->mVertexBuffer);
        }
        if (bgfx::isValid(meshData->mIndexBuffer))
        {
            bgfx::destroy(meshData->mIndexBuffer);
        }

        if (info->mVertexData == nullptr)
        {
            meshData->mVertexBuffer = BGFX_INVALID_HANDLE;
            meshData->mIndexBuffer = BGFX_INVALID_HANDLE;
            meshData->mPrimitiveType = info->mPrimitiveType;
            return;
        }

        meshData->mVertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(info->mVertexData, info->mVertexCount * info->mVertexSize), m_vertexLayout);
        meshData->mIndexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(info->mIndexData, info->mIndexCount * info->mIndexSize));

        delete[] info->mVertexData;
        delete[] info->mIndexData;

        meshData->mBones.Assign(info->mBones.All());
    }

    void RendererBGFX::AddTexture(AddTextureInfo* info)
    {
        BGFXTextureData* textureData = static_cast<BGFXTextureData*>(info->mRenderData);

        if (info->mFormat == TextureFormat::None)
        {
            bgfx::destroy(textureData->mTextureHandle);
        }
        if (bgfx::isValid(textureData->mTextureHandle))
        {
            bgfx::destroy(textureData->mTextureHandle);
        }

        // TODO (@Davey): Layer params may need some fancy focus?
        switch (info->mType)
        {
        case TextureType::Texture2D:
            textureData->mTextureHandle = bgfx::createTexture2D(info->mWidth, info->mHeight, info->mMipCount > 0, 1, PlasmaFormatToBGFX(info->mFormat));
            break;
        case TextureType::TextureCube:
            textureData->mTextureHandle = bgfx::createTextureCube(info->mWidth, info->mMipCount > 0, 1, PlasmaFormatToBGFX(info->mFormat));
            break;
        case TextureType::Texture3D:
            textureData->mTextureHandle = bgfx::createTexture3D(info->mWidth, info->mHeight, info->mDepth, info->mMipCount > 0, PlasmaFormatToBGFX(info->mFormat));
        default:
            break;
        }

       if(bgfx::isValid(textureData->mTextureHandle))
       {
           bgfx::setTexture(0, m_texColor, textureData->mTextureHandle);
       }

        textureData->mType   = info->mType;
        textureData->mFormat = info->mFormat;
        textureData->mWidth  = info->mWidth;
        textureData->mHeight = info->mHeight;
        textureData->mDepth  = info->mDepth;

        textureData->mSamplerSettings = 0;
        textureData->mSamplerSettings |= SamplerSettings::AddressingX(info->mAddressingX);
        textureData->mSamplerSettings |= SamplerSettings::AddressingY(info->mAddressingY);
        textureData->mSamplerSettings |= SamplerSettings::Filtering(info->mFiltering);
        textureData->mSamplerSettings |= SamplerSettings::CompareMode(info->mCompareMode);
        textureData->mSamplerSettings |= SamplerSettings::CompareFunc(info->mCompareFunc);

        delete[] info->mImageData;
        delete[] info->mMipHeaders;
    }

    void RendererBGFX::RemoveMaterial(MaterialRenderData* data)
    {
        mMaterialRenderDataToDestroy.PushBack(static_cast<BGFXMaterialData*>(data));
    }

    void RendererBGFX::RemoveMesh(MeshRenderData* data)
    {
        mMeshRenderDataToDestroy.PushBack(static_cast<BGFXMeshData*>(data));
    }

    void RendererBGFX::RemoveTexture(TextureRenderData* data)
    {
        mTextureRenderDataToDestroy.PushBack(static_cast<BGFXTextureData*>(data));
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

        // Get data off info (we don't technically need to copy this anymore or lock)
        BGFXTextureData* loadingTexture = static_cast<BGFXTextureData*>(info->mLoadingTexture);
        BGFXTextureData* logoTexture    = static_cast<BGFXTextureData*>(info->mLogoTexture);
        BGFXTextureData* whiteTexture   = static_cast<BGFXTextureData*>(info->mWhiteTexture);
        BGFXTextureData* splashTexture  = static_cast<BGFXTextureData*>(info->mSplashTexture);
        BGFXTextureData* fontTexture    = static_cast<BGFXTextureData*>(info->mFontTexture);

        uint logoFrameSize                 = info->mLogoFrameSize;
        Array<StreamedVertex> progressText = info->mProgressText;
        int progressWidth                  = info->mProgressWidth;
        float currentPercent               = info->mCurrentPercent;
        double time                        = info->mTimer.Time();
        bool splashMode                    = info->mSplashMode;
        float alpha                        = splashMode ? info->mSplashFade : 1.0f;


        // TODO (@Davey): Check if this is the intended solution for BGFX vs GL
        IntVec2 size = mResolution;

        Mat4 viewportToNdc;
        viewportToNdc.BuildTransform(Vec3(-1.0f, 1.0f, 0.0f), Mat3::cIdentity,
                                     Vec3(2.0f / size.x, -2.0f / size.y, 1.0f));

        // Logo uv transform for animating
        uint xFrames = logoTexture->mWidth / logoFrameSize;
        uint yFrames = logoTexture->mHeight / logoFrameSize;
        double framesPerSecond = 30.0;
        uint frame = static_cast<uint>(time * framesPerSecond) % (xFrames * yFrames);
        Vec2 logoUvScale = Vec2(1.0f / xFrames, 1.0f / yFrames);
        Vec2 logoUvTranslate = Vec2(static_cast<float>(frame % xFrames), static_cast<float>(frame / xFrames)) *
                               logoUvScale;
        Mat3 logoUvTransform;
        logoUvTransform.BuildTransform(logoUvTranslate, 0.0f, logoUvScale);

        // Object transforms
        Vec3 logoScale = Vec3(static_cast<float>(logoFrameSize), static_cast<float>(logoFrameSize), 1.0f);
        Vec3 logoTranslation = Vec3((size.x - loadingTexture->mWidth + logoScale.x) * 0.5f, size.y * 0.5f, 0.0f);
        Mat4 logoTransform;
        logoTransform.BuildTransform(logoTranslation, Mat3::cIdentity, logoScale);
        logoTransform = viewportToNdc * logoTransform;

        Vec3 loadingScale = Vec3(static_cast<float>(loadingTexture->mWidth),
                                 static_cast<float>(loadingTexture->mHeight), 1.0f);
        Vec3 loadingTranslation = Vec3(size.x * 0.5f, size.y * 0.5f, 0.0f);
        Mat4 loadingTransform;
        loadingTransform.BuildTransform(loadingTranslation, Mat3::cIdentity, loadingScale);
        loadingTransform = viewportToNdc * loadingTransform;

        Vec3 progressScale = Vec3(progressWidth * currentPercent, 20.0f, 1.0f);
        Vec3 progressTranslation =
                Vec3((size.x - loadingScale.x + progressScale.x) * 0.5f, (size.y + loadingScale.y) * 0.5f + 300.0f, 0.0f);
        Mat4 progressTransform;
        progressTransform.BuildTransform(progressTranslation, Mat3::cIdentity, progressScale);
        progressTransform = viewportToNdc * progressTransform;

        Vec3 textScale = Vec3(1.0f);
        Vec3 textTranslation = Vec3((size.x - loadingScale.x) * 0.5f, (size.y + loadingScale.y) * 0.5f + 250.0f, 0.0f);
        Mat4 textTransform;
        textTransform.BuildTransform(textTranslation, Mat3::cIdentity, textScale);
        textTransform = viewportToNdc * textTransform;

        Vec3 splashScale = Vec3(static_cast<float>(size.x), static_cast<float>(size.y),
                                1.0f);
        //if (size.x < splashScale.x)
        //    splashScale *= size.x / splashScale.x;
        //if (size.y < splashScale.y)
        //    splashScale *= size.y / splashScale.y;
        Vec3 splashTranslation = Vec3(size.x * 0.5f, size.y * 0.5f, 0.0f);
        Mat4 splashTransform;
        splashTransform.BuildTransform(splashTranslation, Mat3::cIdentity, splashScale);
        splashTransform = viewportToNdc * splashTransform;

        StreamedVertex quadVertices[] = {
                {Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f)},
                {Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f), Vec4(1.0f)},
                {Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f), Vec4(1.0f)},
                {Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f), Vec4(1.0f)},
                {Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f), Vec4(1.0f)},
                {Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f)},
        };

        bgfx::setViewClear(0
                , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
                , 0x303030ff
                , 1.0f
                , 0
        );

        if(!bgfx::isValid(mLoadingShader))
        {
            //bgfx::ShaderHandle handle = bgfx::createShader();
            //mLoadingShader = bgfx::createProgram(handle, true);
        }

//        bgfx::
//        glUseProgram(mLoadingShader);
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//        GLint textureLoc = glGetUniformLocation(mLoadingShader, "Texture");
//        GLint transformLoc = glGetUniformLocation(mLoadingShader, "Transform");
//        GLint uvTransformLoc = glGetUniformLocation(mLoadingShader, "UvTransform");
//
//        GLint alphaLoc = glGetUniformLocation(mLoadingShader, "Alpha");
//        glUniform1fv(alphaLoc, 1, &alpha);
//
//        GLint textureSlot = 0;
//        glActiveTexture(GL_TEXTURE0 + textureSlot);
//        glUniform1iv(textureLoc, 1, &textureSlot);
//
//        glUniformMatrix3fv(uvTransformLoc, 1, cTransposeMatrices, Mat3::cIdentity.array);
//
//        if (!splashMode)
//        {
//            //// Loading
//            //glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, loadingTransform.array);
//            //glBindTexture(GL_TEXTURE_2D, loadingTexture->mId);
//            //mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
//            //mStreamedVertexBuffer.FlushBuffer(true);
//
//            // Logo
//            glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, splashTransform.array);
//            glBindTexture(GL_TEXTURE_2D, splashTexture->mId);
//            mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
//            mStreamedVertexBuffer.FlushBuffer(true);
//
//            // Progress bar
//            glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, progressTransform.array);
//            glBindTexture(GL_TEXTURE_2D, whiteTexture->mId);
//            mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
//            mStreamedVertexBuffer.FlushBuffer(true);
//
//            // Progress text
//            if (progressText.Size() > 0)
//            {
//                glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, textTransform.array);
//                glBindTexture(GL_TEXTURE_2D, fontTexture->mId);
//                mStreamedVertexBuffer.AddVertices(&progressText[0], progressText.Size(), PrimitiveType::Triangles);
//                mStreamedVertexBuffer.FlushBuffer(true);
//            }
//        }
//        else
//        {
//            // Splash
//            glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, splashTransform.array);
//            glBindTexture(GL_TEXTURE_2D, splashTexture->mId);
//            mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
//            mStreamedVertexBuffer.FlushBuffer(true);
//        }

//        float mtx[16];
//        bx::mtxRotateXY(mtx
//                , 0.0f
//                , time*0.37f
//        );
//
//        meshSubmit(m_mesh, 0, mLoadingShader, mtx);

//        glDisable(GL_BLEND);
//        glUseProgram(0);

//        // Disable v-sync so we don't wait on frames (mostly for single threaded mode)
//        // This could cause tearing, but it's the loading screen.
//        plGlSetSwapInterval(this, 0);
//
//        plGlSwapBuffers(this);
//        TracyGpuCollect;
//
//        int swapInterval = mVsync ? 1 : 0;
//        plGlSetSwapInterval(this, swapInterval);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();
    }

    void RendererBGFX::DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues)
    {
        ZoneScoped;
        mRenderTasks = renderTasks;
        mRenderQueues = renderQueues;

        forRange(RenderTaskRange& taskRange, mRenderTasks->mRenderTaskRanges.All())
            DoRenderTaskRange(taskRange);

        {
            ZoneScopedN("SwapBuffers");
//            TracyGpuZone("SwapBuffer")
//            plGlSwapBuffers(this);
//            TracyGpuCollect;
        }

        {
            ZoneScopedN("DestroyRenderData");
            DelayedRenderDataDestruction();
            //DestroyUnusedSamplers();
        }
    }

    void RendererBGFX::DoRenderTaskRange(RenderTaskRange &taskRange) {
        mFrameBlock = &mRenderQueues->mFrameBlocks[taskRange.mFrameBlockIndex];
        mViewBlock = &mRenderQueues->mViewBlocks[taskRange.mViewBlockIndex];

        uint taskIndex = taskRange.mTaskIndex;
        for (uint i = 0; i < taskRange.mTaskCount; ++i)
        {
            ErrorIf(taskIndex >= mRenderTasks->mRenderTaskBuffer.mCurrentIndex, "Render task data is not valid.");
            RenderTask* task = (RenderTask*)&mRenderTasks->mRenderTaskBuffer.mRenderTaskData[taskIndex];

            switch (task->mId)
            {
                case RenderTaskType::ClearTarget:
                    DoRenderTaskClearTarget(static_cast<RenderTaskClearTarget*>(task));
                    taskIndex += sizeof(RenderTaskClearTarget);
                    break;

                case RenderTaskType::RenderPass:
                {
                    RenderTaskRenderPass* renderPass = static_cast<RenderTaskRenderPass*>(task);
                    DoRenderTaskRenderPass(renderPass);
                    // RenderPass tasks can have multiple following task entries for sub
                    // RenderGroup settings. Have to index past all sub tasks.
                    taskIndex += sizeof(RenderTaskRenderPass) * (renderPass->mSubRenderGroupCount + 1);
                    i += renderPass->mSubRenderGroupCount;
                }
                    break;

                case RenderTaskType::PostProcess:
                    DoRenderTaskPostProcess(static_cast<RenderTaskPostProcess*>(task));
                    taskIndex += sizeof(RenderTaskPostProcess);
                    break;

                case RenderTaskType::BackBufferBlit:
                    DoRenderTaskBackBufferBlit(static_cast<RenderTaskBackBufferBlit*>(task));
                    taskIndex += sizeof(RenderTaskBackBufferBlit);
                    break;

                case RenderTaskType::TextureUpdate:
                    DoRenderTaskTextureUpdate(static_cast<RenderTaskTextureUpdate*>(task));
                    taskIndex += sizeof(RenderTaskTextureUpdate);
                    break;
                case RenderTaskType::ComputePass:
                    Warn("Compute: Implementation Unfinied");
                    DoRenderTaskCompute(static_cast<RenderTaskCompute*>(task));
                    taskIndex += sizeof(RenderTaskCompute);
                    break;
                default:
                    Error("Render task not implemented.");
                    break;
            }
        }
    }

    void RendererBGFX::DoRenderTaskClearTarget(RenderTaskClearTarget *task)
    {
        uint16_t clear = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(0, clear, ToByteColor(task->mColor), task->mDepth, task->mStencil);
    }

    void RendererBGFX::DoRenderTaskRenderPass(RenderTaskRenderPass *task)
    {

    }

    void RendererBGFX::DoRenderTaskPostProcess(RenderTaskPostProcess *task)
    {

    }

    void RendererBGFX::DoRenderTaskBackBufferBlit(RenderTaskBackBufferBlit *task)
    {

    }

    void RendererBGFX::DoRenderTaskTextureUpdate(RenderTaskTextureUpdate *task)
    {
        ZoneScoped;
        AddTextureInfo info;
        info.mRenderData = task->mRenderData;
        info.mWidth = task->mWidth;
        info.mDepth = task->mDepth;
        info.mHeight = task->mHeight;
        info.mType = task->mType;
        info.mFormat = task->mFormat;
        info.mAddressingX = task->mAddressingX;
        info.mAddressingY = task->mAddressingY;
        info.mFiltering = task->mFiltering;
        info.mCompareMode = task->mCompareMode;
        info.mCompareFunc = task->mCompareFunc;
        info.mAnisotropy = task->mAnisotropy;
        info.mMipMapping = task->mMipMapping;

        info.mMipCount = 0;
        info.mTotalDataSize = 0;
        info.mImageData = nullptr;
        info.mMipHeaders = nullptr;

        AddTexture(&info);
    }

    void RendererBGFX::DoRenderTaskCompute(RenderTaskCompute *task)
    {

    }

    String RendererBGFX::GetVendorName(uint16_t id)
    {
        if (id == VendorID_AMD)
        {
            return "AMD";
        }
        else if (id == VendorID_NVIDIA)
        {
            return "NVidia";
        }
        else
        {
            return "Vendor to be added";
        }
    }

    void RendererBGFX::DelayedRenderDataDestruction()
    {
        forRange(BGFXMaterialData* renderData, mMaterialRenderDataToDestroy.All())
            DestroyRenderData(renderData);
        forRange(BGFXMeshData* renderData, mMeshRenderDataToDestroy.All())
            DestroyRenderData(renderData);
        forRange(BGFXTextureData* renderData, mTextureRenderDataToDestroy.All())
            DestroyRenderData(renderData);

        mMaterialRenderDataToDestroy.Clear();
        mMeshRenderDataToDestroy.Clear();
        mTextureRenderDataToDestroy.Clear();
    }

    void  RendererBGFX::DestroyRenderData(BGFXMaterialData* renderData)
    {
        delete renderData;
    }
    void RendererBGFX::DestroyRenderData(BGFXMeshData* renderData)
    {
        if(bgfx::isValid(renderData->mIndexBuffer))
        {
            bgfx::destroy(renderData->mIndexBuffer);
        }

        if(bgfx::isValid(renderData->mVertexBuffer))
        {
            bgfx::destroy(renderData->mVertexBuffer);
        }

        renderData->mBones.Clear();
        delete renderData;
}
    void  RendererBGFX::DestroyRenderData(BGFXTextureData* renderData)
    {
        if(bgfx::isValid(renderData->mTextureHandle))
        {
            bgfx::destroy(renderData->mTextureHandle);
        }
        delete renderData;
    }

    void StreamedVertexBuffer::Initialize() {

    }

    void StreamedVertexBuffer::Destroy() {

    }

    void StreamedVertexBuffer::AddVertices(StreamedVertex *vertices, uint count, PrimitiveType::Enum primitiveType) {

    }

    void StreamedVertexBuffer::AddVertices(StreamedVertexArray &vertices, uint start, uint count,
                                           PrimitiveType::Enum primitiveType) {

    }

    void StreamedVertexBuffer::FlushBuffer(bool deactivate) {

    }
}