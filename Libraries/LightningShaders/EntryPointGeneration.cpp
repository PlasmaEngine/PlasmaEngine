// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

InterfaceInfoGroup::FieldInfo* InterfaceInfoGroup::FindFieldInfo(const ShaderFieldKey& fieldKey)
{
  // @JoshD: Optimize later? This is typically small so it's easy enough to
  // ignore for now as a structural refactor is necessary to make this work with
  // an ordered hash-map.
  for (size_t i = 0; i < mFields.Size(); ++i)
  {
    if (mFields[i].mFieldMeta->MakeFieldKey() == fieldKey)
      return &mFields[i];
  }
  return nullptr;
}

InterfaceInfoGroup::FieldInfo* InterfaceInfoGroup::FindOrCreateFieldInfo(const ShaderFieldKey& fieldKey)
{
  FieldInfo* fieldInfo = FindFieldInfo(fieldKey);
  if (fieldInfo != nullptr)
    return fieldInfo;

  fieldInfo = &mFields.PushBack();
  return fieldInfo;
}

ShaderInterfaceField::ShaderInterfaceField()
{
  mFieldMeta = nullptr;
  mFieldType = nullptr;
}

ShaderInterfaceStruct::ShaderInterfaceStruct()
{
  mInstance = nullptr;
  mType = nullptr;
}

ShaderInterfaceKind::Enum ShaderInterfaceStruct::GetInterfaceKind()
{
  return ShaderInterfaceKind::Struct;
}

ShaderInterfaceKind::Enum ShaderInterfaceStruct::StaticGetInterfaceKind()
{
  return ShaderInterfaceKind::Struct;
}

size_t ShaderInterfaceStruct::GetFieldCount()
{
  return mFields.Size();
}

ShaderInterfaceField* ShaderInterfaceStruct::GetFieldAtIndex(size_t index)
{
  ReturnIf(index >= mFields.Size(), nullptr, "Invalid index");
  return &mFields[index];
}

ShaderInterfaceField* ShaderInterfaceStruct::GetField(const ShaderFieldKey& fieldKey)
{
  for (size_t i = 0; i < mFields.Size(); ++i)
  {
    ShaderInterfaceField& fieldInfo = mFields[i];
    if (fieldInfo.mFieldMeta->MakeFieldKey() == fieldKey)
      return &fieldInfo;
  }
  return nullptr;
}

LightningShaderIROp* ShaderInterfaceStruct::GetFieldPointerByIndex(size_t index,
                                                               EntryPointGeneration* entryPointGeneration,
                                                               BasicBlock* block,
                                                               spv::StorageClass storageClass)
{
  return GetFieldPointerByIndex(mInstance, index, entryPointGeneration, block, storageClass);
}

LightningShaderIROp* ShaderInterfaceStruct::GetFieldPointerByIndex(LightningShaderIROp* instance,
                                                               size_t index,
                                                               EntryPointGeneration* entryPointGeneration,
                                                               BasicBlock* block,
                                                               spv::StorageClass storageClass)
{
  u32 sourceOffset = static_cast<u32>(index);
  return entryPointGeneration->GetMemberInstanceFrom(block, instance, sourceOffset, storageClass);
}

void ShaderInterfaceStruct::DeclareInterfaceType(EntryPointGeneration* entryPointGeneration,
                                                 InterfaceInfoGroup& interfaceGroup,
                                                 EntryPointInfo* entryPointInfo)
{
  InterfaceInfoGroup::FieldList& fieldList = interfaceGroup.mFields;
  if (fieldList.Size() == 0)
    return;

  LightningSpirVFrontEnd* translator = entryPointGeneration->mTranslator;
  LightningSpirVFrontEndContext* context = entryPointGeneration->mContext;
  LightningShaderIRType* currentType = context->mCurrentType;

  // Create the interface struct's type
  String fullBlockName = BuildString(currentType->mName, interfaceGroup.mName);
  String blockTypeName = BuildString(fullBlockName, "Block");
  LightningShaderIRType* blockType =
      translator->MakeStructType(translator->mLibrary, blockTypeName, nullptr, interfaceGroup.mStorageClass);
  translator->MakeShaderTypeMeta(blockType, nullptr);
  blockType->mDebugResultName = fullBlockName;

  for(u32 i = 0; i < static_cast<u32>(fieldList.Size()); ++i)
  {
    // Create the shader interface field from the interface group field
    ShaderInterfaceField& interfaceField = mFields.PushBack();
    entryPointGeneration->CreateShaderInterfaceField(interfaceField, interfaceGroup, i);

    // Create reflection data for this field
    ShaderResourceReflectionData& memberReflection = fieldList[i].mReflectionData;
    memberReflection.mInstanceName = interfaceField.mFieldName;
    memberReflection.mTypeName = interfaceField.mFieldType->mLightningType->ToString();

    // Find the field type we're reading/writing to (storage type function)
    LightningShaderIRType* fieldShaderType = interfaceField.mFieldType->mDereferenceType;
    blockType->AddMember(fieldShaderType, interfaceField.mFieldName);
  }

  mType = blockType;
}

void ShaderInterfaceStruct::DecorateInterfaceType(EntryPointGeneration* entryPointGeneration,
                                                  InterfaceInfoGroup& interfaceGroup,
                                                  EntryPointInfo* entryPointInfo)
{
  if (interfaceGroup.mFields.Size() == 0)
    return;

  LightningSpirVFrontEnd* translator = entryPointGeneration->mTranslator;
  BasicBlock* decorationBlock = &entryPointInfo->mDecorations;
  InterfaceInfoGroup::FieldList& fieldList = interfaceGroup.mFields;

  // Write decoration on the block type
  entryPointGeneration->WriteTypeDecorations(interfaceGroup.mTypeDecorations, decorationBlock, mType);

  // Add all decorations for the member's types
  for (size_t i = 0; i < fieldList.Size(); ++i)
  {
    ShaderInterfaceField& interfaceField = mFields[i];
    InterfaceInfoGroup::FieldInfo& fieldInfo = fieldList[i];
    LightningShaderIRType* memberInterfaceType = interfaceField.mFieldType;

    // Write any decorations that belong to this member type (e.g. fixed arrays
    // need custom decorations on the type itself and we only create the type
    // here).
    entryPointGeneration->WriteTypeDecorations(
        fieldInfo.mTypeDecorations, decorationBlock, memberInterfaceType->mDereferenceType);
  }

  // Write decorations for each parameter in the block (not on the type, but the
  // member itself)
  for (size_t i = 0; i < fieldList.Size(); ++i)
  {
    u32 u32literal = static_cast<u32>(i);
    InterfaceInfoGroup::FieldInfo& fieldInfo = fieldList[i];
    LightningShaderIRConstantLiteral* memberIndexLiteral = translator->GetOrCreateConstantIntegerLiteral(u32literal);

    entryPointGeneration->WriteMemberDecorations(fieldInfo.mDecorations, decorationBlock, mType, memberIndexLiteral);
  }
}

void ShaderInterfaceStruct::DefineInterfaceType(EntryPointGeneration* entryPointGeneration,
                                                InterfaceInfoGroup& interfaceGroup,
                                                EntryPointInfo* entryPointInfo)
{
  LightningSpirVFrontEnd* translator = entryPointGeneration->mTranslator;
  LightningSpirVFrontEndContext* context = entryPointGeneration->mContext;
  BasicBlock* decorationBlock = &entryPointInfo->mDecorations;

  // Create the variable instance
  LightningShaderIRType* interfaceType = mType;
  LightningShaderIROp* interfaceInstance = translator->BuildOpVariable(
      &entryPointInfo->mVariables, interfaceType->mPointerType, interfaceGroup.mStorageClass, context);
  interfaceInstance->mDebugResultName = interfaceGroup.mName;
  mInstance = interfaceInstance;

  // Write decoration on the block instance
  entryPointGeneration->WriteTypeDecorations(interfaceGroup.mInstanceDecorations, decorationBlock, interfaceInstance);

  // If this is an input/output (e.g. not uniform) then add it to the entry
  // point's interface
  if (interfaceGroup.mStorageClass == spv::StorageClassInput || interfaceGroup.mStorageClass == spv::StorageClassOutput)
    entryPointInfo->mInterface.PushBack(interfaceInstance);
}

void ShaderInterfaceStruct::CopyInterfaceType(EntryPointGeneration* entryPointGeneration,
                                              InterfaceInfoGroup& interfaceGroup,
                                              EntryPointInfo* entryPointInfo,
                                              EntryPointHelperFunctionData& copyHelperData)
{
  LightningSpirVFrontEnd* translator = entryPointGeneration->mTranslator;
  LightningSpirVFrontEndContext* context = entryPointGeneration->mContext;
  spv::StorageClass storageClass = interfaceGroup.mStorageClass;

  LightningShaderIRType* currentType = context->mCurrentType;
  LightningShaderIROp* blockVarOp = mInstance;
  LightningShaderIRType* blockType = mInstance->mResultType->mDereferenceType;
  BasicBlock* targetFnBlock = copyHelperData.mBlock;

  // Iterate over all of the fields to copy them to/from the interface variables
  for (size_t i = 0; i < mFields.Size(); ++i)
  {
    // Find the type of the member
    ShaderInterfaceField& fieldInfo = mFields[i];

    // Copy this field to/from all of the different fields on the entry point's
    // owning type that matched to this interface field.
    for (size_t j = 0; j < fieldInfo.mLinkedFields.Size(); ++j)
    {
      ShaderIRFieldMeta* linkedFieldMeta = fieldInfo.mLinkedFields[j];
      String linkedFieldName = linkedFieldMeta->mLightningName;
      // Validate that this member exists on the type we're copying to/from
      if (!currentType->mMemberNamesToIndex.ContainsKey(linkedFieldName))
        continue;

      u32 blockMemberIndex = fieldInfo.mFieldIndex;
      LightningShaderIRType* blockFieldMemberType = fieldInfo.mFieldType;
      u32 memberIndex = currentType->FindMemberIndex(linkedFieldName);
      LightningShaderIRType* memberType = currentType->GetSubType(memberIndex);
      LightningShaderIRType* memberPtrType = memberType->mPointerType;

      // Get a pointer to the field on the block
      LightningShaderIROp* blockIndexConstant = translator->GetIntegerConstant(blockMemberIndex, context);
      LightningShaderIROp* blockAccessChainOp = translator->BuildIROp(
          targetFnBlock, OpType::OpAccessChain, blockFieldMemberType, blockVarOp, blockIndexConstant, context);

      // Get a pointer to the field on 'this'
      LightningShaderIROp* selfIndexConstant = translator->GetIntegerConstant(memberIndex, context);
      LightningShaderIROp* selfAccessChainOp = translator->BuildIROp(
          targetFnBlock, OpType::OpAccessChain, memberPtrType, copyHelperData.mSelfParam, selfIndexConstant, context);

      // Copy the data between the block/self depending on the storage type of
      // this class (write to outputs, otherwise read)
      LightningShaderIROp* source = blockAccessChainOp;
      LightningShaderIROp* dest = selfAccessChainOp;
      if (storageClass == spv::StorageClassOutput)
        Math::Swap(dest, source);

      entryPointGeneration->CopyField(targetFnBlock, memberType, source, dest);
    }
  }
}

ShaderInterfaceGlobals::ShaderInterfaceGlobals()
{
}

ShaderInterfaceKind::Enum ShaderInterfaceGlobals::GetInterfaceKind()
{
  return ShaderInterfaceKind::Globals;
}

ShaderInterfaceKind::Enum ShaderInterfaceGlobals::StaticGetInterfaceKind()
{
  return ShaderInterfaceKind::Globals;
}

size_t ShaderInterfaceGlobals::GetFieldCount()
{
  return mFields.Size();
}

ShaderInterfaceField* ShaderInterfaceGlobals::GetFieldAtIndex(size_t index)
{
  ReturnIf(index >= mFields.Size(), nullptr, "Invalid index");
  return &mFields[index];
}

ShaderInterfaceField* ShaderInterfaceGlobals::GetField(const ShaderFieldKey& fieldKey)
{
  for (size_t i = 0; i < mFields.Size(); ++i)
  {
    ShaderInterfaceField& fieldInfo = mFields[i];
    if (fieldInfo.mFieldMeta->MakeFieldKey() == fieldKey)
      return &fieldInfo;
  }
  return nullptr;
}

LightningShaderIROp* ShaderInterfaceGlobals::GetFieldPointerByIndex(size_t index,
                                                                EntryPointGeneration* entryPointGeneration,
                                                                BasicBlock* block,
                                                                spv::StorageClass storageClass)
{
  return mFieldInstances[index];
}

void ShaderInterfaceGlobals::DeclareInterfaceType(EntryPointGeneration* entryPointGeneration,
                                                  InterfaceInfoGroup& interfaceGroup,
                                                  EntryPointInfo* entryPointInfo)
{
  // Declare interface variables but don't group them together into a struct
  // (e.g. vertex inputs)
  if (interfaceGroup.mFields.Size() == 0)
    return;

  LightningSpirVFrontEnd* translator = entryPointGeneration->mTranslator;

  for (size_t i = 0; i < interfaceGroup.mFields.Size(); ++i)
  {
    // Create the shader interface field from the interface group field
    ShaderInterfaceField& interfaceField = mFields.PushBack();
    entryPointGeneration->CreateShaderInterfaceField(interfaceField, interfaceGroup, i);

    // Create reflection data for this field
    ShaderResourceReflectionData& memberReflection = interfaceGroup.mFields[i].mReflectionData;
    memberReflection.mInstanceName = interfaceField.mFieldName;
    memberReflection.mTypeName = interfaceField.mFieldType->mLightningType->ToString();
  }
}

void ShaderInterfaceGlobals::DecorateInterfaceType(EntryPointGeneration* entryPointGeneration,
                                                   InterfaceInfoGroup& interfaceGroup,
                                                   EntryPointInfo* entryPointInfo)
{
  if (interfaceGroup.mFields.Size() == 0)
    return;

  BasicBlock* decorationBlock = &entryPointInfo->mDecorations;
  InterfaceInfoGroup::FieldList fieldList = interfaceGroup.mFields;

  for (size_t i = 0; i < fieldList.Size(); ++i)
  {
    ShaderInterfaceField& interfaceField = mFields[i];

    LightningShaderIRType* interfaceFieldMemberType = interfaceField.mFieldType;
    // Write any decorations that belong to this member type (e.g. fixed arrays
    // need custom decorations on the type itself and we only create the type
    // here).
    entryPointGeneration->WriteTypeDecorations(
        fieldList[i].mTypeDecorations, decorationBlock, interfaceFieldMemberType->mDereferenceType);
  }
}

void ShaderInterfaceGlobals::DefineInterfaceType(EntryPointGeneration* entryPointGeneration,
                                                 InterfaceInfoGroup& interfaceGroup,
                                                 EntryPointInfo* entryPointInfo)
{
  // Declare interface variables but don't group them together into a struct
  // (e.g. vertex inputs)
  if (interfaceGroup.mFields.Size() == 0)
    return;

  LightningSpirVFrontEnd* translator = entryPointGeneration->mTranslator;
  LightningSpirVFrontEndContext* context = entryPointGeneration->mContext;
  BasicBlock* decorationBlock = &entryPointInfo->mDecorations;
  InterfaceInfoGroup::FieldList fieldList = interfaceGroup.mFields;

  for (size_t i = 0; i < fieldList.Size(); ++i)
  {
    InterfaceInfoGroup::FieldInfo& fieldInfo = fieldList[i];
    ShaderInterfaceField& interfaceField = mFields[i];

    // Find/Create the field's member type (the storage class is part of the
    // type). Even though we don't use this type this is needed to make sure the
    // interface type (with storage class) is created.
    LightningShaderIRType* interfaceFieldMemberType = interfaceField.mFieldType;

    // Create the variable instance in the entry point
    LightningShaderIROp* varOp = translator->BuildOpVariable(
        &entryPointInfo->mVariables, interfaceFieldMemberType, interfaceGroup.mStorageClass, context);
    varOp->mDebugResultName = interfaceField.mFieldName;
    mFieldInstances.PushBack(varOp);

    // Write any decorations on the type instance
    entryPointGeneration->WriteTypeDecorations(fieldInfo.mDecorations, decorationBlock, varOp);

    // Add this variable to the entry point's interface
    // (required to declare OpEntryPoint) if it's an input or output
    if (interfaceGroup.mStorageClass == spv::StorageClassInput ||
        interfaceGroup.mStorageClass == spv::StorageClassOutput)
      entryPointInfo->mInterface.PushBack(varOp);
  }
}

