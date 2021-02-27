// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Hashing.hpp"
#include "Guid.hpp"

namespace Plasma
{

size_t Guid::Hash() const
{
  return HashPolicy<u64>()(mValue);
}

} // namespace Plasma
