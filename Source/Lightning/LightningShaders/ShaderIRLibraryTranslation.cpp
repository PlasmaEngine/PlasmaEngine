// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void UnTranslatedBoundFunction(Lightning::Call& call, Lightning::ExceptionReport& report)
{
  // This function should never be called via lightning. This is a function that
  // cannot (or hasn't yet) been given an actual implementation and is only used
  // for binding purposes.
  call.GetState()->ThrowException("Un-translatable function was called.");
  Error("Un-translatable function was called.");
}

void DummyBoundFunction(Lightning::Call& call, Lightning::ExceptionReport& report)
{
  // Set the return value to a default constructed type (plasma)
  Lightning::DelegateType* functionType = call.GetFunction()->FunctionType;
  Lightning::Type* returnType = functionType->Return;
  size_t byteSize = returnType->GetAllocatedSize();
  byte* returnValue = call.GetReturnUnchecked();
  memset(returnValue, 0, byteSize);
  call.MarkReturnAsSet();
}

void ResolveSimpleFunctionFromOpType(LightningSpirVFrontEnd* translator,
                                     Lightning::FunctionCallNode* functionCallNode,
                                     Lightning::MemberAccessNode* memberAccessNode,
                                     OpType opType,
                                     LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  LightningShaderIROp* result = translator->BuildIROpNoBlockAdd(opType, resultType, context);
  for (size_t i = 0; i < functionCallNode->Arguments.Size(); ++i)
  {
    LightningShaderIROp* arg = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[i], context);
    result->mArguments.PushBack(arg);
  }
  context->GetCurrentBlock()->AddOp(result);
  context->PushIRStack(result);
}

void ResolveVectorTypeCount(LightningSpirVFrontEnd* translator,
                            Lightning::FunctionCallNode* functionCallNode,
                            Lightning::MemberAccessNode* memberAccessNode,
                            LightningSpirVFrontEndContext* context)
{
  Lightning::Type* selfType = memberAccessNode->LeftOperand->ResultType;
  LightningShaderIRType* shaderType = translator->FindType(selfType, memberAccessNode);
  LightningShaderIROp* intConst = translator->GetIntegerConstant(shaderType->mComponents, context);
  context->PushIRStack(intConst);
}

void ResolvePrimitiveGet(LightningSpirVFrontEnd* translator,
                         Lightning::FunctionCallNode* functionCallNode,
                         Lightning::MemberAccessNode* memberAccessNode,
                         LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Since this isn't actually a vector, just return the left operand (ignore
  // the index)
  ILightningShaderIR* selfInstance = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);

  context->PushIRStack(selfInstance);
}

void ResolvePrimitiveSet(LightningSpirVFrontEnd* translator,
                         Lightning::FunctionCallNode* functionCallNode,
                         Lightning::MemberAccessNode* memberAccessNode,
                         LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Since this isn't actually a vector, the "index" result is just the left
  // operand
  ILightningShaderIR* selfInstance = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  // Get the source value
  ILightningShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  translator->BuildStoreOp(currentBlock, selfInstance, sourceIR, context);
}

void ResolveVectorGet(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context)
{
  // Get the 'this' vector type and component type
  Lightning::Type* lightningVectorType = memberAccessNode->LeftOperand->ResultType;
  LightningShaderIRType* vectorType = translator->FindType(lightningVectorType, memberAccessNode->LeftOperand);
  LightningShaderIRType* componentType = GetComponentType(vectorType);

  // Get the index operator from get (must be a value type)
  ILightningShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  ILightningShaderIR* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the vector
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  ILightningShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, componentType->mPointerType, selfInstance, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveVectorSet(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context)
{
  // Get the 'this' vector type and component type
  Lightning::Type* lightningVectorType = memberAccessNode->LeftOperand->ResultType;
  LightningShaderIRType* vectorType = translator->FindType(lightningVectorType, memberAccessNode->LeftOperand);
  LightningShaderIRType* componentType = GetComponentType(vectorType);

  // Get the index operator from get (must be a value type)
  ILightningShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  ILightningShaderIR* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the vector
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  ILightningShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, componentType->mPointerType, selfInstance, indexOperand, context);

  // Get the source value
  ILightningShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void ResolveMatrixGet(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context)
{
  ResolveVectorGet(translator, functionCallNode, memberAccessNode, context);
}

void ResolveMatrixSet(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context)
{
  ResolveVectorSet(translator, functionCallNode, memberAccessNode, context);
}

void ResolveStaticBinaryFunctionOp(LightningSpirVFrontEnd* translator,
                                   Lightning::FunctionCallNode* functionCallNode,
                                   OpType opType,
                                   LightningSpirVFrontEndContext* context)
{
  // Get the result type
  LightningShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  // Walk each operand
  ILightningShaderIR* operand1 = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  ILightningShaderIR* operand2 = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);

  // Generate the fmod op
  ILightningShaderIR* operationOp = translator->BuildCurrentBlockIROp(opType, resultType, operand1, operand2, context);
  context->PushIRStack(operationOp);
}

void TranslatePrimitiveDefaultConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::Type* lightningResultType,
                                          LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRType* resultType = translator->FindType(lightningResultType, nullptr);

  LightningShaderIRType* componentType = resultType;
  Lightning::Any constantLiteral(componentType->mLightningType);
  ILightningShaderIR* constantPlasma = translator->GetConstant(componentType, constantLiteral, context);
  context->PushIRStack(constantPlasma);
}

void TranslatePrimitiveDefaultConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::FunctionCallNode* fnCallNode,
                                          Lightning::StaticTypeNode* staticTypeNode,
                                          LightningSpirVFrontEndContext* context)
{
  TranslatePrimitiveDefaultConstructor(translator, fnCallNode->ResultType, context);
}

void TranslateBackupPrimitiveConstructor(LightningSpirVFrontEnd* translator,
                                         Lightning::FunctionCallNode* fnCallNode,
                                         Lightning::StaticTypeNode* staticTypeNode,
                                         LightningSpirVFrontEndContext* context)
{
  if (fnCallNode->Arguments.Size() == 0)
    TranslatePrimitiveDefaultConstructor(translator, fnCallNode, staticTypeNode, context);
  else if (fnCallNode->Arguments.Size() == 1)
  {
    BasicBlock* currentBlock = context->GetCurrentBlock();
    ILightningShaderIR* operand = translator->WalkAndGetResult(fnCallNode->Arguments[0], context);
    context->PushIRStack(operand);
  }
  else
  {
    translator->SendTranslationError(fnCallNode->Location, "Unknown primitive constructor");
    context->PushIRStack(translator->GenerateDummyIR(fnCallNode, context));
  }
}

void TranslateCompositeDefaultConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::Type* lightningResultType,
                                          LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRType* resultType = translator->FindType(lightningResultType, nullptr);

  LightningShaderIRType* componentType = GetComponentType(resultType);
  Lightning::Any constantLiteral(componentType->mLightningType);
  ILightningShaderIR* constantPlasma = translator->GetConstant(componentType, constantLiteral, context);

  LightningShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, resultType, context);

  for (size_t i = 0; i < resultType->mComponents; ++i)
  {
    constructOp->mArguments.PushBack(constantPlasma);
  }

  currentBlock->mLines.PushBack(constructOp);
  context->PushIRStack(constructOp);
}

void TranslateCompositeDefaultConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::FunctionCallNode* fnCallNode,
                                          Lightning::StaticTypeNode* staticTypeNode,
                                          LightningSpirVFrontEndContext* context)
{
  TranslateCompositeDefaultConstructor(translator, fnCallNode->ResultType, context);
}

void TranslateBackupCompositeConstructor(LightningSpirVFrontEnd* translator,
                                         Lightning::FunctionCallNode* fnCallNode,
                                         Lightning::StaticTypeNode* staticTypeNode,
                                         LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRType* resultType = translator->FindType(fnCallNode->ResultType, fnCallNode);

  // Create the op for construction but don't add it to the current block yet,
  // we need to walk all arguments first
  LightningShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, resultType, context);

  // Walk each argument and add it to the constructor call
  for (size_t i = 0; i < fnCallNode->Arguments.Size(); ++i)
  {
    ILightningShaderIR* argIR = translator->WalkAndGetResult(fnCallNode->Arguments[i], context);
    // CompositeConstruct requires value types
    LightningShaderIROp* argValueOp = translator->GetOrGenerateValueTypeFromIR(argIR, context);
    constructOp->mArguments.PushBack(argValueOp);
  }

  // Now add the constructor op to the block
  currentBlock->mLines.PushBack(constructOp);
  // Also mark this as the return of this tree
  context->PushIRStack(constructOp);
}

void TranslateMatrixDefaultConstructor(LightningSpirVFrontEnd* translator,
                                       Lightning::Type* lightningResultType,
                                       LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRType* resultType = translator->FindType(lightningResultType, nullptr);

  // Construct a default composite of the sub-type
  LightningShaderIRType* componentType = GetComponentType(resultType);
  TranslateCompositeDefaultConstructor(translator, componentType->mLightningType, context);

  ILightningShaderIR* constituent = context->PopIRStack();
  // @JoshD: Leave this out for now since it can produce a glsl translation
  // error
  // constituent->mDebugResultName = "constituent";

  // Construct the composite but delay adding it to the stack until we've added
  // all of the arguments to it
  LightningShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, resultType, context);
  for (size_t i = 0; i < resultType->mComponents; ++i)
  {
    constructOp->mArguments.PushBack(constituent);
  }

  // Now add the matrix constructor the the stack
  currentBlock->mLines.PushBack(constructOp);
  context->PushIRStack(constructOp);
}

