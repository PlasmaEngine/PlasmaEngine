// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void SimpleLightningParser::Setup()
{
  mWalker.Register(&SimpleLightningParser::WalkClassNode);
  mWalker.Register(&SimpleLightningParser::WalkClassVariables);
  mWalker.Register(&SimpleLightningParser::WalkClassConstructor);
  mWalker.Register(&SimpleLightningParser::WalkClassFunction);
  mWalker.Register(&SimpleLightningParser::WalkFunctionCallNode);
  mWalker.Register(&SimpleLightningParser::WalkLocalVariable);
  mWalker.Register(&SimpleLightningParser::WalkStaticTypeOrCreationCallNode);
  mWalker.Register(&SimpleLightningParser::WalkExpressionInitializerNode);
  mWalker.Register(&SimpleLightningParser::WalkUnaryOperationNode);
  mWalker.Register(&SimpleLightningParser::WalkBinaryOperationNode);
  mWalker.Register(&SimpleLightningParser::WalkCastNode);
  mWalker.Register(&SimpleLightningParser::WalkValueNode);
  mWalker.Register(&SimpleLightningParser::WalkLocalRef);
  mWalker.Register(&SimpleLightningParser::WalkMemberAccessNode);
  mWalker.Register(&SimpleLightningParser::WalkIfRootNode);
  mWalker.Register(&SimpleLightningParser::WalkIfNode);
  mWalker.Register(&SimpleLightningParser::WalkContinueNode);
  mWalker.Register(&SimpleLightningParser::WalkBreakNode);
  mWalker.Register(&SimpleLightningParser::WalkReturnNode);
  mWalker.Register(&SimpleLightningParser::WalkWhileNode);
  mWalker.Register(&SimpleLightningParser::WalkDoWhileNode);
  mWalker.Register(&SimpleLightningParser::WalkForNode);
  mWalker.Register(&SimpleLightningParser::WalkForEachNode);
}

String SimpleLightningParser::Run(Lightning::SyntaxNode* node)
{
  SimpleLightningParserContext context;

  mWalker.Walk(this, node, &context);
  return context.mBuilder.ToString();
}

