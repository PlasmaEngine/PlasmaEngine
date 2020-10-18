// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class IconPreview : public PreviewWidget
{
public:
  IconPreview(PreviewWidgetInitializer& initializer, StringParam icon);
  void UpdateTransform();

  Element* mIcon;
};

class ScriptPreview : public IconPreview
{
public:
  explicit ScriptPreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "ScriptIcon"){};
};

class RenderGroupPreview : public IconPreview
{
public:
  explicit RenderGroupPreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "RenderGroupIcon"){};
};

class SoundPreview : public IconPreview
{
public:
  explicit SoundPreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "SoundIcon"){};
};

class NetworkingPreview : public IconPreview
{
public:
  explicit NetworkingPreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "NetworkingIcon"){};
};

class PhysicsPreview : public IconPreview
{
public:
  explicit PhysicsPreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "PhysicsIcon"){};
};

class LevelPreview : public IconPreview
{
public:
  explicit LevelPreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "LevelIcon"){};
};

class SoundCuePreview : public IconPreview
{
public:
  typedef SoundCuePreview LightningSelf;

  explicit SoundCuePreview(PreviewWidgetInitializer& initializer);
  void OnLeftClick(MouseEvent* event);
};

class PhysicsMaterialPreview : public IconPreview
{
public:
  explicit PhysicsMaterialPreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "PhysicsMaterial"){};
};

class EmptyPreview : public IconPreview
{
public:
  explicit EmptyPreview(PreviewWidgetInitializer& initializer) : IconPreview(initializer, "LargeFolder")
  {
    mIcon->SetColor(Vec4(0, 0, 0, 0));
  }
};

class CameraViewportDrawer : public Widget
{
public:
  CameraViewportDrawer(Composite* parent, Cog* cameraObject);

  void SetSize(Vec2 newSize);
  void RenderUpdate(ViewBlock& viewBlock,
                    FrameBlock& frameBlock,
                    Mat4Param parentTx,
                    ColorTransform colorTx,
                    WidgetRect clipRect) override;

  HandleOf<Cog> mCameraObject;
};
class SpacePreview;
class SpacePreviewMouseDrag : public MouseManipulation
{
public:
  SpacePreviewMouseDrag(Mouse* mouse, SpacePreview* preview);

  void OnMouseMove(MouseEvent* event) override;
  void OnRightMouseUp(MouseEvent* event) override;

  SpacePreview* mPreview;
};

class SpacePreview : public PreviewWidget
{
public:
  typedef SpacePreview LightningSelf;

  SpacePreview(PreviewWidgetInitializer& initializer,
               StringParam objectArchetype = CoreArchetypes::Default,
               Cog* objectToView = nullptr);
  ~SpacePreview();

  void SetInteractive(bool interactive) override;

  void OnRightMouseDown(MouseEvent* event);
  void OnMouseScroll(MouseEvent* event);
  void OnDestroy();
  void OnUpdate(UpdateEvent* updateEvent);

  Vec2 GetMinSize() override;
  void UpdateViewDistance();
  void UpdateViewDistance(Vec3 viewDirection);
  void UpdateCameraPosition();
  void AnimatePreview(PreviewAnimate::Enum value) override;
  void UpdateTransform();

  PreviewAnimate::Enum mPreviewAnimate;
  CameraViewportDrawer* mCameraViewportDrawer;
  HandleOf<Space> mSpace;
  bool mOwnsSpace;
  bool mHasGraphical;
  CogId mCamera;
  //CogId mObject;
  // Distance the camera is from the previewed object
  float mLookAtDistance;
  // Angle the look at vector is from the y-axis from [Pi/2, -Pi/2]
  float mVerticalAngle;
  // Angle between the right vector and the positive x-axis from [0, Pi]
  float mHorizontalAngle;
};

class MaterialPreview : public SpacePreview
{
public:
  explicit MaterialPreview(PreviewWidgetInitializer& initializer);
};

class MeshPreview : public SpacePreview
{
public:
 explicit  MeshPreview(PreviewWidgetInitializer& initializer);
};

