// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
LightningDefineType(TextDefinition, builder, type)
{
}

void TextDefinition::Initialize()
{
  mFont = FontManager::GetInstance()->GetRenderFont(FontName, (uint)FontSize, 0);
}

void TextDefinition::Serialize(Serializer& stream)
{
  SerializeName(FontName);
  SerializeName(FontSize);
  SerializeName(FontColor);
}

} // namespace Plasma
