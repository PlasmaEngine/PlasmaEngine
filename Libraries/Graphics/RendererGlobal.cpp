// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

namespace PL
{
Renderer* gRenderer;
}

LightningDefineExternalBaseType(GraphicsDriverSupport, TypeCopyMode::ReferenceType, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);

  LightningBindFieldGetter(mTextureCompression);
  LightningBindFieldGetter(mMultiTargetBlend);
  LightningBindFieldGetter(mSamplerObjects);
}

} // namespace Plasma
