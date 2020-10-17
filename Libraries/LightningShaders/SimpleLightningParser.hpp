// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class SimpleLightningParser;

class SimpleLightningParserContext : public Lightning::WalkerContext<SimpleLightningParser, SimpleLightningParserContext>
{
public:
  StringBuilder mBuilder;
};

/// Simple parser for lightning that turns code into comments
class SimpleLightningParser
{
public:
  void Setup();

  String Run(Lightning::SyntaxNode* node);

  void WalkClassNode(Lightning::ClassNode*& node, SimpleLightningParserContext* context);
  void WalkClassVariables(Lightning::MemberVariableNode*& node, SimpleLightningParserContext* context);
  void WalkClassConstructor(Lightning::ConstructorNode*& node, SimpleLightningParserContext* context);
  void WalkClassFunction(Lightning::FunctionNode*& node, SimpleLightningParserContext* context);

  void WalkFunctionCallNode(Lightning::FunctionCallNode*& node, SimpleLightningParserContext* context);

  void WalkLocalVariable(Lightning::LocalVariableNode*& node, SimpleLightningParserContext* context);
  void WalkStaticTypeOrCreationCallNode(Lightning::StaticTypeNode*& node, SimpleLightningParserContext* context);
  void WalkExpressionInitializerNode(Lightning::ExpressionInitializerNode*& node, SimpleLightningParserContext* context);
  void WalkUnaryOperationNode(Lightning::UnaryOperatorNode*& node, SimpleLightningParserContext* context);
  void WalkBinaryOperationNode(Lightning::BinaryOperatorNode*& node, SimpleLightningParserContext* context);
  void WalkCastNode(Lightning::TypeCastNode*& node, SimpleLightningParserContext* context);
  void WalkValueNode(Lightning::ValueNode*& node, SimpleLightningParserContext* context);
  void WalkLocalRef(Lightning::LocalVariableReferenceNode*& node, SimpleLightningParserContext* context);
  void WalkGetterSetter(Lightning::MemberAccessNode*& node, Lightning::GetterSetter* getSet, SimpleLightningParserContext* context);
  void WalkMemberAccessNode(Lightning::MemberAccessNode*& node, SimpleLightningParserContext* context);
  void WalkMultiExpressionNode(Lightning::MultiExpressionNode*& node, SimpleLightningParserContext* context);

  void WalkIfRootNode(Lightning::IfRootNode*& node, SimpleLightningParserContext* context);
  void WalkIfNode(Lightning::IfNode*& node, SimpleLightningParserContext* context);
  void WalkBreakNode(Lightning::BreakNode*& node, SimpleLightningParserContext* context);
  void WalkContinueNode(Lightning::ContinueNode*& node, SimpleLightningParserContext* context);
  void WalkReturnNode(Lightning::ReturnNode*& node, SimpleLightningParserContext* context);
  void WalkWhileNode(Lightning::WhileNode*& node, SimpleLightningParserContext* context);
  void WalkDoWhileNode(Lightning::DoWhileNode*& node, SimpleLightningParserContext* context);
  void WalkForNode(Lightning::ForNode*& node, SimpleLightningParserContext* context);
  void WalkForEachNode(Lightning::ForEachNode*& node, SimpleLightningParserContext* context);

  typedef Lightning::BranchWalker<SimpleLightningParser, SimpleLightningParserContext> ParserBranchWalker;
  ParserBranchWalker mWalker;
};

} // namespace Plasma
