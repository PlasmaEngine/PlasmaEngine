// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningSpirVFrontEndContext::LightningSpirVFrontEndContext()
{
  mCurrentFunction = nullptr;
  mContinueTarget = nullptr;
  mBreakTarget = nullptr;
}

BasicBlock* LightningSpirVFrontEndContext::GetCurrentBlock()
{
  return mCurrentBlock;
}

void LightningSpirVFrontEndContext::PushMergePoints(BasicBlock* continuePoint, BasicBlock* breakPoint)
{
  mMergePointStack.PushBack(Pair<BasicBlock*, BasicBlock*>(continuePoint, breakPoint));
  mContinueTarget = continuePoint;
  mBreakTarget = breakPoint;
}

void LightningSpirVFrontEndContext::PopMergeTargets()
{
  mMergePointStack.PopBack();
  if (mMergePointStack.Empty())
  {
    mContinueTarget = nullptr;
    mBreakTarget = nullptr;
    return;
  }

  Pair<BasicBlock*, BasicBlock*>& prevMergePoints = mMergePointStack.Back();
  mContinueTarget = prevMergePoints.first;
  mBreakTarget = prevMergePoints.second;
}

void LightningSpirVFrontEndContext::PushIRStack(ILightningShaderIR* ir)
{
  mResultStack.PushBack(ir);
}

ILightningShaderIR* LightningSpirVFrontEndContext::PopIRStack()
{
  ILightningShaderIR* ir = mResultStack.Back();
  mResultStack.PopBack();
  return ir;
}

LightningSpirVFrontEnd::~LightningSpirVFrontEnd()
{
}

void LightningSpirVFrontEnd::SetSettings(LightningShaderSpirVSettingsRef& settings)
{
  mSettings = settings;
  if (!mSettings->IsFinalized())
    mSettings->Finalize();
}

void LightningSpirVFrontEnd::Setup()
{
  mWalker.RegisterNonLeafBase(&LightningSpirVFrontEnd::ExtractLightningAsComments);

  mWalker.Register(&LightningSpirVFrontEnd::WalkClassNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkClassVariables);
  mWalker.Register(&LightningSpirVFrontEnd::WalkClassConstructor);
  mWalker.Register(&LightningSpirVFrontEnd::WalkClassFunction);
  mWalker.Register(&LightningSpirVFrontEnd::WalkFunctionCallNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkLocalVariable);
  mWalker.Register(&LightningSpirVFrontEnd::WalkStaticTypeOrCreationCallNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkExpressionInitializerNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkUnaryOperationNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkBinaryOperationNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkCastNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkValueNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkLocalRef);
  mWalker.Register(&LightningSpirVFrontEnd::WalkMemberAccessNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkIfRootNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkIfNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkContinueNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkBreakNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkReturnNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkWhileNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkDoWhileNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkForNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkForEachNode);
  mWalker.Register(&LightningSpirVFrontEnd::WalkLoopNode);

  mLightningCommentParser.Setup();
}

LightningShaderIRType* LightningSpirVFrontEnd::MakeTypeInternal(LightningShaderIRLibrary* shaderLibrary,
                                                        ShaderIRTypeBaseType::Enum baseType,
                                                        StringParam typeName,
                                                        Lightning::BoundType* lightningType,
                                                        spv::StorageClass storageClass)
{
  LightningShaderIRType* shaderType = shaderLibrary->FindType(typeName);
  ErrorIf(shaderType != nullptr, "Type '%s' already exists.", typeName.c_str());

  shaderType = new LightningShaderIRType();
  shaderType->mBaseType = baseType;
  shaderType->mLightningType = lightningType;
  shaderType->mName = typeName;
  shaderType->mShaderLibrary = shaderLibrary;
  shaderType->mStorageClass = storageClass;
  shaderLibrary->AddType(shaderType->mName, shaderType);
  // Don't set the debug name here, that's up to the callee if they want a debug
  // name
  return shaderType;
}

LightningShaderIRType* LightningSpirVFrontEnd::MakeTypeAndPointer(LightningShaderIRLibrary* shaderLibrary,
                                                          ShaderIRTypeBaseType::Enum baseType,
                                                          StringParam typeName,
                                                          Lightning::BoundType* lightningType,
                                                          spv::StorageClass pointerStorageClass)
{
  ErrorIf(baseType == ShaderIRTypeBaseType::Pointer, "BaseType cannot be a pointer type");
  // Make both the regular shader type and the pointer type
  LightningShaderIRType* shaderType =
      MakeTypeInternal(shaderLibrary, baseType, typeName, lightningType, spv::StorageClassGeneric);
  LightningShaderIRType* pointerType = MakeTypeInternal(
      shaderLibrary, ShaderIRTypeBaseType::Pointer, BuildString(typeName, "_ptr"), lightningType, pointerStorageClass);
  // Link both types up to each other
  pointerType->mDereferenceType = shaderType;
  shaderType->mPointerType = pointerType;
  return shaderType;
}

LightningShaderIRType* LightningSpirVFrontEnd::MakeCoreType(LightningShaderIRLibrary* shaderLibrary,
                                                    ShaderIRTypeBaseType::Enum baseType,
                                                    u32 components,
                                                    LightningShaderIRType* componentType,
                                                    Lightning::BoundType* lightningType,
                                                    bool makePointerType)
{
  LightningShaderIRType* shaderType =
      MakeTypeAndPointer(shaderLibrary, baseType, lightningType->Name, lightningType, spv::StorageClassFunction);
  shaderType->mComponentType = componentType;
  shaderType->mComponents = components;
  return shaderType;
}

LightningShaderIRType* LightningSpirVFrontEnd::MakeStructType(LightningShaderIRLibrary* shaderLibrary,
                                                      StringParam typeName,
                                                      Lightning::BoundType* lightningType,
                                                      spv::StorageClass pointerStorageClass)
{
  return MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::Struct, typeName, lightningType, pointerStorageClass);
}

LightningShaderIRType* LightningSpirVFrontEnd::FindOrCreateInterfaceType(LightningShaderIRLibrary* shaderLibrary,
                                                                 StringParam baseTypeName,
                                                                 Lightning::BoundType* lightningType,
                                                                 ShaderIRTypeBaseType::Enum baseType,
                                                                 spv::StorageClass storageClass)
{
  // Build the fully qualified type name which is need for interface types
  StringBuilder builder;
  builder.Append(baseTypeName);
  if (baseType == ShaderIRTypeBaseType::Pointer)
    builder.Append("_ptr");
  if (storageClass == spv::StorageClassFunction)
    builder.Append("_Function");
  else if (storageClass == spv::StorageClassInput)
    builder.Append("_Input");
  else if (storageClass == spv::StorageClassOutput)
    builder.Append("_Output");
  else if (storageClass == spv::StorageClassUniform)
    builder.Append("_Uniform");
  else if (storageClass == spv::StorageClassStorageBuffer)
    builder.Append("_StorageBuffer");
  else
  {
    Error("Unknown storage class");
  }

  // Check if this type already exists
  String fullTypeName = builder.ToString();
  LightningShaderIRType* shaderType = shaderLibrary->FindType(fullTypeName);
  if (shaderType != nullptr)
    return shaderType;

  // Create the type with the fully qualified name
  shaderType = MakeTypeInternal(shaderLibrary, baseType, fullTypeName, lightningType, storageClass);
  return shaderType;
}

LightningShaderIRType* LightningSpirVFrontEnd::FindOrCreateInterfaceType(LightningShaderIRLibrary* shaderLibrary,
                                                                 Lightning::BoundType* lightningType,
                                                                 ShaderIRTypeBaseType::Enum baseType,
                                                                 spv::StorageClass storageClass)
{
  return FindOrCreateInterfaceType(shaderLibrary, lightningType->Name, lightningType, baseType, storageClass);
}

LightningShaderIRType* LightningSpirVFrontEnd::FindOrCreatePointerInterfaceType(LightningShaderIRLibrary* shaderLibrary,
                                                                        LightningShaderIRType* valueType,
                                                                        spv::StorageClass storageClass)
{
  ErrorIf(valueType->IsPointerType(), "Type must be a value type");
  LightningShaderIRType* pointerType = FindOrCreateInterfaceType(
      shaderLibrary, valueType->mName, valueType->mLightningType, ShaderIRTypeBaseType::Pointer, storageClass);

  pointerType->mDereferenceType = valueType;
  return pointerType;
}

ShaderIRTypeMeta* LightningSpirVFrontEnd::MakeShaderTypeMeta(LightningShaderIRType* shaderType,
                                                         Lightning::NodeList<Lightning::AttributeNode>* nodeAttributeList)
{
  Lightning::BoundType* lightningType = shaderType->mLightningType;
  ShaderIRTypeMeta* typeMeta = new ShaderIRTypeMeta();
  typeMeta->mLightningName = shaderType->mName;
  typeMeta->mLightningType = lightningType;
  // Parse attributes if possible (interface types don't have backing lightning
  // types)
  if (lightningType != nullptr)
    ParseAttributes(lightningType->Attributes, nodeAttributeList, typeMeta);
  shaderType->mMeta = typeMeta;
  mLibrary->mOwnedTypeMeta.PushBack(typeMeta);
  return typeMeta;
}

bool LightningSpirVFrontEnd::Translate(Lightning::SyntaxTree& syntaxTree,
                                   LightningShaderIRProject* project,
                                   LightningShaderIRLibrary* library)
{
  mErrorTriggered = false;
  LightningSpirVFrontEndContext context;
  mContext = &context;
  mLibrary = library;
  mProject = project;

  // Collect all types
  TranslatorBranchWalker classTypeCollectorWalker;
  classTypeCollectorWalker.Register(&LightningSpirVFrontEnd::CollectClassTypes);
  classTypeCollectorWalker.Register(&LightningSpirVFrontEnd::CollectEnumTypes);
  classTypeCollectorWalker.Walk(this, syntaxTree.Root, &context);
  // If this failed somehow then early return
  if (mErrorTriggered)
  {
    mContext = nullptr;
    return !mErrorTriggered;
  }

  // Now that we know about all types we can generate all template types
  // (templates have dependencies on other types)
  PreWalkTemplateTypes(&context);

  // Now we can walk classes again to get variable and function signatures since
  // we have all types
  TranslatorBranchWalker preWalker;
  preWalker.Register(&LightningSpirVFrontEnd::PreWalkClassNode);
  preWalker.Register(&LightningSpirVFrontEnd::PreWalkClassFunction);
  preWalker.Register(&LightningSpirVFrontEnd::PreWalkClassConstructor);
  preWalker.Register(&LightningSpirVFrontEnd::PreWalkClassVariables);
  preWalker.Walk(this, syntaxTree.Root, &context);

  // Check for errors again
  if (mErrorTriggered)
  {
    mContext = nullptr;
    return !mErrorTriggered;
  }

  mWalker.Walk(this, syntaxTree.Root, &context);
  library->mTranslated = true;
  mContext = nullptr;

  // Do an additional pass to make sure stage requirements are met
  // (e.g. a vertex doesn't call a pixel only function)
  StageRequirementsGatherer gatherer(mSettings);
  bool stageRequirementsValid = gatherer.Run(syntaxTree, library, mProject);
  mErrorTriggered |= stageRequirementsValid;

  // Do another pass to find cycles (recursion is illegal in shaders)
  CycleDetection cycleDetection(mSettings);
  bool cyclesFound = cycleDetection.Run(syntaxTree, library, mProject);
  mErrorTriggered |= cyclesFound;

  return !mErrorTriggered;
}

void LightningSpirVFrontEnd::ExtractLightningAsComments(Lightning::SyntaxNode*& node, LightningSpirVFrontEndContext* context)
{
  context->Flags = Lightning::WalkerFlags::ChildrenNotHandled;
  if (!Lightning::StatementNode::IsNodeUsedAsStatement(node))
    return;

  // As long as the statement isn't a scoped based node (if, for, while, etc)
  // then we know it requires delimiting
  if (Lightning::Type::DynamicCast<Lightning::ScopeNode*>(node) != nullptr ||
      Lightning::Type::DynamicCast<Lightning::IfRootNode*>(node) != nullptr)
    return;

  ExtractDebugInfo(node, context->mDebugInfo);
}

void LightningSpirVFrontEnd::ParseAttributes(Lightning::Array<Lightning::Attribute>& lightningAttributes,
                                         Lightning::NodeList<Lightning::AttributeNode>* attributeNodes,
                                         ShaderIRTypeMeta* typeMeta)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderIRAttributeList& shaderAttributes = typeMeta->mAttributes;

  ParseLightningAttributes(lightningAttributes, attributeNodes, shaderAttributes);
  ValidateAllowedAttributes(shaderAttributes, nameSettings.mAllowedClassAttributes, "types");

  Array<size_t> fragmentTypeAttributeIndices;
  for (size_t i = 0; i < shaderAttributes.Size(); ++i)
  {
    ShaderIRAttribute* shaderAttribute = shaderAttributes[i];

    String attributeName = shaderAttribute->mAttributeName;

    if (attributeName == nameSettings.mVertexAttribute)
    {
      fragmentTypeAttributeIndices.PushBack(i);
    }
    else if (attributeName == nameSettings.mGeometryAttribute)
    {
      fragmentTypeAttributeIndices.PushBack(i);
      ValidateSingleParamAttribute(
          shaderAttribute, nameSettings.mMaxVerticesParam, Lightning::ConstantType::Integer, false);
    }
    else if (attributeName == nameSettings.mPixelAttribute)
    {
      fragmentTypeAttributeIndices.PushBack(i);
    }
    else if (attributeName == nameSettings.mComputeAttribute)
    {
      fragmentTypeAttributeIndices.PushBack(i);
      ValidateAndParseComputeAttributeParameters(shaderAttribute, typeMeta);
    }
    else if (attributeName == nameSettings.mStorageClassAttribute)
    {
      ValidateSingleParamAttribute(shaderAttribute, String(), Lightning::ConstantType::Integer, true);
    }
    else
      ValidateAttributeNoParameters(shaderAttribute);
  }

  // If we found more than one fragment type attribute
  if (fragmentTypeAttributeIndices.Size() > 1)
  {
    ShaderIRAttribute* shaderAttribute0 = shaderAttributes[fragmentTypeAttributeIndices[0]];
    ShaderIRAttribute* shaderAttribute1 = shaderAttributes[fragmentTypeAttributeIndices[1]];
    String msg = String::Format("Attribute '%s' cannot be combined with attribute '%s'",
                                shaderAttribute1->mAttributeName.c_str(),
                                shaderAttribute0->mAttributeName.c_str());
    SendTranslationError(shaderAttribute1->GetLocation(), msg);
  }
}

void LightningSpirVFrontEnd::ParseAttributes(Lightning::Array<Lightning::Attribute>& lightningAttributes,
                                         Lightning::NodeList<Lightning::AttributeNode>* attributeNodes,
                                         ShaderIRFunctionMeta* functionMeta)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderIRAttributeList& shaderAttributes = functionMeta->mAttributes;

  ParseLightningAttributes(lightningAttributes, attributeNodes, shaderAttributes);
  ValidateAllowedAttributes(shaderAttributes, nameSettings.mAllowedFunctionAttributes, "functions");

  for (size_t i = 0; i < shaderAttributes.Size(); ++i)
  {
    ShaderIRAttribute* shaderAttribute = shaderAttributes[i];

    String attributeName = shaderAttribute->mAttributeName;
    // Extension should be validated by lightning?
    if (attributeName == nameSettings.mExtensionAttribute)
      continue;
    else
      ValidateAttributeNoParameters(shaderAttribute);
  }
}

void LightningSpirVFrontEnd::ParseAttributes(Lightning::Array<Lightning::Attribute>& lightningAttributes,
                                         Lightning::NodeList<Lightning::AttributeNode>* attributeNodes,
                                         ShaderIRFieldMeta* fieldMeta)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderIRAttributeList& shaderAttributes = fieldMeta->mAttributes;

  ParseLightningAttributes(lightningAttributes, attributeNodes, shaderAttributes);
  ValidateAllowedAttributes(shaderAttributes, nameSettings.mAllowedFieldAttributes, "fields");

  Array<String> staticExclusions;
  staticExclusions.PushBack(nameSettings.mStaticAttribute);

  for (size_t i = 0; i < shaderAttributes.Size(); ++i)
  {
    ShaderIRAttribute* shaderAttribute = shaderAttributes[i];

    String attributeName = shaderAttribute->mAttributeName;
    // Inputs
    if (attributeName == nameSettings.mInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mFragmentInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mStageInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mHardwareBuiltInInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateHardwareBuiltIn(fieldMeta, shaderAttribute, true);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mAppBuiltInInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);

      // If this is an explicit attribute then check to see if it matches
      // something in the uniform buffers
      if (!shaderAttribute->mImplicitAttribute)
      {
        FragmentType::Enum currentFragmentType = mContext->mCurrentType->mMeta->mFragmentType;
        String fieldType = fieldMeta->mLightningType->ToString();
        // Make sure to get the actual field name being searched for (in case of
        // overrides)
        String appBuiltInName = fieldMeta->GetFieldAttributeName(shaderAttribute);
        bool isValid = mSettings->IsValidUniform(currentFragmentType, fieldType, appBuiltInName);
        if (!isValid)
        {
          String msg = String::Format("Field '%s : %s' does not match any "
                                      "provided %s with fragment type '%s'.",
                                      appBuiltInName.c_str(),
                                      fieldType.c_str(),
                                      shaderAttribute->mAttributeName.c_str(),
                                      FragmentType::Names[currentFragmentType]);
          SendTranslationError(shaderAttribute->GetLocation(), msg);
          break;
        }
      }
    }
    else if (attributeName == nameSettings.mPropertyInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mSpecializationConstantInputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    // Outputs
    else if (attributeName == nameSettings.mOutputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mFragmentOutputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mStageOutputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mHardwareBuiltInOutputAttribute)
    {
      ValidateNameOverrideAttribute(shaderAttribute);
      ValidateHardwareBuiltIn(fieldMeta, shaderAttribute, false);
      ValidateAttributeExclusions(shaderAttribute, shaderAttributes, staticExclusions);
    }
    else if (attributeName == nameSettings.mSpecializationConstantAttribute)
    {
      ValidateAttributeNoParameters(shaderAttribute);

      Array<String> dependencies;
      dependencies.PushBack(nameSettings.mStaticAttribute);
      ValidateAttributeDependencies(shaderAttribute, shaderAttributes, dependencies);
    }
    // Assume all other attribute don't take any parameters
    else
      ValidateAttributeNoParameters(shaderAttribute);
  }
}

void LightningSpirVFrontEnd::ParseLightningAttributes(Lightning::Array<Lightning::Attribute>& lightningAttributes,
                                              Lightning::NodeList<Lightning::AttributeNode>* attributeNodes,
                                              ShaderIRAttributeList& shaderAttributes)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // Parse the attributes and their parameters
  for (size_t attributeIndex = 0; attributeIndex < lightningAttributes.Size(); ++attributeIndex)
  {
    Lightning::Attribute& attribute = lightningAttributes[attributeIndex];

    Lightning::AttributeNode* node = nullptr;
    if (attributeNodes != nullptr)
      node = (*attributeNodes)[attributeIndex];
    ParseLightningAttribute(attribute, node, shaderAttributes);
  }
}

