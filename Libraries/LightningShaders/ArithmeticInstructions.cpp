// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Resolves a binary operator node given the expected return type.
void ResolveBinaryOp(LightningSpirVFrontEnd* translator,
                     Lightning::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     LightningSpirVFrontEndContext* context)
{
  if (binaryOpNode->OperatorInfo.Io & Lightning::IoMode::WriteLValue)
    translator->PerformBinaryAssignmentOp(binaryOpNode, opType, context);
  else
    translator->PerformBinaryOp(binaryOpNode, opType, context);
}

// Resolves a binary operator node where the lhs and rhs of the node have
// already been resolved. This can be necessary when one of the sides in the
// node has to undergo a transformation first (e.g vector / scalar has to first
// promote the scalar to a vector)
void ResolveBinaryOp(LightningSpirVFrontEnd* translator,
                     Lightning::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     ILightningShaderIR* lhs,
                     ILightningShaderIR* rhs,
                     LightningSpirVFrontEndContext* context)
{
  if (binaryOpNode->OperatorInfo.Io & Lightning::IoMode::WriteLValue)
    translator->PerformBinaryAssignmentOp(binaryOpNode, opType, lhs, rhs, context);
  else
    translator->PerformBinaryOp(binaryOpNode, opType, lhs, rhs, context);
}

void ResolveUnaryOperator(LightningSpirVFrontEnd* translator,
                          Lightning::UnaryOperatorNode* unaryOpNode,
                          OpType opType,
                          LightningSpirVFrontEndContext* context)
{
  translator->PerformUnaryOp(unaryOpNode, opType, context);
}

template <OpType opType>
void ResolveIntIncDecUnaryOperator(LightningSpirVFrontEnd* translator,
                                   Lightning::UnaryOperatorNode* unaryOpNode,
                                   LightningSpirVFrontEndContext* context)
{
  // Create the int literal '1'
  ILightningShaderIR* constantOne = translator->GetIntegerConstant(1, context);
  translator->PerformUnaryIncDecOp(unaryOpNode, constantOne, opType, context);
}

template <OpType opType>
void ResolveFloatIncDecUnaryOperator(LightningSpirVFrontEnd* translator,
                                     Lightning::UnaryOperatorNode* unaryOpNode,
                                     LightningSpirVFrontEndContext* context)
{
  // Create the float literal '1'
  LightningShaderIRType* floatType = translator->FindType(LightningTypeId(float), unaryOpNode, context);
  ILightningShaderIR* constantOne = translator->GetConstant(floatType, 1.0f, context);
  translator->PerformUnaryIncDecOp(unaryOpNode, constantOne, opType, context);
}

void ResolveFMod(LightningSpirVFrontEnd* translator,
                 Lightning::FunctionCallNode* functionCallNode,
                 Lightning::MemberAccessNode* memberAccessNode,
                 LightningSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, spv::OpFMod, context);
}

void ResolveDot(LightningSpirVFrontEnd* translator,
                Lightning::FunctionCallNode* functionCallNode,
                Lightning::MemberAccessNode* memberAccessNode,
                LightningSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, spv::OpDot, context);
}

// Resolves vector op vector(scalar). Needed for some operations like vector /
// scalar which has to turn into vector / vector(scalar) since the componentized
// operations don't exist.
template <OpType opType>
void ResolveVectorOpSplatScalar(LightningSpirVFrontEnd* translator,
                                Lightning::BinaryOperatorNode* binaryOpNode,
                                LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Get the vector operand
  ILightningShaderIR* vectorOperand = translator->WalkAndGetResult(binaryOpNode->LeftOperand, context);

  // Convert the scalar operand into a vector of the same type as the left hand
  // side
  LightningShaderIRType* vectorType = translator->FindType(binaryOpNode->LeftOperand->ResultType, binaryOpNode);
  LightningShaderIROp* scalarOperand = translator->WalkAndGetValueTypeResult(binaryOpNode->RightOperand, context);
  LightningShaderIROp* splattedScalarOperand =
      translator->ConstructCompositeFromScalar(currentBlock, vectorType, scalarOperand, context);

  // Perform the op
  ResolveBinaryOp(translator, binaryOpNode, opType, vectorOperand, splattedScalarOperand, context);
}

