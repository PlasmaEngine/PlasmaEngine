// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

    class DearImgui : public ExplicitSingleton<DearImgui, EventObject>
    {
    public:
        /// Define the self type for connections.
        typedef DearImgui LightningSelf;

        ~DearImgui();
        DearImgui();

    protected:
    private:
        void OnUpdate(UpdateEvent* event);
        void OnRender(Event* event);

        void RenderDrawData(ViewBlock& viewBlock, FrameBlock& frameBlock, ImDrawData* data, RectangleParam clipRect);
        void CreateRenderData(ViewBlock& viewBlock,
            FrameBlock& frameBlock,
            RectangleParam clipRect,
            Array<StreamedVertex>& vertices,
            Texture* texture,
            PrimitiveType::Enum primitiveType);
        ViewNode& AddRenderNodes(ViewBlock& viewBlock, FrameBlock& frameBlock, RectangleParam clipRect, Texture* texture);

        Vec3 mTranslation;
        float mAngle;

        Mat4 mWorldTx = Mat4::cIdentity;
        Texture* mFontTexture = nullptr;
        int mCurrentUpdateFrame = -1;
        int mCurrentRenderFrame = -1;
    };

} // namespace Plasma