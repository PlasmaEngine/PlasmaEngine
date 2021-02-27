// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void DummyDefaultConstructorResolver(LightningSpirVFrontEnd* translator,
                                     Lightning::Type* resultType,
                                     LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyConstructorCallResolver(LightningSpirVFrontEnd* translator,
                                  Lightning::FunctionCallNode* fnCallNode,
                                  Lightning::StaticTypeNode* staticTypeNode,
                                  LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberAccessResolver(LightningSpirVFrontEnd* translator,
                               Lightning::MemberAccessNode* memberAccessNode,
                               LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberFunctionResolver(LightningSpirVFrontEnd* translator,
                                 Lightning::FunctionCallNode* functionCallNode,
                                 Lightning::MemberAccessNode* memberAccessNode,
                                 LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberPropertySetterResolver(LightningSpirVFrontEnd* translator,
                                       Lightning::MemberAccessNode* memberAccessNode,
                                       LightningShaderIROp* resultValue,
                                       LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyBinaryOpResolver(LightningSpirVFrontEnd* translator,
                           Lightning::BinaryOperatorNode* binaryOpNode,
                           LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyUnaryOpResolver(LightningSpirVFrontEnd* translator,
                          Lightning::UnaryOperatorNode* binaryOpNode,
                          LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyTypeCastResolver(LightningSpirVFrontEnd* translator,
                           Lightning::TypeCastNode* binaryOpNode,
                           LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyTemplateTypeReslover(LightningSpirVFrontEnd* translator, Lightning::BoundType* boundType)
{
  Error("This should not be called");
}

void DummyExpressionInitializerResolver(LightningSpirVFrontEnd* translator,
                                        Lightning::ExpressionInitializerNode*& node,
                                        LightningSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

} // namespace Plasma