template <OpType opType>
void ResolveSimpleStaticBinaryFunctionOp(LightningSpirVFrontEnd* translator,
                                         Lightning::FunctionCallNode* functionCallNode,
                                         Lightning::MemberAccessNode* memberAccessNode,
                                         LightningSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, opType, context);
}

// Some binary functions are special and have to be flipped due to the column
// vs. row major differences of lightning and spirv.
void ResolveFlippedStaticBinaryFunctionOp(LightningSpirVFrontEnd* translator,
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
  ILightningShaderIR* operationOp = translator->BuildCurrentBlockIROp(opType, resultType, operand2, operand1, context);
  context->PushIRStack(operationOp);
}

void ResolveMatrixTimesVector(LightningSpirVFrontEnd* translator,
                              Lightning::FunctionCallNode* functionCallNode,
                              Lightning::MemberAccessNode* memberAccessNode,
                              LightningSpirVFrontEndContext* context)
{
  ResolveFlippedStaticBinaryFunctionOp(translator, functionCallNode, OpType::OpVectorTimesMatrix, context);
}

void ResolveMatrixTimesMatrix(LightningSpirVFrontEnd* translator,
                              Lightning::FunctionCallNode* functionCallNode,
                              Lightning::MemberAccessNode* memberAccessNode,
                              LightningSpirVFrontEndContext* context)
{
  ResolveFlippedStaticBinaryFunctionOp(translator, functionCallNode, OpType::OpMatrixTimesMatrix, context);
}

void ResolveMatrixTranspose(LightningSpirVFrontEnd* translator,
                            Lightning::FunctionCallNode* functionCallNode,
                            Lightning::MemberAccessNode* memberAccessNode,
                            LightningSpirVFrontEndContext* context)
{
  // Get the result type
  LightningShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  // Walk each operand
  ILightningShaderIR* operand = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);

  // Generate the transpose op
  ILightningShaderIR* operationOp = translator->BuildCurrentBlockIROp(OpType::OpTranspose, resultType, operand, context);
  context->PushIRStack(operationOp);
}

