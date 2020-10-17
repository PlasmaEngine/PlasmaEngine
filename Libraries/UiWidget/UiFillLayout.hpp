// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class UiFillLayout : public UiLayout
{
public:
  /// Meta Initialization.
  LightningDeclareType(UiFillLayout, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;

  /// UiLayout Interface.
  Vec2 Measure(Rectangle& rect) override;
  void DoLayout(Rectangle& rect, UiTransformUpdateEvent* e) override;

  static void FillToParent(UiWidget* child);
  static void FillToRectangle(RectangleParam rect, UiWidget* widget);
};

} // namespace Plasma
