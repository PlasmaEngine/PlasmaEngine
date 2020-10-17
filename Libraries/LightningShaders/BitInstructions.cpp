// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void ResolveIntBitCount(LightningSpirVFrontEnd* translator,
                        Lightning::FunctionCallNode* functionCallNode,
                        Lightning::MemberAccessNode* memberAccessNode,
                        LightningSpirVFrontEndContext* context)
{
  LightningShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  LightningShaderIROp* baseOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  LightningShaderIROp* bitCountOp = translator->BuildCurrentBlockIROp(OpType::OpBitCount, resultType, baseOp, context);
  context->PushIRStack(bitCountOp);
}

// Register function callbacks for all bit operations (see Bit Instructions in
// the spir-v spec). Some functions aren't implemented here as lightning doesn't
// have a corresponding function. Everything else should be implemented on the
// ShaderIntrinsics type.
void RegisterBitOps(LightningSpirVFrontEnd* translator, LightningShaderIRLibrary* shaderLibrary, LightningTypeGroups& types)
{
  Lightning::Core& core = Lightning::Core::GetInstance();
  Lightning::BoundType* mathType = core.MathType;
  TypeResolvers& mathTypeResolver = shaderLibrary->mTypeResolvers[mathType];
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  Lightning::BoundType* realType = core.RealType;
  Lightning::BoundType* intType = core.IntegerType;
  Lightning::BoundType* boolType = core.BooleanType;

  // Register ops that are on all integer vector types
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Lightning::BoundType* lightningType = types.mIntegerVectorTypes[i];
    String lightningTypeName = lightningType->ToString();

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "CountBits", lightningTypeName),
                                              ResolveIntBitCount);

    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::BitwiseOr, ResolveBinaryOperator<OpType::OpBitwiseOr>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::BitwiseAnd, ResolveBinaryOperator<OpType::OpBitwiseAnd>);
    opResolvers.RegisterBinaryOpResolver(
        lightningType, lightningType, Lightning::Grammar::BitwiseXor, ResolveBinaryOperator<OpType::OpBitwiseXor>);
    opResolvers.RegisterUnaryOpResolver(lightningType, Lightning::Grammar::BitwiseNot, ResolveUnaryOperator<OpType::OpNot>);
  }
}

} // namespace Plasma