// Register function callbacks for the various arithmetic operators (see
// Arithmetic Instructions in the spir-v spec).
void RegisterArithmeticOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;
  TypeResolvers& mathTypeResolver = shaderLibrary->mTypeResolvers[mathType];
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  Lightning::BoundType* realType = core.RealType;
  Lightning::BoundType* intType = core.IntegerType;

  opResolvers.RegisterUnaryOpResolver(
      intType, Lightning::Grammar::Increment, ResolveIntIncDecUnaryOperator<OpType::OpIAdd>);
  opResolvers.RegisterUnaryOpResolver(
      intType, Lightning::Grammar::Decrement, ResolveIntIncDecUnaryOperator<OpType::OpISub>);
  opResolvers.RegisterUnaryOpResolver(
      realType, Lightning::Grammar::Increment, ResolveFloatIncDecUnaryOperator<OpType::OpFAdd>);
  opResolvers.RegisterUnaryOpResolver(
      realType, Lightning::Grammar::Decrement, ResolveFloatIncDecUnaryOperator<OpType::OpFSub>);

  // Register ops that are on all float vector types
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mRealVectorTypes[i];
    String lightningTypeName = lightningType->ToString();

    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentAdd, ResolveBinaryOperator<spv::OpFAdd>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentSubtract, ResolveBinaryOperator<OpType::OpFSub>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpFMul>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentDivide, ResolveBinaryOperator<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentModulo, ResolveBinaryOperator<OpType::OpFMod>);

    opResolvers.RegisterBinaryOpResolver(lightningType, lightningType, Lightning::Grammar::Add, ResolveBinaryOperator<spv::OpFAdd>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::Subtract, ResolveBinaryOperator<OpType::OpFSub>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::Multiply, ResolveBinaryOperator<OpType::OpFMul>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::Divide, ResolveBinaryOperator<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::Modulo, ResolveBinaryOperator<OpType::OpFMod>);

    opResolvers.RegisterUnaryOpResolver(lightningType, Lightning::Grammar::Subtract, ResolveUnaryOperator<OpType::OpFNegate>);

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "FMod", lightningTypeName, lightningTypeName),
                                              ResolveFMod);
  }

  // Register ops that are only on float vector types (no scalars). Some of
  // these are because of lightning and not spirv.
  for (size_t i = 1; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mRealVectorTypes[i];
    String lightningTypeName = lightningType->ToString();

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "Dot", lightningTypeName, lightningTypeName),
                                              ResolveDot);

    opResolvers.RegisterBinaryOpResolver(
        lightningType, realType, Lightning::Grammar::Multiply, ResolveBinaryOperator<spv::OpVectorTimesScalar>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, realType, Lightning::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpVectorTimesScalar>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, realType, Lightning::Grammar::Divide, ResolveVectorOpSplatScalar<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, realType, Lightning::Grammar::AssignmentDivide, ResolveVectorOpSplatScalar<OpType::OpFDiv>);
  }

  // Register ops that are on all integer vector types
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mIntegerVectorTypes[i];
    String lightningTypeName = lightningType->ToString();

    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentAdd, ResolveBinaryOperator<spv::OpIAdd>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentSubtract, ResolveBinaryOperator<OpType::OpISub>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpIMul>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentDivide, ResolveBinaryOperator<OpType::OpSDiv>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::AssignmentModulo, ResolveBinaryOperator<OpType::OpSMod>);

    opResolvers.RegisterBinaryOpResolver(lightningType, lightningType, Lightning::Grammar::Add, ResolveBinaryOperator<spv::OpIAdd>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::Subtract, ResolveBinaryOperator<OpType::OpISub>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::Multiply, ResolveBinaryOperator<OpType::OpIMul>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::Divide, ResolveBinaryOperator<OpType::OpSDiv>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::Modulo, ResolveBinaryOperator<OpType::OpSMod>);

    opResolvers.RegisterUnaryOpResolver(lightningType, Lightning::Grammar::Subtract, ResolveUnaryOperator<OpType::OpSNegate>);
  }

  // Register ops that are only on int vector types (no scalars). Some of these
  // are because of lightning and not spirv.
  // @JoshD: SpirV doesn't have any actual vector operations on integers.
  // Some could be supported using more complicated instructions (e.g. vector *
  // scalar = vector * vector(scalar))
  for (size_t i = 1; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mIntegerVectorTypes[i];
    String lightningTypeName = lightningType->ToString();

    // VectorTimesScalar is only on real types
    // LightningTypePair vectorScalarTypePair(lightningType, intType);
    // translator->mBinaryOpInstructions[BinaryOpTypeId(vectorScalarTypePair,
    // Lightning::Grammar::Multiply)] = OpType::OpVectorTimesScalar;
    // translator->mBinaryOpInstructions[BinaryOpTypeId(vectorScalarTypePair,
    // Lightning::Grammar::AssignmentMultiply)] = OpType::OpVectorTimesScalar;
  }

  // Register all real matrix instructions.
  for (u32 y = 2; y <= 4; ++y)
  {
    for (u32 x = 2; x <= 4; ++x)
    {
      Lightning::BoundType* lightningType = types.GetMatrixType(y, x);
      String lightningTypeName = lightningType->ToString();
      Lightning::BoundType* vectorType = types.mRealVectorTypes[x - 1];

      opResolvers.RegisterBinaryOpResolver(
          lightningType, lightningType, Lightning::Grammar::Multiply, ResolveBinaryOperator<spv::OpMatrixTimesScalar>);
      mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "Transpose", lightningTypeName),
                                                ResolveMatrixTranspose);
      // Matrix times vector
      mathTypeResolver.RegisterFunctionResolver(
          GetStaticFunction(mathType, "Multiply", lightningTypeName, vectorType->ToString()), ResolveMatrixTimesVector);

      // Iterate over all of the other matrix dimensions to make the
      // multiplication functions (e.g. Real2x3 * real3x2, Real2x3 * Real3x3,
      // etc...)
      for (u32 z = 2; z <= 4; ++z)
      {
        Lightning::BoundType* rhsMatrixType = types.GetMatrixType(x, z);
        mathTypeResolver.RegisterFunctionResolver(
            GetStaticFunction(mathType, "Multiply", lightningTypeName, rhsMatrixType->ToString()),
            ResolveMatrixTimesMatrix);
      }
    }
  }
}

} // namespace Plasma
