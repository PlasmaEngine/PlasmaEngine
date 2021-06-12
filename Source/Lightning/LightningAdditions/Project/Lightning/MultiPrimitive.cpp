#include "Lightning.hpp"

namespace Lightning
{
  LightningDefineType(MultiPrimitive, builder, type)
  {
    type->HandleManager = LightningManagerId(PointerManager);
  }

  MultiPrimitive::MultiPrimitive(BoundType* primitiveMemberType, size_t primitiveMemberCount)
    : PrimitiveMemberType(primitiveMemberType),
      PrimitiveMemberCount(primitiveMemberCount)
  {
  }
}
