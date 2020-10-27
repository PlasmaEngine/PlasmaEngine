// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void FixedArrayDefaultConstructor(LightningSpirVFrontEnd* translator,
                                  Lightning::Type* resultType,
                                  LightningSpirVFrontEndContext* context)
{
  // Just build a variable op for the given fixed array type
  LightningShaderIRType* shaderType = translator->FindType(resultType, nullptr);
  context->PushIRStack(translator->BuildOpVariable(shaderType->mPointerType, context));
}

void FixedArrayBackupConstructor(LightningSpirVFrontEnd* translator,
                                 Lightning::FunctionCallNode* fnCallNode,
                                 Lightning::StaticTypeNode* staticTypeNode,
                                 LightningSpirVFrontEndContext* context)
{
  FixedArrayDefaultConstructor(translator, staticTypeNode->ResultType, context);
}

bool ValidateIndexLiteral(LightningSpirVFrontEnd* translator,
                          Lightning::ExpressionNode* node,
                          LightningShaderIROp* indexOperand,
                          u32 maxValue,
                          LightningSpirVFrontEndContext* context)
{
  // Check if this is a constant (literal). If not we can't validate.
  if (indexOperand->mOpType != OpType::OpConstant)
    return true;

  // Get the literal value
  LightningShaderIRConstantLiteral* literal = indexOperand->mArguments[0]->As<LightningShaderIRConstantLiteral>();
  u32 indexLiteral = literal->mValue.Get<u32>();
  // If it's in the valid range then this index is valid;
  if (0 <= indexLiteral && indexLiteral < maxValue)
    return true;

  // Otherwise the value is invalid so report an error and emit a dummy op-code
  String message = String::Format("Invalid index. Index can only be in the range of [0, %d)", maxValue);
  translator->SendTranslationError(node->Location, message);
  context->PushIRStack(translator->GenerateDummyIR(node, context));
  return false;
}