class PhysicsMeshPreview : public SpacePreview
{
public:
  explicit PhysicsMeshPreview(PreviewWidgetInitializer& initializer);
};

class ConvexMeshPreview : public SpacePreview
{
public:
  explicit ConvexMeshPreview(PreviewWidgetInitializer& initializer);
};

class MultiConvexMeshPreview : public SpacePreview
{
public:
  explicit MultiConvexMeshPreview(PreviewWidgetInitializer& initializer);
};

class ArchetypePreview : public SpacePreview
{
public:
  typedef ArchetypePreview LightningSelf;

  explicit ArchetypePreview(PreviewWidgetInitializer& initializer);
  Handle GetEditObject() override;

  const float cSpritePreviewThreshold = 0.1f;
};

class SpriteSourcePreview : public SpacePreview
{
public:
  explicit SpriteSourcePreview(PreviewWidgetInitializer& initializer);
};

class AnimationPreview : public SpacePreview
{
public:
  typedef AnimationPreview LightningSelf;

  explicit AnimationPreview(PreviewWidgetInitializer& initializer);

  Handle GetEditObject() override;
  void OnReload(ResourceEvent* event);

  HandleOf<Animation> mAnimation;
};

class CogPreview : public SpacePreview
{
public:
  explicit CogPreview(PreviewWidgetInitializer& initializer);
};

class TexturePreview : public PreviewWidget
{
public:
  explicit TexturePreview(PreviewWidgetInitializer& initializer);
  void UpdateTransform();

  TextureView* mImage;
};

// preview
class FontPreview : public SpacePreview
{
public:
  explicit FontPreview(PreviewWidgetInitializer& initializer);
};

class TilePaletteSourcePreview : public PreviewWidget
{
public:
  explicit TilePaletteSourcePreview(PreviewWidgetInitializer& initializer);
  void SizeToContents();
  void UpdateTransform();

private:
  TilePaletteView* mTilePaletteView;
  HandleOf<TilePaletteSource> mSource;
};

class ColorGradientPreview : public PreviewWidget
{
public:
  explicit ColorGradientPreview(PreviewWidgetInitializer& initializer);
  ~ColorGradientPreview();

  void UpdateTransform() override;
  Vec2 GetHalfSize() override;

  PixelBuffer* mGradientBlockBuffer;
  TextureView* mGradientBlockDisplay;
};

class SampleCurveDrawer : public Widget
{
public:
  SampleCurveDrawer(Composite* parent, HandleParam object);

  void AddCurve(ViewBlock& viewBlock, FrameBlock& frameBlock, WidgetRect clipRect, SampleCurve* curveObject);
  void RenderUpdate(
      ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect);

  Handle mObject;
};

// SampleCurvePreview
class SampleCurvePreview : public PreviewWidget
{
public:
  explicit SampleCurvePreview(PreviewWidgetInitializer& initializer);
  void UpdateTransform() override;

  //Element* mBackground;
  GraphWidget* mGraph;
  SampleCurveDrawer* mDrawer;
};

class ResourceTablePreview : public PreviewWidget
{
public:
  explicit ResourceTablePreview(PreviewWidgetInitializer& initializer);

  void AnimatePreview(PreviewAnimate::Enum value) override;
  void UpdateTransform() override;

  PreviewWidgetGroup* mGroup;
};

class SpaceArchetypePreview : public IconPreview
{
public:
  SpaceArchetypePreview(PreviewWidgetInitializer& initializer, Archetype* archetype);
  ~SpaceArchetypePreview();

  Handle GetEditObject() override;

  HandleOf<Space> mObject;
};

class GameArchetypePreview : public IconPreview
{
public:
  GameArchetypePreview(PreviewWidgetInitializer& initializer, Archetype* archetype);
  ~GameArchetypePreview();

  Handle GetEditObject() override;

  HandleOf<GameSession> mObject;
  bool UsingEditorGameSession;
};

} // namespace Plasma