void SimpleLightningParser::WalkClassNode(Lightning::ClassNode*& node, SimpleLightningParserContext* context)
{
  if (node->CopyMode == Lightning::TypeCopyMode::ValueType)
    context->mBuilder << "struct " << node->Name.Token;
  else
    context->mBuilder << "class " << node->Name.Token;
}
void SimpleLightningParser::WalkClassVariables(Lightning::MemberVariableNode*& node, SimpleLightningParserContext* context)
{
}
void SimpleLightningParser::WalkClassConstructor(Lightning::ConstructorNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << "constructor(";
  mWalker.Walk(this, node->Parameters, context);
  context->mBuilder << ")";
}
void SimpleLightningParser::WalkClassFunction(Lightning::FunctionNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << "function " << node->Name.Token << "(";
  // Add all of the parameters
  size_t size = node->Parameters.Size();
  for (size_t i = 0; i < size; ++i)
  {
    Lightning::ParameterNode* paramNode = node->Parameters[i];
    context->mBuilder << paramNode->Name.Token << " : " << paramNode->ResultType->ToString();
    if (i != size - 1)
      context->mBuilder << ", ";
  }

  context->mBuilder << ")";
}
void SimpleLightningParser::WalkFunctionCallNode(Lightning::FunctionCallNode*& node, SimpleLightningParserContext* context)
{
  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << "(";
  mWalker.Walk(this, node->Arguments, context);
  context->mBuilder << ")";
}
void SimpleLightningParser::WalkLocalVariable(Lightning::LocalVariableNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << "var " << node->Name.Token;
  if (node->InitialValue != nullptr)
  {
    context->mBuilder << " = ";
    mWalker.Walk(this, node->InitialValue, context);
  }
  context->mBuilder << ";";
}
void SimpleLightningParser::WalkStaticTypeOrCreationCallNode(Lightning::StaticTypeNode*& node,
                                                         SimpleLightningParserContext* context)
{
  context->mBuilder << node->ReferencedType->Name;
}
void SimpleLightningParser::WalkExpressionInitializerNode(Lightning::ExpressionInitializerNode*& node,
                                                      SimpleLightningParserContext* context)
{
}
void SimpleLightningParser::WalkUnaryOperationNode(Lightning::UnaryOperatorNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << node->Operator->Token;
  context->mBuilder << "(";
  mWalker.Walk(this, node->Operand, context);
  context->mBuilder << ")";
}
void SimpleLightningParser::WalkBinaryOperationNode(Lightning::BinaryOperatorNode*& node, SimpleLightningParserContext* context)
{
  HashSet<int> noGroupingOps;
  noGroupingOps.Insert((int)Lightning::Grammar::Assignment);
  noGroupingOps.Insert((int)Lightning::Grammar::AssignmentAdd);
  noGroupingOps.Insert((int)Lightning::Grammar::AssignmentSubtract);
  noGroupingOps.Insert((int)Lightning::Grammar::AssignmentMultiply);
  noGroupingOps.Insert((int)Lightning::Grammar::AssignmentDivide);

  bool needsGrouping = !noGroupingOps.Contains(node->OperatorInfo.Operator);

  if (needsGrouping)
    context->mBuilder << "(";

  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << " " << node->Operator->Token << " ";
  mWalker.Walk(this, node->RightOperand, context);

  if (needsGrouping)
    context->mBuilder << ")";
}
void SimpleLightningParser::WalkCastNode(Lightning::TypeCastNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << "(" << node->ResultType->ToString() << ")";
  mWalker.Walk(this, node->Operand, context);
}
void SimpleLightningParser::WalkValueNode(Lightning::ValueNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << node->Value.Token;
}
void SimpleLightningParser::WalkLocalRef(Lightning::LocalVariableReferenceNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << node->AccessedVariable->Name;
}
void SimpleLightningParser::WalkGetterSetter(Lightning::MemberAccessNode*& node,
                                         Lightning::GetterSetter* getSet,
                                         SimpleLightningParserContext* context)
{
}
void SimpleLightningParser::WalkMemberAccessNode(Lightning::MemberAccessNode*& node, SimpleLightningParserContext* context)
{
  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << Lightning::Grammar::GetKeywordOrSymbol(node->Operator);
  context->mBuilder << node->Name;
}
void SimpleLightningParser::WalkMultiExpressionNode(Lightning::MultiExpressionNode*& node, SimpleLightningParserContext* context)
{
}
void SimpleLightningParser::WalkIfRootNode(Lightning::IfRootNode*& node, SimpleLightningParserContext* context)
{
  // context->mBuilder << "if";
  // mWalker.Walk(this, node->IfParts, context);
}
void SimpleLightningParser::WalkIfNode(Lightning::IfNode*& node, SimpleLightningParserContext* context)
{
  if (node->IsFirstPart)
    context->mBuilder << "if";
  else if (node->Condition == nullptr)
    context->mBuilder << "else";
  else
    context->mBuilder << "else if";

  if (node->Condition != nullptr)
  {
    context->mBuilder << "(";
    mWalker.Walk(this, node->Condition, context);
    context->mBuilder << ")";
  }
  // mWalker.Walk(this, node->IfParts, context);
}
void SimpleLightningParser::WalkBreakNode(Lightning::BreakNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << "break";
}
void SimpleLightningParser::WalkContinueNode(Lightning::ContinueNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << "continue";
}
void SimpleLightningParser::WalkReturnNode(Lightning::ReturnNode*& node, SimpleLightningParserContext* context)
{
  context->mBuilder << "return";
  if (node->ReturnValue)
  {
    context->mBuilder << " ";
    mWalker.Walk(this, node->ReturnValue, context);
  }
}
void SimpleLightningParser::WalkWhileNode(Lightning::WhileNode*& node, SimpleLightningParserContext* context)
{
}
void SimpleLightningParser::WalkDoWhileNode(Lightning::DoWhileNode*& node, SimpleLightningParserContext* context)
{
}
void SimpleLightningParser::WalkForNode(Lightning::ForNode*& node, SimpleLightningParserContext* context)
{
}
void SimpleLightningParser::WalkForEachNode(Lightning::ForEachNode*& node, SimpleLightningParserContext* context)
{
}

} // namespace Plasma
