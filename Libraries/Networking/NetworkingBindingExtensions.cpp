// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Meta Net Property
LightningDefineType(MetaNetProperty, builder, type)
{
  LightningBindField(mNetPropertyConfig)->AddAttribute(PropertyAttributes::cOptional);
  LightningBindField(mNetChannelConfig)->AddAttribute(PropertyAttributes::cOptional);
}

} // namespace Plasma