void ShaderInterfaceGlobals::CopyInterfaceType(EntryPointGeneration* entryPointGeneration,
                                               InterfaceInfoGroup& interfaceGroup,
                                               EntryPointInfo* entryPointInfo,
                                               EntryPointHelperFunctionData& copyHelperData)
{
  EntryPointHelperFunctionData functionData = copyHelperData;
  spv::StorageClass storageClass = interfaceGroup.mStorageClass;
  LightningSpirVFrontEnd* translator = entryPointGeneration->mTranslator;
  LightningSpirVFrontEndContext* context = entryPointGeneration->mContext;

  LightningShaderIRType* currentType = context->mCurrentType;
  BasicBlock* targetFnBlock = functionData.mBlock;

  // Iterate over all of the fields to copy them to/from the interface variables
  for (size_t i = 0; i < mFields.Size(); ++i)
  {
    // Find the type of the member
    ShaderInterfaceField& fieldInfo = mFields[i];

    // Copy this field to/from all of the different fields on the entry point's
    // owning type that matched to this interface field.
    for (size_t j = 0; j < fieldInfo.mLinkedFields.Size(); ++j)
    {
      ShaderIRFieldMeta* linkedFieldMeta = fieldInfo.mLinkedFields[j];
      String linkedFieldName = linkedFieldMeta->mLightningName;

      // Validate that this member exists in the fields we're copying to/from
      if (!currentType->mMemberNamesToIndex.ContainsKey(linkedFieldName))
        continue;

      int memberIndex = currentType->FindMemberIndex(linkedFieldName);
      LightningShaderIRType* memberType = currentType->GetSubType(memberIndex);
      LightningShaderIRType* memberPtrType = memberType->mPointerType;

      // Get a pointer to the field on 'this'
      LightningShaderIROp* selfIndexConstant = translator->GetIntegerConstant(memberIndex, context);
      LightningShaderIROp* selfAccessChainOp = translator->BuildIROp(
          targetFnBlock, OpType::OpAccessChain, memberPtrType, functionData.mSelfParam, selfIndexConstant, context);

      // Copy the data between the interface var/self depending on the storage
      // type of this class (write to outputs, otherwise read)
      LightningShaderIROp* source = mFieldInstances[i];
      LightningShaderIROp* dest = selfAccessChainOp;
      if (storageClass == spv::StorageClassOutput)
        Math::Swap(dest, source);

      entryPointGeneration->CopyField(targetFnBlock, memberType, source, dest);
    }
  }
}

ShaderInterfaceStructArray::ShaderInterfaceStructArray()
{
  mType = nullptr;
  mInstance = nullptr;
  mStructType = nullptr;
}

ShaderInterfaceKind::Enum ShaderInterfaceStructArray::GetInterfaceKind()
{
  return ShaderInterfaceKind::Array;
}

ShaderInterfaceKind::Enum ShaderInterfaceStructArray::StaticGetInterfaceKind()
{
  return ShaderInterfaceKind::Array;
}

bool ShaderInterfaceStructArray::ContainsField(const ShaderFieldKey& fieldKey)
{
  return mStructType->GetField(fieldKey) != nullptr;
}

LightningShaderIROp* ShaderInterfaceStructArray::GetPointerByIndex(ILightningShaderIR* index,
                                                               ShaderFieldKey& fieldKey,
                                                               EntryPointGeneration* entryPointGeneration,
                                                               BasicBlock* block,
                                                               spv::StorageClass storageClass)
{
  LightningShaderIRType* shaderSubType = mStructType->mType;
  LightningShaderIROp* itemInstance = entryPointGeneration->mTranslator->BuildIROp(
      block, OpType::OpAccessChain, shaderSubType->mPointerType, mInstance, index, entryPointGeneration->mContext);

  // Find the field
  ShaderInterfaceField* fieldInfo = mStructType->GetField(fieldKey);
  if (fieldInfo != nullptr)
    return entryPointGeneration->GetMemberInstanceFrom(block, itemInstance, fieldInfo->mFieldIndex, storageClass);

  Error("Failed to find field");
  return nullptr;
}

GeometryStageInfo::GeometryStageInfo(LightningSpirVFrontEnd* translator,
                                     LightningShaderIRType* shaderType,
                                     Lightning::GenericFunctionNode* node)
{
  mShaderType = shaderType;
  mInputStreamType = translator->FindType(node->Parameters[0]);
  mOutputStreamType = translator->FindType(node->Parameters[1]);
  mInputVertexType = mInputStreamType->mParameters[0]->As<LightningShaderIRType>();
  mOutputVertexType = mOutputStreamType->mParameters[0]->As<LightningShaderIRType>();
  mInputStreamUserData = mInputStreamType->mLightningType->Has<Lightning::GeometryStreamUserData>();
  mOutputStreamUserData = mOutputStreamType->mLightningType->Has<Lightning::GeometryStreamUserData>();
}

EntryPointGeneration::EntryPointGeneration()
{
  mTranslator = nullptr;
  mContext = nullptr;
}

EntryPointGeneration::~EntryPointGeneration()
{
  Clear();
}

void EntryPointGeneration::DeclareVertexInterface(LightningSpirVFrontEnd* translator,
                                                  Lightning::GenericFunctionNode* node,
                                                  LightningShaderIRFunction* function,
                                                  LightningSpirVFrontEndContext* context)
{
  Clear();

  mTranslator = translator;
  mContext = context;

  LightningShaderIRType* currentType = context->mCurrentType;

  // Generate the basic entry point (a copy inputs + copy outputs
  // function that is called by the actual main function)
  EntryPointHelperFunctionData copyInputsData = GenerateCopyHelper(function, "CopyInputs");
  EntryPointHelperFunctionData copyOutputsData = GenerateCopyHelper(function, "CopyOutputs");
  BuildBasicEntryPoint(node, function, copyInputsData.mFunction, copyOutputsData.mFunction);
  EntryPointInfo* entryPointInfo = currentType->mEntryPoint;

  // Collect all interface parameters for this shader (inputs, outputs,
  // uniforms, etc...)
  ShaderInterfaceInfo interfaceInfo;
  CollectInterfaceVariables(function, interfaceInfo, ShaderStage::Vertex);

  // Mark the inputs of pixel shaders as a non-struct non-block set of inputs.
  interfaceInfo.mInputs.mIsStruct = false;
  interfaceInfo.mInputs.mStorageClass = spv::StorageClassInput;
  // Vertex inputs also must have locations specified
  AddVertexLocationDecorations(interfaceInfo.mInputs);
  // The outputs of a vertex fragment have to be a block struct.
  interfaceInfo.mOutputs.mIsStruct = true;
  interfaceInfo.mOutputs.mStorageClass = spv::StorageClassOutput;
  interfaceInfo.mOutputs.mName = "Out";
  // This also requires the location decoration so subsequent stages can match
  // to this output
  interfaceInfo.mOutputs.mTypeDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBlock));
  interfaceInfo.mOutputs.mInstanceDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationLocation, 0));
  // Also decorate uniforms
  DecorateUniformGroups(interfaceInfo);

  // Now we can generically write out all stage input/output/uniform/built-in
  // groups
  DeclareStageBlocks(interfaceInfo, entryPointInfo, copyInputsData, copyOutputsData);

  // Make sure to add the terminator op for both functions
  copyInputsData.mBlock->mTerminatorOp =
      translator->BuildIROp(copyInputsData.mBlock, OpType::OpReturn, nullptr, context);
  copyOutputsData.mBlock->mTerminatorOp =
      translator->BuildIROp(copyOutputsData.mBlock, OpType::OpReturn, nullptr, context);

  FindAndDecorateGlobals(currentType, entryPointInfo);
  CopyReflectionDataToEntryPoint(entryPointInfo, interfaceInfo);
  entryPointInfo->mFragmentType = FragmentType::Vertex;
}

void EntryPointGeneration::DeclareGeometryInterface(LightningSpirVFrontEnd* translator,
                                                    Lightning::GenericFunctionNode* node,
                                                    LightningShaderIRFunction* function,
                                                    LightningSpirVFrontEndContext* context)
{
  Clear();

  mTranslator = translator;
  mContext = context;

  LightningShaderIRType* currentType = context->mCurrentType;
  LightningShaderIRType* voidType = translator->FindType(LightningTypeId(void), node);
  GeometryStageInfo stageInfo(translator, currentType, node);

  LightningShaderIROp* copyInputsStreamVar;
  EntryPointHelperFunctionData copyInputsData =
      GenerateGeometryCopyHelper(function, "CopyInputs", stageInfo.mInputStreamType, copyInputsStreamVar);

  LightningShaderIROp* selfVar;
  BasicBlock* currentBlock;
  CreateEntryPointFunction(node, function, selfVar, currentBlock);
  EntryPointInfo* entryPointInfo = currentType->mEntryPoint;

  // Write out the in/out/uniform variables as well as the input/output helper
  // functions (no output yet)
  WriteGeometryStageInterface(function, stageInfo, entryPointInfo, copyInputsData, copyInputsStreamVar);

  // Write out the true entry point
  {
    context->mCurrentBlock = currentBlock;

    // Create the input/output stream variables
    LightningShaderIROp* inputVar = translator->BuildOpVariable(stageInfo.mInputStreamType->mPointerType, context);
    inputVar->mDebugResultName = "inputStream";
    LightningShaderIROp* outputVar = translator->BuildOpVariable(stageInfo.mOutputStreamType->mPointerType, context);
    outputVar->mDebugResultName = "outputStream";

    // Construct the user defined entry point's arguments
    Array<ILightningShaderIR*> entryPointArguments;
    entryPointArguments.PushBack(selfVar);
    entryPointArguments.PushBack(inputVar);
    entryPointArguments.PushBack(outputVar);

    // Create the function call to the entry point
    translator->BuildIROp(
        currentBlock, OpType::OpFunctionCall, voidType, copyInputsData.mFunction, selfVar, inputVar, context);
    LightningShaderIROp* fnCallOp = translator->BuildIROpNoBlockAdd(OpType::OpFunctionCall, voidType, context);
    fnCallOp->mArguments.PushBack(function);
    translator->WriteFunctionCallArguments(entryPointArguments, voidType, function->mFunctionType, fnCallOp, context);
    currentBlock->AddOp(fnCallOp);

    // Add the termination statement
    translator->BuildIROp(currentBlock, OpType::OpReturn, nullptr, context);
  }

  // Add geometry capabilities to this entry point
  entryPointInfo->mCapabilities.PushBack(spv::CapabilityGeometry);

  // Add geometry shader stage specific execution mode values
  BasicBlock* executionModes = &entryPointInfo->mExecutionModes;
  LightningShaderIRFunction* entryPointFn = entryPointInfo->mEntryPointFn;

  SpirVNameSettings& nameSettings = translator->mSettings->mNameSettings;
  // Write out the max vertices value
  ShaderIRAttribute* geometryAttribute = currentType->FindFirstAttribute(nameSettings.mGeometryAttribute);
  ShaderIRAttributeParameter* maxVerticesParam = geometryAttribute->FindFirstParameter(nameSettings.mMaxVerticesParam);
  int maxVertices = maxVerticesParam->GetIntValue();
  LightningShaderIRConstantLiteral* modeLiteral =
      translator->GetOrCreateConstantIntegerLiteral(spv::ExecutionModeOutputVertices);
  LightningShaderIRConstantLiteral* vertexCountLiteral = translator->GetOrCreateConstantIntegerLiteral(maxVertices);
  translator->BuildIROp(
      executionModes, OpType::OpExecutionMode, nullptr, entryPointFn, modeLiteral, vertexCountLiteral, context);

  // Write out the input stream execution type (e.g. Points/Lines)
  modeLiteral = translator->GetOrCreateConstantIntegerLiteral(stageInfo.mInputStreamUserData->mExecutionMode);
  translator->BuildIROp(executionModes, OpType::OpExecutionMode, nullptr, entryPointFn, modeLiteral, context);

  // Write out the output stream execution type (e.g. Points/Lines)
  modeLiteral = translator->GetOrCreateConstantIntegerLiteral(stageInfo.mOutputStreamUserData->mExecutionMode);
  translator->BuildIROp(executionModes, OpType::OpExecutionMode, nullptr, entryPointFn, modeLiteral, context);
  entryPointInfo->mFragmentType = FragmentType::Geometry;
}

void EntryPointGeneration::DeclarePixelInterface(LightningSpirVFrontEnd* translator,
                                                 Lightning::GenericFunctionNode* node,
                                                 LightningShaderIRFunction* function,
                                                 LightningSpirVFrontEndContext* context)
{
  Clear();

  mTranslator = translator;
  mContext = context;

  LightningShaderIRType* currentType = context->mCurrentType;

  // Generate the basic entry point (a copy inputs + copy outputs
  // function that is called by the actual main function)
  EntryPointHelperFunctionData copyInputsData = GenerateCopyHelper(function, "CopyInputs");
  EntryPointHelperFunctionData copyOutputsData = GenerateCopyHelper(function, "CopyOutputs");

  BuildBasicEntryPoint(node, function, copyInputsData.mFunction, copyOutputsData.mFunction);
  EntryPointInfo* entryPointInfo = currentType->mEntryPoint;

  // Collect all interface parameters for this shader (inputs, outputs,
  // uniforms, etc...)
  ShaderInterfaceInfo interfaceInfo;
  CollectInterfaceVariables(function, interfaceInfo, ShaderStage::Pixel);

  // Pixel inputs have to be a block struct
  interfaceInfo.mInputs.mIsStruct = true;
  interfaceInfo.mInputs.mStorageClass = spv::StorageClassInput;
  interfaceInfo.mInputs.mName = "In";
  interfaceInfo.mInputs.mTypeDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBlock));
  interfaceInfo.mInputs.mInstanceDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationLocation, 0));
  // Pixel outputs can be a struct but cannot be a block
  interfaceInfo.mOutputs.mIsStruct = false;
  interfaceInfo.mOutputs.mStorageClass = spv::StorageClassOutput;
  interfaceInfo.mOutputs.mName = "Out";
  // Add location decorations to all of the outputs, properly handling the render target name locations.
  AddPixelLocationDecorations(interfaceInfo.mOutputs);
  // Also decorate uniforms
  DecorateUniformGroups(interfaceInfo);
  AddFlatDecorations(interfaceInfo.mInputs);

  // Now we can generically write out all stage input/output/uniform/built-in
  // groups
  DeclareStageBlocks(interfaceInfo, entryPointInfo, copyInputsData, copyOutputsData);

  // Make sure to add the terminator op for both functions
  copyInputsData.mBlock->mTerminatorOp =
      translator->BuildIROp(copyInputsData.mBlock, OpType::OpReturn, nullptr, context);
  copyOutputsData.mBlock->mTerminatorOp =
      translator->BuildIROp(copyOutputsData.mBlock, OpType::OpReturn, nullptr, context);

  FindAndDecorateGlobals(currentType, entryPointInfo);
  CopyReflectionDataToEntryPoint(entryPointInfo, interfaceInfo);
  WriteExecutionModeOriginUpperLeft(entryPointInfo);
  entryPointInfo->mFragmentType = FragmentType::Pixel;
}

