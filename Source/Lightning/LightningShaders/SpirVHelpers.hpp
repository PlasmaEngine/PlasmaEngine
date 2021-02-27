// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

// for enum types
template <>
struct PlasmaShared HashPolicy<spv::Op> : public HashPolicy<int>
{
};

template <>
struct PlasmaShared HashPolicy<spv::Capability> : public HashPolicy<int>
{
};

} // namespace Plasma
