// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningShaderIROp* GetOrCreateLanguageSpecConstant(LightningSpirVFrontEnd* translator,
                                                 void* specKey,
                                                 int defaultValue,
                                                 StringParam specName,
                                                 LightningSpirVFrontEndContext* context)
{
  // Check if this constant already exists
  LightningShaderIROp* specConstantOp = translator->mLibrary->FindSpecializationConstantOp(specKey);
  if (specConstantOp == nullptr)
  {
    // If it doesn't, create it (hardcoded int)
    LightningShaderIRType* intType = translator->mLibrary->FindType(LightningTypeId(int));
    LightningShaderIRConstantLiteral* defaultLiteral = translator->GetOrCreateConstantIntegerLiteral(defaultValue);
    specConstantOp = translator->CreateSpecializationConstant(specKey, OpType::OpSpecConstant, intType, context);
    specConstantOp->mArguments.PushBack(defaultLiteral);
    specConstantOp->mDebugResultName = specName;
  }
  return specConstantOp;
}

LightningShaderIROp* GetLanguageSpecConstant(LightningSpirVFrontEnd* translator, LightningSpirVFrontEndContext* context)
{
  LightningShaderSpirVSettings* settings = translator->mSettings;
  String languageIdName = settings->GetLanguageSpecializationName();
  void* languageKey = settings->GetLanguageSpecializationKey();
  return GetOrCreateLanguageSpecConstant(translator, languageKey, 0, languageIdName, context);
}

LightningShaderIROp* GetLanguageVersionSpecConstant(LightningSpirVFrontEnd* translator, LightningSpirVFrontEndContext* context)
{
  LightningShaderSpirVSettings* settings = translator->mSettings;
  String languageVersionName = settings->GetLanguageVersionSpecializationName();
  void* languageVersionKey = translator->mSettings->GetLanguageVersionSpecializationKey();
  return GetOrCreateLanguageSpecConstant(translator, languageVersionKey, 150, languageVersionName, context);
}

void ResolveIsLanguage(LightningSpirVFrontEnd* translator,
                       Lightning::FunctionCallNode* functionCallNode,
                       Lightning::MemberAccessNode* memberAccessNode,
                       LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* boolType = translator->mLibrary->FindType(LightningTypeId(bool));

  // Get the specialization constant for the language id (is this glsl?)
  LightningShaderIROp* languageSpecConst = GetLanguageSpecConstant(translator, context);
  // Check if the given language id is equal to the current language
  LightningShaderIROp* comparisonLanguageOp =
      translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  LightningShaderIROp* result =
      translator->BuildCurrentBlockIROp(OpType::OpIEqual, boolType, languageSpecConst, comparisonLanguageOp, context);

  context->PushIRStack(result);
}

void ResolveIsLanguageMinMaxVersion(LightningSpirVFrontEnd* translator,
                                    Lightning::FunctionCallNode* functionCallNode,
                                    Lightning::MemberAccessNode* memberAccessNode,
                                    LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* boolType = translator->mLibrary->FindType(LightningTypeId(bool));

  // Get the specialization constant for the language id (is this glsl?)
  LightningShaderIROp* languageSpecConst = GetLanguageSpecConstant(translator, context);
  // Get the specialization constant for the language version (is this glsl 450,
  // hlsl 100?)
  LightningShaderIROp* languageVersionSpecConst = GetLanguageVersionSpecConstant(translator, context);
  // Read all of the arguments
  LightningShaderIROp* comparisonLanguageOp =
      translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  LightningShaderIROp* minVersionOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);
  LightningShaderIROp* maxVersionOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[2], context);

  // Check the language id
  LightningShaderIROp* isLanguageOp =
      translator->BuildCurrentBlockIROp(OpType::OpIEqual, boolType, languageSpecConst, comparisonLanguageOp, context);
  // Check the minVersion <= languageVersion <= maxVersion
  LightningShaderIROp* isGreaterThanMinOp = translator->BuildCurrentBlockIROp(
      OpType::OpSLessThanEqual, boolType, minVersionOp, languageVersionSpecConst, context);
  LightningShaderIROp* isLessThanMaxOp = translator->BuildCurrentBlockIROp(
      OpType::OpSLessThanEqual, boolType, languageVersionSpecConst, maxVersionOp, context);
  // Combine the comparisons together into one bool
  LightningShaderIROp* isInVersionRange =
      translator->BuildCurrentBlockIROp(OpType::OpLogicalAnd, boolType, isGreaterThanMinOp, isLessThanMaxOp, context);
  LightningShaderIROp* result =
      translator->BuildCurrentBlockIROp(OpType::OpLogicalAnd, boolType, isLanguageOp, isInVersionRange, context);

  context->PushIRStack(result);
}

void RegisterShaderIntrinsics(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary)
{
  // Find the shader intrinsics type
  Lightning::BoundType* shaderIntrinsicsType =
      shaderLibrary->mLightningLibrary->BoundTypes.FindValue("ShaderIntrinsics", nullptr);
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[shaderIntrinsicsType];

  // Walk all functions and register any resolvers if they exist
  Lightning::MemberRange<Lightning::Function> fuctionRange = shaderIntrinsicsType->GetFunctions();
  for (; !fuctionRange.Empty(); fuctionRange.PopFront())
  {
    Lightning::Function* fn = fuctionRange.Front();
    // During creation of all of the functions on this type we currently set the
    // user data to member resolver function to use (this is assumed to match)
    if (fn->UserData != nullptr)
    {
      typeResolver.RegisterFunctionResolver(fn, (MemberFunctionResolverIRFn)fn->UserData);
    }
  }
}

} // namespace Plasma