void EntryPointGeneration::DeclareComputeInterface(LightningSpirVFrontEnd* translator,
                                                   Lightning::GenericFunctionNode* node,
                                                   LightningShaderIRFunction* function,
                                                   LightningSpirVFrontEndContext* context)
{
  Clear();

  mTranslator = translator;
  mContext = context;

  LightningShaderIRType* currentType = context->mCurrentType;

  // Generate the basic entry point (a copy inputs + copy outputs
  // function that is called by the actual main function)
  EntryPointHelperFunctionData copyInputsData = GenerateCopyHelper(function, "CopyInputs");
  EntryPointHelperFunctionData copyOutputsData = GenerateCopyHelper(function, "CopyOutputs");

  BuildBasicEntryPoint(node, function, copyInputsData.mFunction, copyOutputsData.mFunction);
  EntryPointInfo* entryPointInfo = currentType->mEntryPoint;

  // Collect all interface parameters for this shader (inputs, outputs,
  // uniforms, etc...)
  ShaderInterfaceInfo interfaceInfo;
  CollectInterfaceVariables(function, interfaceInfo, ShaderStage::Compute);

  // Pixel inputs have to be a block struct
  interfaceInfo.mInputs.mIsStruct = true;
  interfaceInfo.mInputs.mStorageClass = spv::StorageClassInput;
  interfaceInfo.mInputs.mName = "In";
  interfaceInfo.mInputs.mTypeDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBlock));
  interfaceInfo.mInputs.mInstanceDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationLocation, 0));
  // Pixel outputs can be a struct but cannot be a block
  interfaceInfo.mOutputs.mIsStruct = true;
  interfaceInfo.mOutputs.mStorageClass = spv::StorageClassOutput;
  interfaceInfo.mOutputs.mName = "Out";
  // By decorating the struct with a location all of the members are
  // automatically assigned locations
  interfaceInfo.mOutputs.mInstanceDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationLocation, 0));
  // Also decorate uniforms
  DecorateUniformGroups(interfaceInfo);
  AddFlatDecorations(interfaceInfo.mInputs);

  // Now we can generically write out all stage input/output/uniform/built-in
  // groups
  // @JoshD: There's a chance that uniforms/interface blocks aren't supported
  // the same way in compute shaders.
  DeclareStageBlocks(interfaceInfo, entryPointInfo, copyInputsData, copyOutputsData);

  // Make sure to add the terminator op for both functions
  copyInputsData.mBlock->mTerminatorOp =
      translator->BuildIROp(copyInputsData.mBlock, OpType::OpReturn, nullptr, context);
  copyOutputsData.mBlock->mTerminatorOp =
      translator->BuildIROp(copyOutputsData.mBlock, OpType::OpReturn, nullptr, context);

  FindAndDecorateGlobals(currentType, entryPointInfo);
  CopyReflectionDataToEntryPoint(entryPointInfo, interfaceInfo);

  // Get the user data for the compute shader
  Lightning::ComputeFragmentUserData* computeUserData = currentType->mLightningType->Has<Lightning::ComputeFragmentUserData>();
  // Write out the local size execution mode
  BasicBlock* block = &entryPointInfo->mExecutionModes;
  LightningShaderIROp* executionModeOp =
      mTranslator->BuildIROp(block, OpType::OpExecutionMode, nullptr, entryPointInfo->mEntryPointFn, context);
  executionModeOp->mArguments.PushBack(translator->GetOrCreateConstantIntegerLiteral(spv::ExecutionModeLocalSize));
  executionModeOp->mArguments.PushBack(translator->GetOrCreateConstantIntegerLiteral(computeUserData->mLocalSizeX));
  executionModeOp->mArguments.PushBack(translator->GetOrCreateConstantIntegerLiteral(computeUserData->mLocalSizeY));
  executionModeOp->mArguments.PushBack(translator->GetOrCreateConstantIntegerLiteral(computeUserData->mLocalSizeZ));
  entryPointInfo->mFragmentType = FragmentType::Compute;
}

void EntryPointGeneration::Clear()
{
  DeleteObjectsIn(mUniforms);
  DeleteObjectsIn(mBuiltIns);
  DeleteObjectsIn(mInputs);
  DeleteObjectsIn(mOutputs);

  mUniqueTypes.Clear();
  mUniqueOps.Clear();
  mUsedBindingIds.Clear();
}

void EntryPointGeneration::CreateEntryPointFunction(Lightning::GenericFunctionNode* node,
                                                    LightningShaderIRFunction* function,
                                                    LightningShaderIROp*& selfVarOp,
                                                    BasicBlock*& entryPointBlock)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderIRType* currentType = context->mCurrentType;
  LightningShaderIRType* voidType = translator->FindType(LightningTypeId(void), node);

  // Generate the entry point info for this type
  EntryPointInfo* entryPoint = new EntryPointInfo();
  currentType->mEntryPoint = entryPoint;

  // Generate the entry point main function
  LightningShaderIRFunction* entryPointFn = currentType->CreateFunction(translator->mLibrary);
  context->mCurrentFunction = entryPointFn;
  entryPoint->mEntryPointFn = entryPointFn;
  // Make the entry point funciton name unique by including the name of the
  // current type.
  entryPointFn->mDebugResultName = BuildString("EntryPoint_", function->mName, "_", context->mCurrentType->mName);
  entryPointFn->mName = entryPointFn->mDebugResultName;
  Array<Lightning::Type*> signature;
  translator->GenerateFunctionType(node, entryPointFn, LightningTypeId(void), signature, context);

  // Write all shared ops of the entry point. No matter what, an entry point
  // will construct the self type.
  entryPointBlock = translator->BuildBlock(String(), context);

  // Create and call a function to initialize global variables
  LightningShaderIRFunction* globalsInitializerFn = CreateGlobalsInitializerFunction(node);
  translator->BuildIROp(entryPointBlock, OpType::OpFunctionCall, voidType, globalsInitializerFn, context);

  // Construct the self type
  selfVarOp = translator->BuildOpVariable(currentType->mPointerType, context);
  selfVarOp->mDebugResultName = "self";

  // Get the default constructor so we can initialize the entry point class.
  // This constructor could be auto-generated or manually created by the user so
  // check both.
  LightningShaderIRFunction* defaultConstructorFn = currentType->mAutoDefaultConstructor;
  if (defaultConstructorFn == nullptr)
    defaultConstructorFn = translator->mLibrary->FindFunction(currentType->mLightningType->GetDefaultConstructor());
  ErrorIf(defaultConstructorFn == nullptr,
          "Unable to find a default constructor on a entry point type. "
          "The user must've made a constructor that hid the default");

  // Invoke the default constructor
  LightningShaderIRType* defaultConstructorReturnType = defaultConstructorFn->GetReturnType();
  LightningShaderIROp* constructorOp = translator->BuildIROp(
      entryPointBlock, OpType::OpFunctionCall, defaultConstructorReturnType, defaultConstructorFn, selfVarOp, context);
}

void EntryPointGeneration::BuildBasicEntryPoint(Lightning::GenericFunctionNode* node,
                                                LightningShaderIRFunction* function,
                                                LightningShaderIRFunction* copyInputsFn,
                                                LightningShaderIRFunction* copyOutputsFn)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderIRType* voidType = translator->FindType(LightningTypeId(void), node);

  LightningShaderIROp* selfVarOp;
  BasicBlock* currentBlock;
  CreateEntryPointFunction(node, function, selfVarOp, currentBlock);

  // Call CopyInputs if it exists
  if (copyInputsFn != nullptr)
    translator->BuildIROp(currentBlock, OpType::OpFunctionCall, voidType, copyInputsFn, selfVarOp, context);
  // Call the defined entry point in lightning
  translator->BuildIROp(currentBlock, OpType::OpFunctionCall, voidType, function, selfVarOp, context);
  // Call CopyOutputs if it exists
  if (copyOutputsFn != nullptr)
    translator->BuildIROp(currentBlock, OpType::OpFunctionCall, voidType, copyOutputsFn, selfVarOp, context);
  // Add the required terminator op
  translator->BuildIROp(currentBlock, OpType::OpReturn, nullptr, context);
}

LightningShaderIRFunction* EntryPointGeneration::CreateGlobalsInitializerFunction(Lightning::GenericFunctionNode* node)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderIRType* currentType = context->mCurrentType;

  // Create a function to initialize the global variables for this entry point.
  LightningShaderIRFunction* initializeGlobalsFunction = currentType->CreateFunction(translator->mLibrary);
  initializeGlobalsFunction->mDebugResultName = initializeGlobalsFunction->mName = "InitializeGlobals";
  // Make the function type
  Array<Lightning::Type*> parameters;
  translator->GenerateFunctionType(node, initializeGlobalsFunction, LightningTypeId(void), parameters, context);

  // Create the initial block of this function
  BasicBlock* fnBlock = new BasicBlock();
  initializeGlobalsFunction->mBlocks.PushBack(fnBlock);
  translator->FixBlockTerminators(fnBlock, context);

  // Mark this on the entry point
  currentType->mEntryPoint->mGlobalsInitializerFunction = initializeGlobalsFunction;

  return initializeGlobalsFunction;
}

EntryPointHelperFunctionData EntryPointGeneration::GenerateCopyHelper(LightningShaderIRFunction* userMainFn,
                                                                      StringParam name)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  EntryPointHelperFunctionData result;

  LightningShaderIRType* currentType = context->mCurrentType;
  // Create the helper function and add it to the current type
  LightningShaderIRFunction* helperFn = currentType->CreateFunction(translator->mLibrary);
  context->mCurrentFunction = helperFn;
  helperFn->mFunctionType = userMainFn->mFunctionType;
  // Add a debug name
  helperFn->mDebugResultName = BuildString(name, "_", userMainFn->mName);

  // Create the self parameter
  LightningShaderIROp* selfParam = translator->BuildIROp(
      &helperFn->mParameterBlock, OpType::OpFunctionParameter, currentType->mPointerType, context);
  selfParam->mDebugResultName = "self";
  // Create the initial block to add all further instructions to
  BasicBlock* currentBlock = translator->BuildBlock(String(), context);

  // Fill out the results
  result.mFunction = helperFn;
  result.mSelfParam = selfParam;
  result.mBlock = currentBlock;

  return result;
}

EntryPointHelperFunctionData EntryPointGeneration::GenerateGeometryCopyHelper(LightningShaderIRFunction* userMainFn,
                                                                              StringParam name,
                                                                              LightningShaderIRType* inputStreamType,
                                                                              LightningShaderIROp*& inputStreamVar)
{
  // Geometry shader's copy helper is different than everything else because it
  // has to take the stream type

  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderIRType* currentType = context->mCurrentType;

  // Create the helper function and add it to the current type
  LightningShaderIRFunction* helperFn = currentType->CreateFunction(translator->mLibrary);
  context->mCurrentFunction = helperFn;
  Array<Lightning::Type*> signature;
  signature.PushBack(currentType->mLightningType->IndirectType);
  signature.PushBack(inputStreamType->mLightningType->IndirectType);
  translator->GenerateFunctionType(nullptr, helperFn, LightningTypeId(void), signature, context);
  // Add a debug name
  helperFn->mDebugResultName = BuildString(name, "_", userMainFn->mName);
  helperFn->mName = helperFn->mDebugResultName;

  // Create the function parameters
  BasicBlock* parameterBlock = &helperFn->mParameterBlock;
  // Create the self parameter
  LightningShaderIROp* selfParam = translator->BuildIROp(
      &helperFn->mParameterBlock, OpType::OpFunctionParameter, currentType->mPointerType, context);
  selfParam->mDebugResultName = "self";
  // Create the stream parameter
  inputStreamVar =
      translator->BuildIROp(parameterBlock, OpType::OpFunctionParameter, inputStreamType->mPointerType, context);
  inputStreamVar->mDebugResultName = "inputStream";

  // Create the initial block to add all further instructions to
  BasicBlock* currentBlock = translator->BuildBlock(String(), context);

  // Fill out the results
  EntryPointHelperFunctionData result;
  result.mFunction = helperFn;
  result.mSelfParam = selfParam;
  result.mBlock = currentBlock;

  return result;
}

void EntryPointGeneration::WriteGeometryStageInterface(LightningShaderIRFunction* function,
                                                       GeometryStageInfo& stageInfo,
                                                       EntryPointInfo* entryPointInfo,
                                                       EntryPointHelperFunctionData& copyInputsData,
                                                       LightningShaderIROp* copyInputsStreamVar)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  BasicBlock* copyInputsBlock = copyInputsData.mBlock;

  // Find all stream types used by this entry point
  CollectGeometryStreamTypes(function, stageInfo);

  // Collect input/output variables on the stream types (the per vertex data)
  ShaderInterfaceInfo vertexInputInterfaceInfo, vertexOutputInterfaceInfo;
  CollectInputInterfaceVariables(function, vertexInputInterfaceInfo, stageInfo.mInputVertexType, ShaderStage::Geometry);
  CollectOutputInterfaceVariables(
      function, vertexOutputInterfaceInfo, stageInfo.mOutputVertexType, ShaderStage::Geometry);

  // Collect builtIn-inputs/uniforms on the composite itself (per instance run).
  // There are no outputs available here.
  ShaderInterfaceInfo compositeInterfaceInfo;
  CollectInputInterfaceVariables(function, compositeInterfaceInfo, stageInfo.mShaderType, ShaderStage::Geometry);
  CollectUniformInterfaceVariables(function, compositeInterfaceInfo, stageInfo.mShaderType, ShaderStage::Geometry);
  // Also decorate uniforms
  DecorateUniformGroups(compositeInterfaceInfo);

  // Write out all of the composite in/out parameters
  DeclareStageBlocks(compositeInterfaceInfo, entryPointInfo, copyInputsData, copyInputsData);

  // Write the inputs/outputs for the vertex type
  Array<ShaderInterfaceType*> inputStreamInterfaceTypes, inputVertexInterfaceTypes;
  DeclareGeometryVertexInputs(stageInfo,
                              entryPointInfo,
                              vertexInputInterfaceInfo,
                              copyInputsData,
                              copyInputsStreamVar,
                              inputStreamInterfaceTypes,
                              inputVertexInterfaceTypes);
  DeclareGeometryVertexOutputs(stageInfo, entryPointInfo, vertexOutputInterfaceInfo, inputStreamInterfaceTypes);

  // Write the copy inputs terminator
  copyInputsBlock->mTerminatorOp = translator->BuildIROp(copyInputsBlock, OpType::OpReturn, nullptr, context);

  DeleteObjectsInContainer(inputStreamInterfaceTypes);
  DeleteObjectsInContainer(inputVertexInterfaceTypes);

  FindAndDecorateGlobals(context->mCurrentType, entryPointInfo);
  CopyReflectionDataToEntryPoint(entryPointInfo, compositeInterfaceInfo);
}

void EntryPointGeneration::CollectGeometryStreamTypes(LightningShaderIRFunction* function, GeometryStageInfo& stageInfo)
{
  // @JoshD: This is a bit of an odd problem as the signature of a geometry
  // shader only specifies what stream it thinks it'll use, but when composited
  // the shader will use a different stream than the fragment. The only way to
  // know what streams are currently used are to iterate over all instructions
  // in the entire dependency chain and find out all stream types. As an
  // approximation for now, iterate over all locally declared variables in the
  // entry point function. This will at least get the fragment's stream.

  HashSet<LightningShaderIRType*> outputStreamTypes;
  // Always start with the entry point's stream type
  outputStreamTypes.Insert(stageInfo.mOutputStreamType);
  stageInfo.mOutputStreamTypes.PushBack(stageInfo.mOutputStreamType);
  // Iterate over all block local variables to try and find any streams
  for (size_t i = 0; i < function->mBlocks.Size(); ++i)
  {
    BasicBlock* block = function->mBlocks[i];
    for (size_t j = 0; j < block->mLocalVariables.Size(); ++j)
    {
      LightningShaderIROp* op = block->mLocalVariables[j]->As<LightningShaderIROp>();
      if (op == nullptr)
        continue;

      LightningShaderIRType* streamPointerType = op->mResultType;
      Lightning::GeometryStreamUserData* streamUserData =
          streamPointerType->mLightningType->Has<Lightning::GeometryStreamUserData>();
      if (streamUserData == nullptr || streamUserData->mInput == true)
        continue;

      LightningShaderIRType* streamValueType = streamPointerType->mDereferenceType;
      if (!outputStreamTypes.Contains(streamValueType))
      {
        outputStreamTypes.Insert(streamValueType);
        stageInfo.mOutputStreamTypes.PushBack(streamValueType);
      }
    }
  }
}

void EntryPointGeneration::DeclareGeometryVertexInputs(GeometryStageInfo& stageInfo,
                                                       EntryPointInfo* entryPointInfo,
                                                       ShaderInterfaceInfo& vertexInputInterfaceInfo,
                                                       EntryPointHelperFunctionData& copyInputsData,
                                                       LightningShaderIROp* copyInputsStreamVar,
                                                       Array<ShaderInterfaceType*>& inputStreamInterfaceTypes,
                                                       Array<ShaderInterfaceType*>& inputVertexInterfaceTypes)
{
  BasicBlock* copyInputsBlock = copyInputsData.mBlock;
  String baseShaderName = stageInfo.mShaderType->mName;

  // Declare the input vertex stream type and sub-type (no built-ins)
  {
    InterfaceInfoGroup& interfaceGroup = vertexInputInterfaceInfo.mInputs;
    interfaceGroup.mStorageClass = spv::StorageClassInput;

    // Add decorations
    interfaceGroup.mTypeDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBlock));
    interfaceGroup.mInstanceDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationLocation, 0));

    GeometryInOutTypeInfo geometryInfo;
    geometryInfo.mArraySize = stageInfo.mInputStreamType->mParameters[1];
    // Name has to be mangled if more than one geometry shader exists in a
    // library
    geometryInfo.mItemTypeName = BuildString("VertexInType_", baseShaderName);
    geometryInfo.mArrayTypeName = BuildString("VertexInStreamType", baseShaderName);
    geometryInfo.mInstanceName = "In";

    ShaderInterfaceStructArray* arrayInterfaceType = DeclareGeometryVertexInput(
        interfaceGroup, entryPointInfo, geometryInfo, inputStreamInterfaceTypes, inputVertexInterfaceTypes);
    if (arrayInterfaceType != nullptr)
      CopyFromInterfaceType(copyInputsBlock,
                            copyInputsStreamVar,
                            arrayInterfaceType->mInstance,
                            arrayInterfaceType,
                            spv::StorageClassFunction,
                            spv::StorageClassInput);
  }

  // Declare the per-vertex built-ins (constants like position, point size,
  // etc...). There should only ever be one value in the built-ins group here
  // otherwise we'll have an error from declaring a duplicate type name
  // (validate later?).
  AutoDeclare(range, vertexInputInterfaceInfo.mBuiltInGroups.Values());
  for (; !range.Empty(); range.PopFront())
  {
    InterfaceInfoGroup& interfaceGroup = range.Front();
    interfaceGroup.mStorageClass = spv::StorageClassInput;

    GeometryInOutTypeInfo geometryInfo;
    geometryInfo.mArraySize = stageInfo.mInputStreamType->mParameters[1];
    geometryInfo.mItemTypeName = BuildString("BuiltInVertexInType", baseShaderName);
    geometryInfo.mArrayTypeName = BuildString("BuiltInVertexInStreamType", baseShaderName);
    // Currently the backend requires this be named "gl_in" or the translation
    // won't work
    geometryInfo.mInstanceName = "gl_in";

    ShaderInterfaceStructArray* arrayInterfaceType = DeclareGeometryVertexInput(
        interfaceGroup, entryPointInfo, geometryInfo, inputStreamInterfaceTypes, inputVertexInterfaceTypes);
    if (arrayInterfaceType != nullptr)
      CopyFromInterfaceType(copyInputsBlock,
                            copyInputsStreamVar,
                            arrayInterfaceType->mInstance,
                            arrayInterfaceType,
                            spv::StorageClassFunction,
                            spv::StorageClassInput);
  }
}

