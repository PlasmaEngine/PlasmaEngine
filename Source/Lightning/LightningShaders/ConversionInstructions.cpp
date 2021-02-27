// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void ResolveOpBitcast(LightningSpirVFrontEnd* translator,
                      Lightning::FunctionCallNode* functionCallNode,
                      Lightning::MemberAccessNode* memberAccessNode,
                      LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  ILightningShaderIR* operand = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  ILightningShaderIR* operation = translator->BuildCurrentBlockIROp(OpType::OpBitcast, resultType, operand, context);
  context->PushIRStack(operation);
}

template <OpType opType>
void ResolveOpCast(LightningSpirVFrontEnd* translator, Lightning::TypeCastNode* node, LightningSpirVFrontEndContext* context)
{
  translator->PerformTypeCast(node, opType, context);
}

void ResolveFromBoolCast(LightningSpirVFrontEnd* translator,
                         Lightning::TypeCastNode* node,
                         ILightningShaderIR* plasma,
                         ILightningShaderIR* one,
                         LightningSpirVFrontEndContext* context)
{
  LightningShaderIROp* condition = translator->WalkAndGetValueTypeResult(node->Operand, context);
  LightningShaderIRType* destType = translator->FindType(node->ResultType, node);
  BasicBlock* currentBlock = context->GetCurrentBlock();
  ILightningShaderIR* operation = translator->GenerateFromBoolCast(currentBlock, condition, destType, plasma, one, context);
  context->PushIRStack(operation);
}

void ResolveBoolToIntCast(LightningSpirVFrontEnd* translator, Lightning::TypeCastNode* node, LightningSpirVFrontEndContext* context)
{
  ILightningShaderIR* one = translator->GetIntegerConstant(1, context);
  ILightningShaderIR* plasma = translator->GetIntegerConstant(0, context);
  ResolveFromBoolCast(translator, node, plasma, one, context);
}

void ResolveBoolToRealCast(LightningSpirVFrontEnd* translator,
                           Lightning::TypeCastNode* node,
                           LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* realType = translator->FindType(LightningTypeId(float), node, context);
  ILightningShaderIR* one = translator->GetConstant(realType, 1.0f, context);
  ILightningShaderIR* plasma = translator->GetConstant(realType, 0.0f, context);
  ResolveFromBoolCast(translator, node, plasma, one, context);
}

void ResolveToBoolCast(LightningSpirVFrontEnd* translator,
                       Lightning::TypeCastNode* node,
                       OpType op,
                       ILightningShaderIR* plasma,
                       LightningSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  LightningShaderIROp* condition = translator->WalkAndGetValueTypeResult(node->Operand, context);
  LightningShaderIRType* destType = translator->FindType(node->ResultType, node);
  ILightningShaderIR* operation = translator->GenerateToBoolCast(currentBlock, op, condition, destType, plasma, context);
  context->PushIRStack(operation);
}

void ResolveIntToBoolCast(LightningSpirVFrontEnd* translator, Lightning::TypeCastNode* node, LightningSpirVFrontEndContext* context)
{
  ILightningShaderIR* plasma = translator->GetIntegerConstant(0, context);
  ResolveToBoolCast(translator, node, OpType::OpINotEqual, plasma, context);
}

void ResolveRealToBoolCast(LightningSpirVFrontEnd* translator,
                           Lightning::TypeCastNode* node,
                           LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* realType = translator->FindType(LightningTypeId(float), node, context);
  ILightningShaderIR* plasma = translator->GetConstant(realType, 0.0f, context);
  ResolveToBoolCast(translator, node, OpType::OpFOrdNotEqual, plasma, context);
}

// Register function callbacks for all conversion operations (see Conversion
// Instructions in the spir-v spec). Some functions aren't implemented here as
// lightning doesn't have a corresponding function. Everything else should be
// implemented on the ShaderIntrinsics type.
void RegisterConversionOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  // Iterate over all dimensions of vector types (including scalar) to build
  // all supported conversions. Note: Bool conversions are not explicit
  // instructions in spir-v and must be generated from multiple instructions.
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* floatType = types.mRealVectorTypes[i];
    Lightning::BoundType* intType = types.mIntegerVectorTypes[i];
    Lightning::BoundType* boolType = types.mBooleanVectorTypes[i];

    opResolvers.RegisterTypeCastOpResolver(floatType, intType, ResolveOpCast<OpType::OpConvertFToS>);
    opResolvers.RegisterTypeCastOpResolver(intType, floatType, ResolveOpCast<OpType::OpConvertSToF>);

    opResolvers.RegisterTypeCastOpResolver(boolType, intType, ResolveBoolToIntCast);
    opResolvers.RegisterTypeCastOpResolver(intType, boolType, ResolveIntToBoolCast);

    opResolvers.RegisterTypeCastOpResolver(boolType, floatType, ResolveBoolToRealCast);
    opResolvers.RegisterTypeCastOpResolver(floatType, boolType, ResolveRealToBoolCast);
  }

  // Reinterpret cast is only supported between int and real (scalar) in lightning.
  Lightning::BoundType* realType = core.RealType;
  Lightning::BoundType* intType = core.IntegerType;
  // Register the re-interpret cast functions
  TypeResolvers& realTypeResolver = shaderLibrary->mTypeResolvers[realType];
  realTypeResolver.RegisterFunctionResolver(GetStaticFunction(realType, "Reinterpret", intType->ToString()),
                                            ResolveOpBitcast);
  TypeResolvers& intTypeResolver = shaderLibrary->mTypeResolvers[intType];
  intTypeResolver.RegisterFunctionResolver(GetStaticFunction(intType, "Reinterpret", realType->ToString()),
                                           ResolveOpBitcast);
}

} // namespace Plasma
