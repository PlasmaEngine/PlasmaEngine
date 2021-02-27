// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "LightningShaderIRReflection.hpp"

namespace Plasma
{

ShaderResourceReflectionData::ShaderResourceReflectionData()
{
  mBinding = -1;
  mLocation = -1;
  mSizeInBytes = 0;
  mDescriptorSet = 0;
  mStride = 0;
  mOffsetInBytes = 0;
}

} // namespace Plasma