ShaderInterfaceStructArray*
EntryPointGeneration::DeclareGeometryVertexInput(InterfaceInfoGroup& interfaceGroup,
                                                 EntryPointInfo* entryPointInfo,
                                                 GeometryInOutTypeInfo& info,
                                                 Array<ShaderInterfaceType*>& inputStreamInterfaceTypes,
                                                 Array<ShaderInterfaceType*>& inputVertexInterfaceTypes)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  InterfaceInfoGroup::FieldList fieldList = interfaceGroup.mFields;
  if (fieldList.Empty())
    return nullptr;

  ShaderInterfaceStruct* inputVertexInterface = new ShaderInterfaceStruct();
  inputVertexInterfaceTypes.PushBack(inputVertexInterface);
  inputVertexInterface->DeclareInterfaceType(this, interfaceGroup, entryPointInfo);

  // Create the array item's struct type
  LightningShaderIRType* arrayItemType = inputVertexInterface->mType;
  arrayItemType->mDebugResultName = info.mItemTypeName;
  // All type/member type decorations in the interface group belong to the
  // struct, not the array itself
  inputVertexInterface->DecorateInterfaceType(this, interfaceGroup, entryPointInfo);

  // Create the input stream (array) type.
  LightningShaderIRType* arrayType = translator->MakeTypeAndPointer(translator->mLibrary,
                                                                ShaderIRTypeBaseType::FixedArray,
                                                                info.mArrayTypeName,
                                                                nullptr,
                                                                interfaceGroup.mStorageClass);
  arrayType->mDebugResultName = info.mArrayTypeName;
  arrayType->mParameters.PushBack(arrayItemType);
  arrayType->mParameters.PushBack(info.mArraySize);

  // Fill out the interface type struct with our results
  ShaderInterfaceStructArray* arrayInterfaceType = new ShaderInterfaceStructArray();
  inputStreamInterfaceTypes.PushBack(arrayInterfaceType);
  arrayInterfaceType->mType = arrayType;
  arrayInterfaceType->mStructType = inputVertexInterface;
  // Define the type's instance which also adds instance decorations to the
  // array (e.g. location)
  arrayInterfaceType->DefineInterfaceType(this, interfaceGroup, entryPointInfo);
  arrayInterfaceType->mInstance->mDebugResultName = info.mInstanceName;
  return arrayInterfaceType;
}

void EntryPointGeneration::DeclareGeometryVertexOutputs(GeometryStageInfo& stageInfo,
                                                        EntryPointInfo* entryPointInfo,
                                                        ShaderInterfaceInfo& vertexOutputInterfaceInfo,
                                                        Array<ShaderInterfaceType*>& inputStreamInterfaceTypes)
{
  // Keep track of all the output vertex types created
  Array<ShaderInterfaceType*> outputVertexInterfaceTypes;

  // Declare per-vertex outputs
  {
    InterfaceInfoGroup& interfaceGroup = vertexOutputInterfaceInfo.mOutputs;
    interfaceGroup.mStorageClass = spv::StorageClassOutput;
    interfaceGroup.mName = "Out";

    // Add decorations
    interfaceGroup.mTypeDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBlock));
    interfaceGroup.mInstanceDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationLocation, 0));

    WriteGeometryInterfaceOutput(interfaceGroup, entryPointInfo, outputVertexInterfaceTypes);
  }

  // Declare per-vertex built-in outputs (e.g. Position/PrimitiveId)
  AutoDeclare(range, vertexOutputInterfaceInfo.mBuiltInGroups.Values());
  for (; !range.Empty(); range.PopFront())
  {
    InterfaceInfoGroup& interfaceGroup = range.Front();
    interfaceGroup.mStorageClass = spv::StorageClassOutput;
    interfaceGroup.mName = "gl_out";
    WriteGeometryInterfaceOutput(interfaceGroup, entryPointInfo, outputVertexInterfaceTypes);
  }

  // Generate the append functions that copy an output instance type to the
  // stage output
  WriteGeometryAppendFunctions(stageInfo, entryPointInfo, outputVertexInterfaceTypes, inputStreamInterfaceTypes);

  DeleteObjectsIn(outputVertexInterfaceTypes);
}

void EntryPointGeneration::WriteGeometryInterfaceOutput(InterfaceInfoGroup& interfaceGroup,
                                                        EntryPointInfo* entryPointInfo,
                                                        Array<ShaderInterfaceType*>& interfaceTypes)
{
  if (interfaceGroup.mFields.Empty())
    return;

  // Create the interface depending on if this is a struct or a collection of
  // globals
  ShaderInterfaceType* interfaceType = nullptr;
  if (interfaceGroup.mIsStruct)
    interfaceType = new ShaderInterfaceStruct();
  else
    interfaceType = new ShaderInterfaceGlobals();

  // Same as basic interface declarations except we don't define a copy
  // function.
  interfaceType->DeclareInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceType->DecorateInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceType->DefineInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceTypes.PushBack(interfaceType);
}

void EntryPointGeneration::WriteGeometryAppendFunctions(GeometryStageInfo& stageInfo,
                                                        EntryPointInfo* entryPointInfo,
                                                        Array<ShaderInterfaceType*>& outputVertexInterfaceTypes,
                                                        Array<ShaderInterfaceType*>& inputStreamInterfaceTypes)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  // Generate all of our different append functions
  Array<GeometryAppendFunctionData> appendFunctions;
  appendFunctions.Resize(stageInfo.mOutputStreamTypes.Size());
  for (size_t i = 0; i < stageInfo.mOutputStreamTypes.Size(); ++i)
  {
    // The main one is a "provoking vertex" append function.
    // This takes a provoking vertex index to copy all pass-through data from.
    GenerateProvokingVertexAppend(stageInfo, entryPointInfo, appendFunctions[i], stageInfo.mOutputStreamTypes[i]);
  }

  // Copy all outputs for this append function.
  // @JoshD: This will have to be updated later if there's ever more than a
  // provoking vertex append as this copy logic is specific to a provoking
  // vertex.
  for (size_t i = 0; i < appendFunctions.Size(); ++i)
  {
    GeometryAppendFunctionData& appendFnData = appendFunctions[i];

    for (size_t j = 0; j < outputVertexInterfaceTypes.Size(); ++j)
      CopyGeometryOutputInterface(
          stageInfo, entryPointInfo, appendFnData, *outputVertexInterfaceTypes[j], inputStreamInterfaceTypes);

    // Invoke the append callback if it exists (allows custom api transform
    // logic)
    CallbackSettings& callbackSettings = mTranslator->mSettings->mCallbackSettings;
    if (callbackSettings.mAppendCallback != nullptr)
    {
      AppendCallbackData callbackData;
      callbackData.mEntryPointGeneration = this;
      callbackData.mSettings = mTranslator->mSettings;
      callbackData.mStageInfo = &stageInfo;
      callbackData.mEntryPointInfo = entryPointInfo;
      callbackData.mAppendFnData = &appendFnData;
      callbackData.mOutputVertexInterfaceTypes = &outputVertexInterfaceTypes;
      callbackData.mInputStreamInterfaceTypes = &inputStreamInterfaceTypes;
      callbackSettings.mAppendCallback(callbackData, callbackSettings.mAppendCallbackUserData);
    }

    // Add the instruction to actually emit the vertex
    translator->BuildIROp(appendFnData.mBlock, OpType::OpEmitVertex, nullptr, context);
  }

  // Fix terminators for every block in all append functionsAdd a terminator to
  // every append function
  for (size_t i = 0; i < appendFunctions.Size(); ++i)
  {
    GeometryAppendFunctionData& appendFnData = appendFunctions[i];
    LightningShaderIRFunction* appendFn = appendFnData.mFunction;
    for (size_t j = 0; j < appendFn->mBlocks.Size(); ++j)
      translator->FixBlockTerminators(appendFn->mBlocks[j], context);
  }
}

void EntryPointGeneration::GenerateProvokingVertexAppend(GeometryStageInfo& stageInfo,
                                                         EntryPointInfo* entryPointInfo,
                                                         GeometryAppendFunctionData& appendFnData,
                                                         LightningShaderIRType* outputStreamType)
{
  // A 'provoking vertex' append function is one that takes a provoking vertex
  // index and uses that to generate any missing data for pass-through.
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  LightningShaderIRType* outputVertexType = outputStreamType->mParameters[0]->As<LightningShaderIRType>();

  // Find the provoking vertex append function and it's corresponding shader
  // function that we need to late bind.
  LightningShaderIRType* intType = translator->FindType(LightningTypeId(int), nullptr);
  Lightning::BoundType* lightningStreamType = outputStreamType->mLightningType;
  Lightning::Function* lightningAppendFn =
      GetMemberOverloadedFunction(lightningStreamType, "Append", outputVertexType->mName, "Integer");
  LightningShaderIRFunction* originalAppendFn = translator->mLibrary->FindFunction(lightningAppendFn);

  // Generate the a copy of the original append function
  LightningShaderIRFunction* lateBoundAppendFn = CloneAppendFn(originalAppendFn);
  // Register this as a late bound function. This means the back-end will
  // replace all references to the original function with this function when
  // generating this entry point.
  entryPointInfo->mLateBoundFunctions.InsertOrError(originalAppendFn, lateBoundAppendFn);

  // Grab the first block so we can start adding instructions
  BasicBlock* firstBlock = lateBoundAppendFn->mBlocks[0];
  // Also grab the out data's instance from the parameter block (it's always
  // index 1 here)
  LightningShaderIROp* outDataValueInstance = lateBoundAppendFn->mParameterBlock.mLines[1]->As<LightningShaderIROp>();

  LightningShaderIROp* defaultVertexIndex = lateBoundAppendFn->mParameterBlock.mLines[2]->As<LightningShaderIROp>();

  // To make copying code easier, always generate a local variable that we store
  // the input value type into
  LightningShaderIRType* outVertexPtrType = outputVertexType->mPointerType;
  LightningShaderIROp* outVertexInstance =
      translator->BuildOpVariable(firstBlock, outVertexPtrType, spv::StorageClassFunction, context);
  outVertexInstance->mDebugResultName = "outDataLocal";
  translator->BuildIROp(firstBlock, OpType::OpStore, nullptr, outVertexInstance, outDataValueInstance, context);

  // Fill out the results
  appendFnData.mFunction = lateBoundAppendFn;
  appendFnData.mBlock = firstBlock;
  appendFnData.mOutputDataInstance = outVertexInstance;
  appendFnData.mDefaultVertexId = defaultVertexIndex;
}

LightningShaderIRFunction* EntryPointGeneration::CloneAppendFn(LightningShaderIRFunction* originalAppendFn)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  LightningShaderIRType* fnType = originalAppendFn->mFunctionType;

  // Generate the new append function which has the same name and type as the
  // original
  LightningShaderIRFunction* newAppendFn = new LightningShaderIRFunction();
  translator->mLibrary->mOwnedFunctions.PushBack(newAppendFn);

  newAppendFn->mFunctionType = fnType;
  newAppendFn->mName = originalAppendFn->mName;
  newAppendFn->mDebugResultName = originalAppendFn->mDebugResultName;

  // Copy all parameters
  BasicBlock* originalParameterBlock = &originalAppendFn->mParameterBlock;
  BasicBlock* newParameterBlock = &newAppendFn->mParameterBlock;
  for (size_t i = 0; i < originalParameterBlock->mLines.Size(); ++i)
  {
    LightningShaderIROp* originalParameterOp = originalParameterBlock->mLines[i]->As<LightningShaderIROp>();
    LightningShaderIROp* newParameterOp = translator->BuildIROp(
        newParameterBlock, OpType::OpFunctionParameter, originalParameterOp->mResultType, context);
    // Name the parameter the same
    newParameterOp->mDebugResultName = originalParameterOp->mDebugResultName;
  }

  // Make this a valid function by giving it the basic starting block
  BasicBlock* firstBlock = translator->BuildBlockNoStack(String(), context);
  newAppendFn->mBlocks.PushBack(firstBlock);

  return newAppendFn;
}

void EntryPointGeneration::CopyGeometryOutputInterface(GeometryStageInfo& stageInfo,
                                                       EntryPointInfo* entryPointInfo,
                                                       GeometryAppendFunctionData& appendFnData,
                                                       ShaderInterfaceType& interfaceType,
                                                       Array<ShaderInterfaceType*>& inputStreamInterfaceTypes)
{
  SpirVNameSettings& nameSettings = mTranslator->mSettings->mNameSettings;

  // Walk the vertex output type's fields and find all stage outputs (in order).
  // Use this to make a map of the last output name/type pair so we know who to
  // copy from.
  HashMap<ShaderFieldKey, String> keyMap;
  ShaderIRTypeMeta* typeMeta = appendFnData.mOutputDataInstance->mResultType->mDereferenceType->mMeta;
  for (size_t i = 0; i < typeMeta->mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[i];
    // Walk all attributes on this field searching for stage/hardware output
    // attributes
    for (size_t j = 0; j < fieldMeta->mAttributes.Size(); ++j)
    {
      ShaderIRAttribute* attribute = fieldMeta->mAttributes.GetAtIndex(j);
      if (attribute->mAttributeName == nameSettings.mStageOutputAttribute ||
          attribute->mAttributeName == nameSettings.mHardwareBuiltInOutputAttribute)
      {
        ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(attribute);
        keyMap[fieldKey] = fieldMeta->mLightningName;
      }
    }
  }

  // Walk all of the fields in the interface type to try and copy a result into
  // it
  BasicBlock* block = appendFnData.mBlock;
  size_t count = interfaceType.GetFieldCount();
  for (size_t i = 0; i < count; ++i)
  {
    ShaderInterfaceField* interfaceField = interfaceType.GetFieldAtIndex(i);
    ShaderIRFieldMeta* fieldMeta = interfaceField->mFieldMeta;

    // Walk all attributes on this field searching for stage/hardware output
    // attributes
    for (size_t j = 0; j < fieldMeta->mAttributes.Size(); ++j)
    {
      ShaderIRAttribute* attribute = fieldMeta->mAttributes.GetAtIndex(j);
      if (attribute->mAttributeName != nameSettings.mStageOutputAttribute &&
          attribute->mAttributeName != nameSettings.mHardwareBuiltInOutputAttribute)
        continue;

      // Get the output field key corresponding to this output
      ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(attribute);

      // See if the vertex output declared an output with the same key
      String fieldName = keyMap.FindValue(fieldKey, String());
      // If so, copy from the vertex output struct
      if (!fieldName.Empty())
      {
        LightningShaderIROp* dest = interfaceType.GetFieldPointerByIndex(i, this, block, spv::StorageClassOutput);
        LightningShaderIROp* source =
            GetNamedMemberInstanceFrom(block, appendFnData.mOutputDataInstance, fieldName, spv::StorageClassFunction);
        ErrorIf(source == nullptr, "Source should never be null");
        CopyField(block, source->mResultType->mDereferenceType, source, dest);
      }
      // Otherwise we need to perform a pass-through
      else
      {
        // Get the pointer to the destination target
        LightningShaderIROp* dest = interfaceType.GetFieldPointerByIndex(i, this, block, spv::StorageClassOutput);
        // Walk all input stream types to try and find an input that matches
        // this output
        for (size_t j = 0; j < inputStreamInterfaceTypes.Size(); ++j)
        {
          // If this input stream contains the given input
          ShaderInterfaceStructArray* inputStreamType = inputStreamInterfaceTypes[j]->As<ShaderInterfaceStructArray>();
          if (inputStreamType->ContainsField(fieldKey))
          {
            // Get the instance pointer to the member in the array at the given
            // index and copy from it.
            // @JoshD: This doesn't seem like it would ever need to deal with
            // attribute name overrides. Is this true?
            LightningShaderIROp* inputInstance = inputStreamType->GetPointerByIndex(
                appendFnData.mDefaultVertexId, fieldKey, this, block, spv::StorageClassInput);
            CopyField(block, inputInstance->mResultType->mDereferenceType, inputInstance, dest);
          }
        }
      }
    }
  }
}

