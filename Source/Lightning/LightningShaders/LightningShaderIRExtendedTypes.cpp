// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "LightningShaderIRExtendedTypes.hpp"

namespace Plasma
{

LightningShaderIRImageType::LightningShaderIRImageType()
{
  mIRType = nullptr;
}

LightningShaderIRImageType::LightningShaderIRImageType(LightningShaderIRType* type)
{
  Load(type);
}

bool LightningShaderIRImageType::Load(LightningShaderIRType* type)
{
  if (type->mBaseType != ShaderIRTypeBaseType::Image)
  {
    mIRType = nullptr;
    return false;
  }

  mIRType = type;
  return true;
}

LightningShaderIRType* LightningShaderIRImageType::GetSampledType()
{
  return mIRType->mParameters[0]->As<LightningShaderIRType>();
}

int LightningShaderIRImageType::GetDim()
{
  return GetIntegerConstantParameterValue(1);
}

int LightningShaderIRImageType::GetDepth()
{
  return GetIntegerConstantParameterValue(2);
}

int LightningShaderIRImageType::GetArrayed()
{
  return GetIntegerConstantParameterValue(3);
}

int LightningShaderIRImageType::GetMultiSampled()
{
  return GetIntegerConstantParameterValue(4);
}

int LightningShaderIRImageType::GetSampled()
{
  return GetIntegerConstantParameterValue(5);
}

int LightningShaderIRImageType::GetFormat()
{
  return GetIntegerConstantParameterValue(6);
}

bool LightningShaderIRImageType::IsStorageImage()
{
  return GetSampled() == 2;
}

int LightningShaderIRImageType::GetIntegerConstantParameterValue(int parameterIndex)
{
  ILightningShaderIR* parameter = mIRType->mParameters[parameterIndex];
  LightningShaderIRConstantLiteral* constantLiteral = parameter->As<LightningShaderIRConstantLiteral>();
  if (constantLiteral == nullptr)
    return -1;
  int value = constantLiteral->mValue.Get<int>();
  return value;
}

LightningShaderIRRuntimeArrayType::LightningShaderIRRuntimeArrayType()
{
  mIRType = nullptr;
}

bool LightningShaderIRRuntimeArrayType::Load(LightningShaderIRType* type)
{
  ShaderIRTypeMeta* typeMeta = type->mMeta;
  if (typeMeta == nullptr)
    return false;

  Lightning::BoundType* lightningType = typeMeta->mLightningType;
  if (lightningType == nullptr)
    return false;

  // Only visit runtime array types (the struct that wraps the actual spirv
  // runtime array)
  if (lightningType->TemplateBaseName != SpirVNameSettings::mRuntimeArrayTypeName)
    return false;

  mIRType = type;
  return true;
}

LightningShaderIRType* LightningShaderIRRuntimeArrayType::GetSpirVRuntimeArrayType()
{
  if (mIRType == nullptr)
    return nullptr;

  return mIRType->mParameters[0]->As<LightningShaderIRType>();
}

LightningShaderIRType* LightningShaderIRRuntimeArrayType::GetContainedType()
{
  LightningShaderIRType* spirVRuntimeArrayType = GetSpirVRuntimeArrayType();
  if (spirVRuntimeArrayType == nullptr)
    return nullptr;

  return spirVRuntimeArrayType->mParameters[0]->As<LightningShaderIRType>();
}

} // namespace Plasma
