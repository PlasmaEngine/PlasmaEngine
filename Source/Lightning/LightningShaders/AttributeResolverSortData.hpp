#pragma once

#include "LightningShadersStandard.hpp"

#include "IAttributeResolver.hpp"

namespace Plasma
{

//-------------------------------------------------------------------AttributeResolverSortData
class AttributeResolverSortData
{
public:
  IAttributeResolver* mResolver = nullptr;
  LightningShaderIRType* mShaderType = nullptr;
  Lightning::SyntaxNode* mNode = nullptr;
  size_t mAttributeIndex = 0;

  static int Sort(const AttributeResolverSortData& lhs, const AttributeResolverSortData& rhs)
  {
    float lhsPriority = lhs.mResolver->GetPriority();
    float rhsPriority = rhs.mResolver->GetPriority();
    if(lhsPriority == rhsPriority)
      return 0;
    return lhsPriority < rhsPriority;
  }
};

}//namespace Plasma 