// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "LightningSpirVFrontEndValidation.hpp"

namespace Plasma
{

void ValidateEntryPoint(LightningSpirVFrontEnd* translator,
                        Lightning::GenericFunctionNode* node,
                        LightningSpirVFrontEndContext* context)
{
  Lightning::DelegateType* functionType = node->DefinedFunction->FunctionType;
  if (functionType->Return != LightningTypeId(void))
  {
    translator->SendTranslationError(node->Location, "Entry points must have a return type of 'void'.");
    return;
  }

  LightningShaderIRType* currentType = context->mCurrentType;
  FragmentType::Enum fragmentType = currentType->mMeta->mFragmentType;
  // Quick validation on entry points. Entry points can't be on random helper
  // classes.
  if (fragmentType == FragmentType::None)
  {
    translator->SendTranslationError(node->Location, "Entry point requires fragment type.");
    return;
  }
  else if (fragmentType == FragmentType::Geometry)
    ValidateGeometryEntryPoint(translator, node, context);
  else
    ValidateBasicEntryPoint(translator, node, context);
}

void ValidateBasicEntryPoint(LightningSpirVFrontEnd* translator,
                             Lightning::GenericFunctionNode* node,
                             LightningSpirVFrontEndContext* context)
{
  // Vertex/Pixel entry points can't have any arguments.
  if (node->Parameters.Size() != 0)
  {
    translator->SendTranslationError(node->Location, "Entry point function cannot have arguments.");
    return;
  }
}

void ValidateGeometryEntryPoint(LightningSpirVFrontEnd* translator,
                                Lightning::GenericFunctionNode* node,
                                LightningSpirVFrontEndContext* context)
{
  SpirVNameSettings& nameSettings = translator->mSettings->mNameSettings;

  if (node->Parameters.Size() != 2)
  {
    translator->SendTranslationError(node->Location,
                                     "Geometry shader entry point must have a "
                                     "signature of (inputType, outputType)");
    return;
  }

  LightningShaderIRType* currentType = context->mCurrentType;
  LightningShaderIRType* inputType = translator->FindType(node->Parameters[0]);
  LightningShaderIRType* outputType = translator->FindType(node->Parameters[1]);

  Lightning::GeometryStreamUserData* inputUserData = inputType->mLightningType->Has<Lightning::GeometryStreamUserData>();
  Lightning::GeometryStreamUserData* outputUserData = outputType->mLightningType->Has<Lightning::GeometryStreamUserData>();

  // Validate that the parameters are the correct input/output types.
  if (inputUserData == nullptr || inputUserData->mInput == false)
  {
    translator->SendTranslationError(node->Parameters[0]->Location, "Argument 1 must be an input stream type.");
  }
  if (outputUserData == nullptr || outputUserData->mInput == true)
  {
    translator->SendTranslationError(node->Parameters[1]->Location, "Argument 2 must be an output stream type.");
  }

  // Validate a specified max vertices value
  ShaderIRAttributeParameter* maxVerticesParam = nullptr;
  ShaderIRAttribute* geometryAttribute = currentType->FindFirstAttribute(nameSettings.mGeometryAttribute);
  if (geometryAttribute != nullptr)
    maxVerticesParam = geometryAttribute->FindFirstParameter(nameSettings.mMaxVerticesParam);

  if (maxVerticesParam == nullptr)
  {
    translator->SendTranslationError(node->Location, "Geometry fragment expects max vertices");
    return;
  }
}

} // namespace Plasma