void TranslateMatrixDefaultConstructor(LightningSpirVFrontEnd* translator,
                                       Lightning::FunctionCallNode* fnCallNode,
                                       Lightning::StaticTypeNode* staticTypeNode,
                                       LightningSpirVFrontEndContext* context)
{
  TranslateMatrixDefaultConstructor(translator, fnCallNode->ResultType, context);
}

void TranslateMatrixFullConstructor(LightningSpirVFrontEnd* translator,
                                    Lightning::FunctionCallNode* fnCallNode,
                                    Lightning::StaticTypeNode* staticTypeNode,
                                    LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  LightningShaderIRType* matrixType = translator->FindType(fnCallNode->ResultType, nullptr);
  LightningShaderIRType* componentType = GetComponentType(matrixType);

  // Construct the matrix type but delay adding it as an instruction until all
  // of the arguments have been created
  LightningShaderIROp* matrixConstructOp =
      translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, matrixType, context);
  for (u32 i = 0; i < matrixType->mComponents; ++i)
  {
    // Create each vector type but delay add it for the same reason as the
    // matrix
    LightningShaderIROp* componentConstructOp =
        translator->BuildIROpNoBlockAdd(spv::OpCompositeConstruct, componentType, context);
    for (u32 j = 0; j < componentType->mComponents; ++j)
    {
      // Walk the given parameter and add it the the vector
      u32 argIndex = i * componentType->mComponents + j;
      ILightningShaderIR* param = translator->WalkAndGetValueTypeResult(fnCallNode->Arguments[argIndex], context);
      componentConstructOp->mArguments.PushBack(param);
    }
    // Now that we've finished constructing the parameters for the vector,
    // actually add it to the the current block and add it as a parameter to the
    // matrix
    currentBlock->mLines.PushBack(componentConstructOp);
    matrixConstructOp->mArguments.PushBack(componentConstructOp);
  }

  // Now the matrix is fully constructed so add it to the block
  currentBlock->mLines.PushBack(matrixConstructOp);
  context->PushIRStack(matrixConstructOp);
}

LightningShaderIROp* RecursivelyTranslateCompositeSplatConstructor(LightningSpirVFrontEnd* translator,
                                                               Lightning::FunctionCallNode* fnCallNode,
                                                               Lightning::StaticTypeNode* staticTypeNode,
                                                               LightningShaderIRType* type,
                                                               LightningShaderIROp* splatValueOp,
                                                               LightningSpirVFrontEndContext* context)
{
  // Terminate on the base case. Potentially need to handle translating the
  // splat value op to the correct scalar type.
  if (type->mBaseType == ShaderIRTypeBaseType::Float || type->mBaseType == ShaderIRTypeBaseType::Int ||
      type->mBaseType == ShaderIRTypeBaseType::Bool)
    return splatValueOp;

  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRType* componentType = GetComponentType(type);

  // Construct the composite but delay adding it as an instruction until all of
  // the sub-composites are created
  LightningShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, type, context);
  for (size_t i = 0; i < type->mComponents; ++i)
  {
    // Construct each constituent
    LightningShaderIROp* constituentOp = RecursivelyTranslateCompositeSplatConstructor(
        translator, fnCallNode, staticTypeNode, componentType, splatValueOp, context);
    // @JoshD: Leave this out for now since this produces a glsl translation
    // error
    // constituentOp->mDebugResultName = "constituent";
    constructOp->mArguments.PushBack(constituentOp);
  }
  // Now we can add the instruction to construct this composite since we've
  // created all of the parameters
  currentBlock->mLines.PushBack(constructOp);
  return constructOp;
}

void TranslateCompositeSplatConstructor(LightningSpirVFrontEnd* translator,
                                        Lightning::FunctionCallNode* fnCallNode,
                                        Lightning::StaticTypeNode* staticTypeNode,
                                        LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRType* resultType = translator->FindType(fnCallNode->ResultType, fnCallNode);

  // Get the splat scalar value type
  ILightningShaderIR* splatValue = translator->WalkAndGetResult(fnCallNode->Arguments[0], context);
  LightningShaderIROp* splatValueOp = translator->GetOrGenerateValueTypeFromIR(splatValue, context);

  // Recursively construct all composite types with this final scalar type
  LightningShaderIROp* constructOp = RecursivelyTranslateCompositeSplatConstructor(
      translator, fnCallNode, staticTypeNode, resultType, splatValueOp, context);

  context->PushIRStack(constructOp);
}

bool IsVectorSwizzle(StringParam memberName)
{
  // Verify that all components are in the valid range
  for (size_t i = 0; i < memberName.SizeInBytes(); ++i)
  {
    byte memberValue = *(memberName.Data() + i);

    if (memberValue < 'W' || memberValue > 'Z')
      return false;
  }
  return true;
}