void EntryPointGeneration::CollectInterfaceVariables(LightningShaderIRFunction* function,
                                                     ShaderInterfaceInfo& interfaceInfo,
                                                     ShaderStage::Enum shaderStage)
{
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderIRType* currentType = context->mCurrentType;

  CollectInputInterfaceVariables(function, interfaceInfo, currentType, shaderStage);
  CollectOutputInterfaceVariables(function, interfaceInfo, currentType, shaderStage);
  CollectUniformInterfaceVariables(function, interfaceInfo, currentType, shaderStage);
}

void EntryPointGeneration::CollectInputInterfaceVariables(LightningShaderIRFunction* function,
                                                          ShaderInterfaceInfo& interfaceInfo,
                                                          LightningShaderIRType* type,
                                                          ShaderStage::Enum shaderStage)
{
  SpirVNameSettings& nameSettings = mTranslator->mSettings->mNameSettings;

  ShaderIRTypeMeta* typeMeta = type->mMeta;
  for (size_t i = 0; i < typeMeta->mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[i];

    // If this is a stage input
    ShaderIRAttribute* stageInputAttribute =
        fieldMeta->mAttributes.FindFirstAttribute(nameSettings.mStageInputAttribute);
    if (stageInputAttribute != nullptr)
    {
      // Create the field info
      ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(stageInputAttribute);
      InterfaceInfoGroup::FieldInfo* fieldInfo = interfaceInfo.mInputs.FindOrCreateFieldInfo(fieldKey);
      // Create a new field meta for this field based upon the field we're
      // matching to. In particular, make sure this field's name is set based
      // upon the attribute name.
      fieldInfo->mFieldMeta = fieldMeta->Clone(mTranslator->mLibrary);
      fieldInfo->mFieldMeta->mLightningName = fieldMeta->GetFieldAttributeName(stageInputAttribute);
      fieldInfo->mFieldMeta->mOwner = nullptr;
      // Add the given field's meta as someone we need to write to (multiple can
      // exist)
      fieldInfo->mLinkedFields.PushBack(fieldMeta);
    }
    // If this is a hardware built-in input
    ShaderIRAttribute* hardwareBuiltInAttribute =
        fieldMeta->mAttributes.FindFirstAttribute(nameSettings.mHardwareBuiltInInputAttribute);
    if (hardwareBuiltInAttribute != nullptr)
    {
      // Process the built-in. This should always return the info group that the
      // this maps to (unless an error happens)
      InterfaceInfoGroup* builtInGroup = ProcessBuiltIn(function,
                                                        interfaceInfo,
                                                        shaderStage,
                                                        fieldMeta,
                                                        hardwareBuiltInAttribute,
                                                        spv::StorageClassInput,
                                                        "PerVertexIn");
      if (builtInGroup != nullptr)
      {
        // On the field info, add the given field's meta as someone to write to
        ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(hardwareBuiltInAttribute);
        InterfaceInfoGroup::FieldInfo* fieldInfo = builtInGroup->FindFieldInfo(fieldKey);
        fieldInfo->mLinkedFields.PushBack(fieldMeta);
      }
    }
  }
}

void EntryPointGeneration::CollectOutputInterfaceVariables(LightningShaderIRFunction* function,
                                                           ShaderInterfaceInfo& interfaceInfo,
                                                           LightningShaderIRType* type,
                                                           ShaderStage::Enum shaderStage)
{
  SpirVNameSettings& nameSettings = mTranslator->mSettings->mNameSettings;
  ShaderIRTypeMeta* typeMeta = type->mMeta;
  for (size_t i = 0; i < typeMeta->mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[i];

    // @JoshD: Need to update this at some point to support multiple output
    // names by iterating on attributes

    // If this is a stage output
    ShaderIRAttribute* stageOutputAttribute =
        fieldMeta->mAttributes.FindFirstAttribute(nameSettings.mStageOutputAttribute);
    if (stageOutputAttribute != nullptr)
    {
      // Create the field info
      ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(stageOutputAttribute);
      InterfaceInfoGroup::FieldInfo* fieldInfo = interfaceInfo.mOutputs.FindOrCreateFieldInfo(fieldKey);
      // Create a new field meta for this field based upon the field we're
      // matching to. In particular, make sure this field's name is set based
      // upon the attribute name.
      fieldInfo->mFieldMeta = fieldMeta->Clone(mTranslator->mLibrary);
      fieldInfo->mFieldMeta->mLightningName = fieldMeta->GetFieldAttributeName(stageOutputAttribute);
      fieldInfo->mFieldMeta->mOwner = nullptr;
      // Add the given field's meta as someone we need to read from (multiple
      // can exist)
      fieldInfo->mLinkedFields.PushBack(fieldMeta);
    }
    // If this is a hardware built-in output
    ShaderIRAttribute* hardwareBuiltInAttribute =
        fieldMeta->mAttributes.FindFirstAttribute(nameSettings.mHardwareBuiltInOutputAttribute);
    if (hardwareBuiltInAttribute != nullptr)
    {
      // Process the built-in. This should always return the info group that the
      // this maps to (unless an error happens)
      InterfaceInfoGroup* builtInGroup = ProcessBuiltIn(function,
                                                        interfaceInfo,
                                                        shaderStage,
                                                        fieldMeta,
                                                        hardwareBuiltInAttribute,
                                                        spv::StorageClassOutput,
                                                        "PerVertexOut");
      if (builtInGroup != nullptr)
      {
        // On the field info, add the given field's meta as someone to read from
        ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(hardwareBuiltInAttribute);
        InterfaceInfoGroup::FieldInfo* fieldInfo = builtInGroup->FindFieldInfo(fieldKey);
        fieldInfo->mLinkedFields.PushBack(fieldMeta);
      }
    }
  }
}

void EntryPointGeneration::CollectUniformInterfaceVariables(LightningShaderIRFunction* function,
                                                            ShaderInterfaceInfo& interfaceInfo,
                                                            LightningShaderIRType* type,
                                                            ShaderStage::Enum shaderStage)
{
  SpirVNameSettings& nameSettings = mTranslator->mSettings->mNameSettings;

  // Build mappings for the given uniform block descriptions so we can
  // efficiently check each field we come across.
  // @JoshD: Should pre-process this once per run, not per shader.
  HashMap<ShaderFieldKey, UniformBufferDescription*> mapping;
  ProcessUniformBlockSettings(shaderStage, mapping);

  ShaderIRTypeMeta* typeMeta = type->mMeta;
  for (size_t i = 0; i < typeMeta->mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[i];

    // Walk all attributes searching for a uniform attribute.
    // @JoshD: At some point actually deal with AppBuiltIn vs. Property
    // differently (with compositor it doesn't matter)
    for (size_t j = 0; j < fieldMeta->mAttributes.Size(); ++j)
    {
      ShaderIRAttribute* attribute = fieldMeta->mAttributes.GetAtIndex(j);
      if (attribute->mAttributeName == nameSettings.mAppBuiltInInputAttribute ||
          attribute->mAttributeName == nameSettings.mPropertyInputAttribute)
      {
        // Don't include non-copyable types in the uniform buffer (these were
        // declared globally).
        // @JoshD: This might need to be changed later if a non-copyable type
        // can be put in a uniform buffer.
        if (fieldMeta->mLightningType->HasAttribute(nameSettings.mNonCopyableAttributeName))
          continue;

        InterfaceInfoGroup* interfaceGroup =
            ProcessUniformBlock(function, interfaceInfo, fieldMeta, attribute, mapping);

        ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(attribute);
        // Store the name of the resolved field on the resultant field info
        InterfaceInfoGroup::FieldInfo* fieldInfo = interfaceGroup->FindOrCreateFieldInfo(fieldKey);
        fieldInfo->mLinkedFields.PushBack(fieldMeta);
        break;
      }
    }
  }
}

void EntryPointGeneration::ProcessUniformBlockSettings(ShaderStage::Enum stageToProcess,
                                                       HashMap<ShaderFieldKey, UniformBufferDescription*>& mapping)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderSpirVSettings* settings = translator->mSettings;

  // Iterate over all of the user defined uniform buffer descriptions and map
  // the field key to the block it came from. This allows efficient processing
  // when iterating over fields later.
  for (size_t i = 0; i < settings->mUniformBufferDescriptions.Size(); ++i)
  {
    UniformBufferDescription& description = settings->mUniformBufferDescriptions[i];
    // Buffer descriptions can target certain stages. This may be an
    // optimization if certain stages bind less data (up to Nate to test)
    if (!description.mAllowedStages.IsSet(stageToProcess))
      continue;

    // For each field, map it to the uniform buffer description it came from
    for (size_t fieldIndex = 0; fieldIndex < description.mFields.Size(); ++fieldIndex)
    {
      ShaderIRFieldMeta* fieldMeta = description.mFields[fieldIndex];
      ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey();
      mapping.Insert(fieldKey, &description);
    }
  }
}

InterfaceInfoGroup* EntryPointGeneration::ProcessBuiltIn(LightningShaderIRFunction* function,
                                                         ShaderInterfaceInfo& interfaceInfo,
                                                         ShaderStage::Enum shaderStage,
                                                         ShaderIRFieldMeta* fieldMeta,
                                                         ShaderIRAttribute* attribute,
                                                         spv::StorageClass storageClass,
                                                         StringParam name)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderSpirVSettings* settings = translator->mSettings;

  FragmentType::Enum fragmentType = ShaderStageToFragmentType(shaderStage);

  // Get the built-ins map we read from depending on if this is an input or
  // output
  BuiltInStageDescription::FieldKeyToBlockMap* mappings = nullptr;
  BuiltInStageDescription& builtInDescriptions = settings->mBuiltIns[fragmentType];
  if (storageClass == spv::StorageClassOutput)
    mappings = &builtInDescriptions.mInternalOutputMappings;
  else
    mappings = &builtInDescriptions.mInternalInputMappings;

  // Get the description for this built-in. If we fail to find this then
  // something was incorrectly marked as a built-in and cannot be satisfied.
  ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(attribute);
  BuiltInBlockDescription* block = mappings->FindValue(fieldKey, nullptr);
  if (block == nullptr)
  {
    String fullMsg = String::Format(
        "BuiltIn '%s' is not valid in shader stage %s", fieldKey.mKey.c_str(), FragmentType::Names[fragmentType]);
    if (fieldMeta->mLightningProperty != nullptr)
      translator->SendTranslationError(fieldMeta->mLightningProperty->Location, "Invalid BuiltIn", fullMsg);
    return nullptr;
  }

  bool exists = interfaceInfo.mBuiltInGroups.ContainsKey(block);
  InterfaceInfoGroup& group = interfaceInfo.mBuiltInGroups[block];

  // If this is an interface block that's already been defined then there's
  // nothing more to do. Interface blocks are declared all up front.
  if (block->mInterfaceBlock && exists)
    return &group;

  // If this is an interface block then declare the whole thing at once
  if (block->mInterfaceBlock)
  {
    group.mStorageClass = storageClass;
    // Add attributes for the block (the entire struct is an interface block)
    if (block->mInterfaceBlock)
    {
      group.mName = name;
      group.mTypeDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBlock));
      group.mIsStruct = true;
      group.mIsBuiltIn = true;
    }

    // Add all fields on the block at once. Since this is an interface block the
    // entire thing has to be copied otherwise subsequent shader stages won't
    // have a matching interface.
    for (size_t i = 0; i < block->mFields.Size(); ++i)
    {
      BuiltInBlockDescription::BuiltInFieldMeta& builtInMeta = block->mFields[i];
      InterfaceInfoGroup::FieldInfo& fieldInfo = group.mFields.PushBack();
      fieldInfo.mFieldMeta = builtInMeta.mMeta;
      fieldInfo.mDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBuiltIn, builtInMeta.mId));
    }
  }
  // Otherwise these are just global built ins
  else
  {
    // If the block doesn't already exist then set some default values on it
    if (!exists)
    {
      group.mStorageClass = storageClass;
      group.mIsStruct = false;
      group.mIsBuiltIn = true;
    }

    // Write the info for the built in that we're now using out
    BuiltInBlockDescription::BuiltInFieldMeta* foundBuiltInMeta = block->FindField(fieldKey);
    if (foundBuiltInMeta != nullptr)
    {
      // Find if we've already created this field or not. There can be multiple
      // read/writes to the same variable due to name overrides so this can get
      // called more than once.
      InterfaceInfoGroup::FieldInfo* fieldInfo = group.FindFieldInfo(fieldKey);
      if (fieldInfo == nullptr)
      {
        // If not, create and fill out the field info
        fieldInfo = group.FindOrCreateFieldInfo(fieldKey);
        fieldInfo->mFieldMeta = foundBuiltInMeta->mMeta;
        fieldInfo->mDecorations.PushBack(
            InterfaceInfoGroup::DecorationParam(spv::DecorationBuiltIn, foundBuiltInMeta->mId));
      }
    }
  }
  return &group;
}

InterfaceInfoGroup*
EntryPointGeneration::ProcessUniformBlock(LightningShaderIRFunction* function,
                                          ShaderInterfaceInfo& interfaceInfo,
                                          ShaderIRFieldMeta* fieldMeta,
                                          ShaderIRAttribute* attribute,
                                          HashMap<ShaderFieldKey, UniformBufferDescription*>& mapping)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderSpirVSettings* settings = translator->mSettings;

  // Find what uniform block description this field maps to (null means the
  // material)
  ShaderFieldKey fieldKey = fieldMeta->MakeFieldKey(attribute);
  UniformBufferDescription* uniformBlock = mapping.FindValue(fieldKey, nullptr);

  // Check if we've already visited this uniform block. If so we can skip some
  // first-time setup
  bool exists = interfaceInfo.mUniformGroups.ContainsKey(uniformBlock);

  InterfaceInfoGroup& uniformGroup = interfaceInfo.mUniformGroups[uniformBlock];
  // Determine if this is a user defined uniform buffer or if this is the
  // default (the material)
  bool isDefaultBuffer = (uniformBlock == nullptr);

  // If this is user defined (not default) and it already exists then
  // there's nothing to do. User defined buffers are declared all up-front.
  if (!isDefaultBuffer && exists)
    return &uniformGroup;

  // If this is the default buffer then grab the default settings
  if (isDefaultBuffer)
    uniformBlock = &settings->mDefaultUniformBufferDescription;

  // If this is the first time visiting this buffer then set the struct
  // parameters (storage class, name, id, etc...)
  if (!exists)
  {
    uniformGroup.mName = uniformBlock->mDebugName;
    uniformGroup.mStorageClass = spv::StorageClassUniform;
    uniformGroup.mIsStruct = true;

    u32 bindingId = uniformBlock->mBindingId;
    // If this is the material buffer and we don't allow id overlap
    // then offset the binding id by the fragment type
    if (isDefaultBuffer && !settings->mAllowUniformMaterialBufferIndexOverap)
      bindingId += context->mCurrentType->mMeta->mFragmentType;

    uniformGroup.mInstanceDecorations.PushBack(
        InterfaceInfoGroup::DecorationParam(spv::DecorationDescriptorSet, uniformBlock->mDescriptorSetId));
    uniformGroup.mInstanceDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBinding, bindingId));
    // Set the reflection data for the buffer
    ShaderResourceReflectionData& reflectionData = uniformGroup.mReflectionData;
    reflectionData.mDescriptorSet = uniformBlock->mDescriptorSetId;
    reflectionData.mBinding = bindingId;
    reflectionData.mInstanceName = uniformGroup.mName;
  }

  // Mark that we've used this binding id
  mUsedBindingIds.Insert(uniformGroup.mReflectionData.mBinding);

  // If this is a user defined buffer then the entire buffer must match all at
  // once. This means we just copy all fields from the user defined description.
  if (!isDefaultBuffer)
  {
    for (size_t i = 0; i < uniformBlock->mFields.Size(); ++i)
    {
      InterfaceInfoGroup::FieldInfo fieldInfo;
      fieldInfo.mFieldMeta = uniformBlock->mFields[i];
      uniformGroup.mFields.PushBack(fieldInfo);
    }
  }
  // Otherwise this is the default material where all misc fields go.
  // Add each field only when we encounter it.
  else
  {
    InterfaceInfoGroup::FieldInfo fieldInfo;
    fieldInfo.mFieldMeta = fieldMeta->Clone(mTranslator->mLibrary);
    fieldInfo.mFieldMeta->mLightningName = fieldMeta->GetFieldAttributeName(attribute);
    uniformGroup.mFields.PushBack(fieldInfo);
  }
  return &uniformGroup;
}

