// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

    struct ImGui_Impl_Plasma_data
    {
    };

    DearImgui::DearImgui() : mAngle{ 0 }, mTranslation{ 0, 0, 0 }
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ErrorIf(io.BackendPlatformUserData != nullptr, "ImGui backend was already initialized.");

        ImGui_Impl_Plasma_data* bd = IM_NEW(ImGui_Impl_Plasma_data)();
        io.BackendLanguageUserData = (void*)bd;
        io.BackendPlatformName = "PlasmaEngine";

        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

        io.DisplaySize.x = 1280;
        io.DisplaySize.y = 720;

        Connect(PL::gEngine->has(TimeSystem), "UiUpdate", this, &DearImgui::OnUpdate);
        Connect(PL::gEngine->has(GraphicsEngine), "UiRenderUpdate", this, &DearImgui::OnRender);
    }

    DearImgui::~DearImgui()
    {
        ImGuiIO& io = ImGui::GetIO();
        if (!ImGui::GetCurrentContext())
        {
            return;
        }

        DisconnectAll(PL::gEngine->has(GraphicsEngine), this);
        DisconnectAll(PL::gEngine->has(TimeSystem), this);
        ImGui::DestroyContext();
    }

    void DearImgui::OnUpdate(UpdateEvent* event)
    {
        mCurrentUpdateFrame++;
        ImGuiIO& io = ImGui::GetIO();

        if (mFontTexture == nullptr)
        {
            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

            mFontTexture = new Texture();
            mFontTexture->Upload(width, height, TextureFormat::RGBA8, (byte*)pixels, sizeof(int) * width * height, true);

            io.Fonts->SetTexID(mFontTexture);
        }

        ImGui::NewFrame();

        bool open = true;
        ImGui::ShowDemoWindow(&open);
    }

    void Build2dTransform(Mat4& m, Vec3Param t, float r);
   
    struct ColorTransform
    {
        Vec4 ColorMultiply;
    };

    void DearImgui::OnRender(Event* event)
    {
        mCurrentRenderFrame++;

        Assert(mCurrentRenderFrame == mCurrentUpdateFrame, "Update and Render are not in sync.");

        ImGui::GetIO();

        if (!ImGui::GetCurrentContext())
        {
            return;
        }
        ImGui::Render();
        ImDrawData* data = ImGui::GetDrawData();

        if (data->DisplaySize.x <= 0.0f || data->DisplaySize.y <= 0.0f)
            return;

        ColorTransform colorTx = { Vec4(1, 1, 1, 1) };
        Mat4 localTx;
        this->mTranslation = Vec3{ 0, 0, 0 };
        this->mAngle = 0;
        Build2dTransform(localTx, this->mTranslation, this->mAngle);
        mWorldTx = localTx * Mat4::cIdentity;

        Vec2 size = Vec2{ data->DisplaySize.x, data->DisplaySize.y };
        Vec2 pos = Vec2{ data->DisplayPos.x, data->DisplayPos.y };

        Rectangle clipRect = Rectangle::PointAndSize(Vec2(pos.x, pos.y), Vec2(size.x, size.y));

        Vec4 clearColor = Vec4{ 0.5, 0.7, 0.3, 0 };

        GraphicsEngine* graphics = PL::gEngine->has(GraphicsEngine);
        RenderTasks& renderTasks = *graphics->mRenderTasksBack;
        RenderQueues& renderQueues = *graphics->mRenderQueuesBack;
        renderQueues.mRenderTasks = &renderTasks;

        FrameBlock& frameBlock = renderQueues.mFrameBlocks.PushBack();
        ViewBlock& viewBlock = renderQueues.mViewBlocks.PushBack();
        frameBlock.mRenderQueues = &renderQueues;

        Mat4 translation;
        translation.Translate(size.x * -0.5f, size.y * -0.5f, 0.0f);
        Mat4 scale;
        scale.Scale(1.0f, -1.0f, 1.0f);
        viewBlock.mWorldToView = scale * translation;
        BuildOrthographicTransformPlasma(viewBlock.mViewToPerspective, size.y, size.x / size.y, -1.0f, 1.0f);

        Mat4 apiPerspective;
        PL::gRenderer->BuildOrthographicTransform(apiPerspective, size.y, size.x / size.y, -1.0f, 1.0f);
        viewBlock.mPlasmaPerspectiveToApiPerspective = apiPerspective * viewBlock.mViewToPerspective.Inverted();

        //
        RenderDrawData(viewBlock, frameBlock, data, clipRect);
        //

        IndexRange& indexRange = viewBlock.mRenderGroupRanges.PushBack();
        indexRange.start = 0;
        indexRange.end = viewBlock.mViewNodes.Size();

        RenderTaskRange& renderTaskRange = renderTasks.mRenderTaskRanges.PushBack();
        renderTaskRange.mFrameBlockIndex = renderQueues.mFrameBlocks.Size() - 1;
        renderTaskRange.mViewBlockIndex = renderQueues.mViewBlocks.Size() - 1;
        renderTaskRange.mTaskIndex = renderTasks.mRenderTaskBuffer.mCurrentIndex;
        renderTaskRange.mTaskCount = 0;

        HandleOf<RenderTarget> renderTarget =
            PL::gEngine->has(GraphicsEngine)->GetRenderTarget((uint)size.x, (uint)size.y, TextureFormat::RGBA8);

        GraphicsRenderSettings renderSettings;
        renderSettings.SetColorTarget(renderTarget);
        renderSettings.mBlendSettings[0].SetBlendAlpha();
        renderSettings.mScissorMode = ScissorMode::Enabled;

        BoundType* defaultRenderPass = MetaDatabase::GetInstance()->FindType("ColorOutput");
        ReturnIf(defaultRenderPass == nullptr, , "We expected to have a type defined called ColorOutput");

        HandleOf<MaterialBlock> renderPassHandle = LightningAllocate(MaterialBlock, defaultRenderPass);
        MaterialBlock& renderPass = renderPassHandle;

        Material* spriteMaterial = MaterialManager::FindOrNull("AlphaSprite");
        uint shaderInputsId = 0;

        {
            Pair<u64, uint> key((u64)spriteMaterial->mResourceId, shaderInputsId);
            IndexRange range = spriteMaterial->AddShaderInputs(renderTasks.mShaderInputs, renderTasks.mShaderInputsVersion);
            renderTasks.mShaderInputRanges.Insert(key, range);
        }
        {
            Pair<u64, uint> key(cFragmentShaderInputsId, shaderInputsId);
            IndexRange range = renderPass.AddShaderInputs(renderTasks.mShaderInputs);
            renderTasks.mShaderInputRanges.Insert(key, range);
        }

        RenderTaskHelper helper(renderTasks.mRenderTaskBuffer);
        helper.AddRenderTaskClearTarget(renderSettings, clearColor, 0, 0, 0xFF);
        helper.AddRenderTaskRenderPass(renderSettings, 0, defaultRenderPass->Name, "IMGUI", shaderInputsId);

        ScreenViewport viewport = { 0, 0, (int)size.x, (int)size.y };
        helper.AddRenderTaskBackBufferBlit(renderTarget, viewport);

        renderTaskRange.mTaskCount = 3;

        PL::gEngine->has(GraphicsEngine)->ClearRenderTargets();
    }

    void DearImgui::RenderDrawData(ViewBlock& viewBlock, FrameBlock& frameBlock, ImDrawData* data, RectangleParam clipRect)
    {
        ImVec2 clipOff = data->DisplayPos;

        Array<StreamedVertex> vertices = {};

        for (int i = 0; i < data->CmdListsCount; i++)
        {
            const ImDrawList* cmdList = data->CmdLists[i];

            // Extract vertex data
            for (int i = 0; i < cmdList->IdxBuffer.Size; i++)
            {
                const ImDrawVert* srcVertex = &cmdList->VtxBuffer[cmdList->IdxBuffer[i]];

                StreamedVertex vertex = StreamedVertex(Vec3(srcVertex->pos.x, srcVertex->pos.y, 0),
                    Vec2(srcVertex->uv.x, srcVertex->uv.y),
                    ToFloatColor(srcVertex->col));
                vertices.PushBack(vertex);
            }

            // draw
            for (int cmdIndex = 0; cmdIndex < cmdList->CmdBuffer.Size; cmdIndex++)
            {
                const ImDrawCmd* cmd = &cmdList->CmdBuffer[cmdIndex];

                if (cmd->UserCallback)
                {
                    // todo:
                }
                else
                {
                    ImVec2 clipMin(cmd->ClipRect.x - clipOff.x, cmd->ClipRect.y - clipOff.y);
                    ImVec2 clipMax(cmd->ClipRect.z - clipOff.x, cmd->ClipRect.w - clipOff.y);
                    if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                        continue;

                    Rectangle clip = Rectangle::PointAndSize(Vec2(clipMin.x, clipMin.y), Vec2(clipMax.x, clipMax.y));

                    Texture* texture = reinterpret_cast<Texture*>(cmd->TextureId);

                    CreateRenderData(viewBlock, frameBlock, clip, vertices, texture, PrimitiveType::Triangles);
                }
            }
        }
    }

    void DearImgui::CreateRenderData(ViewBlock& viewBlock,
        FrameBlock& frameBlock,
        RectangleParam clipRect,
        Array<StreamedVertex>& vertices,
        Texture* texture,
        PrimitiveType::Enum primitiveType)
    {
        if (vertices.Empty())
            return;

        StreamedVertexArray& streamedVertices = frameBlock.mRenderQueues->mStreamedVertices;

        ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, texture);

        for (uint i = 0; i < vertices.Size(); ++i)
        {
            StreamedVertex vertex = vertices[i];
            vertex.mPosition = Math::TransformPoint(viewNode.mLocalToView, vertex.mPosition);
            streamedVertices.PushBack(vertex);
        }

        viewNode.mStreamedVertexType = primitiveType;
        viewNode.mStreamedVertexCount = streamedVertices.Size() - viewNode.mStreamedVertexStart;
    }

    ViewNode& DearImgui::AddRenderNodes(ViewBlock& viewBlock, FrameBlock& frameBlock, RectangleParam clipRect, Texture* texture)
    {
        FrameNode& frameNode = frameBlock.mFrameNodes.PushBack();
        ViewNode& viewNode = viewBlock.mViewNodes.PushBack();

        frameNode.mGraphicalEntry = nullptr;
        viewNode.mGraphicalEntry = nullptr;

        frameNode.mBorderThickness = 1.0f;
        frameNode.mBlendSettingsOverride = false;
        frameNode.mRenderingType = RenderingType::Streamed;
        frameNode.mCoreVertexType = CoreVertexType::Streamed;
        frameNode.mLocalToWorld = mWorldTx.Transposed();
        viewNode.mFrameNodeIndex = frameBlock.mFrameNodes.Size() - 1;
        viewNode.mLocalToView = viewBlock.mWorldToView * frameNode.mLocalToWorld;

        frameNode.mClip = Vec4(clipRect.GetLeft(), clipRect.GetBottom(), clipRect.GetRight(), clipRect.GetTop());

        // maybe cache this lookup on root
        Material* spriteMaterial = nullptr;

        spriteMaterial = MaterialManager::FindOrNull("AlphaSprite");

        frameNode.mMeshRenderData = nullptr;
        frameNode.mMaterialRenderData = spriteMaterial->mRenderData;
        frameNode.mTextureRenderData = texture->mRenderData;

        // default setup for adding streamed data
        viewNode.mStreamedVertexType = PrimitiveType::Triangles;
        viewNode.mStreamedVertexStart = frameBlock.mRenderQueues->mStreamedVertices.Size();
        viewNode.mStreamedVertexCount = 0;

        return viewNode;
    }

} // namespace Plasma