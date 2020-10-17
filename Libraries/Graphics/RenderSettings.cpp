// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(ShaderInputs, builder, type)
{
  PlasmaBindDocumented();
  LightningBindDestructor();

  LightningBindMethod(Create);

  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, bool));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, int));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, IntVec2));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, IntVec3));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, IntVec4));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, float));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, Vec2));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, Vec3));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, Vec4));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, Mat3));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, Mat4));
  LightningBindOverloadedMethod(Add, LightningInstanceOverload(void, String, String, Texture*));
  LightningBindMethod(Remove);
  LightningBindMethod(Clear);
}

ShaderInputs::~ShaderInputs()
{
  ErrorIf(mGuardId != cGuardId, "Expected the guard id to be set");
  mGuardId = 0;
}

HandleOf<ShaderInputs> ShaderInputs::Create()
{
  return new ShaderInputs();
}

void ShaderInputs::Add(String fragmentName, String inputName, bool input)
{
  Add(fragmentName, inputName, ShaderInputType::Bool, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, int input)
{
  Add(fragmentName, inputName, ShaderInputType::Int, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, IntVec2 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec2, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, IntVec3 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec3, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, IntVec4 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec4, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, float input)
{
  Add(fragmentName, inputName, ShaderInputType::Float, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Vec2 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec2, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Vec3 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec3, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Vec4 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec4, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Mat3 input)
{
  Add(fragmentName, inputName, ShaderInputType::Mat3, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Mat4 input)
{
  Add(fragmentName, inputName, ShaderInputType::Mat4, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Texture* input)
{
  Add(fragmentName, inputName, ShaderInputType::Texture, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, ShaderInputType::Enum type, AnyParam value)
{
  LightningShaderGenerator* shaderGenerator = PL::gEngine->has(GraphicsEngine)->mShaderGenerator;

  ShaderInput shaderInput = shaderGenerator->CreateShaderInput(fragmentName, inputName, type, value);
  if (shaderInput.mShaderInputType != ShaderInputType::Invalid)
    mShaderInputs.Insert(StringPair(fragmentName, inputName), shaderInput);
}

void ShaderInputs::Remove(String fragmentName, String inputName)
{
  mShaderInputs.Erase(StringPair(fragmentName, inputName));
}

void ShaderInputs::Clear()
{
  mShaderInputs.Clear();
}

DefineThreadSafeReferenceCountedHandle(GraphicsBlendSettings);
LightningDefineType(GraphicsBlendSettings, builder, type)
{
  PlasmaBindDocumented();
  PlasmaBindThreadSafeReferenceCountedHandle();
  LightningBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  // Will probably remove these functions, so don't want them bound
  // LightningBindMethod(SetBlendAlpha);
  // LightningBindMethod(SetBlendAdditive);

  LightningBindGetterSetterProperty(BlendMode);
  LightningBindGetterSetterProperty(BlendEquation);
  LightningBindGetterSetterProperty(SourceFactor);
  LightningBindGetterSetterProperty(DestFactor);
  LightningBindGetterSetterProperty(BlendEquationAlpha);
  LightningBindGetterSetterProperty(SourceFactorAlpha);
  LightningBindGetterSetterProperty(DestFactorAlpha);

  BlendSettings::Constructed = &GraphicsBlendSettings::ConstructedStatic;
  BlendSettings::Destructed = &GraphicsBlendSettings::DestructedStatic;
}

void GraphicsBlendSettings::ConstructedStatic(BlendSettings* settings)
{
  ((GraphicsBlendSettings*)settings)->ConstructedInstance();
}

void GraphicsBlendSettings::DestructedStatic(BlendSettings* settings)
{
  ((GraphicsBlendSettings*)settings)->DestructedInstance();
}

void GraphicsBlendSettings::ConstructedInstance()
{
  ConstructThreadSafeReferenceCountedHandle();
}

void GraphicsBlendSettings::DestructedInstance()
{
  DestructThreadSafeReferenceCountedHandle();
}

DefineThreadSafeReferenceCountedHandle(GraphicsDepthSettings);
LightningDefineType(GraphicsDepthSettings, builder, type)
{
  PlasmaBindDocumented();
  PlasmaBindThreadSafeReferenceCountedHandle();
  LightningBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  // Will probably remove these functions, so don't want them bound
  // LightningBindMethod(SetDepthRead);
  // LightningBindMethod(SetDepthWrite);
  // LightningBindMethod(SetStencilTestMode);
  // LightningBindMethod(SetStencilIncrement);
  // LightningBindMethod(SetStencilDecrement);

  LightningBindGetterSetterProperty(DepthMode);
  LightningBindGetterSetterProperty(DepthCompareFunc);
  LightningBindGetterSetterProperty(StencilMode);
  LightningBindGetterSetterProperty(StencilCompareFunc);
  LightningBindGetterSetterProperty(StencilFailOp);
  LightningBindGetterSetterProperty(DepthFailOp);
  LightningBindGetterSetterProperty(DepthPassOp);
  LightningBindGetterSetterProperty(StencilCompareFuncBackFace);
  LightningBindGetterSetterProperty(StencilFailOpBackFace);
  LightningBindGetterSetterProperty(DepthFailOpBackFace);
  LightningBindGetterSetterProperty(DepthPassOpBackFace);

  LightningBindFieldProperty(mStencilReadMask);
  LightningBindFieldProperty(mStencilWriteMask);
  LightningBindFieldProperty(mStencilTestValue);
  LightningBindFieldProperty(mStencilReadMaskBackFace);
  LightningBindFieldProperty(mStencilWriteMaskBackFace);
  LightningBindFieldProperty(mStencilTestValueBackFace);

  DepthSettings::Constructed = &GraphicsDepthSettings::ConstructedStatic;
  DepthSettings::Destructed = &GraphicsDepthSettings::DestructedStatic;
}

void GraphicsDepthSettings::ConstructedStatic(DepthSettings* settings)
{
  ((GraphicsDepthSettings*)settings)->ConstructedInstance();
}

void GraphicsDepthSettings::DestructedStatic(DepthSettings* settings)
{
  ((GraphicsDepthSettings*)settings)->DestructedInstance();
}

void GraphicsDepthSettings::ConstructedInstance()
{
  ConstructThreadSafeReferenceCountedHandle();
}

void GraphicsDepthSettings::DestructedInstance()
{
  DestructThreadSafeReferenceCountedHandle();
}

LightningDefineType(GraphicsRenderSettings, builder, type)
{
  PlasmaBindDocumented();
  LightningBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  LightningBindSetter(ColorTarget);
  LightningBindSetter(DepthTarget);
  LightningBindGetter(MultiRenderTarget);

  LightningBindGetterSetter(BlendSettings);
  LightningBindGetterSetter(DepthSettings);
  LightningBindGetterSetter(CullMode);
  LightningBindGetterSetter(GlobalShaderInputs);
}

GraphicsRenderSettings::GraphicsRenderSettings()
{
  ClearAll();
}

void GraphicsRenderSettings::ClearAll()
{
  RenderSettings::ClearAll();
  mGlobalShaderInputs = nullptr;
}

void GraphicsRenderSettings::SetColorTarget(RenderTarget* target)
{
  if (target == nullptr)
  {
    mColorTextures[0] = nullptr;
    mColorTargets[0] = nullptr;
  }
  else
  {
    mColorTextures[0] = target->mTexture;
    mColorTargets[0] = target->mTexture->mRenderData;
  }
}

void GraphicsRenderSettings::SetDepthTarget(RenderTarget* target)
{
  if (target == nullptr)
  {
    mDepthTexture = nullptr;
    mDepthTarget = nullptr;
  }
  else
  {
    mDepthTexture = target->mTexture;
    mDepthTarget = target->mTexture->mRenderData;
  }
}

void GraphicsRenderSettings::SetColorTargetMrt(RenderTarget* target, uint index)
{
  if (index >= 8)
    return DoNotifyException("Error", "Invalid index. Must be 0-7.");

  if (index > 0)
    mSingleColorTarget = false;

  if (target == nullptr)
  {
    mColorTextures[index] = nullptr;
    mColorTargets[index] = nullptr;
  }
  else
  {
    mColorTextures[index] = target->mTexture;
    mColorTargets[index] = target->mTexture->mRenderData;
  }
}

GraphicsBlendSettings* GraphicsRenderSettings::GetBlendSettings()
{
  return (GraphicsBlendSettings*)RenderSettings::GetBlendSettings();
}

void GraphicsRenderSettings::SetBlendSettings(GraphicsBlendSettings* blendSettings)
{
  RenderSettings::SetBlendSettings(blendSettings);
}

GraphicsDepthSettings* GraphicsRenderSettings::GetDepthSettings()
{
  return (GraphicsDepthSettings*)RenderSettings::GetDepthSettings();
}

void GraphicsRenderSettings::SetDepthSettings(GraphicsDepthSettings* depthSettings)
{
  RenderSettings::SetDepthSettings(depthSettings);
}

GraphicsBlendSettings* GraphicsRenderSettings::GetBlendSettingsMrt(uint index)
{
  if (index >= 8)
  {
    DoNotifyException("Error", "Invalid index. Must be 0-7.");
    return nullptr;
  }

  return (GraphicsBlendSettings*)&mBlendSettings[index];
}

void GraphicsRenderSettings::SetBlendSettingsMrt(GraphicsBlendSettings* blendSettings, uint index)
{
  if (index >= 8)
    return DoNotifyException("Error", "Invalid index. Must be 0-7.");

  mBlendSettings[index] = *blendSettings;
}

ShaderInputs* GraphicsRenderSettings::GetGlobalShaderInputs()
{
  return mGlobalShaderInputs;
}

void GraphicsRenderSettings::SetGlobalShaderInputs(ShaderInputs* shaderInputs)
{
  mGlobalShaderInputs = shaderInputs;
}

MultiRenderTarget* GraphicsRenderSettings::GetMultiRenderTarget()
{
  MultiRenderTarget* multiRenderTarget = new MultiRenderTarget(this);
  multiRenderTarget->mReferenceCount.mCount--;
  return multiRenderTarget;
}

LightningDefineType(ColorTargetMrt, builder, type)
{
  PlasmaBindDocumented();

  LightningBindMethod(Set);
}

void ColorTargetMrt::Set(uint index, RenderTarget* colorTarget)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object.");
  mRenderSettings->SetColorTargetMrt(colorTarget, index);
}

LightningDefineType(BlendSettingsMrt, builder, type)
{
  PlasmaBindDocumented();

  LightningBindMethod(Get);
  LightningBindMethod(Set);
}

void BlendSettingsMrt::Set(uint index, GraphicsBlendSettings* blendSettings)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object.");
  mRenderSettings->SetBlendSettingsMrt(blendSettings, index);
}

GraphicsBlendSettings* BlendSettingsMrt::Get(uint index)
{
  if (mRenderSettings.IsNull())
  {
    DoNotifyException("Error", "Attempting to call member on null object.");
    return nullptr;
  }
  return mRenderSettings->GetBlendSettingsMrt(index);
}

LightningDefineType(MultiRenderTarget, builder, type)
{
  PlasmaBindDocumented();

  LightningBindGetterAs(ColorTargetMrt, "ColorTarget");
  LightningBindSetter(ColorTarget0);
  LightningBindSetter(ColorTarget1);
  LightningBindSetter(ColorTarget2);
  LightningBindSetter(ColorTarget3);
  LightningBindSetter(ColorTarget4);
  LightningBindSetter(ColorTarget5);
  LightningBindSetter(ColorTarget6);
  LightningBindSetter(ColorTarget7);

  LightningBindGetterAs(BlendSettingsMrt, "BlendSettings");
  LightningBindGetterSetter(BlendSettings0);
  LightningBindGetterSetter(BlendSettings1);
  LightningBindGetterSetter(BlendSettings2);
  LightningBindGetterSetter(BlendSettings3);
  LightningBindGetterSetter(BlendSettings4);
  LightningBindGetterSetter(BlendSettings5);
  LightningBindGetterSetter(BlendSettings6);
  LightningBindGetterSetter(BlendSettings7);
}

MultiRenderTarget::MultiRenderTarget(HandleOf<GraphicsRenderSettings> renderSettings) :
    mRenderSettings(renderSettings),
    mColorTargetMrt(renderSettings),
    mBlendSettingsMrt(renderSettings)
{
}

} // namespace Plasma