void EntryPointGeneration::DeclareStageBlocks(ShaderInterfaceInfo& interfaceInfo,
                                              EntryPointInfo* entryPointInfo,
                                              EntryPointHelperFunctionData& copyInputsData,
                                              EntryPointHelperFunctionData& copyOutputsData)
{
  // Declare inputs/outputs
  DeclareBlock(interfaceInfo.mInputs, entryPointInfo, copyInputsData, mInputs);
  DeclareBlock(interfaceInfo.mOutputs, entryPointInfo, copyOutputsData, mOutputs);

  // Write out BuiltIns. These can be inputs and outputs as well as interface
  // blocks or globals
  DeclareGroupBlocks(interfaceInfo.mBuiltInGroups, entryPointInfo, copyInputsData, copyOutputsData, mBuiltIns);
  // Also write out interface blocks for all uniform blocks. Uniform block
  // groupings can be specified by the C++ api.
  DeclareGroupBlocks(interfaceInfo.mUniformGroups, entryPointInfo, copyInputsData, copyOutputsData, mUniforms);
}

void EntryPointGeneration::DeclareGroupBlocks(ShaderInterfaceInfo::InterfaceGroupMap& interfaceGroups,
                                              EntryPointInfo* entryPointInfo,
                                              EntryPointHelperFunctionData& copyInputsData,
                                              EntryPointHelperFunctionData& copyOutputsData,
                                              Array<ShaderInterfaceType*>& outArray)
{
  AutoDeclare(range, interfaceGroups.All());
  for (; !range.Empty(); range.PopFront())
  {
    InterfaceInfoGroup& interfaceGroup = range.Front().second;
    // Determine if this is an input/output type to choose which copy helper to
    // use
    if (interfaceGroup.mStorageClass == spv::StorageClassOutput)
      DeclareBlock(interfaceGroup, entryPointInfo, copyOutputsData, outArray);
    else
      DeclareBlock(interfaceGroup, entryPointInfo, copyInputsData, outArray);
  }
}

void EntryPointGeneration::DeclareBlock(InterfaceInfoGroup& interfaceGroup,
                                        EntryPointInfo* entryPointInfo,
                                        EntryPointHelperFunctionData& copyHelperData,
                                        Array<ShaderInterfaceType*>& outArray)
{
  if (interfaceGroup.mFields.Size() == 0)
    return;

  if (interfaceGroup.mIsStruct)
    DeclareBlockStruct(interfaceGroup, entryPointInfo, copyHelperData, outArray);
  else
    DeclareBlockNoStruct(interfaceGroup, entryPointInfo, copyHelperData, outArray);
}

void EntryPointGeneration::DeclareBlockStruct(InterfaceInfoGroup& interfaceGroup,
                                              EntryPointInfo* entryPointInfo,
                                              EntryPointHelperFunctionData& copyHelperData,
                                              Array<ShaderInterfaceType*>& outArray)
{
  ShaderInterfaceStruct* interfaceStruct = new ShaderInterfaceStruct();
  interfaceStruct->DeclareInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceStruct->DecorateInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceStruct->DefineInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceStruct->CopyInterfaceType(this, interfaceGroup, entryPointInfo, copyHelperData);
  outArray.PushBack(interfaceStruct);
}

void EntryPointGeneration::DeclareBlockNoStruct(InterfaceInfoGroup& interfaceGroup,
                                                EntryPointInfo* entryPointInfo,
                                                EntryPointHelperFunctionData& copyHelperData,
                                                Array<ShaderInterfaceType*>& outArray)
{
  ShaderInterfaceGlobals* interfaceGlobals = new ShaderInterfaceGlobals();
  ;
  interfaceGlobals->DeclareInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceGlobals->DecorateInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceGlobals->DefineInterfaceType(this, interfaceGroup, entryPointInfo);
  interfaceGlobals->CopyInterfaceType(this, interfaceGroup, entryPointInfo, copyHelperData);
  outArray.PushBack(interfaceGlobals);
}

void EntryPointGeneration::CopyField(BasicBlock* targetFnBlock,
                                     LightningShaderIRType* sourceLoadType,
                                     LightningShaderIROp* source,
                                     LightningShaderIROp* dest)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  LightningShaderIRType* sourceValueType = source->mResultType->mDereferenceType;
  LightningShaderIRType* destValueType = dest->mResultType->mDereferenceType;

  // For fixed arrays we need to copy over each element as the array types are
  // different (padding, stride, etc...)
  if (sourceValueType->mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    LightningShaderIRType* sourceElementType = sourceValueType->mParameters[0]->As<LightningShaderIRType>();
    LightningShaderIRType* destElementType = destValueType->mParameters[0]->As<LightningShaderIRType>();

    // To make copying easier, we use CompositeExtract to get element values
    // instead of pointers. This is so we don't have to worry about the storage
    // class on the type.
    LightningShaderIROp* sourceValue =
        translator->BuildIROp(targetFnBlock, OpType::OpLoad, sourceValueType, source, context);
    for (u32 i = 0; i < sourceValueType->mComponents; ++i)
    {
      // Extract from the source (by value)
      LightningShaderIRConstantLiteral* indexLiteral = translator->GetOrCreateConstantIntegerLiteral(i);
      LightningShaderIROp* sourceElement = translator->BuildIROp(
          targetFnBlock, OpType::OpCompositeExtract, sourceElementType, sourceValue, indexLiteral, context);
      // Get the destination element (by pointer)
      LightningShaderIROp* indexConstant = translator->GetIntegerConstant(i, context);
      LightningShaderIROp* destElement = translator->BuildIROp(
          targetFnBlock, OpType::OpAccessChain, destElementType->mPointerType, dest, indexConstant, context);
      // Store to the destination
      translator->BuildStoreOp(targetFnBlock, destElement, sourceElement, context);
    }
  }
  // Everything else simply load then store (always load so we don't care about
  // the storage class)
  else
  {
    ShaderIRTypeBaseType::Enum sourceBaseType = sourceValueType->GetBasePrimitiveType();
    ShaderIRTypeBaseType::Enum destBaseType = destValueType->GetBasePrimitiveType();

    // Convert the source integer into the destination boolean
    if (sourceBaseType == ShaderIRTypeBaseType::Int && destBaseType == ShaderIRTypeBaseType::Bool)
    {
      LightningShaderIROp* sourceValue = translator->GetOrGenerateValueTypeFromIR(targetFnBlock, source, context);
      LightningShaderIROp* result =
          translator->GenerateIntegerToBoolCast(targetFnBlock, sourceValue, destValueType, context);
      translator->BuildStoreOp(targetFnBlock, dest, result, context);
      return;
    }
    // Convert the source boolean into the destination integer
    if (sourceBaseType == ShaderIRTypeBaseType::Bool && destBaseType == ShaderIRTypeBaseType::Int)
    {
      // Cast the bool to an integer (selection between 0 and 1)
      LightningShaderIROp* sourceValue = translator->GetOrGenerateValueTypeFromIR(targetFnBlock, source, context);
      LightningShaderIROp* result =
          translator->GenerateBoolToIntegerCast(targetFnBlock, sourceValue, destValueType, context);
      translator->BuildStoreOp(targetFnBlock, dest, result, context);
      return;
    }

    translator->BuildStoreOp(targetFnBlock, dest, source, context);
  }
}

void EntryPointGeneration::CopyFromInterfaceType(BasicBlock* block,
                                                 LightningShaderIROp* dest,
                                                 LightningShaderIROp* source,
                                                 ShaderInterfaceType* sourceInterface,
                                                 spv::StorageClass destStorageClass,
                                                 spv::StorageClass sourceStorageClass)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  SpirVNameSettings& nameSettings = mTranslator->mSettings->mNameSettings;

  LightningShaderIRType* sourcePtrType = source->mResultType;
  ErrorIf(!sourcePtrType->IsPointerType(), "Source must be a pointer type");
  LightningShaderIRType* sourceValueType = sourcePtrType->mDereferenceType;

  LightningShaderIRType* destPtrType = dest->mResultType;
  ErrorIf(!destPtrType->IsPointerType(), "Destination must be a pointer type");
  LightningShaderIRType* destValueType = destPtrType->mDereferenceType;

  ShaderIRTypeBaseType::Enum sourceBaseType = sourceValueType->mBaseType;
  if (sourceBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    ShaderInterfaceStructArray* arrayInterface = sourceInterface->As<ShaderInterfaceStructArray>();
    // Get the size of the array
    LightningShaderIROp* sizeConstant = sourceValueType->mParameters[1]->As<LightningShaderIROp>();
    LightningShaderIRConstantLiteral* sizeLiteral = sizeConstant->mArguments[0]->As<LightningShaderIRConstantLiteral>();
    int sourceSize = sizeLiteral->mValue.Get<int>();

    for (int index = 0; index < sourceSize; ++index)
    {
      LightningShaderIROp* indexConstant = translator->GetIntegerConstant(index, context);
      LightningShaderIRType* sourceArrayItemType = sourceValueType->mParameters[0]->As<LightningShaderIRType>();
      LightningShaderIRType* destArrayItemType = destValueType->mParameters[0]->As<LightningShaderIRType>();
      LightningShaderIROp* sourceItem = translator->BuildIROp(
          block, OpType::OpAccessChain, sourceArrayItemType->mPointerType, source, indexConstant, context);
      LightningShaderIROp* destItem = translator->BuildIROp(
          block, OpType::OpAccessChain, destArrayItemType->mPointerType, dest, indexConstant, context);
      CopyFromInterfaceType(
          block, destItem, sourceItem, arrayInterface->mStructType, destStorageClass, sourceStorageClass);
    }
  }
  else if (sourceBaseType == ShaderIRTypeBaseType::Struct)
  {
    // Get the struct interface so we can manually access via an instance
    // (Hacky)
    ShaderInterfaceStruct* structInterface = sourceInterface->As<ShaderInterfaceStruct>();

    // Walk all fields that the source has
    size_t fieldCount = sourceInterface->GetFieldCount();
    for (size_t i = 0; i < fieldCount; ++i)
    {
      // For each field, walk all linked fields (fields to that map to this
      // one).
      ShaderInterfaceField* interfaceField = sourceInterface->GetFieldAtIndex(i);
      for (size_t j = 0; j < interfaceField->mLinkedFields.Size(); ++j)
      {
        ShaderIRFieldMeta* linkedFieldMeta = interfaceField->mLinkedFields[j];

        // Get the location to copy to.
        LightningShaderIROp* destMemberPtr =
            GetNamedMemberInstanceFrom(block, dest, linkedFieldMeta->mLightningName, destStorageClass);
        // Note: The source was generated from the target and should have linked
        // fields that always match. If this name is missing then the
        // source/dest didn't form a valid pair to copy between.
        if (destMemberPtr == nullptr)
        {
          Error("Destination field name '%s' is invalid", linkedFieldMeta->mLightningName.c_str());
          continue;
        }

        // Get the source field instance to copy from given the source instance
        LightningShaderIROp* sourceMemberPtr =
            structInterface->GetFieldPointerByIndex(source, i, this, block, sourceStorageClass);
        CopyField(block, sourceMemberPtr->mResultType->mDereferenceType, sourceMemberPtr, destMemberPtr);
      }
    }
  }
}

LightningShaderIROp* EntryPointGeneration::GetMemberInstanceFrom(BasicBlock* block,
                                                             LightningShaderIROp* source,
                                                             u32 sourceOffset,
                                                             spv::StorageClass sourceStorageClass)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  LightningShaderIRType* sourceValueType = source->mResultType->mDereferenceType;

  // Get the type of the source member. This might be different than what's on
  // the class due to storage classes. Storage classes are only stored on the
  // pointer type but structs contain value types so there's no way to check the
  // member's actual type to know what it is and the member type can't be
  // different just because it is contained in a different parent storage class
  // type.
  LightningShaderIRType* sourceMemberValueType = sourceValueType->GetSubType(sourceOffset);
  LightningShaderIRType* sourceMemberPtrType = sourceMemberValueType->mPointerType;
  if (sourceStorageClass != spv::StorageClassFunction)
    sourceMemberPtrType =
        translator->FindOrCreatePointerInterfaceType(translator->mLibrary, sourceMemberValueType, sourceStorageClass);

  // Get the member pointer to the source struct's member
  LightningShaderIROp* sourceOffsetConstant = translator->GetIntegerConstant(sourceOffset, context);
  LightningShaderIROp* sourceMemberPtr =
      translator->BuildIROp(block, OpType::OpAccessChain, sourceMemberPtrType, source, sourceOffsetConstant, context);
  return sourceMemberPtr;
}

LightningShaderIROp* EntryPointGeneration::GetNamedMemberInstanceFrom(BasicBlock* block,
                                                                  LightningShaderIROp* source,
                                                                  StringParam memberName,
                                                                  spv::StorageClass sourceStorageClass)
{
  ErrorIf(!source->mResultType->IsPointerType(), "Source must be a pointer type");

  // Find the member index by name
  LightningShaderIRType* sourceValueType = source->mResultType->mDereferenceType;
  u32 sourceOffset = sourceValueType->FindMemberIndex(memberName);
  // If this member didn't exist then we can't get the member so return null
  if (sourceOffset == LightningShaderIRType::cInvalidIndex)
    return nullptr;

  // Otherwise, return the member found via index
  return GetMemberInstanceFrom(block, source, sourceOffset, sourceStorageClass);
}

LightningShaderIROp* EntryPointGeneration::GetNamedMemberInstanceFrom(BasicBlock* block,
                                                                  LightningShaderIROp* source,
                                                                  const ShaderFieldKey& fieldKey,
                                                                  spv::StorageClass sourceStorageClass)
{
  ErrorIf(!source->mResultType->IsPointerType(), "Source must be a pointer type");

  // Find the member index by name
  LightningShaderIRType* sourceValueType = source->mResultType->mDereferenceType;
  u32 sourceOffset = sourceValueType->FindMemberIndex(fieldKey);
  // If this member didn't exist then we can't get the member so return null
  if (sourceOffset == LightningShaderIRType::cInvalidIndex)
    return nullptr;

  // Otherwise, return the member found via index
  return GetMemberInstanceFrom(block, source, sourceOffset, sourceStorageClass);
}

void EntryPointGeneration::DecorateUniformGroups(ShaderInterfaceInfo& interfaceInfo)
{
  AutoDeclare(range, interfaceInfo.mUniformGroups.All());
  for (; !range.Empty(); range.PopFront())
  {
    InterfaceInfoGroup& interfaceGroup = range.Front().second;
    DecorateUniformGroup(interfaceGroup);
  }
}

void EntryPointGeneration::DecorateUniformGroup(InterfaceInfoGroup& infoGroup)
{
  // Mark the uniform group as a block
  infoGroup.mTypeDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationBlock));

  // Add offsets to each member in the uniform group
  AddOffsetDecorations(infoGroup);
}

void EntryPointGeneration::AddOffsetDecorations(InterfaceInfoGroup& infoGroup)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  u32 currentByteOffset = 0;
  InterfaceInfoGroup::FieldList& fieldList = infoGroup.mFields;
  for (size_t i = 0; i < fieldList.Size(); ++i)
  {
    // Find the type of the member
    InterfaceInfoGroup::FieldInfo& fieldInfo = fieldList[i];
    ShaderIRFieldMeta* fieldMeta = fieldInfo.mFieldMeta;
    LightningShaderIRType* memberType = translator->FindType(fieldMeta->mLightningType, nullptr);

    // Different types have different alignment requirements.
    // Roughly speaking, alignment is 1 float, 2 float, and 4 float.
    u32 requiredAlignment = memberType->GetByteAlignment();
    u32 requiredSize = memberType->GetByteSize();
    // Compute the starting byte offset so it lines up with this type's required
    // alignment
    currentByteOffset = GetSizeAfterAlignment(currentByteOffset, requiredAlignment);

    // Add the offset decoration
    fieldInfo.mDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationOffset, currentByteOffset));

    // Store the offset and size of this field
    ShaderResourceReflectionData& fieldReflectionData = fieldInfo.mReflectionData;
    fieldReflectionData.mOffsetInBytes = currentByteOffset;
    fieldReflectionData.mSizeInBytes = requiredSize;

    AddMemberTypeDecorations(memberType, fieldInfo, fieldReflectionData);

    // Increment the current offset by the size of this type (not alignment)
    currentByteOffset += requiredSize;
  }

  // Store the total size of this buffer
  infoGroup.mReflectionData.mSizeInBytes = currentByteOffset;
}