void ResolveFixedArrayGet(LightningSpirVFrontEnd* translator,
                          Lightning::FunctionCallNode* functionCallNode,
                          Lightning::MemberAccessNode* memberAccessNode,
                          LightningSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type
  Lightning::Type* lightningArrayType = memberAccessNode->LeftOperand->ResultType;
  LightningShaderIRType* arrayType = translator->FindType(lightningArrayType, memberAccessNode->LeftOperand);
  LightningShaderIRType* elementType = arrayType->mParameters[0]->As<LightningShaderIRType>();

  // Get the index operator (must be a value type)
  ILightningShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  LightningShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Validate the index if it's a literal
  if (!ValidateIndexLiteral(translator, functionCallNode, indexOperand, arrayType->mComponents, context))
    return;

  // Generate the access chain to get the element within the array
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  ILightningShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, elementType->mPointerType, selfInstance, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveFixedArraySet(LightningSpirVFrontEnd* translator,
                          Lightning::FunctionCallNode* functionCallNode,
                          Lightning::MemberAccessNode* memberAccessNode,
                          LightningSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type
  Lightning::Type* lightningArrayType = memberAccessNode->LeftOperand->ResultType;
  LightningShaderIRType* arrayType = translator->FindType(lightningArrayType, memberAccessNode->LeftOperand);
  LightningShaderIRType* elementType = arrayType->mParameters[0]->As<LightningShaderIRType>();

  // Get the index operator (must be a value type)
  ILightningShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  LightningShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Validate the index if it's a literal
  if (!ValidateIndexLiteral(translator, functionCallNode, indexOperand, arrayType->mComponents, context))
    return;

  // Generate the access chain to get the element within the array
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  ILightningShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, elementType->mPointerType, selfInstance, indexOperand, context);

  // Get the source value
  ILightningShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void ResolveFixedArrayCount(LightningSpirVFrontEnd* translator,
                            Lightning::FunctionCallNode* functionCallNode,
                            Lightning::MemberAccessNode* memberAccessNode,
                            LightningSpirVFrontEndContext* context)
{
  // Return the integer constant expression as the array count (could call the
  // intrinsic for count but why not use this)
  Lightning::Type* lightningArrayType = memberAccessNode->LeftOperand->ResultType;
  LightningShaderIRType* arrayType = translator->FindType(lightningArrayType, memberAccessNode->LeftOperand);
  context->PushIRStack(arrayType->mParameters[1]);
}

void FixedArrayExpressionInitializerResolver(LightningSpirVFrontEnd* translator,
                                             Lightning::ExpressionInitializerNode*& node,
                                             LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* fixedArrayType = translator->FindType(node->ResultType, node);

  // Validate that the initializer list has the exact number of expected
  // arguments
  size_t statementCount = node->InitializerStatements.Size();
  if (fixedArrayType->mComponents != statementCount)
  {
    String errMsg = String::Format(
        "Array initializer was given %d item(s) and it expected %d.", statementCount, fixedArrayType->mComponents);
    translator->SendTranslationError(node->Location, errMsg);
    context->PushIRStack(translator->GenerateDummyIR(node, context));
    return;
  }

  // Start constructing the array by walking all of the arguments
  LightningShaderIROp* compositeConstructOp =
      translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, fixedArrayType, context);
  for (size_t i = 0; i < node->InitializerStatements.Size(); ++i)
  {
    Lightning::ExpressionNode* expNode = node->InitializerStatements[i];

    String errorMessage;
    Lightning::ExpressionNode* argumentValueNode = nullptr;

    // Verify that this statement is a call to OperatorInsert (.Add). If it is
    // then walk the argument to get the initial value we set (required to be a
    // value type).
    Lightning::FunctionCallNode* addCallNode = Lightning::Type::DynamicCast<Lightning::FunctionCallNode*>(expNode);
    if (addCallNode != nullptr)
    {
      Lightning::MemberAccessNode* memberAccessNode =
          Lightning::Type::DynamicCast<Lightning::MemberAccessNode*>(addCallNode->LeftOperand);
      if (memberAccessNode != nullptr && memberAccessNode->Name == Lightning::OperatorInsert)
        argumentValueNode = addCallNode->Arguments[0];
      else
        errorMessage = "Array initializer lists can only contain insertion "
                       "operator sub-expressions";
    }
    else
    {
      errorMessage = "It's invalid to have a member initializer statement in "
                     "an array initializer";
    }

    // If we got a valid node then translate it, otherwise send an error and
    // generate a dummy variable
    ILightningShaderIR* argument = nullptr;
    if (argumentValueNode != nullptr)
    {
      argument = translator->WalkAndGetValueTypeResult(addCallNode->Arguments[0], context);
    }
    else
    {
      translator->SendTranslationError(expNode->Location, errorMessage);
      argument = translator->GenerateDummyIR(expNode, context);
    }

    compositeConstructOp->mArguments.PushBack(argument);
  }

  context->mCurrentBlock->AddOp(compositeConstructOp);
  context->PushIRStack(compositeConstructOp);

  // If the initializer list was put on a variable then we're just
  // attempting to set all of the values in a compact format.
  Lightning::LocalVariableNode* localVariableNode = Lightning::Type::DynamicCast<Lightning::LocalVariableNode*>(node->LeftOperand);
  if (localVariableNode == nullptr)
    return;

  // In this case, the initial value of the initializer node will be the
  // target variable to copy to. If this is a function call node then the this
  // is either the result from a function or a constructor call, both of which
  // require no copy back.
  Lightning::FunctionCallNode* functionCallNode =
      Lightning::Type::DynamicCast<Lightning::FunctionCallNode*>(localVariableNode->InitialValue);
  if (functionCallNode != nullptr)
    return;

  // Otherwise, this is should be either a local variable reference
  // or a member access (e.g. 'this.Array'). In either case this represents
  // where to store back. As long as the result of the initial value is
  // actually a pointer type then we can store to it, otherwise something
  // odd is happening (like array{1, 2} {1, 2}) so we report an error.
  ILightningShaderIR* target = translator->WalkAndGetResult(localVariableNode->InitialValue, context);
  LightningShaderIROp* targetOp = target->As<LightningShaderIROp>();
  if (!targetOp->IsResultPointerType())
  {
    translator->SendTranslationError(node->Location,
                                     "Invalid array initializer list. The left "
                                     "hand side must be an l-value");
    context->PushIRStack(translator->GenerateDummyIR(node, context));
    return;
  }

  translator->BuildStoreOp(targetOp, compositeConstructOp, context);
}