void LightningSpirVFrontEnd::ParseLightningAttribute(Lightning::Attribute& lightningAttribute,
                                             Lightning::AttributeNode* attributeNode,
                                             ShaderIRAttributeList& shaderAttributes)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  ShaderIRAttribute* shaderAttribute = shaderAttributes.AddAttribute(lightningAttribute.Name, attributeNode);

  for (size_t paramIndex = 0; paramIndex < lightningAttribute.Parameters.Size(); ++paramIndex)
  {
    Lightning::AttributeParameter& param = lightningAttribute.Parameters[paramIndex];
    shaderAttribute->mParameters.PushBack(ShaderIRAttributeParameter(param, nullptr));
  }

  // Set locations if available
  if (attributeNode != nullptr)
  {
    for (size_t paramIndex = 0; paramIndex < lightningAttribute.Parameters.Size(); ++paramIndex)
    {
      Lightning::SyntaxNode* paramNode = attributeNode->AttributeCall->Arguments[paramIndex];
      shaderAttribute->mParameters[paramIndex].SetLocationNode(paramNode);
    }
  }

  // The [Input] attribute implies several other attributes
  if (lightningAttribute.Name == nameSettings.mInputAttribute)
  {
    // Create a copy of the [Input] attribute with all of the sub-attributes
    // (copy the attributes first since they're not safely stored)
    Array<ShaderIRAttributeParameter> params = shaderAttribute->mParameters;
    for (size_t i = 0; i < nameSettings.mInputSubAttributes.Size(); ++i)
    {
      String subAttributeName = nameSettings.mInputSubAttributes[i];
      ShaderIRAttribute* subAttribute = shaderAttributes.AddAttribute(subAttributeName, attributeNode);
      subAttribute->mParameters.Append(params.All());
      subAttribute->mImplicitAttribute = true;
    }
  }
  // [Output] also has sub-attributes
  else if (lightningAttribute.Name == nameSettings.mOutputAttribute)
  {
    Array<ShaderIRAttributeParameter> params = shaderAttribute->mParameters;
    for (size_t i = 0; i < nameSettings.mOutputSubAttributes.Size(); ++i)
    {
      String subAttributeName = nameSettings.mOutputSubAttributes[i];
      ShaderIRAttribute* subAttribute = shaderAttributes.AddAttribute(subAttributeName, attributeNode);
      subAttribute->mParameters.Append(params.All());
      subAttribute->mImplicitAttribute = true;
    }
  }
}

void LightningSpirVFrontEnd::ValidateAllowedAttributes(ShaderIRAttributeList& shaderAttributes,
                                                   HashMap<String, AttributeInfo>& allowedAttributes,
                                                   StringParam errorTypeName)
{
  for (size_t i = 0; i < shaderAttributes.Size(); ++i)
  {
    ShaderIRAttribute* attribute = shaderAttributes[i];

    if (allowedAttributes.ContainsKey(attribute->mAttributeName))
      continue;

    String msg =
        String::Format("Attribute '%s' is not allowed on %s", attribute->mAttributeName.c_str(), errorTypeName.c_str());
    SendTranslationError(attribute->GetLocation(), msg);
  }
}

void LightningSpirVFrontEnd::ValidateNameOverrideAttribute(ShaderIRAttribute* shaderAttribute)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // No parameters is fine
  size_t paramCount = shaderAttribute->mParameters.Size();
  if (paramCount == 0)
    return;

  // More than one parameter is a guaranteed error
  if (paramCount > 1)
  {
    String msg = String::Format("Too many parameters to attribute '%s'. "
                                "Signature must be '%s : String'",
                                shaderAttribute->mAttributeName.c_str(),
                                SpirVNameSettings::mNameOverrideParam.c_str());
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }
  // One parameter might be an error
  if (paramCount == 1)
  {
    ShaderIRAttributeParameter& param = shaderAttribute->mParameters[0];

    // The param type must be a string
    if (param.GetType() != Lightning::ConstantType::String)
    {
      String msg = String::Format("Invalid parameter type to attribute '%s'. Signature "
                                  "must be '%s : String'",
                                  shaderAttribute->mAttributeName.c_str(),
                                  SpirVNameSettings::mNameOverrideParam.c_str());
      SendTranslationError(shaderAttribute->GetLocation(), msg);
      return;
    }

    // The parameter name must either be empty or it must be 'name'
    String paramName = param.GetName();
    if (!paramName.Empty() && paramName != SpirVNameSettings::mNameOverrideParam)
    {
      String msg = String::Format("Invalid parameter name to attribute '%s'. Signature "
                                  "must be '%s : String'",
                                  shaderAttribute->mAttributeName.c_str(),
                                  SpirVNameSettings::mNameOverrideParam.c_str());
      SendTranslationError(shaderAttribute->GetLocation(), msg);
      return;
    }

    // Validate that the name parameter is a valid identifier (needed for the
    // compositor and a few other places)
    String paramValue = param.GetStringValue();
    if (Lightning::LibraryBuilder::CheckUpperIdentifier(paramValue) == false)
    {
      String msg = String::Format("Parameter '%s' must be a valid lightning uppercase identifier.",
                                  SpirVNameSettings::mNameOverrideParam.c_str());
      SendTranslationError(shaderAttribute->GetLocation(), msg);
      return;
    }
  }
}

void LightningSpirVFrontEnd::ValidateSingleParamAttribute(ShaderIRAttribute* shaderAttribute,
                                                      StringParam expectedParamName,
                                                      Lightning::ConstantType::Enum expectedParamType,
                                                      bool allowEmptyName)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // No parameters is an error
  size_t paramCount = shaderAttribute->mParameters.Size();
  if (paramCount == 0)
  {
    String msg = String::Format("Not enough parameters to attribute '%s'. Signature must be '%s : %s'",
                                shaderAttribute->mAttributeName.c_str(),
                                expectedParamName.c_str(),
                                Lightning::ConstantType::Names[expectedParamType]);
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }

  // More than one parameter is an error
  if (paramCount > 1)
  {
    String msg = String::Format("Too many parameters to attribute '%s'. Signature must be '%s : %s'",
                                shaderAttribute->mAttributeName.c_str(),
                                expectedParamName.c_str(),
                                Lightning::ConstantType::Names[expectedParamType]);
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }

  // One parameter might be an error
  ShaderIRAttributeParameter& param = shaderAttribute->mParameters[0];

  // The param type must match
  Lightning::ConstantType::Enum actualParamType = param.GetType();
  if (actualParamType != expectedParamType)
  {
    String msg = String::Format("Invalid parameter type '%s' to attribute "
                                "'%s'. Signature must be '%s : %s'",
                                Lightning::ConstantType::Names[actualParamType],
                                shaderAttribute->mAttributeName.c_str(),
                                expectedParamName.c_str(),
                                Lightning::ConstantType::Names[expectedParamType]);
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }

  // The parameter name must also match
  String actualParamName = param.GetName();
  if ((actualParamName.Empty() && !allowEmptyName) || (actualParamName != expectedParamName))
  {
    String msg = String::Format("Invalid parameter name '%s' to attribute "
                                "'%s'. Signature must be '%s : %s'",
                                actualParamName.c_str(),
                                shaderAttribute->mAttributeName.c_str(),
                                expectedParamName.c_str(),
                                Lightning::ConstantType::Names[expectedParamType]);
    SendTranslationError(shaderAttribute->GetLocation(), msg);
    return;
  }
}

void LightningSpirVFrontEnd::ValidateAttributeNoParameters(ShaderIRAttribute* shaderAttribute)
{
  // No parameters is fine
  size_t paramCount = shaderAttribute->mParameters.Size();
  if (paramCount != 0)
  {
    String msg = String::Format("Invalid parameter count. Attribute '%s' doesn't allow any parameters",
                                shaderAttribute->mAttributeName.c_str());
    SendTranslationError(shaderAttribute->GetLocation(), msg);
  }
}

void LightningSpirVFrontEnd::ValidateAttributeDependencies(ShaderIRAttribute* shaderAttribute,
                                                       ShaderIRAttributeList& shaderAttributeList,
                                                       Array<String>& dependencies)
{
  // Walk all dependencies, keeping track of what we're missing
  Array<String> missingDependencies;
  for (size_t i = 0; i < dependencies.Size(); ++i)
  {
    String dependencyName = dependencies[i];
    if (shaderAttributeList.FindFirstAttribute(dependencyName) == nullptr)
      missingDependencies.PushBack(dependencyName);
  }

  // If there's any missing dependencies then display an error
  if (!missingDependencies.Empty())
  {
    StringBuilder errBuilder;
    errBuilder.AppendFormat("Attribute '%s' requires attribute(s): ", shaderAttribute->mAttributeName.c_str());
    for (size_t i = 0; i < missingDependencies.Size(); ++i)
    {
      errBuilder.Append(missingDependencies[i]);
      if (i != missingDependencies.Size() - 1)
        errBuilder.Append(", ");
    }
    SendTranslationError(shaderAttribute->GetLocation(), errBuilder.ToString());
  }
}

void LightningSpirVFrontEnd::ValidateAttributeExclusions(ShaderIRAttribute* shaderAttribute,
                                                     ShaderIRAttributeList& shaderAttributeList,
                                                     Array<String>& exclusions)
{
  // Walk all dependencies, keeping track of any that are found
  Array<String> foundExclusions;
  for (size_t i = 0; i < exclusions.Size(); ++i)
  {
    String exclusionName = exclusions[i];
    if (shaderAttributeList.FindFirstAttribute(exclusionName) != nullptr)
      foundExclusions.PushBack(exclusionName);
  }

  // If there's any exclusions found then display an error
  if (!foundExclusions.Empty())
  {
    StringBuilder errBuilder;
    errBuilder.AppendFormat("Attribute '%s' cannot be combined with attribute(s): ",
                            shaderAttribute->mAttributeName.c_str());
    for (size_t i = 0; i < foundExclusions.Size(); ++i)
    {
      errBuilder.Append(foundExclusions[i]);
      if (i != foundExclusions.Size() - 1)
        errBuilder.Append(", ");
    }
    SendTranslationError(shaderAttribute->GetLocation(), errBuilder.ToString());
  }
}

void LightningSpirVFrontEnd::ValidateHardwareBuiltIn(ShaderIRFieldMeta* fieldMeta,
                                                 ShaderIRAttribute* shaderAttribute,
                                                 bool isInput)
{
  // If this is an explicit attribute then check to see if it matches something
  // in the uniform buffers
  if (!shaderAttribute->mImplicitAttribute)
  {
    FragmentType::Enum currentFragmentType = mContext->mCurrentType->mMeta->mFragmentType;
    String fieldType = fieldMeta->mLightningType->ToString();
    // Make sure to get the actual field name being searched for (in case of
    // overrides)
    String appBuiltInName = fieldMeta->GetFieldAttributeName(shaderAttribute);
    bool isValid = mSettings->IsValidHardwareBuiltIn(currentFragmentType, fieldType, appBuiltInName, isInput);
    if (!isValid)
    {
      String msg = String::Format("Field '%s : %s' does not match any provided "
                                  "%s with fragment type '%s'.",
                                  appBuiltInName.c_str(),
                                  fieldType.c_str(),
                                  shaderAttribute->mAttributeName.c_str(),
                                  FragmentType::Names[currentFragmentType]);
      SendTranslationError(shaderAttribute->GetLocation(), msg);
    }
  }
}

void LightningSpirVFrontEnd::ValidateAndParseComputeAttributeParameters(ShaderIRAttribute* shaderAttribute,
                                                                    ShaderIRTypeMeta* typeMeta)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  // Create the user data for the compute fragment to store the parameters we
  // parse from the attribute.
  Lightning::HandleOf<Lightning::ComputeFragmentUserData> handle = LightningAllocate(Lightning::ComputeFragmentUserData);
  // Default all local sizes to 1 (they're all optional)
  handle->mLocalSizeX = 1;
  handle->mLocalSizeY = 1;
  handle->mLocalSizeZ = 1;
  typeMeta->mLightningType->Add(*handle);

  for (size_t i = 0; i < shaderAttribute->mParameters.Size(); ++i)
  {
    ShaderIRAttributeParameter& param = shaderAttribute->mParameters[i];
    String paramName = param.GetName();
    if (paramName == nameSettings.mComputeLocalSizeXParam)
      ValidateLocalSize(param, 128, handle->mLocalSizeX);
    else if (paramName == nameSettings.mComputeLocalSizeYParam)
      ValidateLocalSize(param, 128, handle->mLocalSizeY);
    else if (paramName == nameSettings.mComputeLocalSizeZParam)
      ValidateLocalSize(param, 64, handle->mLocalSizeZ);
    else
    {
      String msg = String::Format("Attribute paramater '%s' is invalid.", paramName.c_str());
      SendTranslationError(param.GetLocation(), msg);
    }
  }
}

void LightningSpirVFrontEnd::ValidateLocalSize(ShaderIRAttributeParameter& param, int max, int& toStore)
{
  int intValue = param.GetIntValue();
  if (intValue <= 0 || intValue > max)
  {
    String msg = String::Format("Parameter '%s' must be in the range of [1, %d].", param.GetName().c_str(), max);
    SendTranslationError(param.GetLocation(), msg);
    return;
  }
  toStore = intValue;
}

String LightningSpirVFrontEnd::BuildFunctionTypeString(Lightning::Function* lightningFunction, LightningSpirVFrontEndContext* context)
{
  // Get the return type of the function (use void if there isn't one)
  Lightning::Type* lightningReturnType;
  if (lightningFunction != nullptr && lightningFunction->FunctionType->Return != nullptr)
    lightningReturnType = lightningFunction->FunctionType->Return;
  else
    lightningReturnType = LightningTypeId(void);

  StringBuilder functionTypeBuilder;
  functionTypeBuilder.Append("(");
  if (lightningFunction != nullptr)
  {
    Lightning::DelegateType* functionType = lightningFunction->FunctionType;
    // Handle adding the self parameter first if this is a member function
    if (!lightningFunction->IsStatic)
    {
      // Get the actual 'this' type from the function (deals with extension
      // methods)
      Lightning::BoundType* lightningSelfType = lightningFunction->Owner;
      functionTypeBuilder.Append(lightningSelfType->IndirectType->ToString());
      functionTypeBuilder.Append(",");
    }
    // Add all parameters
    for (size_t i = 0; i < functionType->Parameters.Size(); ++i)
    {
      Lightning::DelegateParameter& delegateParameter = functionType->Parameters[i];
      functionTypeBuilder.Append(delegateParameter.ParameterType->ToString());
      functionTypeBuilder.Append(",");
    }
  }
  functionTypeBuilder.Append(")");

  // Add the return type
  functionTypeBuilder.Append(" : ");
  String returnType = lightningReturnType->ToString();
  functionTypeBuilder.Append(returnType);

  return functionTypeBuilder.ToString();
}

String LightningSpirVFrontEnd::BuildFunctionTypeString(Lightning::BoundType* lightningReturnType,
                                                   Array<Lightning::Type*>& signature,
                                                   LightningSpirVFrontEndContext* context)
{
  ErrorIf(lightningReturnType == nullptr, "Signature must have at least one argument (return type)");

  StringBuilder functionTypeBuilder;
  // Add all parameters
  functionTypeBuilder.Append("(");
  for (size_t i = 0; i < signature.Size(); ++i)
  {
    functionTypeBuilder.Append(signature[i]->ToString());
    functionTypeBuilder.Append(",");
  }
  functionTypeBuilder.Append(")");

  // Add the return type
  functionTypeBuilder.Append(" : ");
  String returnTypeStr = lightningReturnType->ToString();
  functionTypeBuilder.Append(returnTypeStr);

  return functionTypeBuilder.ToString();
}

void LightningSpirVFrontEnd::GenerateFunctionType(Lightning::SyntaxNode* locationNode,
                                              LightningShaderIRFunction* function,
                                              Lightning::Function* lightningFunction,
                                              LightningSpirVFrontEndContext* context)
{
  ErrorIf(lightningFunction == nullptr, "");
  String functionTypeStr = BuildFunctionTypeString(lightningFunction, context);
  function->mFunctionType = mLibrary->FindType(functionTypeStr, true);
  // If the function type already exists then we're done
  if (function->mFunctionType != nullptr)
    return;

  // Otherwise we have to generate the delegate type
  LightningShaderIRType* functionType = new LightningShaderIRType();
  function->mFunctionType = functionType;
  function->mFunctionType->mBaseType = ShaderIRTypeBaseType::Function;
  function->mFunctionType->mShaderLibrary = mLibrary;
  mLibrary->AddType(functionTypeStr, function->mFunctionType);

  // Extract the function type for the lightning function if we can
  Lightning::DelegateType* lightningFunctionType = nullptr;
  if (lightningFunction != nullptr)
    lightningFunctionType = lightningFunction->FunctionType;

  // Find the return type. If there isn't one then use void.
  Lightning::Type* lightningReturnType;
  if (lightningFunctionType != nullptr && lightningFunctionType->Return != nullptr)
    lightningReturnType = lightningFunctionType->Return;
  else
    lightningReturnType = LightningTypeId(void);

  // Add a reference to the return type
  LightningShaderIRType* returnType = FindType(lightningReturnType, locationNode);
  // If the return is a value type that can't be copied then display an error.
  if (ContainsAttribute(returnType, SpirVNameSettings::mNonCopyableAttributeName))
  {
    String msg =
        String::Format("Type '%s' is an invalid return type as it cannot be copied.", returnType->mName.c_str());
    SendTranslationError(locationNode->Location, msg);
  }
  functionType->mParameters.PushBack(returnType);

  if (lightningFunctionType != nullptr)
  {
    // If this is a member function then we have to add references to the
    // 'this' type and make the first argument the 'this' pointer
    if (!lightningFunction->IsStatic)
    {
      // Get the actual 'this' type from the function (deals with extension
      // methods)
      Lightning::BoundType* lightningThisType = lightningFunction->Owner;
      LightningShaderIRType* thisType = FindType(lightningThisType, locationNode);
      LightningShaderIRType* thisPointerType = thisType->mPointerType;
      functionType->mParameters.PushBack(thisPointerType);
    }

    // Add all parameters and add dependencies on all parameter types
    for (size_t i = 0; i < lightningFunctionType->Parameters.Size(); ++i)
    {
      Lightning::DelegateParameter& parameter = lightningFunctionType->Parameters[i];

      LightningShaderIRType* shaderParameterType = FindType(parameter.ParameterType, locationNode);
      LightningShaderIRType* shaderParamPointerType = shaderParameterType->mPointerType;

      // Make sure to pass through the correct shader type (pointer/value) based
      // upon the lightning type.
      if (parameter.ParameterType->IsIndirectionType(parameter.ParameterType))
        functionType->mParameters.PushBack(shaderParamPointerType);
      else
      {
        // If this is a value type parameter that can't be copied then display
        // an error.
        if (ContainsAttribute(shaderParameterType, SpirVNameSettings::mNonCopyableAttributeName))
        {
          String msg = String::Format("Type '%s' cannot be copied. This parameter must "
                                      "be passed through by reference (ref keyword).",
                                      shaderParameterType->mName.c_str());
          SendTranslationError(locationNode->Location, msg);
        }
        functionType->mParameters.PushBack(shaderParameterType);
      }
    }
  }
}

void LightningSpirVFrontEnd::GenerateFunctionType(Lightning::SyntaxNode* locationNode,
                                              LightningShaderIRFunction* function,
                                              Lightning::BoundType* lightningReturnType,
                                              Array<Lightning::Type*>& signature,
                                              LightningSpirVFrontEndContext* context)
{
  ErrorIf(lightningReturnType == nullptr, "Signature must have a return type");

  String functionTypeStr = BuildFunctionTypeString(lightningReturnType, signature, context);
  function->mFunctionType = mLibrary->FindType(functionTypeStr);
  // If the function type already exists then we're done
  if (function->mFunctionType != nullptr)
    return;

  // Otherwise we have to generate the delegate type
  LightningShaderIRType* functionType = new LightningShaderIRType();
  function->mFunctionType = functionType;
  function->mFunctionType->mBaseType = ShaderIRTypeBaseType::Function;
  function->mFunctionType->mShaderLibrary = mLibrary;
  mLibrary->AddType(functionTypeStr, function->mFunctionType);

  // Set the function return type
  LightningShaderIRType* returnType = FindType(lightningReturnType, locationNode);
  functionType->mParameters.PushBack(returnType);

  // Add all parameters and add dependencies on all parameter types
  for (size_t i = 0; i < signature.Size(); ++i)
  {
    Lightning::Type* parameter = signature[i];

    LightningShaderIRType* shaderParameterType = FindType(parameter, locationNode);
    LightningShaderIRType* shaderParamPointerType = shaderParameterType->mPointerType;

    // Make sure to pass through the correct shader type (pointer/value) based
    // upon the lightning type.
    if (parameter->IsIndirectionType(parameter))
      functionType->mParameters.PushBack(shaderParamPointerType);
    else
      functionType->mParameters.PushBack(shaderParameterType);
  }
}