void ResolveScalarComponentAccess(LightningSpirVFrontEnd* translator,
                                  Lightning::MemberAccessNode* memberAccessNode,
                                  byte componentName,
                                  LightningSpirVFrontEndContext* context)
{
  // A scalar component access on a scalar type is just the scalar itself (e.g.
  // a.X => a)
  ILightningShaderIR* operandResult = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  context->PushIRStack(operandResult);
}

void ResolveScalarSwizzle(LightningSpirVFrontEnd* translator,
                          Lightning::MemberAccessNode* memberAccessNode,
                          StringParam memberName,
                          LightningSpirVFrontEndContext* context)
{
  // Figure out what the result type of this swizzle is
  LightningShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);

  // The swizzle instruction (vector shuffle) requires value types
  ILightningShaderIR* operandResult = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* operandValueOp = translator->GetOrGenerateValueTypeFromIR(operandResult, context);

  // Vector swizzle doesn't exist on scalar types so just splat construct the
  // relevant vector type
  LightningShaderIROp* constructOp = translator->BuildCurrentBlockIROp(OpType::OpCompositeConstruct, resultType, context);
  for (size_t i = 0; i < memberName.SizeInBytes(); ++i)
    constructOp->mArguments.PushBack(operandValueOp);
  context->PushIRStack(constructOp);
}

void ScalarBackupFieldResolver(LightningSpirVFrontEnd* translator,
                               Lightning::MemberAccessNode* memberAccessNode,
                               LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  String memberName = memberAccessNode->Name;

  // Deal with single component access
  if (memberName.SizeInBytes() == 1)
  {
    byte memberValue = *memberName.Data();
    ResolveScalarComponentAccess(translator, memberAccessNode, memberValue, context);
    return;
  }

  // Deal with swizzles
  if (memberName.SizeInBytes() <= 4 && IsVectorSwizzle(memberName))
  {
    ResolveScalarSwizzle(translator, memberAccessNode, memberName, context);
    return;
  }

  // Deal with the remainder later
  //__debugbreak();
}

void ResolveVectorComponentAccess(LightningSpirVFrontEnd* translator,
                                  LightningShaderIROp* selfInstance,
                                  LightningShaderIRType* componentType,
                                  byte componentName,
                                  LightningSpirVFrontEndContext* context)
{
  // Convert the index to [0, 3] using a nice modulus trick
  int index = componentName - 'X';
  index = (index + 4) % 4;

  // If the operand was a pointer type then we have to use an access chain to
  // extract the sub-pointer
  if (selfInstance->IsResultPointerType())
  {
    LightningShaderIROp* indexOp = translator->GetIntegerConstant(index, context);
    LightningShaderIROp* accessChainOp = translator->BuildCurrentBlockIROp(
        OpType::OpAccessChain, componentType->mPointerType, selfInstance, indexOp, context);
    context->PushIRStack(accessChainOp);
  }
  // Otherwise this was a value (temporary, e.g. (Real3() + Real3()).X) so
  // use composite extract instead which works on value types.
  else
  {
    LightningShaderIRConstantLiteral* indexLiteral = translator->GetOrCreateConstantLiteral(index);
    LightningShaderIROp* accessChainOp = translator->BuildCurrentBlockIROp(
        OpType::OpCompositeExtract, componentType, selfInstance, indexLiteral, context);
    context->PushIRStack(accessChainOp);
  }
}

void ResolveVectorComponentAccess(LightningSpirVFrontEnd* translator,
                                  Lightning::MemberAccessNode* memberAccessNode,
                                  byte componentName,
                                  LightningSpirVFrontEndContext* context)
{
  // Walk the left hand side of the member access node
  ILightningShaderIR* operandResult = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* operandResultOp = operandResult->As<LightningShaderIROp>();

  // Get what the result type should be (e.g. Real3.X -> Real)
  LightningShaderIRType* operandType =
      translator->FindType(memberAccessNode->LeftOperand->ResultType, memberAccessNode->LeftOperand);
  LightningShaderIRType* componentType = GetComponentType(operandType);

  ResolveVectorComponentAccess(translator, operandResultOp, componentType, componentName, context);
}