void FixedArrayResolver(LightningSpirVFrontEnd* translator, Lightning::BoundType* lightningFixedArrayType)
{
  LightningShaderIRLibrary* shaderLibrary = translator->mLibrary;

  // Get the template arguments
  Lightning::Type* lightningElementType = lightningFixedArrayType->TemplateArguments[0].TypeValue;
  // Deal with nested template types that haven't already been resolved
  Lightning::BoundType* lightningElementBoundType = Lightning::Type::GetBoundType(lightningElementType);
  if (!lightningElementBoundType->TemplateBaseName.Empty())
    translator->PreWalkTemplateType(lightningElementBoundType, translator->mContext);
  LightningShaderIRType* elementType = translator->FindType(lightningElementType, nullptr);
  int length = (int)lightningFixedArrayType->TemplateArguments[1].IntegerValue;

  // Create the array type
  LightningShaderIRType* fixedArrayType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                     ShaderIRTypeBaseType::FixedArray,
                                                                     lightningFixedArrayType->Name,
                                                                     lightningFixedArrayType,
                                                                     spv::StorageClassFunction);

  // Set the parameters (and currently components for convenience)
  fixedArrayType->mComponents = length;
  fixedArrayType->mParameters.PushBack(elementType);
  // The length has to be an integer constant, not literal
  fixedArrayType->mParameters.PushBack(translator->GetIntegerConstant(length, translator->mContext));
  translator->MakeShaderTypeMeta(fixedArrayType, nullptr);

  Lightning::BoundType* intType = LightningTypeId(int);
  String intTypeName = intType->Name;

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[lightningFixedArrayType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningFixedArrayType, Lightning::OperatorGet, intTypeName), ResolveFixedArrayGet);
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningFixedArrayType, Lightning::OperatorSet, intTypeName, lightningElementType->ToString()),
      ResolveFixedArraySet);
  typeResolver.RegisterFunctionResolver(GetInstanceProperty(lightningFixedArrayType, "Count")->Get, ResolveFixedArrayCount);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

