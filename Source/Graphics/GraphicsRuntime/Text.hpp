// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class RenderFont;

class TextDefinition : public BaseDefinition
{
public:
  LightningDeclareType(TextDefinition, TypeCopyMode::ReferenceType);

  Vec4 FontColor;
  String FontName;
  float FontSize;
  RenderFont* mFont;

  // BaseDefinition Interface
  void Initialize() override;
  void Serialize(Serializer& stream) override;
};

} // namespace Plasma
