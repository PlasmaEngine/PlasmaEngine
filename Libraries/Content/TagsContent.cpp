// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "TagsContent.hpp"

namespace Plasma
{

LightningDefineType(ContentTags, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
}

void ContentTags::Serialize(Serializer& stream)
{
  SerializeName(mTags);
}

} // namespace Plasma