void ResolveRuntimeArrayGet(LightningSpirVFrontEnd* translator,
                            Lightning::FunctionCallNode* functionCallNode,
                            Lightning::MemberAccessNode* memberAccessNode,
                            LightningSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type (from the containing struct
  // type)
  Lightning::Type* lightningArrayType = memberAccessNode->LeftOperand->ResultType;
  LightningShaderIRType* structArrayType = translator->FindType(lightningArrayType, memberAccessNode->LeftOperand);
  LightningShaderIRType* spirvArrayType = structArrayType->mParameters[0]->As<LightningShaderIRType>();
  LightningShaderIRType* elementType = spirvArrayType->mParameters[0]->As<LightningShaderIRType>();

  // Get the index operator (must be a value type)
  ILightningShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  LightningShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Walk the left operand to get the wrapper struct type
  LightningShaderIROp* constant0 = translator->GetIntegerConstant(0, context);
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);

  // To get the actual data we'll do a double access chain. Argument 0 will
  // be the index to get the spirv runtime array (always index 0),
  // then argument 1 will be the index into the array.
  // Note: We have to do a special access chain so the result type is of the
  // correct storage class (uniform)
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  ILightningShaderIR* accessChainOp =
      translator->BuildCurrentBlockAccessChain(elementType, selfInstance, constant0, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveRuntimeArraySet(LightningSpirVFrontEnd* translator,
                            Lightning::FunctionCallNode* functionCallNode,
                            Lightning::MemberAccessNode* memberAccessNode,
                            LightningSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type
  Lightning::Type* lightningArrayType = memberAccessNode->LeftOperand->ResultType;
  LightningShaderIRType* structArrayType = translator->FindType(lightningArrayType, memberAccessNode->LeftOperand);
  LightningShaderIRType* spirvArrayType = structArrayType->mParameters[0]->As<LightningShaderIRType>();
  LightningShaderIRType* elementType = spirvArrayType->mParameters[0]->As<LightningShaderIRType>();

  // Get the index operator (must be a value type)
  ILightningShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  LightningShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the array
  LightningShaderIROp* constant0 = translator->GetIntegerConstant(0, context);
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  ILightningShaderIR* accessChainOp =
      translator->BuildCurrentBlockAccessChain(elementType, selfInstance, constant0, indexOperand, context);

  // Get the source value
  ILightningShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void ResolveRuntimeArrayCount(LightningSpirVFrontEnd* translator,
                              Lightning::FunctionCallNode* functionCallNode,
                              Lightning::MemberAccessNode* memberAccessNode,
                              LightningSpirVFrontEndContext* context)
{
  // The runtime array length instruction is a bit odd as it requires the struct
  // the array is contained in as well as the member index offset into the
  // struct for where the runtime array is actually contained.
  LightningShaderIRType* intType = translator->FindType(LightningTypeId(int), functionCallNode);
  LightningShaderIRType* uintType = translator->FindType(LightningTypeId(Lightning::UnsignedInt), functionCallNode);
  // Get the wrapper struct instance that contains the real runtime array
  ILightningShaderIR* structOwnerOp = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  // We create the runtime array wrapper struct such that the real array is
  // always at member index 0
  LightningShaderIRConstantLiteral* plasmaLiteral = translator->GetOrCreateConstantIntegerLiteral(0);
  ILightningShaderIR* uintLengthResult = translator->BuildCurrentBlockIROp(OpType::OpArrayLength, uintType, structOwnerOp, plasmaLiteral, context);
  ILightningShaderIR* intLengthResult = translator->BuildCurrentBlockIROp(OpType::OpArrayLength, intType, uintLengthResult, context);
  context->PushIRStack(intLengthResult);
}

void RuntimeArrayResolver(LightningSpirVFrontEnd* translator, Lightning::BoundType* lightningRuntimeArrayType)
{
  // Runtime arrays have to be created in an odd way. Per the Vulkan spec,
  // OpTypeRuntimeArray must only be used for the last member of an
  // OpTypeStruct. That is, a runtime array must be declared as (glsl sample):
  // buffer StructTypeName
  //{
  //  float ActualRuntimeArray[];
  //} InstanceVarName;
  // To do this the lightning runtime array is translated as the wrapper struct
  // that contains the spirv runtime array since all op codes must go through
  // the struct's instance variable.

  LightningShaderIRLibrary* shaderLibrary = translator->mLibrary;

  // Get the template arguments
  Lightning::Type* lightningElementType = lightningRuntimeArrayType->TemplateArguments[0].TypeValue;
  // Deal with nested template types that haven't already been resolved
  Lightning::BoundType* lightningElementBoundType = Lightning::Type::GetBoundType(lightningElementType);
  if (!lightningElementBoundType->TemplateBaseName.Empty())
    translator->PreWalkTemplateType(lightningElementBoundType, translator->mContext);

  LightningShaderIRType* elementType = translator->FindType(lightningElementType, nullptr);

  // The name of the wrapper struct type name must match the lightning runtime
  // array type name since the front end will be searching for the type by
  // this name. Make the internal runtime array type name something unique.
  String lightningTypeName = lightningRuntimeArrayType->Name;
  String internalArrayName = BuildString("SpirV", lightningTypeName);

  // Create the true runtime array type
  LightningShaderIRType* runtimeArrayType = translator->MakeTypeAndPointer(
      shaderLibrary, ShaderIRTypeBaseType::RuntimeArray, internalArrayName, nullptr, spv::StorageClassStorageBuffer);
  runtimeArrayType->mParameters.PushBack(elementType);
  translator->MakeShaderTypeMeta(runtimeArrayType, nullptr);

  // Now generate the wrapper struct around the runtime array
  LightningShaderIRType* wrapperStructType =
      translator->MakeStructType(shaderLibrary, lightningTypeName, lightningRuntimeArrayType, spv::StorageClassStorageBuffer);
  wrapperStructType->AddMember(runtimeArrayType, "Data");
  // Always use the actual type name with "Buffer" appended for the wrapper type
  // name
  wrapperStructType->mDebugResultName = BuildString(lightningTypeName, "Buffer");
  translator->MakeShaderTypeMeta(wrapperStructType, nullptr);

  Lightning::BoundType* intType = LightningTypeId(int);
  String intTypeName = intType->Name;

  // Register resolvers for the few functions we care about.
  // Note: Add is illegal since this is provided by the client
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[lightningRuntimeArrayType];
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningRuntimeArrayType, Lightning::OperatorGet, intTypeName), ResolveRuntimeArrayGet);
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningRuntimeArrayType, Lightning::OperatorSet, intTypeName, lightningElementType->ToString()),
      ResolveRuntimeArraySet);
  typeResolver.RegisterFunctionResolver(GetInstanceProperty(lightningRuntimeArrayType, "Count")->Get,
                                        ResolveRuntimeArrayCount);
}