void ResolveVectorSwizzle(LightningSpirVFrontEnd* translator,
                          ILightningShaderIR* selfInstance,
                          LightningShaderIRType* resultType,
                          StringParam memberName,
                          LightningSpirVFrontEndContext* context)
{
  // The swizzle instruction (vector shuffle) requires value types
  LightningShaderIROp* operandValueOp = translator->GetOrGenerateValueTypeFromIR(selfInstance, context);

  // Build the base instruction
  LightningShaderIROp* swizzleOp =
      translator->BuildCurrentBlockIROp(OpType::OpVectorShuffle, resultType, operandValueOp, operandValueOp, context);

  // For every swizzle element, figure out what index the sub-item is and add
  // that as an argument
  for (size_t i = 0; i < memberName.SizeInBytes(); ++i)
  {
    byte memberValue = *(memberName.Data() + i);
    int index = (memberValue - 'X' + 4) % 4;
    LightningShaderIRConstantLiteral* indexLiteral = translator->GetOrCreateConstantLiteral(index);
    swizzleOp->mArguments.PushBack(indexLiteral);
  }
  context->PushIRStack(swizzleOp);
}

void ResolveVectorSwizzle(LightningSpirVFrontEnd* translator,
                          Lightning::MemberAccessNode* memberAccessNode,
                          StringParam memberName,
                          LightningSpirVFrontEndContext* context)
{
  // Figure out what the result type of this swizzle is
  LightningShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);

  // Get the operand to swizzle
  ILightningShaderIR* operandResult = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  ResolveVectorSwizzle(translator, operandResult, resultType, memberName, context);
}

void VectorBackupFieldResolver(LightningSpirVFrontEnd* translator,
                               Lightning::MemberAccessNode* memberAccessNode,
                               LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  String memberName = memberAccessNode->Name;

  // Deal with single component access
  if (memberName.SizeInBytes() == 1)
  {
    byte memberValue = *memberName.Data();
    ResolveVectorComponentAccess(translator, memberAccessNode, memberValue, context);
    return;
  }

  // Deal with swizzles
  if (memberName.SizeInBytes() <= 4 && IsVectorSwizzle(memberName))
  {
    ResolveVectorSwizzle(translator, memberAccessNode, memberName, context);
    return;
  }

  // Otherwise we don't know how to translate this so report an error
  translator->SendTranslationError(memberAccessNode->Location, "Cannot resolve vector field");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

void ResolverVectorSwizzleSetter(LightningSpirVFrontEnd* translator,
                                 ILightningShaderIR* selfInstance,
                                 LightningShaderIROp* resultValue,
                                 LightningShaderIRType* resultType,
                                 StringParam memberName,
                                 LightningSpirVFrontEndContext* context)
{
  // To generate the new vector we need to perform vector shuffle which requires
  // a value type.
  LightningShaderIROp* instanceValue = translator->GetOrGenerateValueTypeFromIR(selfInstance, context);

  // The easiest way to set via a swizzle is by constructing a brand new vector
  // from elements of the old and new vector using the shuffle instruction
  // (parameters set below)
  LightningShaderIROp* shuffleOp =
      translator->BuildCurrentBlockIROp(OpType::OpVectorShuffle, resultType, instanceValue, resultValue, context);

  // The shuffle operator picks components from the two vectors by index as if
  // they were laid out in one contiguous block of memory. By default assume
  // that all components will come from the same location in the original
  // vector. To copy elements from the new vector we loop over the member access
  // and set the relevant set index (by name) to the memory index of the new
  // vector.

  // Keep track of how many components this vector has
  u32 instanceComponentCount = instanceValue->mResultType->mComponents;
  // Hard-coded max of 4 (vectors cannot be bigger)
  u32 indices[4] = {0, 1, 2, 3};

  for(u32 i = 0; i < static_cast<u32>(memberName.SizeInBytes()); ++i)
  {
    byte memberValue = *(memberName.Data() + i);
    u32 index = (memberValue - 'X' + 4) % 4;
    indices[index] = i + instanceComponentCount;
  }

  // Actually create all of the arguments (have to create literals)
  for (u32 i = 0; i < instanceComponentCount; ++i)
  {
    LightningShaderIRConstantLiteral* literal = translator->GetOrCreateConstantIntegerLiteral(indices[i]);
    shuffleOp->mArguments.PushBack(literal);
  }

  // Store the result back into the instance
  translator->BuildStoreOp(selfInstance, shuffleOp, context);
}

void ResolverVectorSwizzleSetter(LightningSpirVFrontEnd* translator,
                                 Lightning::MemberAccessNode* memberAccessNode,
                                 LightningShaderIROp* resultValue,
                                 LightningSpirVFrontEndContext* context)
{
  String memberName = memberAccessNode->Name;
  LightningShaderIRType* resultType =
      translator->FindType(memberAccessNode->LeftOperand->ResultType, memberAccessNode->LeftOperand);

  // Get the instance we will be setting to
  ILightningShaderIR* instance = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  ResolverVectorSwizzleSetter(translator, instance, resultValue, resultType, memberName, context);
}

void VectorBackupPropertySetter(LightningSpirVFrontEnd* translator,
                                Lightning::MemberAccessNode* memberAccessNode,
                                LightningShaderIROp* resultValue,
                                LightningSpirVFrontEndContext* context)
{
  String memberName = memberAccessNode->Name;

  // Deal with scalar setters (fallback to the component access)
  if (memberName.SizeInBytes() == 1)
  {
    ResolveVectorComponentAccess(translator, memberAccessNode, *memberName.Data(), context);
    translator->BuildStoreOp(context->GetCurrentBlock(), context->PopIRStack(), resultValue, context);
    return;
  }

  // Resolve swizzles
  if (IsVectorSwizzle(memberName))
  {
    ResolverVectorSwizzleSetter(translator, memberAccessNode, resultValue, context);
    return;
  }

  // Otherwise we don't know how to translate this so report an error
  translator->SendTranslationError(memberAccessNode->Location, "Cannot set non-vector swizzle");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

bool MatrixElementAccessResolver(LightningSpirVFrontEnd* translator,
                                 Lightning::MemberAccessNode* memberAccessNode,
                                 LightningSpirVFrontEndContext* context,
                                 Lightning::MatrixUserData& matrixUserData)
{
  // We only translate Myx here.
  String data = memberAccessNode->Name;
  if (data.SizeInBytes() != 3)
    return false;

  StringRange range = data.All();
  Rune first = range.Front();
  if (first != 'M')
    return false;

  range.PopFront();
  Rune indexYRune = range.Front();
  range.PopFront();
  Rune indexXRune = range.Front();

  // Get the indices of the access and validate them
  size_t indexY = indexYRune.value - '0';
  size_t indexX = indexXRune.value - '0';
  if (indexX >= matrixUserData.SizeX || indexY >= matrixUserData.SizeY)
    return false;

  // Figure out what the result type of the member access is
  LightningShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);
  // Construct constants for the sub-access
  LightningShaderIROp* indexYConstant = translator->GetIntegerConstant((int)indexY, context);
  LightningShaderIROp* indexXConstant = translator->GetIntegerConstant((int)indexX, context);
  // Get the matrix instance
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* operandResult = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  // Construct the op to access the matrix element
  // @JoshD: Revisit when looking over matrix column/row order
  LightningShaderIROp* accessOp = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, resultType->mPointerType, operandResult, indexYConstant, indexXConstant, context);
  context->PushIRStack(accessOp);
  return true;
}