LightningShaderIRFunction* LightningSpirVFrontEnd::GenerateIRFunction(Lightning::SyntaxNode* node,
                                                              Lightning::NodeList<Lightning::AttributeNode>* nodeAttributeList,
                                                              LightningShaderIRType* owningType,
                                                              Lightning::Function* lightningFunction,
                                                              StringParam functionName,
                                                              LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* currentType = owningType;

  // Generate the function
  LightningShaderIRFunction* function = currentType->CreateFunction(mLibrary);
  function->mName = GetOverloadedName(functionName, lightningFunction);
  function->mDebugResultName = function->mName;
  context->mCurrentFunction = function;

  // If we don't have a backing function then don't insert this
  // into the function map or extract debug information
  if (lightningFunction != nullptr)
  {
    ExtractDebugInfo(node, function->mDebugInfo);
    mLibrary->mFunctions.InsertOrError(lightningFunction, function);
  }

  // We need to generate the type of a function (delegate type).
  // Just like other types, this needs to be generated only once.
  GenerateFunctionType(node, function, lightningFunction, context);

  ShaderIRFunctionMeta* functionMeta = currentType->mMeta->CreateFunction(mLibrary);
  functionMeta->mLightningName = function->mName;
  functionMeta->mLightningFunction = lightningFunction;
  function->mMeta = functionMeta;

  ParseAttributes(lightningFunction->Attributes, nodeAttributeList, functionMeta);
  // If this function is actually a get/set of a property then parse
  // the attributes of property (that has implements and so on)
  if (lightningFunction->OwningProperty != nullptr)
    ParseAttributes(lightningFunction->OwningProperty->Attributes, nodeAttributeList, functionMeta);

  AddImplements(node, lightningFunction, function, functionName, context);

  return function;
}

void LightningSpirVFrontEnd::AddImplements(Lightning::SyntaxNode* node,
                                       Lightning::Function* lightningFunction,
                                       LightningShaderIRFunction* shaderFunction,
                                       StringParam functionName,
                                       LightningSpirVFrontEndContext* context)
{
  if (lightningFunction == nullptr)
    return;

  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // Check if this has the implements attribute
  ShaderIRFunctionMeta* fnMeta = shaderFunction->mMeta;
  if (!fnMeta->ContainsAttribute(nameSettings.mImplementsAttribute))
    return;

  // Find the extension attribute that corresponds to the implements
  ShaderIRAttribute* extAttribute = fnMeta->mAttributes.FindFirstAttribute(nameSettings.mExtensionAttribute);
  // Get the type we're extending (the first parameter)
  ShaderIRAttributeParameter param = extAttribute->mParameters[0];
  Lightning::BoundType* extType = Lightning::BoundType::GetBoundType(param.GetTypeValue());

  // Find the function from lightning's bound type (check static or instance as
  // appropriate)
  Lightning::FindMemberOptions::Enum findOptions = Lightning::FindMemberOptions::None;
  if (lightningFunction->IsStatic)
    findOptions = Lightning::FindMemberOptions::Static;

  Lightning::Function* replacementFn = nullptr;
  // Check to see if this is actually a getter/setter
  if (lightningFunction->OwningProperty != nullptr)
  {
    // If so find the property
    Lightning::Property* property = extType->FindProperty(functionName, findOptions);
    // Make sure the property has the same type as the one we're extending
    if (property != nullptr && property->PropertyType == lightningFunction->OwningProperty->PropertyType)
    {
      // Find which function to use by name (fix later?)
      if (lightningFunction->Name == property->Get->Name)
        replacementFn = property->Get;
      else if (lightningFunction->Name == property->Set->Name)
        replacementFn = property->Set;
      else
      {
        Error("This should never happen");
      }
    }
  }
  else
    replacementFn = extType->FindFunction(functionName, lightningFunction->FunctionType, findOptions);

  // We found a replacement function. Add a duplicate mapping for the lightning
  // function we're replacing to the target shader function
  if (replacementFn != nullptr)
  {
    mLibrary->mFunctions.InsertOrError(replacementFn, shaderFunction);
    return;
  }

  // Otherwise we failed to find a function to implement (override). Now report
  // an error to the user.

  // By default, set the error message as we can't find a function to match to
  StringBuilder msgBuilder;
  msgBuilder << "The signature of a function with the [" << nameSettings.mImplementsAttribute
             << "] attribute must match an existing function.\n";

  // For added error reporting, try to find all possible overloads of this
  // function (check static or instance as appropriate)
  const Lightning::FunctionArray* functions = nullptr;
  ShaderIRFunctionMeta* shaderFnMeta = shaderFunction->mMeta;
  if (shaderFnMeta->ContainsAttribute("Static"))
    functions = extType->GetOverloadedStaticFunctions(functionName);
  else
    functions = extType->GetOverloadedInstanceFunctions(functionName);
  // If we find overloads of the same function name then list all possible
  // overloads as candidates
  if (functions != nullptr)
  {
    msgBuilder.Append("Possible overloads are:\n");
    for (size_t i = 0; i < functions->Size(); ++i)
    {
      Lightning::Function* fn = (*functions)[i];
      msgBuilder << "\t" << fn->ToString() << "\n";
    }
  }
  else if (lightningFunction->OwningProperty != nullptr)
  {
    msgBuilder << "Type '" << extType->ToString() << "' does not contain the property '";
    msgBuilder << functionName << " : " << lightningFunction->OwningProperty->PropertyType->ToString() << "'.";
  }
  else
  {
    msgBuilder << "Type '" << extType->ToString() << "' does not contain a function named '" << functionName << "'.";
  }

  SendTranslationError(node->Location, msgBuilder.ToString());
}

void LightningSpirVFrontEnd::CollectClassTypes(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context)
{
  // Make class type's errors (only allow structs).
  if (node->CopyMode != Lightning::TypeCopyMode::ValueType)
  {
    String msg = "Cannot declare class types in lightning fragments. Use struct instead.";
    SendTranslationError(node->Location, msg);
    return;
  }
  if (node->Inheritance.Size() != 0)
  {
    Lightning::SyntaxType* inheritanceNode = node->Inheritance[0];
    String shortMsg = "Inheritance is not supported in lightning fragments.";
    String longMsg = String::Format("Type '%s' inherits from type '%s' which is not supported.",
                                    node->Name.c_str(),
                                    inheritanceNode->ToString().c_str());
    SendTranslationError(inheritanceNode->Location, shortMsg, longMsg);
    return;
  }

  // Make a new ir type for this struct
  String lightningName = node->Name.Token;
  LightningShaderIRType* type = MakeStructType(mLibrary, lightningName, node->Type, spv::StorageClassFunction);
  type->mDebugResultName = lightningName;

  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  ShaderIRTypeMeta* typeMeta = MakeShaderTypeMeta(type, &node->Attributes);
  if (typeMeta->mAttributes.FindFirstAttribute(nameSettings.mPixelAttribute))
    typeMeta->mFragmentType = FragmentType::Pixel;
  else if (typeMeta->mAttributes.FindFirstAttribute(nameSettings.mVertexAttribute))
    typeMeta->mFragmentType = FragmentType::Vertex;
  else if (typeMeta->mAttributes.FindFirstAttribute(nameSettings.mGeometryAttribute))
    typeMeta->mFragmentType = FragmentType::Geometry;
  else if (typeMeta->mAttributes.FindFirstAttribute(nameSettings.mComputeAttribute))
    typeMeta->mFragmentType = FragmentType::Compute;

  context->mCurrentType = type;

  ExtractDebugInfo(node, type->mDebugInfo);
}

void LightningSpirVFrontEnd::CollectEnumTypes(Lightning::EnumNode*& node, LightningSpirVFrontEndContext* context)
{
  // Map the enum type to integer. This is needed to handle
  // any function taking/returning the enum type.
  LightningShaderIRType* intType = mLibrary->FindType(LightningTypeId(int));
  mLibrary->mTypes[node->Name.Token] = intType;

  // For each value in the enum, create an integer constant that
  // can be looked up via the enum integer property
  for (size_t i = 0; i < node->Values.Size(); ++i)
  {
    Lightning::EnumValueNode* valueNode = node->Values[i];

    LightningShaderIROp* constantValue = GetIntegerConstant(valueNode->IntegralValue, context);
    mLibrary->mEnumContants[valueNode->IntegralProperty] = constantValue;
  }
}

void LightningSpirVFrontEnd::PreWalkClassNode(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context)
{
  context->mCurrentType = FindType(node->Type, node);

  // Destructors cannot be supported as there's no actual
  // call to the destructor in lightning's AST.
  if (node->Destructor != nullptr)
  {
    SendTranslationError(node->Location, "Destructors are not supported in shaders");
    return;
  }

  TranslatorBranchWalker* walker = context->Walker;
  walker->Walk(this, node->Variables, context);
  walker->Walk(this, node->Constructors, context);
  walker->Walk(this, node->Functions, context);
  GeneratePreConstructor(node, context);
  GenerateDefaultConstructor(node, context);
  GenerateDummyMemberVariable(node, context);

  PreWalkErrorCheck(context);

  context->mCurrentType = nullptr;
}

void LightningSpirVFrontEnd::PreWalkTemplateTypes(LightningSpirVFrontEndContext* context)
{
  Lightning::Library* lightningLibrary = mLibrary->mLightningLibrary;
  Lightning::BoundTypeMap::range boundTypes = lightningLibrary->BoundTypes.All();
  for (; !boundTypes.Empty(); boundTypes.PopFront())
  {
    Lightning::BoundType* boundType = boundTypes.Front().second;
    PreWalkTemplateType(boundType, context);
  }
}

void LightningSpirVFrontEnd::PreWalkTemplateType(Lightning::BoundType* lightningType, LightningSpirVFrontEndContext* context)
{
  // Make sure this is actually a template
  if (lightningType->TemplateBaseName.Empty())
    return;

  // Deal with already having a translation for the type
  if (mLibrary->FindType(lightningType) != nullptr)
    return;

  // Check if we have a resolver for this template type
  TemplateTypeKey key = GenerateTemplateTypeKey(lightningType);
  TemplateTypeIRResloverFn resolver = mLibrary->FindTemplateResolver(key);
  if (resolver != nullptr)
    resolver(this, lightningType);
}

void LightningSpirVFrontEnd::PreWalkClassVariables(Lightning::MemberVariableNode*& node, LightningSpirVFrontEndContext* context)
{
  // This variable is actually a getter setter. Walk its functions instead
  if (node->CreatedGetterSetter != nullptr)
  {
    if (node->Get != nullptr)
      GenerateIRFunction(
          node, &node->Attributes, context->mCurrentType, node->Get->DefinedFunction, node->Name.Token, context);
    if (node->Set != nullptr)
      GenerateIRFunction(
          node, &node->Attributes, context->mCurrentType, node->Set->DefinedFunction, node->Name.Token, context);
    return;
  }

  // For each member type, find out what the member types are
  LightningShaderIRType* currentType = context->mCurrentType;
  LightningShaderIRType* memberType = FindType(node->ResultType, node);
  ErrorIf(memberType == nullptr, "Invalid member type");

  ShaderIRFieldMeta* fieldMeta = currentType->mMeta->CreateField(mLibrary);
  fieldMeta->mLightningName = node->Name.Token;
  fieldMeta->mLightningType = Lightning::BoundType::GetBoundType(node->ResultType);
  fieldMeta->mLightningProperty = node->CreatedProperty;
  ParseAttributes(node->CreatedField->Attributes, &node->Attributes, fieldMeta);

  // If this is a runtime array (only detectable by the
  // lightning type's template name) then add a global runtime array.
  if (memberType->mLightningType->TemplateBaseName == SpirVNameSettings::mRuntimeArrayTypeName)
  {
    AddRuntimeArray(node, memberType, context);
    return;
  }

  // Check to see if this has a forced storage class
  ShaderIRAttribute* storageClassAttribute = memberType->FindFirstAttribute(SpirVNameSettings::mStorageClassAttribute);
  if (storageClassAttribute != nullptr)
  {
    // @JoshD: Right now I'm assuming this isn't a function storage class.
    // Change later?
    spv::StorageClass forcedStorageClass = (spv::StorageClass)storageClassAttribute->mParameters[0].GetIntValue();
    AddGlobalVariable(node, memberType, forcedStorageClass, context);
    return;
  }

  // Check to see if this node is static
  if (node->IsStatic)
  {
    SpirVNameSettings& nameSettings = mSettings->mNameSettings;
    // Check for specialization constants
    if (fieldMeta->ContainsAttribute(nameSettings.mSpecializationConstantAttribute))
      AddSpecializationConstant(node, memberType, context);
    // Otherwise, add it as a global variables
    else
      AddGlobalVariable(node, memberType, spv::StorageClassPrivate, context);
    return;
  }

  // Only actual add the member if this member belongs to the class (not global)
  currentType->AddMember(memberType, node->Name.Token);
}

void LightningSpirVFrontEnd::AddRuntimeArray(Lightning::MemberVariableNode* node,
                                         LightningShaderIRType* varType,
                                         LightningSpirVFrontEndContext* context)
{
  // Make sure no constructor call exists (illegal as this type
  // must be constructed by the client api not by the shader)
  if (node->InitialValue != nullptr)
  {
    String typeName = node->ResultType->ToString();
    String msg = String::Format("Type '%s' does not support an explicit constructor call.", typeName.c_str());
    SendTranslationError(node->InitialValue->Location, msg);
    return;
  }

  LightningShaderIRType* lightningRuntimeArrayType = varType;
  LightningShaderIRType* actualRuntimeArrayType = varType->mParameters[0]->As<LightningShaderIRType>();
  LightningShaderIRType* containedType = actualRuntimeArrayType->mParameters[0]->As<LightningShaderIRType>();

  // The glsl backend doesn't seem to properly support this (or it's a glsl
  // error). If the fixed array is put in a struct this all works though. This
  // error has to be reported here instead of during template parsing since this
  // is the only place a location is actually known.
  if (containedType->mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    String msg = "Runtime array cannot directly contain a FixedArray. Please "
                 "put the FixedArray in a struct.";
    SendTranslationError(node->Location, msg);
    return;
  }
  // Runtime arrays also cannot contain runtime arrays.
  if (containedType->mLightningType->TemplateBaseName == SpirVNameSettings::mRuntimeArrayTypeName)
  {
    SendTranslationError(node->Location, "Runtime arrays cannot contain runtime arrays");
    return;
  }

  // Make a variable of the wrapper struct type
  LightningShaderIROp* globalVar = BuildIROpNoBlockAdd(OpType::OpVariable, lightningRuntimeArrayType->mPointerType, context);
  // Runtime array structs must be of storage class storage buffer
  LightningShaderIRConstantLiteral* storageClassLiteral =
      GetOrCreateConstantIntegerLiteral((int)spv::StorageClassStorageBuffer);
  globalVar->mArguments.PushBack(storageClassLiteral);
  // Mangle the variable name like other globals (including the fragment type
  // name).
  globalVar->mDebugResultName = GenerateSpirVPropertyName(node->Name.Token, context->mCurrentType);
  // Add this field to our globals (so we can find it later)
  ErrorIf(mLibrary->mLightningFieldToGlobalVariable.ContainsKey(node->CreatedField), "Global variable already exists");

  // Create global variable data to store the instance and initializer function
  GlobalVariableData* globalData = new GlobalVariableData();
  mLibrary->mLightningFieldToGlobalVariable[node->CreatedField] = globalData;
  globalData->mInstance = globalVar;
  // Also map the instance for the backend
  mLibrary->mGlobalVariableToLightningField[globalData->mInstance] = node->CreatedField;
}

void LightningSpirVFrontEnd::AddGlobalVariable(Lightning::MemberVariableNode* node,
                                           LightningShaderIRType* varType,
                                           spv::StorageClass storageClass,
                                           LightningSpirVFrontEndContext* context)
{
  // Make a member variable with the specified storage class
  LightningShaderIROp* globalVar = BuildIROpNoBlockAdd(OpType::OpVariable, varType->mPointerType, context);
  globalVar->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(storageClass));
  // Name the variable. Include the owning type's name for clarity.
  globalVar->mDebugResultName = GenerateSpirVPropertyName(node->Name.Token, context->mCurrentType);
  // Add this field to our globals (so we can find it later)
  ErrorIf(mLibrary->mLightningFieldToGlobalVariable.ContainsKey(node->CreatedField), "Global variable already exists");

  // Create global variable data to store the instance and initializer function
  GlobalVariableData* globalData = new GlobalVariableData();
  mLibrary->mLightningFieldToGlobalVariable[node->CreatedField] = globalData;
  globalData->mInstance = globalVar;
  // Also map the instance for the backend
  mLibrary->mGlobalVariableToLightningField[globalData->mInstance] = node->CreatedField;
}

void LightningSpirVFrontEnd::PreWalkClassConstructor(Lightning::ConstructorNode*& node, LightningSpirVFrontEndContext* context)
{
  GenerateIRFunction(node, &node->Attributes, context->mCurrentType, node->DefinedFunction, "Constructor", context);
}

void LightningSpirVFrontEnd::PreWalkClassFunction(Lightning::FunctionNode*& node, LightningSpirVFrontEndContext* context)
{
  GenerateIRFunction(node, &node->Attributes, context->mCurrentType, node->DefinedFunction, node->Name.Token, context);

  // Try and parse the correct "Main" function for the current fragment type
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  if (node->Name.Token == nameSettings.mMainFunctionName)
    PreWalkMainFunction(node, context);
}

void LightningSpirVFrontEnd::PreWalkMainFunction(Lightning::FunctionNode*& node, LightningSpirVFrontEndContext* context)
{
  // If this is a geometry fragment, try and find the function Main(inputStream,
  // outputStream)
  LightningShaderIRType* currentType = context->mCurrentType;
  FragmentType::Enum fragmentType = currentType->mMeta->mFragmentType;
  if (fragmentType == FragmentType::Geometry)
  {
    if (node->Parameters.Size() == 2)
    {
      LightningShaderIRType* inputType = FindType(node->Parameters[0]);
      LightningShaderIRType* outputType = FindType(node->Parameters[1]);

      Lightning::GeometryStreamUserData* inputUserData = inputType->mLightningType->Has<Lightning::GeometryStreamUserData>();
      Lightning::GeometryStreamUserData* outputUserData = outputType->mLightningType->Has<Lightning::GeometryStreamUserData>();

      // Validate that the parameters are the correct input/output types.
      if (inputUserData == nullptr || inputUserData->mInput == false)
        return;
      if (outputUserData == nullptr || outputUserData->mInput == true)
        return;
      // Check for void return type
      if (node->ReturnType != nullptr && node->ReturnType->ResolvedType != LightningTypeId(void))
        return;

      // Write out user data to the type so the compositor can know what the
      // main function looks like.
      Lightning::HandleOf<Lightning::GeometryFragmentUserData> handle = LightningAllocate(Lightning::GeometryFragmentUserData);
      handle->mInputStreamType = inputType;
      handle->mOutputStreamType = outputType;
      context->mCurrentType->mLightningType->Add(*handle);
      currentType->mHasMainFunction = true;
    }
  }
  else if (fragmentType == FragmentType::Vertex || fragmentType == FragmentType::Pixel ||
           fragmentType == FragmentType::Compute)
  {
    if (node->Parameters.Size() != 0)
      return;
    if (node->ReturnType == nullptr || node->ReturnType->ResolvedType == LightningTypeId(void))
      currentType->mHasMainFunction = true;
  }
}

void LightningSpirVFrontEnd::PreWalkErrorCheck(LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* currentType = context->mCurrentType;
  ShaderIRTypeMeta* typeMeta = currentType->mMeta;

  if (mSettings->mErrorSettings.mFrontEndErrorOnNoMainFunction)
  {
    if (typeMeta->mFragmentType == FragmentType::Geometry)
    {
      if (!currentType->mHasMainFunction)
      {
        String msg = "Geometry shader must have a 'Main' function of signature "
                     "(InputStream, OutputStream).";
        SendTranslationError(currentType->mLightningType->Location, msg);
      }
    }
    else if (typeMeta->mFragmentType != FragmentType::None)
    {
      if (!currentType->mHasMainFunction)
      {
        String msg = "Shader must have a function of signature 'Main()'.";
        SendTranslationError(currentType->mLightningType->Location, msg);
      }
    }
  }
}