void GeometryStreamInputResolver(LightningSpirVFrontEnd* translator, Lightning::BoundType* lightningInputStreamType)
{
  LightningShaderIRLibrary* shaderLibrary = translator->mLibrary;
  LightningSpirVFrontEndContext* context = translator->mContext;

  Lightning::GeometryStreamUserData* streamUserData = lightningInputStreamType->Has<Lightning::GeometryStreamUserData>();

  // Get the template arguments
  Lightning::Type* lightningElementType = lightningInputStreamType->TemplateArguments[0].TypeValue;
  LightningShaderIRType* elementType = translator->FindType(lightningElementType, nullptr);
  int length = streamUserData->mSize;

  // Create the array type
  LightningShaderIRType* inputStreamType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                      ShaderIRTypeBaseType::FixedArray,
                                                                      lightningInputStreamType->Name,
                                                                      lightningInputStreamType,
                                                                      spv::StorageClassFunction);
  // Set the parameters (and currently components for convenience)
  inputStreamType->mComponents = length;
  inputStreamType->mParameters.PushBack(elementType);
  // The length has to be an integer constant, not literal
  inputStreamType->mParameters.PushBack(translator->GetIntegerConstant(length, context));
  translator->MakeShaderTypeMeta(inputStreamType, nullptr);

  Lightning::BoundType* lightningIntType = LightningTypeId(int);
  String intTypeName = lightningIntType->Name;

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[lightningInputStreamType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningInputStreamType, Lightning::OperatorGet, intTypeName), ResolveFixedArrayGet);
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(lightningInputStreamType, Lightning::OperatorSet, intTypeName, lightningElementType->ToString()),
      ResolveFixedArraySet);
  typeResolver.RegisterFunctionResolver(GetInstanceProperty(lightningInputStreamType, "Count")->Get,
                                        ResolveFixedArrayCount);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

void OutputStreamRestart(LightningSpirVFrontEnd* translator,
                         Lightning::FunctionCallNode* functionCallNode,
                         Lightning::MemberAccessNode* memberAccessNode,
                         LightningSpirVFrontEndContext* context)
{
  translator->BuildCurrentBlockIROp(OpType::OpEndPrimitive, nullptr, context);
}

void GeometryStreamOutputResolver(LightningSpirVFrontEnd* translator, Lightning::BoundType* lightningFixedArrayType)
{
  LightningShaderIRLibrary* shaderLibrary = translator->mLibrary;
  LightningSpirVFrontEndContext* context = translator->mContext;

  // Get the template arguments
  Lightning::Type* lightningElementType = lightningFixedArrayType->TemplateArguments[0].TypeValue;
  LightningShaderIRType* elementType = translator->FindType(lightningElementType, nullptr);

  Lightning::BoundType* lightningIntType = LightningTypeId(int);
  String intTypeName = lightningIntType->Name;
  LightningShaderIRType* intType = translator->mLibrary->FindType(lightningIntType);

  // Create the array type
  LightningShaderIRType* fixedArrayType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                     ShaderIRTypeBaseType::Struct,
                                                                     lightningFixedArrayType->Name,
                                                                     lightningFixedArrayType,
                                                                     spv::StorageClassFunction);
  fixedArrayType->mDebugResultName = "OutputStream";
  // Add the element type
  fixedArrayType->AddMember(elementType, "Output");
  translator->MakeShaderTypeMeta(fixedArrayType, nullptr);

  // Create the append function. We need this to be an actual function that will
  // be late bound later via the entry point.
  Lightning::Function* lightningAppendFn =
      GetMemberOverloadedFunction(lightningFixedArrayType, "Append", lightningElementType->ToString(), intTypeName);
  LightningShaderIRFunction* appendFn =
      translator->GenerateIRFunction(nullptr, nullptr, fixedArrayType, lightningAppendFn, lightningAppendFn->Name, context);
  // Add the parameters for the function
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, fixedArrayType->mPointerType, context)
      ->mDebugResultName = "stream";
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, elementType, context)
      ->mDebugResultName = "outputData";
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, intType, context)->mDebugResultName =
      "vertexId";
  // Make this a valid function by adding the first block with a return
  // statement
  BasicBlock* firstBlock = translator->BuildBlockNoStack(String(), context);
  appendFn->mBlocks.PushBack(firstBlock);
  translator->BuildIROp(firstBlock, OpType::OpReturn, nullptr, context);

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[lightningFixedArrayType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(lightningFixedArrayType, "Restart"),
                                        OutputStreamRestart);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

} // namespace Plasma