void MatrixBackupFieldResolver(LightningSpirVFrontEnd* translator,
                               Lightning::MemberAccessNode* memberAccessNode,
                               LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  Lightning::Type* matrixType = memberAccessNode->LeftOperand->ResultType;
  Lightning::MatrixUserData& userData = matrixType->ComplexUserData.ReadObject<Lightning::MatrixUserData>(0);
  // Handle accessing an element (e.g. M12) from the matrix
  if (MatrixElementAccessResolver(translator, memberAccessNode, context, userData))
    return;

  // @JoshD: Handle the rest of matrix translation later
  translator->SendTranslationError(memberAccessNode->Location, "Member access cannot be translated");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

void TranslateQuaternionDefaultConstructor(LightningSpirVFrontEnd* translator,
                                           Lightning::Type* lightningResultType,
                                           LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Get all of the types related to quaternions
  LightningShaderIRType* quaternionType = translator->FindType(lightningResultType, nullptr);
  LightningShaderIRType* vec4Type = quaternionType->GetSubType(0);
  LightningShaderIRType* realType = GetComponentType(vec4Type);

  // Construct the default vec4 for the quaternion (identity)
  ILightningShaderIR* constantPlasma = translator->GetConstant(realType, 0.0f, context);
  ILightningShaderIR* constantOne = translator->GetConstant(realType, 1.0f, context);
  LightningShaderIROp* vec4ConstructOp = translator->BuildCurrentBlockIROp(OpType::OpCompositeConstruct, vec4Type, context);
  vec4ConstructOp->mArguments.PushBack(constantPlasma);
  vec4ConstructOp->mArguments.PushBack(constantPlasma);
  vec4ConstructOp->mArguments.PushBack(constantPlasma);
  vec4ConstructOp->mArguments.PushBack(constantOne);

  // Construct the quaternion from the vec4
  LightningShaderIROp* constructOp = translator->BuildIROp(
      context->GetCurrentBlock(), OpType::OpCompositeConstruct, quaternionType, vec4ConstructOp, context);
  context->PushIRStack(constructOp);
}

void TranslateQuaternionDefaultConstructor(LightningSpirVFrontEnd* translator,
                                           Lightning::FunctionCallNode* fnCallNode,
                                           Lightning::StaticTypeNode* staticTypeNode,
                                           LightningSpirVFrontEndContext* context)
{
  TranslateQuaternionDefaultConstructor(translator, fnCallNode->ResultType, context);
}

void QuaternionBackupFieldResolver(LightningSpirVFrontEnd* translator,
                                   Lightning::MemberAccessNode* memberAccessNode,
                                   LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  String memberName = memberAccessNode->Name;

  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  LightningShaderIRType* vec4Type = selfInstance->mResultType->mDereferenceType->GetSubType(0);
  LightningShaderIRType* realType = GetComponentType(vec4Type);

  // Get the vec4 of the quaternion
  ILightningShaderIR* vec4OffsetConstant = translator->GetIntegerConstant(0, context);
  LightningShaderIROp* vec4Instance = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, vec4Type->mPointerType, selfInstance, vec4OffsetConstant, context);

  // Deal with single component access
  if (memberName.SizeInBytes() == 1)
  {
    byte memberValue = *memberName.Data();
    ResolveVectorComponentAccess(translator, vec4Instance, realType, memberValue, context);
    return;
  }

  // Deal with swizzles
  if (memberName.SizeInBytes() <= 4 && IsVectorSwizzle(memberName))
  {
    LightningShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);
    ResolveVectorSwizzle(translator, vec4Instance, resultType, memberName, context);
    return;
  }

  // Deal with the remainder later
  translator->SendTranslationError(memberAccessNode->Location, "Cannot resolve quaternion field");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