void LightningSpirVFrontEnd::WalkClassNode(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* type = FindType(node->Type, node);
  context->mCurrentType = type;

  mWalker.Walk(this, node->Variables, context);
  mWalker.Walk(this, node->Constructors, context);
  mWalker.Walk(this, node->Functions, context);

  context->mCurrentType = nullptr;
}

void LightningSpirVFrontEnd::WalkClassVariables(Lightning::MemberVariableNode*& node, LightningSpirVFrontEndContext* context)
{
  // This variable is actually a getter setter. Walk it's functions instead
  if (node->CreatedGetterSetter != nullptr)
  {
    if (node->Get != nullptr)
      WalkClassFunction(node->Get, context);
    if (node->Set != nullptr)
      WalkClassFunction(node->Set, context);
    return;
  }

  if (node->IsStatic)
    GenerateStaticVariableInitializer(node, context);
}

void LightningSpirVFrontEnd::GeneratePreConstructor(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* currentType = context->mCurrentType;
  Lightning::Function* lightningFunction = node->PreConstructor;
  LightningShaderIRFunction* function =
      GenerateIRFunction(node, nullptr, context->mCurrentType, lightningFunction, lightningFunction->Name, context);

  BasicBlock* currentBlock = BuildBlock(String(), context);
  context->mCurrentBlock = currentBlock;

  // Declare the self param
  LightningShaderIROp* selfOp =
      BuildIROp(&function->mParameterBlock, OpType::OpFunctionParameter, currentType->mPointerType, context);
  selfOp->mDebugResultName = "self";

  // Generate the default initializer values for all member variables
  for (size_t i = 0; i < node->Variables.Size(); ++i)
  {
    Lightning::VariableNode* varNode = node->Variables[i];
    String varName = varNode->Name.Token;

    // If for some reason this variable isn't a member (e.g. samplers) then skip
    // initialization. This variable was probably promoted to a global.
    if (currentType->mMemberNamesToIndex.ContainsKey(varName) == false)
      continue;

    // Generate a pointer to the member variable
    int memberIndex = currentType->mMemberNamesToIndex[varName];
    LightningShaderIRType* memberType = currentType->GetSubType(memberIndex);
    LightningShaderIROp* offsetConstant = GetIntegerConstant(memberIndex, context);
    LightningShaderIROp* memberPtrOp =
        BuildIROp(currentBlock, OpType::OpAccessChain, memberType->mPointerType, selfOp, offsetConstant, context);

    // If the variable has an initializer then walk it and set the variable
    if (varNode->InitialValue != nullptr)
    {
      ILightningShaderIR* initialValue = WalkAndGetResult(varNode->InitialValue, context);
      LightningShaderIROp* valueOp = GetOrGenerateValueTypeFromIR(initialValue, context);
      BuildStoreOp(memberPtrOp, initialValue, context);
    }
    // Otherwise call the default constructor for this type
    else
    {
      DefaultConstructType(varNode, memberType, memberPtrOp, context);
    }
  }

  // Generate the required terminator op (return)
  currentBlock->mTerminatorOp = BuildIROp(currentBlock, OpType::OpReturn, nullptr, context);
  context->mCurrentBlock = nullptr;
}

void LightningSpirVFrontEnd::GenerateDefaultConstructor(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context)
{
  // If the type already has a default-constructor then don't auto-generate one.
  // Otherwise we have to generate one just to call the pre-constructor.
  if (node->Type->GetDefaultConstructor() != nullptr)
    return;

  LightningShaderIRType* currentType = context->mCurrentType;

  // Generate the function
  LightningShaderIRFunction* function = currentType->CreateFunction(mLibrary);
  function->mName = "DefaultConstructor";
  function->mDebugResultName = function->mName;
  context->mCurrentFunction = function;

  // We need to generate the type of a function (delegate type).
  // Just like other types, this needs to be generated only once but we
  // unfortunately don't have a backing lightning function so manually generate the
  // signature.
  Array<Lightning::Type*> signature;
  signature.PushBack(node->Type->IndirectType);
  GenerateFunctionType(node, function, LightningTypeId(void), signature, context);

  ShaderIRFunctionMeta* functionMeta = currentType->mMeta->CreateFunction(mLibrary);
  functionMeta->mLightningName = function->mName;
  function->mMeta = functionMeta;

  // Make sure to add the self parameter
  LightningShaderIROp* selfOp =
      BuildIROp(&function->mParameterBlock, OpType::OpFunctionParameter, currentType->mPointerType, context);
  selfOp->mDebugResultName = "self";

  // Begin the block of instructions for the function
  BasicBlock* currentBlock = BuildBlock(String(), context);
  context->mCurrentBlock = currentBlock;

  // Manually invoke the pre-constructor
  LightningShaderIRFunction* preConstructorFn = mLibrary->mFunctions.FindValue(node->PreConstructor, nullptr);
  LightningShaderIROp* preConstructorCallOp = GenerateFunctionCall(currentBlock, preConstructorFn, context);
  preConstructorCallOp->mArguments.PushBack(selfOp);
  currentType->mAutoDefaultConstructor = function;

  // Manually generate the terminator
  currentBlock->mTerminatorOp = BuildIROp(currentBlock, OpType::OpReturn, nullptr, context);
}

void LightningSpirVFrontEnd::GenerateDummyMemberVariable(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* currentType = context->mCurrentType;

  // If the type has parameters then we don't need to generate a dummy variable
  if (!currentType->mParameters.Empty())
    return;

  // Otherwise, generate a dummy int on this class
  Lightning::Type* dummyType = LightningTypeId(int);
  String dummyName = "Dummy";
  LightningShaderIRType* memberType = FindType(dummyType, node);

  ShaderIRFieldMeta* fieldMeta = currentType->mMeta->CreateField(mLibrary);
  fieldMeta->mLightningName = dummyName;
  fieldMeta->mLightningType = memberType->mLightningType;

  // Only actual add the member if this member belongs to the class (not global)
  currentType->AddMember(memberType, dummyName);
}

void LightningSpirVFrontEnd::GenerateStaticVariableInitializer(Lightning::MemberVariableNode*& node,
                                                           LightningSpirVFrontEndContext* context)
{
  // Ignore specialization constants. They're global but they can't have an
  // initializer function.
  if (mLibrary->FindSpecializationConstantOp(node->CreatedField) != nullptr)
    return;

  // Find the global variable data
  GlobalVariableData* globalData = mLibrary->FindGlobalVariable(node->CreatedField);
  ReturnIf(globalData == nullptr, , "Global variable data doesn't exist");

  // Generate the function
  LightningShaderIRFunction* shaderFunction = new LightningShaderIRFunction();
  // Name the function "`OwningType`_`VarName`_Initializer"
  shaderFunction->mName = BuildString(context->mCurrentType->mName, "_", node->Name.Token, "_Initializer");
  shaderFunction->mDebugResultName = shaderFunction->mName;
  // Generate the meta for the function
  ShaderIRFunctionMeta* functionMeta = new ShaderIRFunctionMeta();
  mLibrary->mOwnedFunctionMeta.PushBack(functionMeta);
  mLibrary->mOwnedFunctions.PushBack(shaderFunction);
  shaderFunction->mMeta = functionMeta;
  // Generate the function type. The signature is () : Void.
  Array<Lightning::Type*> signature;
  GenerateFunctionType(node, shaderFunction, LightningTypeId(void), signature, context);

  // Mark the initializer function
  globalData->mInitializerFunction = shaderFunction;
  // Create the initial block
  BasicBlock* currentBlock = new BasicBlock();
  shaderFunction->mBlocks.PushBack(currentBlock);

  // Mark the context to operate on the current block and function
  context->mCurrentBlock = currentBlock;
  context->mCurrentFunction = shaderFunction;

  // If the variable has an initializer then walk it and set the variable
  if (node->InitialValue != nullptr)
  {
    ILightningShaderIR* initialValue = WalkAndGetResult(node->InitialValue, context);
    BuildStoreOp(globalData->mInstance, initialValue, context);
  }
  // Otherwise call the default constructor for this type
  else
  {
    LightningShaderIROp* globalVarInstance = globalData->mInstance;
    LightningShaderIRType* globalVarValueType = globalVarInstance->mResultType->mDereferenceType;
    DefaultConstructType(node, globalVarValueType, globalVarInstance, context);
  }
  FixBlockTerminators(currentBlock, context);
}

void LightningSpirVFrontEnd::WalkClassConstructor(Lightning::ConstructorNode*& node, LightningSpirVFrontEndContext* context)
{
  GenerateFunctionParameters(node, context);

  LightningShaderIRType* currentType = context->mCurrentType;
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRFunction* preConstructorFn = mLibrary->FindFunction(currentType->mLightningType->PreConstructor);

  // Manually invoke the pre-constructor
  ILightningShaderIR* selfType = context->mLightningVariableToIR[node->DefinedFunction->This];
  LightningShaderIRType* preConstructorReturnType = preConstructorFn->GetReturnType();
  LightningShaderIROp* preConstructorCallOp = GenerateFunctionCall(currentBlock, preConstructorFn, context);
  preConstructorCallOp->mArguments.PushBack(selfType);

  GenerateFunctionBody(node, context);
}

void LightningSpirVFrontEnd::WalkClassFunction(Lightning::FunctionNode*& node, LightningSpirVFrontEndContext* context)
{
  GenerateFunctionParameters(node, context);
  GenerateFunctionBody(node, context);
}

void LightningSpirVFrontEnd::DefaultConstructType(Lightning::SyntaxNode* locationNode,
                                              LightningShaderIRType* type,
                                              LightningShaderIROp* selfVar,
                                              LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  // If this type has an auto-generated default constructor then call it
  LightningShaderIRFunction* autoDefaultConstructor = type->mAutoDefaultConstructor;
  if (autoDefaultConstructor != nullptr)
  {
    LightningShaderIRType* returnType = autoDefaultConstructor->GetReturnType();
    LightningShaderIROp* preConstructorCallOp = GenerateFunctionCall(currentBlock, autoDefaultConstructor, context);
    preConstructorCallOp->mArguments.PushBack(selfVar);
    return;
  }

  // If we can find a default constructor resolver for this type then call it
  // (e.g. Real)
  TypeResolvers* typeResolver = mLibrary->FindTypeResolver(type->mLightningType);
  if (typeResolver != nullptr && typeResolver->mDefaultConstructorResolver != nullptr)
  {
    typeResolver->mDefaultConstructorResolver(this, type->mLightningType, context);
    ILightningShaderIR* resultIR = context->PopIRStack();
    LightningShaderIROp* valueOp = GetOrGenerateValueTypeFromIR(resultIR, context);
    BuildStoreOp(currentBlock, selfVar, valueOp, context);
    return;
  }

  // Check if this type is forced to be a global type. If so then ignore the
  // default constructor because this class doesn't actually "own" the variable.
  if (type->IsGlobalType())
    return;

  SendTranslationError(locationNode->Location, "Couldn't default construct type '%s'", type->mName.c_str());
}

void LightningSpirVFrontEnd::GenerateFunctionParameters(Lightning::GenericFunctionNode* node,
                                                    LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* currentType = context->mCurrentType;
  // Debug sanity
  context->mCurrentBlock = nullptr;

  // Get the shader function defined for this lightning function
  Lightning::Function* lightningFunction = node->DefinedFunction;
  LightningShaderIRFunction* function = mLibrary->FindFunction(lightningFunction, false);
  ErrorIf(function == nullptr, "Class function wasn't already created");
  context->mCurrentFunction = function;

  // Generate a function's parameter block. This contains the declarations and
  // ids for all input parameters

  // First check if this is a member function. If so then we need to declare
  // that we take in the this pointer.
  if (!lightningFunction->IsStatic)
  {
    // Get the actual 'this' type from the function (deals with extension
    // methods)
    Lightning::BoundType* lightningThisType = lightningFunction->Owner;
    LightningShaderIRType* thisType = FindType(lightningThisType, node)->mPointerType;
    LightningShaderIROp* op = BuildIROp(&function->mParameterBlock, OpType::OpFunctionParameter, thisType, context);
    op->mDebugResultName = "self";
    // Map the 'this' variable to the generated this parameter
    context->mLightningVariableToIR[node->DefinedFunction->This] = op;
  }

  BasicBlock* currentBlock = BuildBlock(String(), context);

  // Add all parameters
  for (size_t i = 0; i < node->Parameters.Size(); ++i)
  {
    Lightning::ParameterNode* parameter = node->Parameters[i];

    LightningShaderIRType* shaderParameterType = FindType(parameter->ResultType, parameter);
    LightningShaderIRType* shaderParamPointerType = shaderParameterType->mPointerType;

    if (parameter->ResultType->IsIndirectionType(parameter->ResultType))
      shaderParameterType = shaderParamPointerType;

    // @JoshD: Fix later. We should declare variables for all non-pointer
    // types as the beginning statements so that the signature is actually
    // correct.

    // We take all parameters by pointer type. This makes it significantly
    // easier to generate code in case the user ever assigns the the input
    LightningShaderIROp* op =
        BuildIROp(&function->mParameterBlock, OpType::OpFunctionParameter, shaderParameterType, context);

    if (!shaderParameterType->IsPointerType())
    {
      LightningShaderIROp* varOp = BuildOpVariable(shaderParamPointerType, context);
      varOp->mDebugResultName = BuildString(parameter->Name.Token, "_Local");
      BuildStoreOp(currentBlock, varOp, op, context);

      context->mLightningVariableToIR.Insert(parameter->CreatedVariable, varOp);
    }
    else
    {
      context->mLightningVariableToIR.Insert(parameter->CreatedVariable, op);
    }

    op->mDebugResultName = parameter->Name.Token;
  }

  // Create the first block in the function

  context->mCurrentBlock = currentBlock;
}

void LightningSpirVFrontEnd::GenerateFunctionBody(Lightning::GenericFunctionNode* node, LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* currentType = context->mCurrentType;
  LightningShaderIRFunction* function = context->mCurrentFunction;
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Start walking all statements. This may create new blocks
  mWalker.Walk(this, node->Statements, context);

  // Fix all blocks to have exactly one terminator.
  for (size_t i = 0; i < function->mBlocks.Size(); ++i)
    FixBlockTerminators(function->mBlocks[i], context);

  // Cleanup the context
  context->mCurrentFunction = nullptr;
  context->mCurrentBlock = nullptr;
  // Clear mappings of lightning variables to shader variable declarations
  // since the all local variables are now out of scope.
  context->mLightningVariableToIR.Clear();

  // Generate actual spirv entry point for this function.
  if (function->mMeta->ContainsAttribute("EntryPoint"))
  {
    GenerateEntryPoint(node, function, context);
  }
}

void LightningSpirVFrontEnd::GenerateEntryPoint(Lightning::GenericFunctionNode* node,
                                            LightningShaderIRFunction* function,
                                            LightningSpirVFrontEndContext* context)
{
  // Run some error checking on the entry point function.
  ValidateEntryPoint(this, node, context);

  // If we had an error don't try to generate the actual entry function.
  if (mErrorTriggered)
    return;

  LightningShaderIRType* currentType = context->mCurrentType;
  FragmentType::Enum fragmentType = currentType->mMeta->mFragmentType;
  EntryPointGeneration entryPointGeneration;

  if (fragmentType == FragmentType::Pixel)
    entryPointGeneration.DeclarePixelInterface(this, node, function, context);
  else if (fragmentType == FragmentType::Vertex)
    entryPointGeneration.DeclareVertexInterface(this, node, function, context);
  else if (fragmentType == FragmentType::Geometry)
    entryPointGeneration.DeclareGeometryInterface(this, node, function, context);
  else if (fragmentType == FragmentType::Compute)
    entryPointGeneration.DeclareComputeInterface(this, node, function, context);
  // else
  // @JoshD: Revisit
  //__debugbreak();
}

void LightningSpirVFrontEnd::WalkFunctionCallNode(Lightning::FunctionCallNode*& node, LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Check if this is a constructor call
  Lightning::StaticTypeNode* constructorNode = Lightning::Type::DynamicCast<Lightning::StaticTypeNode*>(node->LeftOperand);
  if (constructorNode != nullptr)
  {
    WalkConstructorCallNode(node, constructorNode, context);
    return;
  }

  // Check if this is a member access function call (could be a function,
  // member, etc...)
  Lightning::MemberAccessNode* memberAccessNode = Lightning::Type::DynamicCast<Lightning::MemberAccessNode*>(node->LeftOperand);
  if (memberAccessNode)
  {
    WalkMemberAccessCallNode(node, memberAccessNode, context);
    return;
  }
}