void EntryPointGeneration::AddMemberTypeDecorations(LightningShaderIRType* memberType,
                                                    InterfaceInfoGroup::FieldInfo& fieldInfo,
                                                    ShaderResourceReflectionData& memberReflection)
{
  // Matrices require the row/column decoration and their stride
  if (memberType->mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    // Hardcode stride to size of a vec4 for performance reasons.
    // @JoshD: Maybe make a packing option for this later?
    int matrixStride = 16;
    fieldInfo.mDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationMatrixStride, matrixStride));
    fieldInfo.mDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationColMajor));
    memberReflection.mStride = matrixStride;
  }
  else if (memberType->mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    LightningShaderIRType* elementType = memberType->mParameters[0]->As<LightningShaderIRType>();
    // @JoshD: Hardcode to matrix/vector/scalar types for now.
    // Eventually could be bigger if structs are allowed to be bound as
    // uniforms.
    u32 stride = GetStride(elementType, 16.0f);
    fieldInfo.mTypeDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationArrayStride, stride));
    memberReflection.mStride = stride;
    // Recursively add any element type decorations (e.g. an array of matrices)
    AddMemberTypeDecorations(elementType, fieldInfo, memberReflection);
  }
}

void EntryPointGeneration::AddVertexLocationDecorations(InterfaceInfoGroup& infoGroup)
{
  VertexDefinitionDescription& vertexDefinitions = mTranslator->mSettings->mVertexDefinitions;

  HashMap<ShaderFieldKey, u32> vertexDefinitionLocations;
  u32 lastVertexDefinitionIndex = static_cast<u32>(vertexDefinitions.mFields.Size());
  // Find the locations of the pre-defined vertex definition settings
  for (u32 i = 0; i < static_cast<u32>(vertexDefinitions.mFields.Size()); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = vertexDefinitions.mFields[i];
    vertexDefinitionLocations[fieldMeta->MakeFieldKey()] = i;
  }

  // Decorate each field as to what location it is (just by index)
  InterfaceInfoGroup::FieldList& fieldList = infoGroup.mFields;
  for (size_t i = 0; i < fieldList.Size(); ++i)
  {
    InterfaceInfoGroup::FieldInfo& fieldInfo = fieldList[i];

    u32 location;
    constexpr u32 invalidLocation = static_cast<u32>(-1);
    // If this property is in the vertex definition settings then use that id,
    // otherwise just start appending after the last vertex definition id.
    location = vertexDefinitionLocations.FindValue(fieldInfo.mFieldMeta->MakeFieldKey(), invalidLocation);
    if (location == invalidLocation)
      location = lastVertexDefinitionIndex++;

    fieldInfo.mDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationLocation, location));
    fieldInfo.mReflectionData.mLocation = static_cast<int>(i);
  }
}

void EntryPointGeneration::AddPixelLocationDecorations(InterfaceInfoGroup& infoGroup)
{
  Lightning::StringArray& renderTargetNames = mTranslator->mSettings->mRenderTargetNames;
  Lightning::BoundType* renderTargetType = mTranslator->mSettings->mRenderTargetType;

  HashMap<ShaderFieldKey, u32> renderTargetLocations;
  u32 lastRenderTargetIndex = static_cast<u32>(renderTargetNames.Size());
  // Find the locations of the pre-defined render target names
  for (u32 i = 0; i < static_cast<u32>(renderTargetNames.Size()); ++i)
  {
    ShaderFieldKey renderTargetKey(renderTargetNames[i], renderTargetType->ToString());
    renderTargetLocations[renderTargetKey] = i;
  }

  // Decorate each field as to what location it is (just by index)
  InterfaceInfoGroup::FieldList& fieldList = infoGroup.mFields;
  for (size_t i = 0; i < fieldList.Size(); ++i)
  {
    InterfaceInfoGroup::FieldInfo& fieldInfo = fieldList[i];

    u32 location;
    constexpr u32 invalidLocation = static_cast<u32>(-1);
    // If this property is a valid render target then use that id,
    // otherwise just start appending after the last id (probably doesn't make sense here...).
    location = renderTargetLocations.FindValue(fieldInfo.mFieldMeta->MakeFieldKey(), invalidLocation);
    if(location == invalidLocation)
      location = lastRenderTargetIndex++;

    fieldInfo.mDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationLocation, location));
    fieldInfo.mReflectionData.mLocation = static_cast<u32>(i);
  }
}

void EntryPointGeneration::AddFlatDecorations(InterfaceInfoGroup& infoGroup)
{
  // Find any integer fields and decorate them to use flat interpolation
  // (required by spirv/glsl/vulkan spec)
  InterfaceInfoGroup::FieldList& fieldList = infoGroup.mFields;
  for (size_t i = 0; i < fieldList.Size(); ++i)
  {
    InterfaceInfoGroup::FieldInfo& fieldInfo = fieldList[i];
    Lightning::BoundType* lightningType = fieldInfo.mFieldMeta->mLightningType;
    if (lightningType == nullptr)
      continue;

    LightningShaderIRType* shaderType = mTranslator->mLibrary->FindType(lightningType);
    if (shaderType == nullptr)
    {
      Error("Missing shader type from lightning type");
      continue;
    }

    // If the underlying primitive type of this field is an integer.
    ShaderIRTypeBaseType::Enum basePrimitiveType = shaderType->GetBasePrimitiveType();
    if (basePrimitiveType == ShaderIRTypeBaseType::Int || basePrimitiveType == ShaderIRTypeBaseType::Bool)
      fieldInfo.mDecorations.PushBack(InterfaceInfoGroup::DecorationParam(spv::DecorationFlat));
  }
}

void EntryPointGeneration::WriteTypeDecorations(Array<InterfaceInfoGroup::DecorationParam>& decorations,
                                                BasicBlock* decorationBlock,
                                                ILightningShaderIR* toDecorate)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  // Write decoration on the block type
  for (size_t i = 0; i < decorations.Size(); ++i)
  {
    InterfaceInfoGroup::DecorationParam decoration = decorations[i];

    // Write out the basic decoration
    LightningShaderIRConstantLiteral* builtInDecoration =
        translator->GetOrCreateConstantIntegerLiteral(decoration.mDecorationType);
    LightningShaderIROp* decorationOp =
        translator->BuildIROp(decorationBlock, OpType::OpDecorate, nullptr, toDecorate, builtInDecoration, context);

    // If the decoration has additional parameters then write them out (limit 1
    // for now)
    if (decoration.mValue >= 0)
    {
      LightningShaderIRConstantLiteral* builtInKindLiteral =
          translator->GetOrCreateConstantIntegerLiteral(decoration.mValue);
      decorationOp->mArguments.PushBack(builtInKindLiteral);
    }
  }
}

void EntryPointGeneration::WriteMemberDecorations(Array<InterfaceInfoGroup::DecorationParam>& decorations,
                                                  BasicBlock* decorationBlock,
                                                  ILightningShaderIR* toDecorate,
                                                  LightningShaderIRConstantLiteral* memberIndexLiteral)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  // Write decoration on the block type
  for (size_t i = 0; i < decorations.Size(); ++i)
  {
    InterfaceInfoGroup::DecorationParam& decoration = decorations[i];

    // Write out the basic decoration
    LightningShaderIRConstantLiteral* builtInDecoration =
        translator->GetOrCreateConstantIntegerLiteral(decoration.mDecorationType);
    LightningShaderIROp* decorationOp = translator->BuildIROp(
        decorationBlock, OpType::OpMemberDecorate, nullptr, toDecorate, memberIndexLiteral, context);
    decorationOp->mArguments.PushBack(builtInDecoration);

    // If the decoration has additional parameters then write them out (limit 1
    // for now)
    if (decoration.mValue >= 0)
    {
      LightningShaderIRConstantLiteral* builtInKindLiteral =
          translator->GetOrCreateConstantIntegerLiteral(decoration.mValue);
      decorationOp->mArguments.PushBack(builtInKindLiteral);
    }
  }
}

void EntryPointGeneration::FindAndDecorateGlobals(LightningShaderIRType* currentType, EntryPointInfo* entryPointInfo)
{
  // SpirV spec section 14.5.2: uniform constants must have descriptor set and
  // binding decorations specified. Unfortunately, there's currently no way in
  // the entry point generation to know what samplers are used as they might be
  // from related fragments which this type doesn't know about (as
  // images/samplers are currently declared in each fragment). Temporarily to
  // work around this, walk the entire dependency chain of this type (same as
  // the backend) and then walk all globals which will contain all referenced
  // variables we need. This should later be moved/cached or something to avoid
  // building the dependency chain twice.
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  TypeDependencyCollector collector(currentType->mShaderLibrary);
  collector.Collect(currentType);

  DecorateImagesAndSamplers(collector, entryPointInfo);
  DecorateRuntimeArrays(collector, entryPointInfo);
}

void EntryPointGeneration::DecorateImagesAndSamplers(TypeDependencyCollector& collector, EntryPointInfo* entryPointInfo)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  ShaderStageInterfaceReflection& stageReflectionData = entryPointInfo->mStageReflectionData;

  AutoDeclare(range, collector.mReferencedGlobals.All());
  for (; !range.Empty(); range.PopFront())
  {
    // Get the base type of this global variable
    LightningShaderIROp* globalVarInstance = range.Front();
    LightningShaderIRType* opValueType = globalVarInstance->GetValueType();
    ShaderIRTypeBaseType::Enum baseType = opValueType->mBaseType;
    String resourceName = globalVarInstance->mDebugResultName;

    // Find the correct id for this variable depending on its base type
    u32 resourceBindingId = 0;
    ShaderStageResource* resourceInfo = nullptr;
    ShaderStageInterfaceReflection::SampledImageRemappings* remappings = nullptr;
    if (baseType == ShaderIRTypeBaseType::Image)
    {
      // An image might be used with a sampler or it might be a storage image.
      // We read the 'Sampled' parameter of the image type to determine which
      // one it is and add the corresponding reflection data (storage images
      // have to be bound differently so they need to be marked separately)
      LightningShaderIRImageType imageType(opValueType);
      if (imageType.IsStorageImage())
        resourceInfo = &stageReflectionData.mStorageImages.PushBack();
      else
        resourceInfo = &stageReflectionData.mImages.PushBack();

      resourceBindingId = FindBindingId();
      remappings = &stageReflectionData.mImageRemappings[resourceName];
      remappings->mImageRemappings.PushBack(resourceName);
    }
    else if (baseType == ShaderIRTypeBaseType::Sampler)
    {
      resourceBindingId = FindBindingId();
      resourceInfo = &stageReflectionData.mSamplers.PushBack();
      remappings = &stageReflectionData.mSamplerRemappings[resourceName];
      remappings->mSamplerRemappings.PushBack(resourceName);
    }
    else if (baseType == ShaderIRTypeBaseType::SampledImage)
    {
      // Sampled images are special as they use an image and sampler id.
      // We need to find an id that is empty in both the sampler and image sets.
      resourceBindingId = FindBindingId();
      resourceInfo = &stageReflectionData.mSampledImages.PushBack();
      remappings = &stageReflectionData.mSampledImageRemappings[resourceName];
      remappings->mSampledImageRemappings.PushBack(resourceName);
    }

    // Otherwise this type isn't related to images/samplers so we can skip it
    if (resourceInfo == nullptr)
      continue;

    // @JoshD: How should descriptor sets be handled?
    u32 descriptorSetId = 0;
    // Add the binding and descriptor set decorations to the instance
    BasicBlock* decorationBlock = &entryPointInfo->mDecorations;
    translator->BuildDecorationOp(
        decorationBlock, globalVarInstance, spv::DecorationBinding, resourceBindingId, context);
    translator->BuildDecorationOp(
        decorationBlock, globalVarInstance, spv::DecorationDescriptorSet, descriptorSetId, context);

    // Generate the reflection data
    resourceInfo->mReflectionData.mInstanceName = resourceName;
    resourceInfo->mReflectionData.mDescriptorSet = descriptorSetId;
    resourceInfo->mReflectionData.mLocation = resourceBindingId;
    resourceInfo->mReflectionData.mBinding = resourceBindingId;
    resourceInfo->mReflectionData.mTypeName = opValueType->mLightningType->ToString();
  }
}

void EntryPointGeneration::DecorateRuntimeArrays(TypeDependencyCollector& collector, EntryPointInfo* entryPointInfo)
{
  // Decorating runtime arrays is a little complicated and could
  // use some refactoring to avoid code duplication

  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  BasicBlock* decorationBlock = &entryPointInfo->mDecorations;

  u32 descriptorSetId = 0;
  AutoDeclare(range, collector.mReferencedGlobals.All());
  for (; !range.Empty(); range.PopFront())
  {
    // Get the base type of this global variable
    LightningShaderIROp* globalVarInstance = range.Front();
    LightningShaderIRType* opValueType = globalVarInstance->GetValueType();
    Lightning::BoundType* lightningType = opValueType->mLightningType;

    // Only visit runtime array types (the struct that wraps the actual spirv
    // runtime array)
    if (lightningType == nullptr || lightningType->TemplateBaseName != SpirVNameSettings::mRuntimeArrayTypeName)
      continue;

    // Get all of the related types of a runtime array
    LightningShaderIRType* lightningRuntimeArrayType = opValueType;
    LightningShaderIRType* spirvRuntimeArrayType = lightningRuntimeArrayType->mParameters[0]->As<LightningShaderIRType>();
    LightningShaderIRType* elementType = spirvRuntimeArrayType->mParameters[0]->As<LightningShaderIRType>();

    // Decorate the instance with the correct binding and descriptor set
    u32 bindingId = FindBindingId();
    translator->BuildDecorationOp(decorationBlock, globalVarInstance, spv::DecorationBinding, bindingId, context);
    translator->BuildDecorationOp(
        decorationBlock, globalVarInstance, spv::DecorationDescriptorSet, descriptorSetId, context);

    // Decorate the struct wrapper type. This requires
    // marking it as a block and adding the offset of the actual runtime array.
    translator->BuildDecorationOp(decorationBlock, opValueType, spv::DecorationBlock, context);
    translator->BuildMemberDecorationOp(decorationBlock, opValueType, 0, spv::DecorationOffset, 0, context);

    // Create a new reflection object for the runtime array.
    ShaderStageInterfaceReflection& stageReflectionData = entryPointInfo->mStageReflectionData;
    ShaderStageResource& stageResource = stageReflectionData.mStructedStorageBuffers.PushBack();
    ShaderResourceReflectionData& reflectionData = stageResource.mReflectionData;
    // Fill out common data like names and binding ids
    reflectionData.mInstanceName = globalVarInstance->mDebugResultName;
    reflectionData.mBinding = bindingId;
    reflectionData.mDescriptorSet = descriptorSetId;
    reflectionData.mTypeName = lightningRuntimeArrayType->mName;

    // Now actually recurse on the runtime array declarations as
    // this can be a struct of structs or other complicated things.
    AddRuntimeArrayDecorations(
        decorationBlock, lightningRuntimeArrayType, spirvRuntimeArrayType, elementType, stageResource);
  }
}

void EntryPointGeneration::AddRuntimeArrayDecorations(BasicBlock* decorationBlock,
                                                      LightningShaderIRType* lightningRuntimeArrayType,
                                                      LightningShaderIRType* spirvRuntimeArrayType,
                                                      LightningShaderIRType* elementType,
                                                      ShaderStageResource& stageResource)
{
  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  ShaderResourceReflectionData& reflectionData = stageResource.mReflectionData;

  // Walk over the element type and determine its size and required
  // alignment. Also decorate the type if required.
  u32 maxAlignment = 0;
  u32 totalSize = 0;
  switch (elementType->mBaseType)
  {
  // This is a struct, we have to recursively decorate the struct
  // (reflection won't grab nested struct data but this will generate valid
  // spirv)
  case ShaderIRTypeBaseType::Struct:
  {
    RecursivelyDecorateStructType(decorationBlock, elementType, stageResource);
    totalSize = reflectionData.mSizeInBytes;
    maxAlignment = reflectionData.mStride;
    break;
  }
  // This case should never happen as an error should already be generated.
  case ShaderIRTypeBaseType::FixedArray:
  {
    translator->SendTranslationError(nullptr, "Runtime array cannot directly contain a FixedArray.");
    maxAlignment = elementType->GetByteAlignment();
    totalSize = elementType->GetByteSize();
    break;
  }
  // Matrices are almost the same as everything else, they just need a few extra
  // declarations. In particular a matrix needs to have column/row major
  // specified and its stride.
  case ShaderIRTypeBaseType::Matrix:
  {
    maxAlignment = elementType->GetByteAlignment();
    totalSize = elementType->GetByteSize();

    // Hardcode stride to size of a vec4 for performance reasons.
    // @JoshD: Maybe make a packing option for this later?
    int matrixStride = 16;
    translator->BuildMemberDecorationOp(
        decorationBlock, lightningRuntimeArrayType, 0, spv::DecorationMatrixStride, matrixStride, context);
    translator->BuildMemberDecorationOp(decorationBlock, lightningRuntimeArrayType, 0, spv::DecorationColMajor, context);
    break;
  }
  default:
  {
    // For everything else, just get the byte alignment and size
    maxAlignment = elementType->GetByteAlignment();
    totalSize = elementType->GetByteSize();
    break;
  }
  }
  // Compute the stride of this type, making sure to pad out to the correct
  // alignment
  u32 stride = GetSizeAfterAlignment(totalSize, maxAlignment);
  reflectionData.mSizeInBytes = totalSize;
  reflectionData.mStride = stride;
  // Now we can actually decorate the runtime array's rull stride
  translator->BuildDecorationOp(decorationBlock, spirvRuntimeArrayType, spv::DecorationArrayStride, stride, context);
}