void QuaternionBackupPropertySetter(LightningSpirVFrontEnd* translator,
                                    Lightning::MemberAccessNode* memberAccessNode,
                                    LightningShaderIROp* resultValue,
                                    LightningSpirVFrontEndContext* context)
{
  String memberName = memberAccessNode->Name;

  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  LightningShaderIRType* vec4Type = selfInstance->mResultType->mDereferenceType->GetSubType(0);
  LightningShaderIRType* realType = GetComponentType(vec4Type);

  // Get the vec4 of the quaternion
  ILightningShaderIR* vec4OffsetConstant = translator->GetIntegerConstant(0, context);
  LightningShaderIROp* vec4Instance = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, vec4Type->mPointerType, selfInstance, vec4OffsetConstant, context);

  // Deal with single component access
  if (memberName.SizeInBytes() == 1)
  {
    byte memberValue = *memberName.Data();
    ResolveVectorComponentAccess(translator, vec4Instance, realType, memberValue, context);
    translator->BuildStoreOp(context->GetCurrentBlock(), context->PopIRStack(), resultValue, context);
    return;
  }

  // Resolve swizzles
  if (IsVectorSwizzle(memberName))
  {
    LightningShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);
    ResolverVectorSwizzleSetter(translator, vec4Instance, resultValue, vec4Type, memberAccessNode->Name, context);
    return;
  }

  // Otherwise we don't know how to translate this so report an error
  translator->SendTranslationError(memberAccessNode->Location, "Cannot set non-vector swizzle");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

void ResolveQuaternionTypeCount(LightningSpirVFrontEnd* translator,
                                Lightning::FunctionCallNode* functionCallNode,
                                Lightning::MemberAccessNode* memberAccessNode,
                                LightningSpirVFrontEndContext* context)
{
  // Quaternion count is always 4
  LightningShaderIROp* intConst = translator->GetIntegerConstant(4, context);
  context->PushIRStack(intConst);
}

void ResolveQuaternionGet(LightningSpirVFrontEnd* translator,
                          Lightning::FunctionCallNode* functionCallNode,
                          Lightning::MemberAccessNode* memberAccessNode,
                          LightningSpirVFrontEndContext* context)
{
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  LightningShaderIRType* vec4Type = selfInstance->mResultType->mDereferenceType->GetSubType(0);
  LightningShaderIRType* realType = GetComponentType(vec4Type);

  // Get the vec4 of the quaternion
  ILightningShaderIR* vec4OffsetConstant = translator->GetIntegerConstant(0, context);
  LightningShaderIROp* vec4Instance = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, vec4Type->mPointerType, selfInstance, vec4OffsetConstant, context);

  // Get the index operator from get (must be a value type)
  ILightningShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  ILightningShaderIR* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the vector
  ILightningShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, realType->mPointerType, vec4Instance, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveQuaternionSet(LightningSpirVFrontEnd* translator,
                          Lightning::FunctionCallNode* functionCallNode,
                          Lightning::MemberAccessNode* memberAccessNode,
                          LightningSpirVFrontEndContext* context)
{
  ILightningShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  LightningShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  LightningShaderIRType* vec4Type = selfInstance->mResultType->mDereferenceType->GetSubType(0);
  LightningShaderIRType* realType = GetComponentType(vec4Type);

  // Get the vec4 of the quaternion
  ILightningShaderIR* offsetConstant = translator->GetIntegerConstant(0, context);
  LightningShaderIROp* vec4Instance = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, vec4Type->mPointerType, selfInstance, offsetConstant, context);

  // Get the index operator from get (must be a value type)
  ILightningShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  ILightningShaderIR* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the vector
  ILightningShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(
      OpType::OpAccessChain, realType->mPointerType, vec4Instance, indexOperand, context);

  // Get the source value
  ILightningShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void TranslateQuaternionSplatConstructor(LightningSpirVFrontEnd* translator,
                                         Lightning::FunctionCallNode* fnCallNode,
                                         Lightning::StaticTypeNode* staticTypeNode,
                                         LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRType* quaternionType = translator->FindType(fnCallNode->ResultType, fnCallNode);
  LightningShaderIRType* vec4Type = quaternionType->GetSubType(0);

  // Get the splat scalar value type
  ILightningShaderIR* splatValue = translator->WalkAndGetResult(fnCallNode->Arguments[0], context);
  LightningShaderIROp* splatValueOp = translator->GetOrGenerateValueTypeFromIR(splatValue, context);

  // Construct the vec4 type from the splat value
  LightningShaderIROp* subConstructOp = RecursivelyTranslateCompositeSplatConstructor(
      translator, fnCallNode, staticTypeNode, vec4Type, splatValueOp, context);

  // Construct the quaternion from the vec4
  LightningShaderIROp* vec4Value = translator->GetOrGenerateValueTypeFromIR(subConstructOp, context);
  LightningShaderIROp* quaternionConstructOp =
      translator->BuildCurrentBlockIROp(OpType::OpCompositeConstruct, quaternionType, vec4Value, context);

  context->PushIRStack(quaternionConstructOp);
}