void LightningSpirVFrontEnd::WalkConstructorCallNode(Lightning::FunctionCallNode*& node,
                                                 Lightning::StaticTypeNode* constructorNode,
                                                 LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Check for library constructor translation (e.g. Real3())
  ConstructorCallResolverIRFn resolver =
      mLibrary->FindConstructorResolver(node->LeftOperand->ResultType, constructorNode->ConstructorFunction);
  if (resolver != nullptr)
  {
    resolver(this, node, constructorNode, context);
    return;
  }

  // Otherwise assume we can walk all arguments to translate
  LightningShaderIRType* resultType = FindType(node->ResultType, node);

  LightningShaderIRFunction* shaderConstructorFn = nullptr;
  // Deal with auto-generated default constructors (they don't have a generated
  // lightning function)
  if (constructorNode->ConstructorFunction == nullptr && resultType->mAutoDefaultConstructor != nullptr)
    shaderConstructorFn = resultType->mAutoDefaultConstructor;
  // Otherwise, look up the constructor function
  else
    shaderConstructorFn = mLibrary->FindFunction(constructorNode->ConstructorFunction);

  // If we got a shader function then resolve it
  if (shaderConstructorFn != nullptr)
  {
    LightningShaderIROp* classVarOp = BuildOpVariable(resultType->mPointerType, context);

    // Generate the function call but don't add it to the block yet (so we can
    // collect all arguments first)
    LightningShaderIROp* constructorCallOp = GenerateFunctionCall(shaderConstructorFn, context);
    constructorCallOp->mArguments.PushBack(classVarOp);
    WriteFunctionCallArguments(node, LightningTypeId(void), constructorCallOp, context);

    currentBlock->AddOp(constructorCallOp);
    context->PushIRStack(classVarOp);
    return;
  }

  SendTranslationError(node->Location, "Failed to translation constructor call");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void LightningSpirVFrontEnd::WalkMemberAccessCallNode(Lightning::FunctionCallNode*& node,
                                                  Lightning::MemberAccessNode* memberAccessNode,
                                                  LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  Lightning::Function* lightningFunction = memberAccessNode->AccessedFunction;
  // Build the function call op but don't add it as a line in the current block
  // yet. We have to generate a bit more code to reference the parameters
  LightningShaderIRFunction* shaderFunction = mLibrary->FindFunction(lightningFunction);
  if (shaderFunction != nullptr)
  {
    WalkMemberAccessFunctionCallNode(node, memberAccessNode, shaderFunction, context);
    return;
  }
  // Check for a member function resolver. If we find one on the 'this' type
  // then leave translation up to it.
  Lightning::Type* selfType = memberAccessNode->LeftOperand->ResultType;
  // Make sure to handle indirection types otherwise resolvers won't be found
  // (e.g. turn 'Array ref' into 'Array')
  selfType = Lightning::Type::GetBoundType(selfType);
  MemberFunctionResolverIRFn fnResolver = mLibrary->FindFunctionResolver(selfType, lightningFunction);
  if (fnResolver != nullptr)
  {
    fnResolver(this, node, memberAccessNode, context);
    return;
  }

  SpirVExtensionInstruction* extension = mLibrary->FindExtensionInstruction(lightningFunction);
  if (extension != nullptr)
  {
    WalkMemberAccessExtensionInstructionCallNode(node, memberAccessNode, extension, context);
    return;
  }

  String errorMsg = String::Format("Failed to translation function call: '%s'", memberAccessNode->Name.c_str());
  SendTranslationError(node->Location, errorMsg);
  context->PushIRStack(GenerateDummyIR(node, context));
}

void LightningSpirVFrontEnd::WalkMemberAccessFunctionCallNode(Lightning::FunctionCallNode*& node,
                                                          Lightning::MemberAccessNode* memberAccessNode,
                                                          LightningShaderIRFunction* shaderFunction,
                                                          LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Fill out an array with all of the arguments this function takes
  Array<ILightningShaderIR*> arguments;

  // If this is an instance function call then add the self type as the first
  // argument
  Lightning::Function* lightningFunction = memberAccessNode->AccessedFunction;
  if (!lightningFunction->IsStatic)
  {
    ILightningShaderIR* thisOp = WalkAndGetResult(memberAccessNode->LeftOperand, context);
    arguments.PushBack(thisOp);
  }

  // Add all of the regular arguments as well
  for (size_t i = 0; i < node->Arguments.Size(); ++i)
  {
    ILightningShaderIR* argument = WalkAndGetResult(node->Arguments[i], context);
    arguments.PushBack(argument);
  }

  // Now generate a function call from the arguments
  WalkMemberAccessFunctionCall(arguments, memberAccessNode, shaderFunction, context);
}

void LightningSpirVFrontEnd::WalkMemberAccessFunctionCall(Array<ILightningShaderIR*>& arguments,
                                                      Lightning::MemberAccessNode* memberAccessNode,
                                                      LightningShaderIRFunction* shaderFunction,
                                                      LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Generate the function call but don't add it to the block yet (so we can
  // collect all arguments first)
  LightningShaderIROp* functionCallOp = GenerateFunctionCall(shaderFunction, context);

  // The first argument is always the function type
  LightningShaderIRType* returnType = shaderFunction->GetReturnType();
  WriteFunctionCallArguments(arguments, returnType, shaderFunction->mFunctionType, functionCallOp, context);

  // Now add the function since we have all arguments
  currentBlock->AddOp(functionCallOp);
}

LightningShaderIROp* LightningSpirVFrontEnd::GenerateFunctionCall(LightningShaderIRFunction* shaderFunction,
                                                          LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  LightningShaderIRType* functionType = shaderFunction->mFunctionType;
  LightningShaderIRType* returnType = shaderFunction->GetReturnType();

  // Generate the function call but don't add it to the block yet (so we can
  // collect all arguments first)
  LightningShaderIROp* functionCallOp = BuildIROpNoBlockAdd(OpType::OpFunctionCall, returnType, context);

  // The first argument is always the function type
  functionCallOp->mArguments.PushBack(shaderFunction);
  return functionCallOp;
}

LightningShaderIROp* LightningSpirVFrontEnd::GenerateFunctionCall(BasicBlock* block,
                                                          LightningShaderIRFunction* shaderFunction,
                                                          LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* functionCallOp = GenerateFunctionCall(shaderFunction, context);
  block->AddOp(functionCallOp);
  return functionCallOp;
}

void LightningSpirVFrontEnd::WalkMemberAccessExtensionInstructionCallNode(Lightning::FunctionCallNode*& node,
                                                                      Lightning::MemberAccessNode* memberAccessNode,
                                                                      SpirVExtensionInstruction* extensionInstruction,
                                                                      LightningSpirVFrontEndContext* context)
{
  LightningShaderExtensionImport* importLibraryIR = nullptr;
  importLibraryIR = mLibrary->FindExtensionLibraryImport(extensionInstruction->mLibrary);
  if (importLibraryIR == nullptr)
  {
    importLibraryIR = new LightningShaderExtensionImport(extensionInstruction->mLibrary);
    mLibrary->mExtensionLibraryImports.InsertOrError(extensionInstruction->mLibrary, importLibraryIR);
  }

  if (extensionInstruction->mResolverFn != nullptr)
  {
    extensionInstruction->mResolverFn(this, node, memberAccessNode, importLibraryIR, context);
    return;
  }

  // This should never happen unless we registered a resolver that was null.
  String errorMsg =
      String::Format("Failed to translation extension function call: '%s'", memberAccessNode->Name.c_str());
  SendTranslationError(node->Location, errorMsg);
  context->PushIRStack(GenerateDummyIR(node, context));
}

void LightningSpirVFrontEnd::WalkLocalVariable(Lightning::LocalVariableNode*& node, LightningSpirVFrontEndContext* context)
{
  // Should this variable declaration be forwarded to another variable? (Used in
  // expression initializers). If so, we just need to walk the variable this is
  // referencing and return and mark that.
  if (node->ForwardLocalAccessIfPossible)
  {
    mWalker.Walk(this, node->InitialValue, context);

    // Make sure this is a pointer otherwise make a new variable to reference
    // (not a pointer if the initial value is a temp)
    ILightningShaderIR* intialValueIR = context->PopIRStack();
    intialValueIR = GetOrGeneratePointerTypeFromIR(context->GetCurrentBlock(), intialValueIR, context);
    context->mLightningVariableToIR[node->CreatedVariable] = intialValueIR;
    context->PushIRStack(intialValueIR);
    return;
  }

  // Since we're declaring a member variable we have to mark a dependency on the
  // variable type
  LightningShaderIRType* resultShaderType = FindType(node->ResultType, node);
  LightningShaderIRType* resultShaderPointerType = resultShaderType->mPointerType;

  // Build a variable declaration for the pointer type of this variable
  LightningShaderIROp* variableIR = BuildOpVariable(resultShaderPointerType, context);
  variableIR->mDebugResultName = node->Name.Token;
  // Map this variable so that anything else that references
  // it later knows how to get the ir for the declaration
  context->mLightningVariableToIR[node->CreatedVariable] = variableIR;

  // If there's no initial value then we're done (we've already declared the
  // variable)
  if (node->InitialValue == nullptr)
    return;

  // Check if this type is non-copyable. If so we can't assign a default value
  // (generate an error)
  if (CheckForNonCopyableType(resultShaderType, node, context))
    return;

  mWalker.Walk(this, node->InitialValue, context);
  ILightningShaderIR* intialValueIR = context->PopIRStack();

  // Validate that we can assign to the type (the result could be void or
  // something else)
  LightningShaderIROp* intialValueOp = intialValueIR->As<LightningShaderIROp>();
  if (intialValueOp == nullptr || resultShaderType->mBaseType == ShaderIRTypeBaseType::Void)
    return;

  // Assign the initial value (have to get the current block here as
  // walking the initial value can change the current block)
  BasicBlock* currentBlock = context->GetCurrentBlock();
  BuildStoreOp(currentBlock, variableIR, intialValueIR, context);
}

void LightningSpirVFrontEnd::WalkStaticTypeOrCreationCallNode(Lightning::StaticTypeNode*& node,
                                                          LightningSpirVFrontEndContext* context)
{
  SendTranslationError(node->Location, "StaticTypeOrCreationCallNode not translatable.");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void LightningSpirVFrontEnd::WalkExpressionInitializerNode(Lightning::ExpressionInitializerNode*& node,
                                                       LightningSpirVFrontEndContext* context)
{
  // Check if we have an expression initializer list resolver (e.g. FixedArray)
  TypeResolvers* typeResolver = mLibrary->FindTypeResolver(node->ResultType);
  if (typeResolver != nullptr && typeResolver->mExpressionInitializerListResolver != nullptr)
  {
    typeResolver->mExpressionInitializerListResolver(this, node, context);
    return;
  }

  mWalker.Walk(this, node->LeftOperand, context);
  mWalker.Walk(this, node->InitializerStatements, context);
}

void LightningSpirVFrontEnd::WalkUnaryOperationNode(Lightning::UnaryOperatorNode*& node, LightningSpirVFrontEndContext* context)
{
  // If this is a dereference operator then generically generate an OpLoad to
  // turn a pointer type into a value type (should only be used in extension
  // methods that need to pass 'this' into a function)
  if (node->Operator->TokenId == Lightning::Grammar::Dereference)
  {
    ILightningShaderIR* operandResult = WalkAndGetResult(node->Operand, context);
    LightningShaderIROp* operand = operandResult->As<LightningShaderIROp>();
    // Validate this is a pointer type
    if (!operand->mResultType->IsPointerType())
    {
      SendTranslationError(node->Operand->Location, "Operand must be pointer type");
      context->PushIRStack(GenerateDummyIR(node, context));
      return;
    }
    LightningShaderIROp* dereferenceOp =
        BuildCurrentBlockIROp(OpType::OpLoad, operand->mResultType->mDereferenceType, operand, context);
    context->PushIRStack(dereferenceOp);
    return;
  }

  // If this is an address of operator then this is almost always &variable
  // which just returns the variable pointer itself. The only time this isn't a
  // pointer type is on the result of an expression such as &(a + b) which I
  // will currently always report as an error.
  if (node->Operator->TokenId == Lightning::Grammar::AddressOf)
  {
    ILightningShaderIR* operandResult = WalkAndGetResult(node->Operand, context);
    LightningShaderIROp* operand = operandResult->As<LightningShaderIROp>();
    if (!operand->mResultType->IsPointerType())
    {
      SendTranslationError(node->Operand->Location, "Cannot take the address of a temporary");
      context->PushIRStack(GenerateDummyIR(node, context));
    }
    else
      context->PushIRStack(operand);
    return;
  }

  // If the operand type is an enum then treat it like an integer
  Lightning::Type* operandType = node->Operand->ResultType;
  if (operandType->IsEnumOrFlags())
    operandType = LightningTypeId(int);

  // Find and use a resolver if we have one
  UnaryOperatorKey opKey = UnaryOperatorKey(operandType, node->OperatorInfo.Operator);
  UnaryOpResolverIRFn unaryOpResolver = mLibrary->FindOperatorResolver(opKey);
  if (unaryOpResolver != nullptr)
  {
    unaryOpResolver(this, node, context);
    return;
  }

  // Report an error if we failed to find a translation for this unary operator
  SendTranslationError(node->Location, "Unary operator not supported");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void LightningSpirVFrontEnd::WalkBinaryOperationNode(Lightning::BinaryOperatorNode*& node, LightningSpirVFrontEndContext* context)
{
  // Special case assignment (always just copy the values)
  if (node->OperatorInfo.Operator == Lightning::Grammar::Assignment)
  {
    // Deal with setters. This requires 'promoting' the setter above the
    // assignment op (e.g. A = B -> A.Set(B))
    if (ResolveSetter(node, nullptr, node->RightOperand, context))
      return;

    ILightningShaderIR* leftIR = WalkAndGetResult(node->LeftOperand, context);
    ILightningShaderIR* rightIR = WalkAndGetResult(node->RightOperand, context);

    // Check if the left hand side is non-copyable, if so generate an error.
    LightningShaderIROp* leftOp = leftIR->As<LightningShaderIROp>();
    if (CheckForNonCopyableType(leftOp->mResultType, node, context))
      return;

    // Validate that we can write to this op
    ValidateLValue(leftOp, node->Location);
    // Validate that the left hand side is a pointer type otherwise we can't
    // store to it
    ValidateResultType(leftIR, ShaderIRTypeBaseType::Pointer, node->Location);

    // Generate the store
    BasicBlock* currentBlock = context->GetCurrentBlock();
    BuildStoreOp(currentBlock, leftIR, rightIR, context);
    return;
  }

  // If any operand type is an enum then treat it like an integer
  Lightning::Type* leftType = node->LeftOperand->ResultType;
  Lightning::Type* rightType = node->RightOperand->ResultType;
  if (leftType->IsEnumOrFlags())
    leftType = LightningTypeId(int);
  if (rightType->IsEnumOrFlags())
    rightType = LightningTypeId(int);

  // Find a resolver for the given binary op
  BinaryOperatorKey opKey = BinaryOperatorKey(leftType, rightType, node->OperatorInfo.Operator);
  BinaryOpResolverIRFn binaryOpResolver = mLibrary->FindOperatorResolver(opKey);
  if (binaryOpResolver != nullptr)
  {
    binaryOpResolver(this, node, context);
    return;
  }

  // Report an error if we failed to find a translation for this binary operator
  SendTranslationError(node->Location, "Binary operator not supported");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void LightningSpirVFrontEnd::WalkCastNode(Lightning::TypeCastNode*& node, LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // If the operand or result type is an enum then treat it like an integer
  Lightning::Type* operatorType = node->Operand->ResultType;
  Lightning::Type* resultType = node->ResultType;
  if (operatorType->IsEnumOrFlags())
    operatorType = LightningTypeId(int);
  if (resultType->IsEnumOrFlags())
    resultType = LightningTypeId(int);

  // Cast to same type. Do a no-op
  if (operatorType == resultType)
  {
    LightningShaderIROp* operandValueResult = WalkAndGetValueTypeResult(node->Operand, context);
    context->PushIRStack(operandValueResult);
    return;
  }

  // Find a resolver for the cast operator
  TypeCastKey castOpKey(operatorType, resultType);
  TypeCastResolverIRFn resolverFn = mLibrary->FindOperatorResolver(castOpKey);
  if (resolverFn != nullptr)
  {
    resolverFn(this, node, context);
    return;
  }

  // Report an error if we failed to find a translation for this cast operator
  SendTranslationError(node->Location, "Cast operator not supported");
  context->PushIRStack(GenerateDummyIR(node, context));
}

void LightningSpirVFrontEnd::WalkValueNode(Lightning::ValueNode*& node, LightningSpirVFrontEndContext* context)
{
  // @JoshD: This probably has to change later with templates (the type might
  // not exist)

  // Find the result type and mark this function as relying on it
  LightningShaderIRType* resultType = FindType(node->ResultType, node);
  ErrorIf(resultType == nullptr, "No type");

  // Get the constant that represents this value of the given type.
  // This might create a new constant in the constant pool.
  LightningShaderIROp* opConstant = GetConstant(resultType, node->Value.Token, context);
  // Mark the constant as the result of this value node expression
  context->PushIRStack(opConstant);
}

void LightningSpirVFrontEnd::WalkLocalRef(Lightning::LocalVariableReferenceNode*& node, LightningSpirVFrontEndContext* context)
{
  ILightningShaderIR* variableOp = context->mLightningVariableToIR.FindValue(node->AccessedVariable, nullptr);
  ErrorIf(variableOp == nullptr, "Failed to find variable declaration for local reference");

  context->PushIRStack(variableOp);
}

void LightningSpirVFrontEnd::WalkMemberAccessNode(Lightning::MemberAccessNode*& node, LightningSpirVFrontEndContext* context)
{
  if (node->AccessedMember)
  {
    LightningShaderIRType* leftOperandType = FindType(node->LeftOperand->ResultType, node->LeftOperand);

    if (node->AccessedGetterSetter != nullptr)
    {
      // Deal with accessing enums/flags (grab their constant value)
      if (node->AccessedGetterSetter->PropertyType->IsEnumOrFlags())
      {
        LightningShaderIROp* enumConstant = mLibrary->FindEnumConstantOp(node->AccessedGetterSetter);
        // Sanity check (this should never happen)
        if (enumConstant == nullptr)
        {
          SendTranslationError(node->Location, "Enum unable to be translated");
          context->PushIRStack(GenerateDummyIR(node, context));
          return;
        }

        context->PushIRStack(enumConstant);
        return;
      }

      if (node->IoUsage == Lightning::IoMode::ReadRValue)
      {
        Lightning::Type* ownerType = node->AccessedGetterSetter->Owner;
        Lightning::Function* getFn = node->AccessedGetterSetter->Get;
        MemberFunctionResolverIRFn functionResolver = mLibrary->FindFunctionResolver(ownerType, getFn);
        if (functionResolver != nullptr)
        {
          functionResolver(this, nullptr, node, context);
          return;
        }

        Lightning::Function* lightningGetFunction = node->AccessedGetterSetter->Get;
        LightningShaderIRFunction* shaderFunction = mLibrary->FindFunction(lightningGetFunction);
        // If this is an existing lightning function then we have to translate this
        // member access into a function call
        if (shaderFunction != nullptr)
        {
          Array<ILightningShaderIR*> arguments;
          // Pass through 'this' if this is an instance function
          if (!lightningGetFunction->IsStatic)
            arguments.PushBack(WalkAndGetResult(node->LeftOperand, context));

          WalkMemberAccessFunctionCall(arguments, node, shaderFunction, context);
          return;
        }
      }
      else
      {
        // In the syntax tree setters have to be manually deal with at a higher
        // level (binary ops right now). If we get here then something went
        // wrong.
        Error("Setters should always be hit via a binary op node");
        SendTranslationError(node->Location, "Translation of setters is not supported at this time.");
      }
    }
    else if (node->AccessedField != nullptr)
    {
      // Check if this is a global variable. If so just put the variable on the
      // stack and return (no member access chain). This happens with forced
      // global types like samplers.
      GlobalVariableData* globalVarData = mLibrary->FindGlobalVariable(node->AccessedField);
      if (globalVarData != nullptr)
      {
        context->PushIRStack(globalVarData->mInstance);
        return;
      }

      // Check if this is a specialization constant.
      // This is basically the same as a global except it requires a separate
      // lookup.
      LightningShaderIROp* specConstant = mLibrary->FindSpecializationConstantOp(node->AccessedField);
      if (specConstant != nullptr)
      {
        context->PushIRStack(specConstant);
        return;
      }
    }
    else
    {
      SendTranslationError(node->Location, "Translation not yet supported");
    }

    // If we failed to translate something more specialized up above then try to
    // find a field resolver. We have to do this last because some types (like
    // vectors) can have backup field resolvers. If we did this first then
    // getters would fail to get called.
    MemberAccessResolverIRFn fieldResolver =
        mLibrary->FindFieldResolver(leftOperandType->mLightningType, node->AccessedField);
    if (fieldResolver != nullptr)
    {
      fieldResolver(this, node, context);
      return;
    }

    // Find the index and type of the member we're accessing.
    // First map the name to an index.
    String memberName = node->AccessedMember->Name;
    ErrorIf(!leftOperandType->mMemberNamesToIndex.ContainsKey(memberName), "Invalid member name");
    u32 memberIndex = leftOperandType->FindMemberIndex(memberName);
    // Then use that index to get the type of the member
    LightningShaderIRType* memberType = leftOperandType->GetSubType(memberIndex);

    // Get the member variable pointer. If this is a static variable then we
    // access the globals map, otherwise we walk the left operand and get the
    // variable pointer to this id
    ILightningShaderIR* operandResult = nullptr;
    if (!node->IsStatic)
      operandResult = WalkAndGetResult(node->LeftOperand, context);
    else
    {
      GlobalVariableData* globalVarData = mLibrary->FindGlobalVariable(node->AccessedField);
      if (globalVarData != nullptr && globalVarData->mInstance != nullptr)
        operandResult = globalVarData->mInstance;
    }

    if (operandResult == nullptr)
    {
      SendTranslationError(node->Location, "Member variable access couldn't be translated");
      context->PushIRStack(GenerateDummyIR(node, context));
      return;
    }

    LightningShaderIROp* operandResultOp = operandResult->As<LightningShaderIROp>();

    if (operandResultOp->IsResultPointerType())
    {
      // Make the constant for the sub-index of the member with respect to the
      // base
      LightningShaderIROp* memberIndexConstant = GetIntegerConstant(memberIndex, context);
      // Generate a member access to reference this member.
      // Note: This must have the same storage class as the left operand.
      LightningShaderIROp* memberAccessOp =
          BuildCurrentBlockAccessChain(memberType, operandResultOp, memberIndexConstant, context);
      context->PushIRStack(memberAccessOp);
    }
    // @JoshD: Validate (have to find op-code to generate this)
    else
    {
      // Validate this is right (haven't made code to hit this yet)
      //__debugbreak();

      // Make the constant for the sub-index of the member with respect to the
      // base
      LightningShaderIRConstantLiteral* memberIndexLiteral = GetOrCreateConstantLiteral(memberIndex);

      // Build the member access operation
      LightningShaderIROp* memberAccessOp =
          BuildCurrentBlockIROp(OpType::OpCompositeExtract, memberType, operandResultOp, memberIndexLiteral, context);
      context->PushIRStack(memberAccessOp);
    }
  }
  else
  {
    Error("Only member access should reach here right now");
    context->PushIRStack(GenerateDummyIR(node, context));
  }
}

void LightningSpirVFrontEnd::WalkMultiExpressionNode(Lightning::MultiExpressionNode*& node, LightningSpirVFrontEndContext* context)
{
}

struct ConditionBlockData
{
  BasicBlock* mIfTrue;
  BasicBlock* mIfFalse;
  BasicBlock* mMergePoint;
};

void LightningSpirVFrontEnd::WalkIfRootNode(Lightning::IfRootNode*& node, LightningSpirVFrontEndContext* context)
{
  BasicBlock* prevBlock = context->GetCurrentBlock();

  size_t ifParts = node->IfParts.Size();
  Array<ConditionBlockData> blockPairs;
  blockPairs.Reserve(ifParts);

  // Pre-allocate the blocks for the if-statements (makes it easier to point at
  // later blocks)
  for (size_t i = 0; i < ifParts; ++i)
  {
    // Skip any else with no condition. This is covered by the previous block's
    // ifFalse block
    if (node->IfParts[i]->Condition == nullptr)
      continue;

    String indexStr = ToString(i);

    ConditionBlockData data;

    // Don't add any block to the function so we can properly resolve dominance
    // order. This is particularly important for expressions that change the
    // block inside of the conditional (e.g. Logical Or)

    // Always make the if and merge blocks
    data.mIfTrue = BuildBlockNoStack(BuildString("ifTrue", indexStr), context);
    data.mMergePoint = BuildBlockNoStack(BuildString("ifMerge", indexStr), context);

    // The ifFalse block is not always needed though. If this is the last part
    // in the chain then this must be an if with no else (since we skipped
    // else's with no ifs earlier). In this case do a small optimization of
    // making the ifFalse and mergePoint be the same block. This makes code
    // generation a little easier and makes the resultant code look cleaner.
    size_t lastIndex = ifParts - 1;
    if (i != lastIndex)
      data.mIfFalse = BuildBlockNoStack(BuildString("ifFalse", indexStr), context);
    else
      data.mIfFalse = data.mMergePoint;

    blockPairs.PushBack(data);
  }

  // Now emit all of the actual if statements and their bodies
  for (size_t i = 0; i < node->IfParts.Size(); ++i)
  {
    Lightning::IfNode* ifNode = node->IfParts[i];
    ExtractDebugInfo(ifNode, context->mDebugInfo);

    // If this part has a condition then we have to emit the appropriate
    // conditional block and branch conditions
    if (ifNode->Condition)
    {
      ConditionBlockData blockPair = blockPairs[i];
      BasicBlock* ifTrueBlock = blockPairs[i].mIfTrue;
      BasicBlock* ifFalseBlock = blockPairs[i].mIfFalse;
      BasicBlock* ifMerge = blockPairs[i].mMergePoint;

      // Walk the conditional and then branch on this value to either the true
      // or false block
      ILightningShaderIR* conditionalIR = WalkAndGetValueTypeResult(ifNode->Condition, context);

      // Mark the current block we're in (the header block where we write the
      // conditionals) as a selection block and mark it's merge point. Note:
      // This needs to be after we walk the conditional as the block can change
      // (logical and/or expressions)
      BasicBlock* headerBlock = context->GetCurrentBlock();
      headerBlock->mBlockType = BlockType::Selection;
      headerBlock->mMergePoint = ifMerge;

      headerBlock->mTerminatorOp = BuildIROp(
          headerBlock, OpType::OpBranchConditional, nullptr, conditionalIR, ifTrueBlock, ifFalseBlock, context);

      // Start emitting the true block. First mark this as the current active
      // block
      context->mCurrentBlock = ifTrueBlock;
      // Mark the if true block as the next block in dominance order
      context->mCurrentFunction->mBlocks.PushBack(ifTrueBlock);

      // Now walk all of the statements int he block
      mWalker.Walk(this, ifNode->Statements, context);
      // Always emit a branch back to the merge point. If this is dead code
      // because of another termination condition we'll clean this up after
      // generating the entire function.
      ifTrueBlock->mTerminatorOp = BuildIROp(ifTrueBlock, OpType::OpBranch, nullptr, ifMerge, context);

      // Nested if pushed another merge point. Add to a termination condition on
      // the nested if merge point back to our merge point.
      if (context->mCurrentBlock != ifTrueBlock)
      {
        BasicBlock* nestedBlock = context->mCurrentBlock;
        nestedBlock->mTerminatorOp = BuildIROp(nestedBlock, OpType::OpBranch, nullptr, ifMerge, context);
      }

      // Now mark that we're inside the false block
      context->mCurrentBlock = ifFalseBlock;
      // Mark the if false block as the next block in dominance order
      // (if it's not the merge block which is handled at the end)
      if (ifFalseBlock != ifMerge)
        context->mCurrentFunction->mBlocks.PushBack(ifFalseBlock);
      // Keep track of the previous header block so if statements know where to
      // merge back to
      prevBlock = headerBlock;
    }
    // Otherwise this is an else with no if
    else
    {
      BasicBlock* currentBlock = context->GetCurrentBlock();
      // Walk all of the statements in the else
      mWalker.Walk(this, ifNode->Statements, context);

      // Always emit a branch back to the previous block's merge point
      currentBlock->mTerminatorOp = BuildIROp(currentBlock, OpType::OpBranch, nullptr, prevBlock->mMergePoint, context);
      // Nested if pushed another merge point. Add to a termination condition on
      // the nested if merge point back to our merge point.
      if (context->mCurrentBlock != currentBlock)
      {
        BasicBlock* nestedBlock = context->mCurrentBlock;
        currentBlock->mTerminatorOp =
            BuildIROp(nestedBlock, OpType::OpBranch, nullptr, prevBlock->mMergePoint, context);
      }
    }
  }

  // Now write out all merge point blocks in reverse order (requirement of
  // spir-v is that a block must appear before all blocks they dominate).
  // Additionally, add branches for all merge points of all blocks to the
  // previous block's merge point.
  for (size_t i = 0; i < blockPairs.Size(); ++i)
  {
    size_t blockIndex = blockPairs.Size() - i - 1;
    BasicBlock* block = blockPairs[blockIndex].mMergePoint;
    // If this is not the first block then add a branch on the merge point to
    // the previous block's merge point
    if (blockIndex != 0)
      block->mTerminatorOp =
          BuildIROp(block, OpType::OpBranch, nullptr, blockPairs[blockIndex - 1].mMergePoint, context);
    context->mCurrentFunction->mBlocks.PushBack(block);
  }

  // Mark the first merge point as the new current block (everything after the
  // if goes here).
  context->mCurrentBlock = blockPairs[0].mMergePoint;
}

void LightningSpirVFrontEnd::WalkIfNode(Lightning::IfNode*& node, LightningSpirVFrontEndContext* context)
{
  // nothing to do
}

void LightningSpirVFrontEnd::WalkBreakNode(Lightning::BreakNode*& node, LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->mCurrentBlock;
  BasicBlock* breakTarget = context->mBreakTarget;
  ErrorIf(breakTarget == nullptr, "Break statement doesn't have a valid merge point to jump to");

  // Generate a branch to the break target of the current block. Also mark this
  // as a terminator op so we know that no terminator must be generated.
  currentBlock->mTerminatorOp = BuildCurrentBlockIROp(OpType::OpBranch, nullptr, breakTarget, context);
}

void LightningSpirVFrontEnd::WalkContinueNode(Lightning::ContinueNode*& node, LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->mCurrentBlock;
  BasicBlock* continueTarget = context->mContinueTarget;
  ErrorIf(continueTarget == nullptr, "Continue statement doesn't have a valid continue point to jump to");

  // Generate a branch to the continue target of the current block. Also mark
  // this as a terminator op so we know that no terminator must be generated.
  currentBlock->mTerminatorOp = BuildIROp(currentBlock, OpType::OpBranch, nullptr, continueTarget, context);
}

void LightningSpirVFrontEnd::WalkReturnNode(Lightning::ReturnNode*& node, LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // We have to generate different op code depending on if this has a return
  // value or not
  if (node->ReturnValue)
  {
    // @JoshD: Fix
    // For now, assume that all return types must be value types (deal with
    // pointers later)
    ILightningShaderIR* returnResultOp = WalkAndGetResult(node->ReturnValue, context);
    LightningShaderIROp* returnResultValueOp = GetOrGenerateValueTypeFromIR(returnResultOp, context);
    currentBlock->mTerminatorOp = BuildCurrentBlockIROp(OpType::OpReturnValue, nullptr, returnResultValueOp, context);
  }
  else
    currentBlock->mTerminatorOp = BuildCurrentBlockIROp(OpType::OpReturn, nullptr, context);
}

void LightningSpirVFrontEnd::WalkWhileNode(Lightning::WhileNode*& node, LightningSpirVFrontEndContext* context)
{
  WalkGenericLoop(nullptr, nullptr, node, node, context);
}

void LightningSpirVFrontEnd::WalkDoWhileNode(Lightning::DoWhileNode*& node, LightningSpirVFrontEndContext* context)
{
  // A do while looks like a header block that always jumps to a loop block.
  // This loop block will always branch to the continue target (the condition
  // block) unless a break happens which will branch to the merge block (after
  // the loop). The condition block will choose to jump either back to the
  // header block or to the merge point.
  BasicBlock* headerBlock = BuildBlockNoStack("headerBlock", context);
  BasicBlock* loopTrueBlock = BuildBlockNoStack("loop-body", context);
  BasicBlock* conditionBlock = BuildBlockNoStack("conditionBlock", context);
  BasicBlock* mergeBlock = BuildBlockNoStack("after-loop", context);

  // Always jump to the header block
  BuildCurrentBlockIROp(OpType::OpBranch, nullptr, headerBlock, context);

  // Mark the header as the next block and visit it
  context->mCurrentFunction->mBlocks.PushBack(headerBlock);
  GenerateLoopHeaderBlock(headerBlock, loopTrueBlock, mergeBlock, conditionBlock, context);

  // Now create the loop body which has the conditional block as its continue
  // target
  context->mCurrentFunction->mBlocks.PushBack(loopTrueBlock);
  GenerateLoopStatements(node, loopTrueBlock, mergeBlock, conditionBlock, context);

  // Finally create the conditional block (which is the continue target)
  // which will either jump back to the header block or exit the loop
  context->mCurrentFunction->mBlocks.PushBack(conditionBlock);
  GenerateLoopConditionBlock(node, conditionBlock, headerBlock, mergeBlock, context);

  // Afterwards the active block is always the merge point
  context->mCurrentFunction->mBlocks.PushBack(mergeBlock);
  context->mCurrentBlock = mergeBlock;
}

void LightningSpirVFrontEnd::WalkForNode(Lightning::ForNode*& node, LightningSpirVFrontEndContext* context)
{
  Lightning::SyntaxNode* initializer = nullptr;
  if (node->ValueVariable != nullptr)
    initializer = node->ValueVariable;
  else if (node->Initialization != nullptr)
    initializer = node->Initialization;

  WalkGenericLoop(initializer, node->Iterator, node, node, context);
}

void LightningSpirVFrontEnd::WalkForEachNode(Lightning::ForEachNode*& node, LightningSpirVFrontEndContext* context)
{
  SendTranslationError(node->Location, "foreach is not supported.");
}

void LightningSpirVFrontEnd::WalkLoopNode(Lightning::LoopNode*& node, LightningSpirVFrontEndContext* context)
{
  WalkGenericLoop(nullptr, nullptr, nullptr, node, context);
}

void LightningSpirVFrontEnd::WalkGenericLoop(Lightning::SyntaxNode* initializerNode,
                                         Lightning::SyntaxNode* iterator,
                                         Lightning::ConditionalLoopNode* conditionalNode,
                                         Lightning::LoopScopeNode* loopScopeNode,
                                         LightningSpirVFrontEndContext* context)
{
  // Always walk the initializer node first if it exists. The contents of this
  // go before any loop block.
  if (initializerNode != nullptr)
    mWalker.Walk(this, initializerNode, context);

  // A basic while looks like a header block that always jumps to a condition
  // block. The condition block will choose to jump either to the loop block or
  // to the merge point. The loop block will always branch to the continue
  // target unless a break happens which will branch to the merge block (after
  // the loop). The continue block will always jump back to the header block.
  BasicBlock* headerBlock = BuildBlockNoStack("headerBlock", context);
  BasicBlock* conditionBlock = BuildBlockNoStack("conditionBlock", context);
  BasicBlock* loopTrueBlock = BuildBlockNoStack("loop-body", context);
  BasicBlock* continueBlock = BuildBlockNoStack("continueBlock", context);
  BasicBlock* mergeBlock = BuildBlockNoStack("after-loop", context);

  // Always jump to the header block
  BuildCurrentBlockIROp(OpType::OpBranch, nullptr, headerBlock, context);

  // The header always jumps to the conditional
  context->mCurrentFunction->mBlocks.PushBack(headerBlock);
  GenerateLoopHeaderBlock(headerBlock, conditionBlock, mergeBlock, continueBlock, context);

  // The conditional will jump to either the loop body or the merge point (after
  // the loop)
  context->mCurrentFunction->mBlocks.PushBack(conditionBlock);
  GenerateLoopConditionBlock(conditionalNode, conditionBlock, loopTrueBlock, mergeBlock, context);

  // Walk all of the statements in the loop body and jump to either the merge or
  // continue block
  context->mCurrentFunction->mBlocks.PushBack(loopTrueBlock);
  GenerateLoopStatements(loopScopeNode, loopTrueBlock, mergeBlock, continueBlock, context);

  // The continue block always just jumps to the header block
  context->mCurrentFunction->mBlocks.PushBack(continueBlock);
  GenerateLoopContinueBlock(iterator, continueBlock, headerBlock, context);

  // Afterwards the active block is always the merge point
  context->mCurrentFunction->mBlocks.PushBack(mergeBlock);
  context->mCurrentBlock = mergeBlock;
}

void LightningSpirVFrontEnd::GenerateLoopHeaderBlock(BasicBlock* headerBlock,
                                                 BasicBlock* branchTarget,
                                                 BasicBlock* mergeBlock,
                                                 BasicBlock* continueBlock,
                                                 LightningSpirVFrontEndContext* context)
{
  // Mark the header block as a loop block (so we emit the LoopMerge
  // instruction)
  headerBlock->mBlockType = BlockType::Loop;
  // Being a LoopMerge requires setting the merge and continue points
  headerBlock->mMergePoint = mergeBlock;
  headerBlock->mContinuePoint = continueBlock;

  // The header always jumps to the branch target (typically a continue)
  BuildIROp(headerBlock, OpType::OpBranch, nullptr, branchTarget, context);
}

void LightningSpirVFrontEnd::GenerateLoopConditionBlock(Lightning::ConditionalLoopNode* conditionalNode,
                                                    BasicBlock* conditionBlock,
                                                    BasicBlock* branchTrueBlock,
                                                    BasicBlock* branchFalseBlock,
                                                    LightningSpirVFrontEndContext* context)
{
  // The condition builds the conditional and then jumps either to the body of
  // the loop or to the end
  context->mCurrentBlock = conditionBlock;
  // If the conditional node exists
  if (conditionalNode != nullptr)
  {
    ExtractDebugInfo(conditionalNode->Condition, context->mDebugInfo);
    // Get the conditional value (must be a bool via how lightning works)
    ILightningShaderIR* conditional = WalkAndGetValueTypeResult(conditionalNode->Condition, context);
    // Branch to either the true or false branch
    BuildCurrentBlockIROp(
        OpType::OpBranchConditional, nullptr, conditional, branchTrueBlock, branchFalseBlock, context);
  }
  // Otherwise there is no conditional (e.g. loop) so unconditionally branch to
  // the true block
  else
    BuildCurrentBlockIROp(OpType::OpBranch, nullptr, branchTrueBlock, context);
}

void LightningSpirVFrontEnd::GenerateLoopStatements(Lightning::LoopScopeNode* loopScopeNode,
                                                BasicBlock* loopBlock,
                                                BasicBlock* mergeBlock,
                                                BasicBlock* continueBlock,
                                                LightningSpirVFrontEndContext* context)
{
  context->mCurrentBlock = loopBlock;
  // Set the continue and merge points for this block (mainly needed for nested
  // loops)
  loopBlock->mContinuePoint = continueBlock;
  loopBlock->mMergePoint = mergeBlock;
  context->PushMergePoints(continueBlock, mergeBlock);

  // Iterate over all of the statements in the loop body
  mWalker.Walk(this, loopScopeNode->Statements, context);

  // Write out a jump back to the continue block of the loop. Only write this to
  // the active block which will either be the end of the loop block or
  // something like an after if
  if (context->mCurrentBlock->mTerminatorOp == nullptr)
  {
    ILightningShaderIR* currentBlockContinue = BuildCurrentBlockIROp(OpType::OpBranch, nullptr, continueBlock, context);
    currentBlockContinue->mDebugInfo.mComments.Add("auto continue");
  }
  context->PopMergeTargets();
}

void LightningSpirVFrontEnd::GenerateLoopContinueBlock(Lightning::SyntaxNode* iterator,
                                                   BasicBlock* continueBlock,
                                                   BasicBlock* headerBlock,
                                                   LightningSpirVFrontEndContext* context)
{
  // Mark the continue block as the active block
  context->mCurrentBlock = continueBlock;
  // If it exists, walk the iterator statement
  if (iterator != nullptr)
    mWalker.Walk(this, iterator, context);
  // Always jump back to the header block
  BuildIROp(continueBlock, OpType::OpBranch, nullptr, headerBlock, context);
}

void LightningSpirVFrontEnd::FixBlockTerminators(BasicBlock* block, LightningSpirVFrontEndContext* context)
{
  size_t opCount = block->mLines.Size();
  size_t firstTerminatorIndex = opCount;
  for (size_t i = 0; i < opCount; ++i)
  {
    ILightningShaderIR* ir = block->mLines[i];
    LightningShaderIROp* op = ir->As<LightningShaderIROp>();
    // If this op is a terminator then mark its id and break
    if (op->mOpType == OpType::OpReturn || op->mOpType == OpType::OpReturnValue || op->mOpType == OpType::OpBranch ||
        op->mOpType == OpType::OpBranchConditional || op->mOpType == OpType::OpSwitch || op->mOpType == OpType::OpKill)
    {
      firstTerminatorIndex = i;
      break;
    }
  }

  // First terminator is the last op in the block. This block is good and
  // there's nothing more to do.
  if (firstTerminatorIndex == opCount - 1)
    return;

  // No terminator in the block
  if (firstTerminatorIndex >= opCount)
  {
    LightningShaderIRType* returnType = context->mCurrentFunction->GetReturnType();
    // If the return type isn't void then this was likely a block that was
    // generated after a conditional that can't be reached (e.g. if + else and
    // then after the else). These statements should be unreachable otherwise
    // we'd have a lightning error.
    if (returnType->mLightningType != LightningTypeId(void))
      block->mTerminatorOp = BuildIROp(block, OpType::OpUnreachable, nullptr, context);
    // Otherwise just emit a return
    else
      block->mTerminatorOp = BuildIROp(block, OpType::OpReturn, nullptr, context);
    return;
  }

  // There's dead instructions after the terminator. Clean up everything
  // afterwards
  size_t newSize = firstTerminatorIndex + 1;
  for (size_t i = newSize; i < block->mLines.Size(); ++i)
    delete block->mLines[i];
  block->mLines.Resize(newSize);
}

Lightning::Function* LightningSpirVFrontEnd::GetSetter(Lightning::MemberAccessNode* memberAccessNode)
{
  // Check for io-usage of a setter
  int setterFlags = Lightning::IoMode::WriteLValue | Lightning::IoMode::StrictPropertySet;
  bool isSetter = memberAccessNode->IoUsage & setterFlags;
  if (!isSetter)
    return nullptr;

  Lightning::Function* set = nullptr;

  // See if this is a getter/setter 'set'
  if (memberAccessNode->AccessedGetterSetter != nullptr && memberAccessNode->AccessedGetterSetter->Set != nullptr)
    set = memberAccessNode->AccessedGetterSetter->Set;
  // Otherwise check for a property set (happens if this is a field)
  else if (memberAccessNode->AccessedProperty != nullptr && memberAccessNode->AccessedProperty->Set != nullptr)
    set = memberAccessNode->AccessedProperty->Set;
  return set;
}

bool LightningSpirVFrontEnd::ResolveSetter(Lightning::BinaryOperatorNode* node,
                                       LightningShaderIROp* resultValue,
                                       Lightning::SyntaxNode* resultNode,
                                       LightningSpirVFrontEndContext* context)
{
  Lightning::MemberAccessNode* memberAccessNode = Lightning::Type::DynamicCast<Lightning::MemberAccessNode*>(node->LeftOperand);
  if (memberAccessNode == nullptr)
    return false;

  Lightning::Function* set = GetSetter(memberAccessNode);

  // If we didn't find a setter then there's nothing to do
  if (set == nullptr)
    return false;

  LightningShaderIRFunction* shaderFunction = mLibrary->FindFunction(set);
  // If this is an existing lightning function then we have to translate this member
  // access into a function call
  if (shaderFunction != nullptr)
  {
    Array<ILightningShaderIR*> arguments;
    // Pass through 'this' if this is an instance function
    if (!set->IsStatic)
      arguments.PushBack(WalkAndGetResult(memberAccessNode->LeftOperand, context));
    // Get the result value if needed and add it as an argument
    if (resultValue == nullptr)
      resultValue = WalkAndGetValueTypeResult(resultNode, context);
    arguments.PushBack(resultValue);

    // Turn this into a function call
    LightningShaderIROp* functionCallOp = GenerateFunctionCall(context->mCurrentBlock, shaderFunction, context);
    WalkMemberAccessFunctionCall(arguments, memberAccessNode, shaderFunction, context);
    return true;
  }

  // Try to find a resolver for this setter
  MemberPropertySetterResolverIRFn resolver =
      mLibrary->FindSetterResolver(memberAccessNode->LeftOperand->ResultType, set);
  if (resolver != nullptr)
  {
    // If there was no result value then walk the result node to compute it
    // (prevents a creating redundant instructions in pure setter cases)
    if (resultValue == nullptr)
      resultValue = WalkAndGetValueTypeResult(resultNode, context);
    resolver(this, memberAccessNode, resultValue, context);
    return true;
  }

  return false;
}

ILightningShaderIR* LightningSpirVFrontEnd::PerformBinaryOp(Lightning::BinaryOperatorNode*& node,
                                                    OpType opType,
                                                    LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Walk both sides of the op
  ILightningShaderIR* leftInstruction = WalkAndGetResult(node->LeftOperand, context);
  ILightningShaderIR* rightInstruction = WalkAndGetResult(node->RightOperand, context);

  return PerformBinaryOp(node, opType, leftInstruction, rightInstruction, context);
}

ILightningShaderIR* LightningSpirVFrontEnd::PerformBinaryOp(Lightning::BinaryOperatorNode*& node,
                                                    OpType opType,
                                                    ILightningShaderIR* lhs,
                                                    ILightningShaderIR* rhs,
                                                    LightningSpirVFrontEndContext* context)
{
  // All binary operators require value types
  LightningShaderIROp* leftValueOp = GetOrGenerateValueTypeFromIR(lhs, context);
  LightningShaderIROp* rightValueOp = GetOrGenerateValueTypeFromIR(rhs, context);

  // Generate dependencies
  LightningShaderIRType* resultType = FindType(node->ResultType, node);

  // Generate the binary op
  LightningShaderIROp* binaryOp = BuildCurrentBlockIROp(opType, resultType, leftValueOp, rightValueOp, context);
  context->PushIRStack(binaryOp);
  return binaryOp;
}

void LightningSpirVFrontEnd::PerformBinaryAssignmentOp(Lightning::BinaryOperatorNode*& node,
                                                   OpType opType,
                                                   LightningSpirVFrontEndContext* context)
{
  // Walk both sides of the op
  ILightningShaderIR* leftInstruction = WalkAndGetResult(node->LeftOperand, context);
  ILightningShaderIR* rightInstruction = WalkAndGetResult(node->RightOperand, context);
  PerformBinaryAssignmentOp(node, opType, leftInstruction, rightInstruction, context);
}

void LightningSpirVFrontEnd::PerformBinaryAssignmentOp(Lightning::BinaryOperatorNode*& node,
                                                   OpType opType,
                                                   ILightningShaderIR* lhs,
                                                   ILightningShaderIR* rhs,
                                                   LightningSpirVFrontEndContext* context)
{
  // All binary operators require value types so load from pointers if we had
  // them
  LightningShaderIROp* leftValueOp = GetOrGenerateValueTypeFromIR(lhs, context);
  LightningShaderIROp* rightValueOp = GetOrGenerateValueTypeFromIR(rhs, context);

  // Perform the op
  LightningShaderIRType* resultType = FindType(node->LeftOperand->ResultType, node);
  LightningShaderIROp* binaryOpInstruction = BuildCurrentBlockIROp(opType, resultType, leftValueOp, rightValueOp, context);

  // Deal with setters. This requires 'promoting' the setter above the
  // assignment op (e.g. A += B -> A.Set(A + B))
  if (ResolveSetter(node, binaryOpInstruction, node->RightOperand, context))
    return;

  // Validate this op can be assigned to (constants)
  ValidateLValue(leftValueOp, node->Location);

  // Since this is a binary assignment op, the left side must be a pointer type
  LightningShaderIROp* leftOp = lhs->As<LightningShaderIROp>();
  if (!leftOp->IsResultPointerType())
  {
    SendTranslationError(node->Location, "Left operand in a binary assignment op must be a pointer type");
    // Since this is an assignment op, no dummy variable generation is necessary
    return;
  }

  // Store into the variable
  BasicBlock* currentBlock = context->GetCurrentBlock();
  BuildStoreOp(currentBlock, lhs, binaryOpInstruction, context);
}

ILightningShaderIR* LightningSpirVFrontEnd::PerformUnaryOp(Lightning::UnaryOperatorNode*& node,
                                                   OpType opType,
                                                   LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* resultType = FindType(node->ResultType, node);

  // Get the operand for the op (requires a value type)
  LightningShaderIROp* operandValueOp = WalkAndGetValueTypeResult(node->Operand, context);

  LightningShaderIROp* unaryOp = BuildCurrentBlockIROp(opType, resultType, operandValueOp, context);
  context->PushIRStack(unaryOp);
  return unaryOp;
}

ILightningShaderIR* LightningSpirVFrontEnd::PerformUnaryIncDecOp(Lightning::UnaryOperatorNode*& node,
                                                         ILightningShaderIR* constantOne,
                                                         OpType opType,
                                                         LightningSpirVFrontEndContext* context)
{
  // Get the operand to work on (must be a value type)
  ILightningShaderIR* operand = WalkAndGetResult(node->Operand, context);
  LightningShaderIROp* operandValueOp = GetOrGenerateValueTypeFromIR(operand, context);

  // Perform the op (add/sub)
  LightningShaderIRType* resultType = FindType(node->Operand->ResultType, node->Operand);
  ILightningShaderIR* tempOp = BuildCurrentBlockIROp(opType, resultType, operandValueOp, constantOne, context);

  // If the operand was a pointer type instead of a temporary then we need to
  // store the value back into the pointer
  LightningShaderIROp* op = operand->As<LightningShaderIROp>();
  if (op != nullptr && op->IsResultPointerType())
  {
    BasicBlock* currentBlock = context->GetCurrentBlock();
    BuildStoreOp(currentBlock, operand, tempOp, context);
  }
  context->PushIRStack(tempOp);
  return tempOp;
}

ILightningShaderIR* LightningSpirVFrontEnd::PerformTypeCast(Lightning::TypeCastNode*& node,
                                                    OpType opType,
                                                    LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* resultType = FindType(node->ResultType, node);

  // Get the thing we're casting. Casts require a value type so generate a value
  // type if necessary.
  LightningShaderIROp* operandValueResult = WalkAndGetValueTypeResult(node->Operand, context);

  // Generate the cast
  LightningShaderIROp* castOp = BuildCurrentBlockIROp(opType, resultType, operandValueResult, context);
  context->PushIRStack(castOp);
  return castOp;
}

LightningShaderIROp* LightningSpirVFrontEnd::GetIntegerConstant(int value, LightningSpirVFrontEndContext* context)
{
  Lightning::BoundType* lightningIntType = LightningTypeId(int);
  LightningShaderIRType* shaderIntType = mLibrary->FindType(lightningIntType);
  LightningShaderIROp* constantIntOp = GetConstant(shaderIntType, value, context);
  return constantIntOp;
}

  LightningShaderIROp* LightningSpirVFrontEnd::GetIntegerConstant(u32 value, LightningSpirVFrontEndContext* context)
{
  int intValue = static_cast<int>(value);
  Lightning::BoundType* zilchIntType = LightningTypeId(int);
  LightningShaderIRType* shaderIntType = mLibrary->FindType(zilchIntType);
  LightningShaderIROp* constantIntOp = GetConstant(shaderIntType, intValue, context);
  return constantIntOp;
}

LightningShaderIROp* LightningSpirVFrontEnd::GetConstant(LightningShaderIRType* type,
                                                 StringParam value,
                                                 LightningSpirVFrontEndContext* context)
{
  Lightning::Any constantValue;
  ToAny(type, value, constantValue);
  return GetConstant(type, constantValue, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::GetConstant(LightningShaderIRType* type,
                                                 Lightning::Any value,
                                                 LightningSpirVFrontEndContext* context)
{
  // Each constant should only be declared once. Find if it already exists
  ConstantOpKeyType constantKey = ConstantOpKeyType(type, value);
  LightningShaderIROp* opConstant = mLibrary->FindConstantOp(constantKey, false);
  if (opConstant == nullptr)
  {
    // It doesn't exist so create the constant. Constants are two parts:
    // The actual constant value and the op to declare the constant.
    LightningShaderIRConstantLiteral* constant = GetOrCreateConstantLiteral(value);
    opConstant = BuildIROpNoBlockAdd(OpType::OpConstant, type, context);
    opConstant->mArguments.PushBack(constant);
  }

  mLibrary->mConstantOps.InsertNoOverwrite(constantKey, opConstant);

  return opConstant;
}

LightningShaderIROp* LightningSpirVFrontEnd::ConstructCompositeFromScalar(BasicBlock* block,
                                                                  LightningShaderIRType* compositeType,
                                                                  ILightningShaderIR* scalar,
                                                                  LightningSpirVFrontEndContext* context)
{
  // If this is already a scalar type then return the value as-is
  if (compositeType->mComponents == 1)
    return scalar->As<LightningShaderIROp>();

  // Construct the composite from the individual scalar values (splat)
  LightningShaderIROp* composite = BuildIROp(block, OpType::OpCompositeConstruct, compositeType, context);
  for (size_t i = 0; i < compositeType->mComponents; ++i)
    composite->mArguments.PushBack(scalar);
  return composite;
}

LightningShaderIROp* LightningSpirVFrontEnd::AddSpecializationConstant(Lightning::MemberVariableNode* node,
                                                               LightningShaderIRType* varType,
                                                               LightningSpirVFrontEndContext* context)
{
  LightningShaderIRConstantLiteral* defaultLiteral = nullptr;
  // If the initial value of the node is a value node then we can initialize
  // this constant to the actual value. Other types are more complicated as the
  // default value requires actually constructing the type to get value
  // (something like var Pi : Real = Math.Pi). @JoshD: Deal with finding these
  // constants later.
  Lightning::ValueNode* valueNode = Lightning::Type::DynamicCast<Lightning::ValueNode*>(node->InitialValue);
  if (valueNode != nullptr)
  {
    Lightning::Any defaultValue;
    ToAny(varType, valueNode->Value.Token, defaultValue);
    defaultLiteral = GetOrCreateConstantLiteral(defaultValue);
  }

  return AddSpecializationConstantRecursively(
      node->CreatedField, varType, node->Name.Token, defaultLiteral, node->Location, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::AddSpecializationConstantRecursively(void* key,
                                                                          LightningShaderIRType* varType,
                                                                          StringParam varName,
                                                                          LightningShaderIRConstantLiteral* literalValue,
                                                                          Lightning::CodeLocation& codeLocation,
                                                                          LightningSpirVFrontEndContext* context)
{
  // The var name is generated the same as any other global
  String propertyName = GenerateSpirVPropertyName(varName, context->mCurrentType);

  // Deal with scalar types.
  if (varType->mBaseType == ShaderIRTypeBaseType::Bool || varType->mBaseType == ShaderIRTypeBaseType::Int ||
      varType->mBaseType == ShaderIRTypeBaseType::Float)
  {
    // Either use the given literal value or construct a default
    LightningShaderIRConstantLiteral* defaultLiteral = literalValue;
    if (defaultLiteral == nullptr)
    {
      Lightning::Any defaultValue;
      ToAny(varType, String(), defaultValue);
      defaultLiteral = GetOrCreateConstantLiteral(defaultValue);
    }

    LightningShaderIROp* specConstantOp = CreateSpecializationConstant(key, OpType::OpSpecConstant, varType, context);
    specConstantOp->mArguments.PushBack(defaultLiteral);
    specConstantOp->mDebugResultName = propertyName;
    return specConstantOp;
  }

  // Deal with vectors/matrices
  if (varType->mBaseType == ShaderIRTypeBaseType::Vector || varType->mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    LightningShaderIROp* specConstantCompositeOp =
        CreateSpecializationConstant(key, OpType::OpSpecConstantComposite, varType, context);
    specConstantCompositeOp->mDebugResultName = propertyName;
    // Create a sub-constant for each constituent. For vectors these are scalar,
    // for matrices these are vectors.
    char subNames[] = {'X', 'Y', 'Z', 'W'};
    LightningShaderIRType* componentType = GetComponentType(varType);
    for (size_t i = 0; i < varType->mComponents; ++i)
    {
      // Construct the sub-constant's name for debugging purposes.
      String fullSubVarName = BuildString(varName, ".", ToString(subNames[i]));
      // Create the sub-constant. This sub-constant is not given a unique key as
      // there's never a case where we need to lookup vector.x.
      LightningShaderIROp* constituent =
          AddSpecializationConstantRecursively(nullptr, componentType, fullSubVarName, nullptr, codeLocation, context);
      specConstantCompositeOp->mArguments.PushBack(constituent);
    }
    return specConstantCompositeOp;
  }

  // Structs are handled almost the same as vectors/matrices, the only
  // difference is how members are iterated over and how to create their debug
  // names.
  if (varType->mBaseType == ShaderIRTypeBaseType::Struct)
  {
    LightningShaderIROp* specConstantCompositeOp =
        CreateSpecializationConstant(key, OpType::OpSpecConstantComposite, varType, context);
    specConstantCompositeOp->mDebugResultName = propertyName;

    for (u32 i = 0; i < varType->GetSubTypeCount(); ++i)
    {
      String memberName = varType->GetMemberName(i);
      String fullSubVarName = BuildString(varName, ".", memberName);
      LightningShaderIRType* subType = varType->GetSubType(i);
      LightningShaderIROp* constituent =
          AddSpecializationConstantRecursively(nullptr, subType, fullSubVarName, nullptr, codeLocation, context);
      specConstantCompositeOp->mArguments.PushBack(constituent);
    }
    return specConstantCompositeOp;
  }

  String errorMsg = String::Format("Type '%s' is not valid as a specialization constant.", varType->mName.c_str());
  SendTranslationError(codeLocation, errorMsg);
  // Return a dummy constant so that we don't crash. This is not valid spir-v
  // though.
  return CreateSpecializationConstant(key, OpType::OpSpecConstantComposite, varType, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::CreateSpecializationConstant(void* key,
                                                                  OpType opType,
                                                                  LightningShaderIRType* varType,
                                                                  LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* specConstantOp = BuildIROpNoBlockAdd(opType, varType, context);
  // Always add this constant to the library for memory management
  mLibrary->mOwnedSpecializationConstants.PushBack(specConstantOp);
  // Additionally, if there's a valid key then store this in the library's
  // lookup map. This is used to find the constant at a later point in time via
  // the key (such as the lightning field). If the key is null then we don't
  // have/need to lookup this constant later (e.g. a sub-member of a vector).
  if (key != nullptr)
    mLibrary->mSpecializationConstantMap.InsertOrError(key, specConstantOp);

  return specConstantOp;
}

void LightningSpirVFrontEnd::ToAny(LightningShaderIRType* type, StringParam value, Lightning::Any& result)
{
  if (type->mBaseType == ShaderIRTypeBaseType::Bool)
  {
    bool temp;
    ToValue(value, temp);
    result = temp;
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Int)
  {
    int temp;
    ToValue(value, temp);
    result = temp;
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Float)
  {
    float temp;
    ToValue(value, temp);
    result = temp;
  }
  else
  {
    result = value;
  }
}

void LightningSpirVFrontEnd::ExtractDebugInfo(Lightning::SyntaxNode* node, LightningShaderDebugInfo& debugInfo)
{
  if (node == nullptr)
    return;

  debugInfo.mLocations.mCodeLocations.PushBack(node->Location);
  GetLineAsComment(node, debugInfo.mComments);
}

void LightningSpirVFrontEnd::GetLineAsComment(Lightning::SyntaxNode* node, LightningShaderIRComments& comments)
{
  String comment = mLightningCommentParser.Run(node);
  if (!comment.Empty())
  {
    comments.Add(comment);
  }
}

BasicBlock* LightningSpirVFrontEnd::BuildBlock(StringParam labelDebugName, LightningSpirVFrontEndContext* context)
{
  BasicBlock* block = BuildBlockNoStack(labelDebugName, context);
  context->mCurrentFunction->mBlocks.PushBack(block);
  return block;
}

BasicBlock* LightningSpirVFrontEnd::BuildBlockNoStack(StringParam labelDebugName, LightningSpirVFrontEndContext* context)
{
  BasicBlock* block = new BasicBlock();
  block->mDebugResultName = labelDebugName;
  return block;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildIROpNoBlockAdd(OpType opType,
                                                         LightningShaderIRType* resultType,
                                                         LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* result = new LightningShaderIROp(opType);

  if (context != nullptr)
  {
    result->mDebugInfo = context->mDebugInfo;
    result->mResultType = resultType;
    context->mDebugInfo.Clear();
  }
  else
    result->mResultType = resultType;

  return result;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildIROp(BasicBlock* block,
                                               OpType opType,
                                               LightningShaderIRType* resultType,
                                               LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* result = BuildIROpNoBlockAdd(opType, resultType, context);

  block->mLines.PushBack(result);
  return result;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildIROp(BasicBlock* block,
                                               OpType opType,
                                               LightningShaderIRType* resultType,
                                               ILightningShaderIR* arg0,
                                               LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* result = BuildIROp(block, opType, resultType, context);
  result->mArguments.PushBack(arg0);
  return result;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildIROp(BasicBlock* block,
                                               OpType opType,
                                               LightningShaderIRType* resultType,
                                               ILightningShaderIR* arg0,
                                               ILightningShaderIR* arg1,
                                               LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* result = BuildIROp(block, opType, resultType, context);
  result->mArguments.PushBack(arg0);
  result->mArguments.PushBack(arg1);
  return result;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildIROp(BasicBlock* block,
                                               OpType opType,
                                               LightningShaderIRType* resultType,
                                               ILightningShaderIR* arg0,
                                               ILightningShaderIR* arg1,
                                               ILightningShaderIR* arg2,
                                               LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* result = BuildIROp(block, opType, resultType, context);
  result->mArguments.PushBack(arg0);
  result->mArguments.PushBack(arg1);
  result->mArguments.PushBack(arg2);
  return result;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildCurrentBlockIROp(OpType opType,
                                                           LightningShaderIRType* resultType,
                                                           LightningSpirVFrontEndContext* context)
{
  return BuildIROp(context->GetCurrentBlock(), opType, resultType, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildCurrentBlockIROp(OpType opType,
                                                           LightningShaderIRType* resultType,
                                                           ILightningShaderIR* arg0,
                                                           LightningSpirVFrontEndContext* context)
{
  return BuildIROp(context->GetCurrentBlock(), opType, resultType, arg0, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildCurrentBlockIROp(OpType opType,
                                                           LightningShaderIRType* resultType,
                                                           ILightningShaderIR* arg0,
                                                           ILightningShaderIR* arg1,
                                                           LightningSpirVFrontEndContext* context)
{
  return BuildIROp(context->GetCurrentBlock(), opType, resultType, arg0, arg1, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildCurrentBlockIROp(OpType opType,
                                                           LightningShaderIRType* resultType,
                                                           ILightningShaderIR* arg0,
                                                           ILightningShaderIR* arg1,
                                                           ILightningShaderIR* arg2,
                                                           LightningSpirVFrontEndContext* context)
{
  return BuildIROp(context->GetCurrentBlock(), opType, resultType, arg0, arg1, arg2, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildCurrentBlockAccessChain(LightningShaderIRType* baseResultType,
                                                                  LightningShaderIROp* selfInstance,
                                                                  ILightningShaderIR* arg0,
                                                                  LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* resultPointerType = baseResultType->GetPointerType();

  // We have to declare the access chain with the same storage class as
  // the self instance. This shows up primarily with runtime arrays
  // (e.g. accessing the internal data has to be uniform storage class).
  // To avoid creating a duplicate type, only do this if the storage
  // class isn't function, otherwise we already have the correct pointer type.
  spv::StorageClass resultStorageClass = selfInstance->mResultType->mStorageClass;
  if (resultStorageClass != spv::StorageClassFunction)
    resultPointerType = FindOrCreatePointerInterfaceType(mLibrary, baseResultType, resultStorageClass);

  LightningShaderIROp* accessChainOp =
      BuildCurrentBlockIROp(OpType::OpAccessChain, resultPointerType, selfInstance, context);
  accessChainOp->mArguments.PushBack(arg0);
  return accessChainOp;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildCurrentBlockAccessChain(LightningShaderIRType* baseResultType,
                                                                  LightningShaderIROp* selfInstance,
                                                                  ILightningShaderIR* arg0,
                                                                  ILightningShaderIR* arg1,
                                                                  LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* accessChainOp = BuildCurrentBlockAccessChain(baseResultType, selfInstance, arg0, context);
  accessChainOp->mArguments.PushBack(arg1);
  return accessChainOp;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildDecorationOp(BasicBlock* block,
                                                       ILightningShaderIR* decorationTarget,
                                                       spv::Decoration decorationType,
                                                       LightningSpirVFrontEndContext* context)
{
  LightningShaderIRConstantLiteral* decorationTypeLiteral = GetOrCreateConstantIntegerLiteral(decorationType);
  return BuildIROp(block, OpType::OpDecorate, nullptr, decorationTarget, decorationTypeLiteral, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildDecorationOp(BasicBlock* block,
                                                       ILightningShaderIR* decorationTarget,
                                                       spv::Decoration decorationType,
                                                       u32 decorationValue,
                                                       LightningSpirVFrontEndContext* context)
{
  LightningShaderIRConstantLiteral* decorationTypeLiteral = GetOrCreateConstantIntegerLiteral(decorationType);
  LightningShaderIRConstantLiteral* decorationValueLiteral = GetOrCreateConstantIntegerLiteral(decorationValue);
  return BuildIROp(
      block, OpType::OpDecorate, nullptr, decorationTarget, decorationTypeLiteral, decorationValueLiteral, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildMemberDecorationOp(BasicBlock* block,
                                                             ILightningShaderIR* decorationTarget,
                                                             u32 memberOffset,
                                                             spv::Decoration decorationType,
                                                             LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* resultOp = BuildIROp(block, OpType::OpMemberDecorate, nullptr, decorationTarget, context);
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(memberOffset));
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(decorationType));
  return resultOp;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildMemberDecorationOp(BasicBlock* block,
                                                             ILightningShaderIR* decorationTarget,
                                                             u32 memberOffset,
                                                             spv::Decoration decorationType,
                                                             u32 decorationValue,
                                                             LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* resultOp = BuildIROp(block, OpType::OpMemberDecorate, nullptr, decorationTarget, context);
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(memberOffset));
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(decorationType));
  resultOp->mArguments.PushBack(GetOrCreateConstantIntegerLiteral(decorationValue));
  return resultOp;
}

LightningShaderIRConstantLiteral* LightningSpirVFrontEnd::GetOrCreateConstantIntegerLiteral(int value)
{
  return GetOrCreateConstantLiteral(value);
}

LightningShaderIRConstantLiteral* LightningSpirVFrontEnd::GetOrCreateConstantIntegerLiteral(u32 value)
{
  return GetOrCreateConstantLiteral(value);
}

LightningShaderIRConstantLiteral* LightningSpirVFrontEnd::GetOrCreateConstantLiteral(Lightning::Any value)
{
  LightningShaderIRConstantLiteral* constantValue = mLibrary->FindConstantLiteral(value);
  if (constantValue != nullptr)
    return constantValue;

  constantValue = new LightningShaderIRConstantLiteral(value);
  constantValue->mDebugResultName = value.ToString();
  mLibrary->mConstantLiterals.InsertOrError(value, constantValue);
  return constantValue;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildOpVariable(LightningShaderIRType* resultType, LightningSpirVFrontEndContext* context)
{
  // All variable declarations must be at the beginning of the first block of a
  // function. To do this we always grab the first block and add to the local
  // variables section of this block (which is emitted first)
  BasicBlock* firstBlock = context->mCurrentFunction->mBlocks[0];
  BasicBlock* oldBlock = context->mCurrentBlock;
  context->mCurrentBlock = firstBlock;

  LightningShaderIROp* variableOp = BuildOpVariable(firstBlock, resultType, (int)spv::StorageClassFunction, context);

  // Set the context back to the previously active block
  context->mCurrentBlock = oldBlock;

  return variableOp;
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildOpVariable(BasicBlock* block,
                                                     LightningShaderIRType* resultType,
                                                     int storageConstant,
                                                     LightningSpirVFrontEndContext* context)
{
  // Declare the constant for function storage variables
  LightningShaderIRConstantLiteral* functionStorageConstant = GetOrCreateConstantLiteral(storageConstant);
  // Make the variable declaration (with function storage) and add it to the
  // local variables section of the block
  LightningShaderIROp* variableOp = BuildIROpNoBlockAdd(OpType::OpVariable, resultType, context);
  variableOp->mArguments.PushBack(functionStorageConstant);
  block->mLocalVariables.PushBack(variableOp);

  return variableOp;
}

ILightningShaderIR* LightningSpirVFrontEnd::WalkAndGetResult(Lightning::SyntaxNode* node, LightningSpirVFrontEndContext* context)
{
  mWalker.Walk(this, node, context);
  return context->PopIRStack();
}

LightningShaderIROp* LightningSpirVFrontEnd::WalkAndGetValueTypeResult(BasicBlock* block,
                                                               Lightning::SyntaxNode* node,
                                                               LightningSpirVFrontEndContext* context)
{
  ILightningShaderIR* nodeResult = WalkAndGetResult(node, context);
  LightningShaderIROp* valueResult = GetOrGenerateValueTypeFromIR(block, nodeResult, context);
  return valueResult;
}

LightningShaderIROp* LightningSpirVFrontEnd::WalkAndGetValueTypeResult(Lightning::SyntaxNode* node,
                                                               LightningSpirVFrontEndContext* context)
{
  ILightningShaderIR* nodeResult = WalkAndGetResult(node, context);
  // The block must be fetched here as walking the node can change the current
  // block
  LightningShaderIROp* valueResult = GetOrGenerateValueTypeFromIR(context->GetCurrentBlock(), nodeResult, context);
  return valueResult;
}

LightningShaderIROp* LightningSpirVFrontEnd::GetOrGenerateValueTypeFromIR(BasicBlock* block,
                                                                  ILightningShaderIR* instruction,
                                                                  LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* op = instruction->As<LightningShaderIROp>();
  if (!op->IsResultPointerType())
    return op;

  return BuildIROp(block, OpType::OpLoad, op->mResultType->mDereferenceType, instruction, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::GetOrGenerateValueTypeFromIR(ILightningShaderIR* instruction,
                                                                  LightningSpirVFrontEndContext* context)
{
  return GetOrGenerateValueTypeFromIR(context->GetCurrentBlock(), instruction, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::GetOrGeneratePointerTypeFromIR(BasicBlock* block,
                                                                    ILightningShaderIR* instruction,
                                                                    LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* op = instruction->As<LightningShaderIROp>();
  if (op->IsResultPointerType())
    return op;

  LightningShaderIROp* variableOp = BuildOpVariable(op->mResultType->mPointerType, context);
  BuildStoreOp(variableOp, op, context);
  return variableOp;
}

LightningShaderIROp* LightningSpirVFrontEnd::GetOrGeneratePointerTypeFromIR(ILightningShaderIR* instruction,
                                                                    LightningSpirVFrontEndContext* context)
{
  return GetOrGeneratePointerTypeFromIR(context->GetCurrentBlock(), instruction, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildStoreOp(ILightningShaderIR* target,
                                                  ILightningShaderIR* source,
                                                  LightningSpirVFrontEndContext* context,
                                                  bool forceLoadStore)
{
  return BuildStoreOp(context->GetCurrentBlock(), target, source, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::BuildStoreOp(BasicBlock* block,
                                                  ILightningShaderIR* target,
                                                  ILightningShaderIR* source,
                                                  LightningSpirVFrontEndContext* context,
                                                  bool forceLoadStore)
{
  LightningShaderIROp* sourceOp = source->As<LightningShaderIROp>();
  // Change what op we use to store depending on if the source is a value or
  // pointer type. If the source was a pointer type we'd have to load it first
  // before setting, but instead we can just use CopyMemory.
  if (sourceOp->IsResultPointerType())
  {
    // Currently there's various issues with the spirv tool chains that don't
    // handle OpCopyMemory well. For this reason support forcing load/store
    // instead.
    if (forceLoadStore)
    {
      LightningShaderIROp* loadOp =
          BuildIROp(block, OpType::OpLoad, sourceOp->mResultType->mDereferenceType, sourceOp, context);
      return BuildIROp(block, OpType::OpStore, nullptr, target, loadOp, context);
    }
    else
      return BuildIROp(block, OpType::OpCopyMemory, nullptr, target, sourceOp, context);
  }
  else
    return BuildIROp(block, OpType::OpStore, nullptr, target, sourceOp, context);
}

void LightningSpirVFrontEnd::WriteFunctionCallArguments(Lightning::FunctionCallNode*& node,
                                                    LightningShaderIROp* functionCallOp,
                                                    LightningSpirVFrontEndContext* context)
{
  WriteFunctionCallArguments(node, node->ResultType, functionCallOp, context);
}

void LightningSpirVFrontEnd::WriteFunctionCallArguments(Lightning::FunctionCallNode*& node,
                                                    Lightning::Type* returnType,
                                                    LightningShaderIROp* functionCallOp,
                                                    LightningSpirVFrontEndContext* context)
{
  for (size_t i = 0; i < node->Arguments.Size(); ++i)
  {
    ILightningShaderIR* argument = WalkAndGetResult(node->Arguments[i], context);
    WriteFunctionCallArgument(argument, functionCallOp, nullptr, context);
  }

  // If there is a return then we have to push the result onto a stack so any
  // assignments can get the value
  if (returnType != LightningTypeId(void))
  {
    context->PushIRStack(functionCallOp);
  }
}

void LightningSpirVFrontEnd::WriteFunctionCallArguments(Array<ILightningShaderIR*> arguments,
                                                    LightningShaderIRType* returnType,
                                                    LightningShaderIRType* functionType,
                                                    LightningShaderIROp* functionCallOp,
                                                    LightningSpirVFrontEndContext* context)
{
  for (u32 i = 0; i < arguments.Size(); ++i)
  {
    ILightningShaderIR* argument = arguments[i];
    LightningShaderIRType* paramType = functionType->GetSubType(i + 1);
    WriteFunctionCallArgument(argument, functionCallOp, paramType, context);
  }

  // If there is a return then we have to push the result onto a stack so any
  // assignments can get the value
  if (returnType != nullptr && returnType->mLightningType != LightningTypeId(void))
  {
    context->PushIRStack(functionCallOp);
  }
}

void LightningSpirVFrontEnd::WriteFunctionCallArgument(ILightningShaderIR* argument,
                                                   LightningShaderIROp* functionCallOp,
                                                   LightningShaderIRType* paramType,
                                                   LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* paramOp = argument->As<LightningShaderIROp>();

  // If we know the function parameter's expected type is a value type while
  // the given parameter op is a pointer the convert the pointer to a value
  // type.
  if (paramType == nullptr || !paramType->IsPointerType())
  {
    if (paramOp->IsResultPointerType())
      paramOp = GetOrGenerateValueTypeFromIR(paramOp, context);
  }

  functionCallOp->mArguments.PushBack(paramOp);
}

LightningShaderIROp* LightningSpirVFrontEnd::GenerateBoolToIntegerCast(BasicBlock* block,
                                                               LightningShaderIROp* source,
                                                               LightningShaderIRType* destType,
                                                               LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* plasma = GetIntegerConstant(0, context);
  LightningShaderIROp* one = GetIntegerConstant(1, context);
  return GenerateFromBoolCast(block, source, destType, plasma, one, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::GenerateFromBoolCast(BasicBlock* block,
                                                          LightningShaderIROp* source,
                                                          LightningShaderIRType* destType,
                                                          ILightningShaderIR* plasma,
                                                          ILightningShaderIR* one,
                                                          LightningSpirVFrontEndContext* context)
{
  // SpirV doesn't support a bool to type cast. Instead a select op must be
  // generated to choose between two values. This is effectively: bool ?
  // trueResult : falseResult.
  ILightningShaderIR* condition = source;
  ILightningShaderIR* trueValue = one;
  ILightningShaderIR* falseValue = plasma;

  // Handle vector types
  if (destType->mComponents != 1)
  {
    // Construct the result vector types from the individual plasma/one scalars
    LightningShaderIROp* plasmaVec = BuildIROp(block, OpType::OpCompositeConstruct, destType, context);
    LightningShaderIROp* oneVec = BuildIROp(block, OpType::OpCompositeConstruct, destType, context);
    plasmaVec->mDebugResultName = "zero";
    oneVec->mDebugResultName = "one";
    for (size_t i = 0; i < destType->mComponents; ++i)
    {
      plasmaVec->mArguments.PushBack(plasma);
      oneVec->mArguments.PushBack(one);
    }
    trueValue = oneVec;
    falseValue = plasmaVec;
  }

  // Perform a component-wise selection
  LightningShaderIROp* operation = BuildIROp(block, OpType::OpSelect, destType, condition, trueValue, falseValue, context);
  return operation;
}

LightningShaderIROp* LightningSpirVFrontEnd::GenerateIntegerToBoolCast(BasicBlock* block,
                                                               LightningShaderIROp* source,
                                                               LightningShaderIRType* destType,
                                                               LightningSpirVFrontEndContext* context)
{
  ILightningShaderIR* plasma = GetIntegerConstant(0, context);
  return GenerateToBoolCast(block, OpType::OpINotEqual, source, destType, plasma, context);
}

LightningShaderIROp* LightningSpirVFrontEnd::GenerateToBoolCast(BasicBlock* block,
                                                        OpType op,
                                                        LightningShaderIROp* source,
                                                        LightningShaderIRType* destType,
                                                        ILightningShaderIR* plasma,
                                                        LightningSpirVFrontEndContext* context)
{
  // SpirV doesn't support a cast to a bool. Instead this must be generated from
  // a comparison operator with the corrsponding plasma vector. E.g.
  // (Boolean2)Integer2 => Integer2 != Integer2(0)
  ILightningShaderIR* rhs = plasma;
  LightningShaderIROp* condition = source;
  LightningShaderIRType* sourceType = source->mResultType;

  // Handle vector types
  if (sourceType->mComponents != 1)
  {
    // Construct the comparison source vector type from the individual plasma
    // scalar
    LightningShaderIROp* plasmaVec = BuildIROp(block, OpType::OpCompositeConstruct, sourceType, context);
    for (size_t i = 0; i < sourceType->mComponents; ++i)
    {
      plasmaVec->mArguments.PushBack(plasma);
    }
    // Set the comparison rhs op to the vector instead of the scalar
    rhs = plasmaVec;
  }

  LightningShaderIROp* operation = BuildIROp(block, op, destType, condition, rhs, context);
  return operation;
}

LightningShaderIRType* LightningSpirVFrontEnd::GetResultValueType(LightningShaderIROp* op)
{
  LightningShaderIRType* resultType = op->mResultType;
  if (resultType->IsPointerType())
    return resultType->mDereferenceType;
  return resultType;
}

LightningShaderIRType* LightningSpirVFrontEnd::GetPointerValueType(LightningShaderIROp* op)
{
  LightningShaderIRType* resultType = op->mResultType;
  if (resultType->IsPointerType())
    return resultType;
  return resultType->mPointerType;
}

bool LightningSpirVFrontEnd::ContainsAttribute(LightningShaderIRType* shaderType, StringParam attributeName)
{
  ShaderIRTypeMeta* meta = shaderType->mMeta;
  if (meta == nullptr)
    return false;
  return meta->ContainsAttribute(attributeName);
}

bool LightningSpirVFrontEnd::CheckForNonCopyableType(LightningShaderIRType* shaderType,
                                                 Lightning::ExpressionNode* node,
                                                 LightningSpirVFrontEndContext* context,
                                                 bool generateDummyResult)
{
  LightningShaderIRType* shaderValueType = shaderType;
  if (shaderValueType->IsPointerType())
    shaderValueType = shaderValueType->mDereferenceType;

  if (ContainsAttribute(shaderValueType, SpirVNameSettings::mNonCopyableAttributeName))
  {
    String msg = String::Format("Type '%s' cannot be copied.", shaderValueType->mName.c_str());
    SendTranslationError(node->Location, msg);
    if (generateDummyResult)
      context->PushIRStack(GenerateDummyIR(node, context));
    return true;
  }
  return false;
}

LightningShaderIRType* LightningSpirVFrontEnd::FindType(Lightning::Type* type, Lightning::SyntaxNode* syntaxNode, bool reportErrors)
{
  LightningShaderIRType* shaderType = mLibrary->FindType(type);
  if (shaderType != nullptr)
    return shaderType;

  if (reportErrors)
  {
    String msg = String::Format("Failed to find type '%s'", type->ToString().c_str());
    Error("%s", msg.c_str());
    SendTranslationError(syntaxNode->Location, msg);
  }

  // Return some default type so that we don't crash on null
  return mLibrary->FindType(LightningTypeId(void));
}

LightningShaderIRType* LightningSpirVFrontEnd::FindType(Lightning::ExpressionNode* syntaxNode, bool reportErrors)
{
  return FindType(syntaxNode->ResultType, syntaxNode, reportErrors);
}

bool LightningSpirVFrontEnd::ValidateResultType(ILightningShaderIR* instruction,
                                            ShaderIRTypeBaseType::Enum expectedType,
                                            Lightning::CodeLocation& codeLocation,
                                            bool throwException)
{
  LightningShaderIROp* op = instruction->As<LightningShaderIROp>();
  if (op->mResultType != nullptr)
  {
    if (op->mResultType->mBaseType == expectedType)
      return true;
  }
  SendTranslationError(codeLocation, "Invalid result type");
  return false;
}

bool LightningSpirVFrontEnd::ValidateLValue(LightningShaderIROp* op, Lightning::CodeLocation& codeLocation, bool throwException)
{
  OpType opType = op->mOpType;
  if (opType == OpType::OpSpecConstant || opType == OpType::OpSpecConstantComposite)
  {
    String errMsg = String::Format("Specialization constants are read-only");
    SendTranslationError(codeLocation, errMsg);
    return false;
  }

  return true;
}

ILightningShaderIR* LightningSpirVFrontEnd::GenerateDummyIR(Lightning::ExpressionNode* node, LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* shaderResultType = FindType(node->ResultType, node, context);
  return BuildOpVariable(shaderResultType, context);
}

// Send a translation error with a simple message (also marks the translation as
// having failed)
void LightningSpirVFrontEnd::SendTranslationError(Lightning::CodeLocation& codeLocation, StringParam message)
{
  SendTranslationError(codeLocation, message, message);
}

void LightningSpirVFrontEnd::SendTranslationError(Lightning::CodeLocation& codeLocation,
                                              StringParam shortMsg,
                                              StringParam fullMsg)
{
  mErrorTriggered = true;
  if (mProject != nullptr)
    mProject->SendTranslationError(codeLocation, shortMsg, fullMsg);
}

void LightningSpirVFrontEnd::SendTranslationError(Lightning::CodeLocation* codeLocation, StringParam message)
{
  if (codeLocation != nullptr)
  {
    SendTranslationError(*codeLocation, message);
    return;
  }

  Lightning::CodeLocation dummyLocation;
  SendTranslationError(dummyLocation, message);
}

} // namespace Plasma
