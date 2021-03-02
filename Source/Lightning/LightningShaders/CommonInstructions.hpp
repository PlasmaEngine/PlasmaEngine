// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

void DummyDefaultConstructorResolver(LightningSpirVFrontEnd* translator,
                                     Lightning::Type* resultType,
                                     LightningSpirVFrontEndContext* context);
void DummyConstructorCallResolver(LightningSpirVFrontEnd* translator,
                                  Lightning::FunctionCallNode* fnCallNode,
                                  Lightning::StaticTypeNode* staticTypeNode,
                                  LightningSpirVFrontEndContext* context);
void DummyMemberAccessResolver(LightningSpirVFrontEnd* translator,
                               Lightning::MemberAccessNode* memberAccessNode,
                               LightningSpirVFrontEndContext* context);
void DummyMemberFunctionResolver(LightningSpirVFrontEnd* translator,
                                 Lightning::FunctionCallNode* functionCallNode,
                                 Lightning::MemberAccessNode* memberAccessNode,
                                 LightningSpirVFrontEndContext* context);
void DummyMemberPropertySetterResolver(LightningSpirVFrontEnd* translator,
                                       Lightning::MemberAccessNode* memberAccessNode,
                                       LightningShaderIROp* resultValue,
                                       LightningSpirVFrontEndContext* context);
void DummyBinaryOpResolver(LightningSpirVFrontEnd* translator,
                           Lightning::BinaryOperatorNode* binaryOpNode,
                           LightningSpirVFrontEndContext* context);
void DummyUnaryOpResolver(LightningSpirVFrontEnd* translator,
                          Lightning::UnaryOperatorNode* binaryOpNode,
                          LightningSpirVFrontEndContext* context);
void DummyTypeCastResolver(LightningSpirVFrontEnd* translator,
                           Lightning::TypeCastNode* binaryOpNode,
                           LightningSpirVFrontEndContext* context);
void DummyTemplateTypeReslover(LightningSpirVFrontEnd* translator, Lightning::BoundType* boundType);
void DummyExpressionInitializerResolver(LightningSpirVFrontEnd* translator,
                                        Lightning::ExpressionInitializerNode*& node,
                                        LightningSpirVFrontEndContext* context);

} // namespace Plasma