void EntryPointGeneration::RecursivelyDecorateStructType(BasicBlock* decorationBlock,
                                                         LightningShaderIRType* structType,
                                                         ShaderStageResource& stageResource)
{
  // This function is nearly a copy of another one but it operates on different
  // types. This was created to deal with runtime array, but really needs to be
  // unified at a later point in time.

  // Deal with non-struct types (makes recursion easier)
  if (structType->mBaseType != ShaderIRTypeBaseType::Struct)
    return;

  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;

  ShaderResourceReflectionData& reflectionData = stageResource.mReflectionData;

  u32 maxAlignment = 0;
  u32 currentByteOffset = 0;

  // Walk all parameters in the struct and figure out how to decorate them
  for (u32 i = 0; i < static_cast<u32>(structType->mParameters.Size()); ++i)
  {
    // Find the type of the member
    LightningShaderIRType* memberType = structType->mParameters[i]->As<LightningShaderIRType>();

    // Different types have different alignment requirements.
    // Roughly speaking, alignment is 1 float, 2 float, and 4 float.
    u32 requiredSize = memberType->GetByteSize();
    u32 requiredAlignment = memberType->GetByteAlignment();
    maxAlignment = Math::Max(requiredAlignment, maxAlignment);
    currentByteOffset = GetSizeAfterAlignment(currentByteOffset, requiredAlignment);

    // Decorate the offset of this member within the struct
    translator->BuildMemberDecorationOp(
        decorationBlock, structType, i, spv::DecorationOffset, currentByteOffset, context);

    // Store the offset and size of this field in the reflection data
    ShaderResourceReflectionData& fieldReflectionData = stageResource.mMembers.PushBack();
    fieldReflectionData.mTypeName = memberType->mName;
    fieldReflectionData.mInstanceName = structType->GetMemberName(i);
    fieldReflectionData.mOffsetInBytes = currentByteOffset;
    fieldReflectionData.mSizeInBytes = requiredSize;

    // Matrices require the row/column decoration and the stride
    if (memberType->mBaseType == ShaderIRTypeBaseType::Matrix)
    {
      // Hardcode stride to size of a vec4 for performance reasons.
      // @JoshD: Maybe make a packing option for this later?
      u32 matrixStride = 16;
      translator->BuildMemberDecorationOp(
          decorationBlock, structType, i, spv::DecorationMatrixStride, matrixStride, context);
      translator->BuildMemberDecorationOp(decorationBlock, structType, i, spv::DecorationColMajor, context);
      fieldReflectionData.mStride = matrixStride;
    }
    // Structs have to be recursively decorated.
    else if (memberType->mBaseType == ShaderIRTypeBaseType::Struct)
    {
      // @JoshD: Currently stage resources don't support nested structures.
      // This will generate valid spir-v but eventually needs to be updated to
      // support proper reflection data.
      ShaderStageResource subData;
      RecursivelyDecorateStructType(decorationBlock, memberType, subData);
      fieldReflectionData.mStride = subData.mReflectionData.mStride;
      fieldReflectionData.mSizeInBytes = subData.mReflectionData.mSizeInBytes;
    }
    // Fixed arrays need the array stride decoration.
    // Additionally all recursive types might need to be decorated.
    else if (memberType->mBaseType == ShaderIRTypeBaseType::FixedArray)
    {
      // Get the stride of the element type (everything less than
      // 16 bytes has to be padded up to 16 bytes in a fixed array)
      LightningShaderIRType* elementType = memberType->mParameters[0]->As<LightningShaderIRType>();
      u32 stride = GetStride(elementType, 16.0f);
      translator->BuildDecorationOp(decorationBlock, memberType, spv::DecorationArrayStride, stride, context);

      // Recursively decorate the contained type. If this is a
      // struct it could have further requirements about alignment, etc...
      // @JoshD: At some point the reflection data needs to be dealt with here.
      ShaderStageResource subData;
      RecursivelyDecorateStructType(decorationBlock, elementType, subData);
    }

    // Increment the current offset by the size of this type (not alignment)
    currentByteOffset += requiredSize;
  }
  size_t totalSize = currentByteOffset;

  // Compute the stride of this type, making sure to pad out the the correct
  // alignment.
  int stride = GetSizeAfterAlignment(totalSize, maxAlignment);
  reflectionData.mSizeInBytes = totalSize;
  reflectionData.mStride = stride;
}

u32 EntryPointGeneration::FindBindingId()
{
  // Find the first unused id in the map
  u32 id = 0;
  while (true)
  {
    if(!mUsedBindingIds.Contains(id))
    {
      mUsedBindingIds.Insert(id);
      return id;
    }
    ++id;
  }
  Error("This should never happen");
  return 0;
}

void EntryPointGeneration::CopyReflectionDataToEntryPoint(EntryPointInfo* entryPointInfo,
                                                          ShaderInterfaceInfo& interfaceInfo)
{
  ShaderStageInterfaceReflection& reflection = entryPointInfo->mStageReflectionData;
  // Save the type name
  reflection.mShaderTypeName = mContext->mCurrentType->mMeta->mLightningName;

  AutoDeclare(uniformRange, interfaceInfo.mUniformGroups.All());
  for (; !uniformRange.Empty(); uniformRange.PopFront())
  {
    InterfaceInfoGroup& group = uniformRange.Front().second;
    CopyReflectionData(reflection.mUniforms, interfaceInfo, group);
  }

  CopyReflectionData(reflection.mStageInputs, interfaceInfo, interfaceInfo.mInputs);
  CopyReflectionData(reflection.mStageOutputs, interfaceInfo, interfaceInfo.mOutputs);
}

void EntryPointGeneration::CopyReflectionData(Array<ShaderStageResource>& resourceList,
                                              ShaderInterfaceInfo& interfaceInfo,
                                              InterfaceInfoGroup& group)
{
  if (group.mIsStruct)
    CopyReflectionDataStruct(resourceList, interfaceInfo, group);
  else
    CopyReflectionDataGlobals(resourceList, interfaceInfo, interfaceInfo.mInputs);
}

void EntryPointGeneration::CopyReflectionDataStruct(Array<ShaderStageResource>& resourceList,
                                                    ShaderInterfaceInfo& interfaceInfo,
                                                    InterfaceInfoGroup& group)
{
  // Create one new stage resource for the entire group
  ShaderStageResource& stageResource = resourceList.PushBack();
  stageResource.mReflectionData = group.mReflectionData;
  // Copy each field over
  for (size_t i = 0; i < group.mFields.Size(); ++i)
  {
    InterfaceInfoGroup::FieldInfo& fieldInfo = group.mFields[i];
    String fieldTypeName = fieldInfo.mFieldMeta->mLightningName;
    ShaderResourceReflectionData& fieldReflection = stageResource.mMembers.PushBack();
    fieldReflection = fieldInfo.mReflectionData;
    stageResource.mLookupMap[fieldTypeName] = i;
  }
}

void EntryPointGeneration::CopyReflectionDataGlobals(Array<ShaderStageResource>& resourceList,
                                                     ShaderInterfaceInfo& interfaceInfo,
                                                     InterfaceInfoGroup& group)
{
  // Copy each field to a separate stage resource
  for (size_t i = 0; i < group.mFields.Size(); ++i)
  {
    ShaderStageResource& stageResource = resourceList.PushBack();
    stageResource.mReflectionData = group.mFields[i].mReflectionData;
  }
}

void EntryPointGeneration::CreateShaderInterfaceField(ShaderInterfaceField& interfaceField,
                                                      InterfaceInfoGroup& interfaceGroup,
                                                      u32 index)
{
  // @JoshD: Potentially make the interface group store shader interface fields
  // at some point?

  LightningSpirVFrontEnd* translator = mTranslator;
  LightningSpirVFrontEndContext* context = mContext;
  LightningShaderIRLibrary* library = translator->mLibrary;
  spv::StorageClass storageClass = interfaceGroup.mStorageClass;

  // Get the field we're being created from
  InterfaceInfoGroup::FieldInfo& groupField = interfaceGroup.mFields[index];
  ShaderIRFieldMeta* fieldMeta = groupField.mFieldMeta;
  // Clone the field meta so this is a unique instance
  interfaceField.mFieldMeta = fieldMeta->Clone(translator->mLibrary);

  // Get the actual field type to use for creating the shader instance (e.g.
  // bool -> int). If this is a built-in keep the original type.
  Lightning::BoundType* originalLightningType = fieldMeta->mLightningType;

  Lightning::BoundType* actualLightningType = originalLightningType;
  if (!interfaceGroup.mIsBuiltIn)
    actualLightningType = ConvertInterfaceType(fieldMeta->mLightningType);

  // If the field type changed, append the name with the original type (prevents
  // name conflicts)
  interfaceField.mFieldName = fieldMeta->mLightningName;
  if (actualLightningType != fieldMeta->mLightningType)
    interfaceField.mFieldName = BuildString(interfaceField.mFieldName, "_", fieldMeta->mLightningType->ToString());

  // Store reference information about where the field lives and who it maps to
  interfaceField.mFieldIndex = index;
  interfaceField.mLinkedFields = groupField.mLinkedFields;

  // Mark the actual type of this field and create its interface type (interface
  // types require unique storage class)
  interfaceField.mFieldType =
      translator->FindOrCreateInterfaceType(library, actualLightningType, ShaderIRTypeBaseType::Pointer, storageClass);
  interfaceField.mFieldType->mDereferenceType = translator->FindType(actualLightningType, nullptr, true);

  // Also mark the original lightning type in case this changes due to interfaces
  // (needed to know how to map things like bool to int)
  interfaceField.mOriginalFieldType =
      translator->FindOrCreateInterfaceType(library, originalLightningType, ShaderIRTypeBaseType::Pointer, storageClass);
  interfaceField.mOriginalFieldType->mDereferenceType = translator->FindType(originalLightningType, nullptr, true);
}

Lightning::BoundType* EntryPointGeneration::ConvertInterfaceType(Lightning::BoundType* inputType)
{
  if (inputType == LightningTypeId(bool))
    return LightningTypeId(int);
  else if (inputType == LightningTypeId(Lightning::Boolean2))
    return LightningTypeId(Lightning::Integer2);
  else if (inputType == LightningTypeId(Lightning::Boolean3))
    return LightningTypeId(Lightning::Integer3);
  else if (inputType == LightningTypeId(Lightning::Boolean4))
    return LightningTypeId(Lightning::Integer4);
  return inputType;
}

void EntryPointGeneration::WriteExecutionModeOriginUpperLeft(EntryPointInfo* entryPointInfo)
{
  BasicBlock* block = &entryPointInfo->mExecutionModes;
  LightningShaderIRConstantLiteral* literalOp =
      mTranslator->GetOrCreateConstantIntegerLiteral(spv::ExecutionModeOriginUpperLeft);
  mTranslator->BuildIROp(block, OpType::OpExecutionMode, nullptr, entryPointInfo->mEntryPointFn, literalOp, mContext);
}

LightningShaderIROp* EntryPointGeneration::FindField(ShaderFieldKey& fieldKey,
                                                 Array<ShaderInterfaceType*>& interfaces,
                                                 BasicBlock* block,
                                                 spv::StorageClass storageClass)
{
  // Walk all interfaces trying
  for (size_t i = 0; i < interfaces.Size(); ++i)
  {
    ShaderInterfaceType* interface = interfaces[i];

    // Walk all fields on this interface
    size_t count = interface->GetFieldCount();
    for (size_t j = 0; j < count; ++j)
    {
      // If the interface field matches the field key then return the op code
      // that points at the memory of the field
      ShaderInterfaceField* field = interface->GetFieldAtIndex(j);
      if (field->mFieldMeta->MakeFieldKey() == fieldKey)
      {
        return interface->GetFieldPointerByIndex(j, this, block, storageClass);
      }
    }
  }
  // We failed to find the given field
  return nullptr;
}

ShaderIRFieldMeta* EntryPointGeneration::FindFieldViaAttribute(ShaderIRTypeMeta* typeMeta,
                                                               StringParam attributeName,
                                                               ShaderFieldKey& fieldKey)
{
  SpirVNameSettings& nameSettings = mTranslator->mSettings->mNameSettings;
  for (size_t i = 0; i < typeMeta->mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[i];
    for (size_t j = 0; j < fieldMeta->mAttributes.Size(); ++j)
    {
      ShaderIRAttribute* attribute = fieldMeta->mAttributes.GetAtIndex(j);
      if (attribute->mAttributeName == attributeName && fieldMeta->MakeFieldKey(attribute) == fieldKey)
        return fieldMeta;
    }
  }
  return nullptr;
}

void EntryPointGeneration::PerspectiveTransformAppendVertexCallback(AppendCallbackData& callbackData, void* userData)
{
  SpirVNameSettings& nameSettings = callbackData.mSettings->mNameSettings;
  GeometryAppendFunctionData* appendFnData = callbackData.mAppendFnData;
  EntryPointGeneration* self = callbackData.mEntryPointGeneration;
  BasicBlock* block = appendFnData->mBlock;
  LightningShaderIROp* outDataInstance = appendFnData->mOutputDataInstance;

  String real4TypeName = LightningTypeId(Lightning::Real4)->Name;
  String real4x4TypeName = LightningTypeId(Lightning::Real4x4)->Name;

  // Find the perspective position field meta on the output vertex type.
  // This has to search all available stage output attributes to deal with name
  // overrides.
  ShaderFieldKey perspectivePositionKey("PerspectivePosition", real4TypeName);
  LightningShaderIRType* appendVertexDataType = outDataInstance->mResultType->mDereferenceType;
  ShaderIRFieldMeta* perspectivePositionFieldMeta = self->FindFieldViaAttribute(
      appendVertexDataType->mMeta, nameSettings.mStageOutputAttribute, perspectivePositionKey);
  if (perspectivePositionFieldMeta == nullptr)
    return;

  // Find the perspective position field pointer on the output vertex type
  u32* index = appendVertexDataType->mMemberKeysToIndex.FindPointer(perspectivePositionFieldMeta->MakeFieldKey());

  // Find the api perspective position field pointer from the output interface
  // types (should find the hardware built-in interface block)
  ShaderFieldKey apiPerspectivePositionKey(nameSettings.mApiPerspectivePositionName, real4TypeName);
  LightningShaderIROp* apiPerspectivePosition = self->FindField(
      apiPerspectivePositionKey, *callbackData.mOutputVertexInterfaceTypes, block, spv::StorageClassOutput);

  // Find the api perspective transform matrix from all of the uniform buffers
  // available
  ShaderFieldKey perspectiveTransformKey(nameSettings.mPerspectiveToApiPerspectiveName, real4x4TypeName);
  LightningShaderIROp* transformMatrixOp =
      self->FindField(perspectiveTransformKey, self->mUniforms, block, spv::StorageClassUniform);

  // If we have all three variables then we can write a transform
  if (apiPerspectivePosition != nullptr && transformMatrixOp != nullptr && index != nullptr)
  {
    // Get the instance pointer to perspective position
    LightningShaderIROp* source = self->GetMemberInstanceFrom(block, outDataInstance, *index, spv::StorageClassFunction);

    // Matrix multiplication needs value types not pointers, so make sure to
    // dereference if necessary
    LightningShaderIROp* valueMatrix =
        self->mTranslator->GetOrGenerateValueTypeFromIR(block, transformMatrixOp, self->mContext);
    LightningShaderIROp* valueVector = self->mTranslator->GetOrGenerateValueTypeFromIR(block, source, self->mContext);
    // Get the result type of the transform (should always be Real4)
    LightningShaderIRType* resultType = apiPerspectivePosition->mResultType->mDereferenceType;
    // Write Multiply(PerspectiveToApiPerspective, PerspectivePosition)
    LightningShaderIROp* result = self->mTranslator->BuildIROp(
        block, OpType::OpMatrixTimesVector, resultType, valueMatrix, valueVector, self->mContext);
    // Store the result into the final ApiPerspectivePosition output
    self->mTranslator->BuildStoreOp(block, apiPerspectivePosition, result, self->mContext);
  }
}

} // namespace Plasma