void TranslateBackupQuaternionConstructor(LightningSpirVFrontEnd* translator,
                                          Lightning::FunctionCallNode* fnCallNode,
                                          Lightning::StaticTypeNode* staticTypeNode,
                                          LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIRType* quaternionType = translator->FindType(fnCallNode->ResultType, fnCallNode);
  LightningShaderIRType* vec4Type = quaternionType->GetSubType(0);

  // Create the op for construction but don't add it to the current block yet,
  // we need to walk all arguments first
  LightningShaderIROp* vec4ConstructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, vec4Type, context);

  // Walk each argument and add it to the constructor call
  for (size_t i = 0; i < fnCallNode->Arguments.Size(); ++i)
  {
    ILightningShaderIR* argIR = translator->WalkAndGetResult(fnCallNode->Arguments[i], context);
    // CompositeConstruct requires value types
    LightningShaderIROp* argValueOp = translator->GetOrGenerateValueTypeFromIR(argIR, context);
    vec4ConstructOp->mArguments.PushBack(argValueOp);
  }

  // Now add the constructor op to the block
  currentBlock->mLines.PushBack(vec4ConstructOp);

  LightningShaderIROp* quaternionConstructOp =
      translator->BuildCurrentBlockIROp(OpType::OpCompositeConstruct, quaternionType, vec4ConstructOp, context);
  // Also mark this as the return of this tree
  context->PushIRStack(quaternionConstructOp);
}

void ResolveColor(LightningSpirVFrontEnd* translator,
                  Lightning::FunctionCallNode* /*functionCallNode*/,
                  Lightning::MemberAccessNode* memberAccessNode,
                  LightningSpirVFrontEndContext* context)
{
  Lightning::Property* property = memberAccessNode->AccessedProperty;
  LightningShaderIRType* resultType = translator->FindType(property->PropertyType, memberAccessNode);
  LightningShaderIRType* componentType = GetComponentType(resultType);

  // Read the color value from the complex user data
  Lightning::Real4 propertyValue = property->ComplexUserData.ReadObject<Lightning::Real4>(0);

  // Composite construct the color
  LightningShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, resultType, context);
  for (uint i = 0; i < 4; ++i)
  {
    Lightning::Any constantLiteral(propertyValue[i]);
    ILightningShaderIR* component = translator->GetConstant(componentType, constantLiteral, context);
    constructOp->mArguments.PushBack(component);
  }
  context->mCurrentBlock->AddOp(constructOp);
  context->PushIRStack(constructOp);
}

void RegisterColorsOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::Library* coreLibrary = core.GetLibrary();
  // Create the colors type
  Lightning::BoundType* colors = LightningTypeId(Lightning::ColorsClass);
  LightningShaderIRType* colorsShaderType =
      translator->MakeStructType(shaderLibrary, colors->Name, colors, spv::StorageClass::StorageClassGeneric);
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[colors];

  // the only way to get the value for each color during translation is to
  // compile and execute lightning.
  Lightning::Type* real4Type = LightningTypeId(Lightning::Real4);
  forRange (Lightning::Property* lightningProperty, colors->GetProperties())
  {
    // Skip non-static real4 properties (should only be the actual colors)
    if (!lightningProperty->IsStatic || lightningProperty->PropertyType != real4Type)
      continue;

    typeResolver.RegisterFunctionResolver(lightningProperty->Get, ResolveColor);
  }
}

} // namespace Plasma
