// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class LightningSpirVFrontEnd;

class LightningSpirVFrontEndContext : public Lightning::WalkerContext<LightningSpirVFrontEnd, LightningSpirVFrontEndContext>
{
public:
  LightningSpirVFrontEndContext();

  BasicBlock* GetCurrentBlock();

  void PushMergePoints(BasicBlock* continuePoint, BasicBlock* breakPoint);
  void PopMergeTargets();

  void PushIRStack(ILightningShaderIR* ir);
  ILightningShaderIR* PopIRStack();

  LightningShaderIRType* mCurrentType;
  BasicBlock* mCurrentBlock;

  Array<Pair<BasicBlock*, BasicBlock*>> mMergePointStack;
  BasicBlock* mContinueTarget;
  BasicBlock* mBreakTarget;

  Array<ILightningShaderIR*> mResultStack;
  HashMap<Lightning::Variable*, ILightningShaderIR*> mLightningVariableToIR;
  LightningShaderIRFunction* mCurrentFunction;

  LightningShaderDebugInfo mDebugInfo;
};

class LightningSpirVFrontEnd : public BaseShaderIRTranslator
{
public:
  ~LightningSpirVFrontEnd() override;

  // Tell the translator what settings to use for translation (Names, render
  // targets, etc...)
  void SetSettings(LightningShaderSpirVSettingsRef& settings) override;
  void Setup() override;

  // Translate a given project (and it's syntax tree) into the passed in
  // library. Each ShaderType in the library will contain translated pieces of
  // the target language. These pieces can be put together into a final shader
  // with "BuildFinalShader".
  bool Translate(Lightning::SyntaxTree& syntaxTree, LightningShaderIRProject* project, LightningShaderIRLibrary* library) override;

  LightningShaderIRType* MakeTypeInternal(LightningShaderIRLibrary* shaderLibrary,
                                      ShaderIRTypeBaseType::Enum baseType,
                                      StringParam typeName,
                                      Lightning::BoundType* lightningType,
                                      spv::StorageClass storageClass);
  LightningShaderIRType* MakeTypeAndPointer(LightningShaderIRLibrary* shaderLibrary,
                                        ShaderIRTypeBaseType::Enum baseType,
                                        StringParam typeName,
                                        Lightning::BoundType* lightningType,
                                        spv::StorageClass pointerStorageClass);
  LightningShaderIRType* MakeCoreType(LightningShaderIRLibrary* shaderLibrary,
                                  ShaderIRTypeBaseType::Enum baseType,
                                  u32 components,
                                  LightningShaderIRType* componentType,
                                  Lightning::BoundType* lightningType,
                                  bool makePointerType = true);
  LightningShaderIRType* MakeStructType(LightningShaderIRLibrary* shaderLibrary,
                                    StringParam typeName,
                                    Lightning::BoundType* lightningType,
                                    spv::StorageClass pointerStorageClass);
  LightningShaderIRType* FindOrCreateInterfaceType(LightningShaderIRLibrary* shaderLibrary,
                                               StringParam baseTypeName,
                                               Lightning::BoundType* lightningType,
                                               ShaderIRTypeBaseType::Enum baseType,
                                               spv::StorageClass storageClass);
  LightningShaderIRType* FindOrCreateInterfaceType(LightningShaderIRLibrary* shaderLibrary,
                                               Lightning::BoundType* lightningType,
                                               ShaderIRTypeBaseType::Enum baseType,
                                               spv::StorageClass storageClass);
  LightningShaderIRType* FindOrCreatePointerInterfaceType(LightningShaderIRLibrary* shaderLibrary,
                                                      LightningShaderIRType* valueType,
                                                      spv::StorageClass storageClass);
  ShaderIRTypeMeta* MakeShaderTypeMeta(LightningShaderIRType* shaderType,
                                       Lightning::NodeList<Lightning::AttributeNode>* nodeAttributeList);

  void ExtractLightningAsComments(Lightning::SyntaxNode*& node, LightningSpirVFrontEndContext* context);

  // Parse and validate attributes for a type. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Lightning::Array<Lightning::Attribute>& lightningAttributes,
                       Lightning::NodeList<Lightning::AttributeNode>* attributeNodes,
                       ShaderIRTypeMeta* typeMeta);
  // Parse and validate attributes for a function. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Lightning::Array<Lightning::Attribute>& lightningAttributes,
                       Lightning::NodeList<Lightning::AttributeNode>* attributeNodes,
                       ShaderIRFunctionMeta* functionMeta);
  // Parse and validate attributes for a field. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Lightning::Array<Lightning::Attribute>& lightningAttributes,
                       Lightning::NodeList<Lightning::AttributeNode>* attributeNodes,
                       ShaderIRFieldMeta* fieldMeta);
  void ParseLightningAttributes(Lightning::Array<Lightning::Attribute>& lightningAttributes,
                            Lightning::NodeList<Lightning::AttributeNode>* attributeNodes,
                            ShaderIRAttributeList& shaderAttributes);
  // Loads a lightning attribute into an IR attribute. Uses the node's location if
  // available.
  void ParseLightningAttribute(Lightning::Attribute& lightningAttribute,
                           Lightning::AttributeNode* attributeNode,
                           ShaderIRAttributeList& shaderAttributes);
  // Validates that the given attribute list doesn't contain any invalid
  // attributes. Uses 'errorTypeName' to display what kind of thing (e.g.
  // field/function) owned these attributes.
  void ValidateAllowedAttributes(ShaderIRAttributeList& shaderAttributes,
                                 HashMap<String, AttributeInfo>& allowedAttributes,
                                 StringParam errorTypeName);
  void ValidateNameOverrideAttribute(ShaderIRAttribute* shaderAttribute);
  void ValidateSingleParamAttribute(ShaderIRAttribute* shaderAttribute,
                                    StringParam expectedParamName,
                                    Lightning::ConstantType::Enum expectedParamType,
                                    bool allowEmptyName);
  void ValidateAttributeNoParameters(ShaderIRAttribute* shaderAttribute);
  // Validates that the given attribute has all dependency attributes specified
  void ValidateAttributeDependencies(ShaderIRAttribute* shaderAttribute,
                                     ShaderIRAttributeList& shaderAttributeList,
                                     Array<String>& dependencies);
  // Validates that none of the given attribute names are also present. Needed
  // to have exclusive attribute combinations.
  void ValidateAttributeExclusions(ShaderIRAttribute* shaderAttribute,
                                   ShaderIRAttributeList& shaderAttributeList,
                                   Array<String>& exclusions);
  void ValidateHardwareBuiltIn(ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* shaderAttribute, bool isInput);
  void ValidateAndParseComputeAttributeParameters(ShaderIRAttribute* shaderAttribute, ShaderIRTypeMeta* typeMeta);
  void ValidateLocalSize(ShaderIRAttributeParameter& param, int max, int& toStore);

  String BuildFunctionTypeString(Lightning::Function* lightningFunction, LightningSpirVFrontEndContext* context);
  String BuildFunctionTypeString(Lightning::BoundType* lightningReturnType,
                                 Array<Lightning::Type*>& signature,
                                 LightningSpirVFrontEndContext* context);
  void GenerateFunctionType(Lightning::SyntaxNode* locationNode,
                            LightningShaderIRFunction* function,
                            Lightning::Function* lightningFunction,
                            LightningSpirVFrontEndContext* context);
  void GenerateFunctionType(Lightning::SyntaxNode* locationNode,
                            LightningShaderIRFunction* function,
                            Lightning::BoundType* lightningReturnType,
                            Array<Lightning::Type*>& signature,
                            LightningSpirVFrontEndContext* context);
  LightningShaderIRFunction* GenerateIRFunction(Lightning::SyntaxNode* node,
                                            Lightning::NodeList<Lightning::AttributeNode>* nodeAttributeList,
                                            LightningShaderIRType* owningType,
                                            Lightning::Function* lightningFunction,
                                            StringParam functionName,
                                            LightningSpirVFrontEndContext* context);
  void AddImplements(Lightning::SyntaxNode* node,
                     Lightning::Function* lightningFunction,
                     LightningShaderIRFunction* shaderFunction,
                     StringParam functionName,
                     LightningSpirVFrontEndContext* context);

  void CollectClassTypes(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context);
  void CollectEnumTypes(Lightning::EnumNode*& node, LightningSpirVFrontEndContext* context);

  void PreWalkClassNode(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context);
  void PreWalkTemplateTypes(LightningSpirVFrontEndContext* context);
  void PreWalkTemplateType(Lightning::BoundType* lightningType, LightningSpirVFrontEndContext* context);
  void PreWalkClassVariables(Lightning::MemberVariableNode*& node, LightningSpirVFrontEndContext* context);
  void AddRuntimeArray(Lightning::MemberVariableNode* node, LightningShaderIRType* varType, LightningSpirVFrontEndContext* context);
  void AddGlobalVariable(Lightning::MemberVariableNode* node,
                         LightningShaderIRType* varType,
                         spv::StorageClass storageClass,
                         LightningSpirVFrontEndContext* context);
  void PreWalkClassConstructor(Lightning::ConstructorNode*& node, LightningSpirVFrontEndContext* context);
  void PreWalkClassFunction(Lightning::FunctionNode*& node, LightningSpirVFrontEndContext* context);
  void PreWalkMainFunction(Lightning::FunctionNode*& node, LightningSpirVFrontEndContext* context);
  void PreWalkErrorCheck(LightningSpirVFrontEndContext* context);

  void WalkClassNode(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context);
  void WalkClassVariables(Lightning::MemberVariableNode*& node, LightningSpirVFrontEndContext* context);
  void GeneratePreConstructor(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context);
  void GenerateDefaultConstructor(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context);
  void GenerateDummyMemberVariable(Lightning::ClassNode*& node, LightningSpirVFrontEndContext* context);
  void GenerateStaticVariableInitializer(Lightning::MemberVariableNode*& node, LightningSpirVFrontEndContext* context);
  void WalkClassConstructor(Lightning::ConstructorNode*& node, LightningSpirVFrontEndContext* context);
  void WalkClassFunction(Lightning::FunctionNode*& node, LightningSpirVFrontEndContext* context);

  /// Sets selfVar to the default constructed value for the given type.
  void DefaultConstructType(Lightning::SyntaxNode* locationNode,
                            LightningShaderIRType* type,
                            LightningShaderIROp* selfVar,
                            LightningSpirVFrontEndContext* context);
  /// Generate the function parameters for a given function node.
  void GenerateFunctionParameters(Lightning::GenericFunctionNode* node, LightningSpirVFrontEndContext* context);
  /// Generate the function body (statements) for a given function node. May
  /// generate an entry point if needed.
  void GenerateFunctionBody(Lightning::GenericFunctionNode* node, LightningSpirVFrontEndContext* context);
  void GenerateEntryPoint(Lightning::GenericFunctionNode* node,
                          LightningShaderIRFunction* function,
                          LightningSpirVFrontEndContext* context);

  void WalkFunctionCallNode(Lightning::FunctionCallNode*& node, LightningSpirVFrontEndContext* context);
  void WalkConstructorCallNode(Lightning::FunctionCallNode*& node,
                               Lightning::StaticTypeNode* constructorNode,
                               LightningSpirVFrontEndContext* context);
  void WalkMemberAccessCallNode(Lightning::FunctionCallNode*& node,
                                Lightning::MemberAccessNode* memberAccessNode,
                                LightningSpirVFrontEndContext* context);
  void WalkMemberAccessFunctionCallNode(Lightning::FunctionCallNode*& node,
                                        Lightning::MemberAccessNode* memberAccessNode,
                                        LightningShaderIRFunction* shaderFunction,
                                        LightningSpirVFrontEndContext* context);
  void WalkMemberAccessFunctionCall(Array<ILightningShaderIR*>& arguments,
                                    Lightning::MemberAccessNode* memberAccessNode,
                                    LightningShaderIRFunction* shaderFunction,
                                    LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GenerateFunctionCall(LightningShaderIRFunction* shaderFunction, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GenerateFunctionCall(BasicBlock* block,
                                        LightningShaderIRFunction* shaderFunction,
                                        LightningSpirVFrontEndContext* context);
  void WalkMemberAccessExtensionInstructionCallNode(Lightning::FunctionCallNode*& node,
                                                    Lightning::MemberAccessNode* memberAccessNode,
                                                    SpirVExtensionInstruction* extensionInstruction,
                                                    LightningSpirVFrontEndContext* context);

  void WalkLocalVariable(Lightning::LocalVariableNode*& node, LightningSpirVFrontEndContext* context);
  void WalkStaticTypeOrCreationCallNode(Lightning::StaticTypeNode*& node, LightningSpirVFrontEndContext* context);
  void WalkExpressionInitializerNode(Lightning::ExpressionInitializerNode*& node, LightningSpirVFrontEndContext* context);
  void WalkUnaryOperationNode(Lightning::UnaryOperatorNode*& node, LightningSpirVFrontEndContext* context);
  void WalkBinaryOperationNode(Lightning::BinaryOperatorNode*& node, LightningSpirVFrontEndContext* context);
  void WalkCastNode(Lightning::TypeCastNode*& node, LightningSpirVFrontEndContext* context);
  void WalkValueNode(Lightning::ValueNode*& node, LightningSpirVFrontEndContext* context);
  void WalkLocalRef(Lightning::LocalVariableReferenceNode*& node, LightningSpirVFrontEndContext* context);
  void WalkMemberAccessNode(Lightning::MemberAccessNode*& node, LightningSpirVFrontEndContext* context);
  void WalkMultiExpressionNode(Lightning::MultiExpressionNode*& node, LightningSpirVFrontEndContext* context);

  void WalkIfRootNode(Lightning::IfRootNode*& node, LightningSpirVFrontEndContext* context);
  void WalkIfNode(Lightning::IfNode*& node, LightningSpirVFrontEndContext* context);
  void WalkBreakNode(Lightning::BreakNode*& node, LightningSpirVFrontEndContext* context);
  void WalkContinueNode(Lightning::ContinueNode*& node, LightningSpirVFrontEndContext* context);
  void WalkReturnNode(Lightning::ReturnNode*& node, LightningSpirVFrontEndContext* context);
  void WalkWhileNode(Lightning::WhileNode*& node, LightningSpirVFrontEndContext* context);
  void WalkDoWhileNode(Lightning::DoWhileNode*& node, LightningSpirVFrontEndContext* context);
  void WalkForNode(Lightning::ForNode*& node, LightningSpirVFrontEndContext* context);
  void WalkForEachNode(Lightning::ForEachNode*& node, LightningSpirVFrontEndContext* context);
  void WalkLoopNode(Lightning::LoopNode*& node, LightningSpirVFrontEndContext* context);
  // Loop helper functions
  void WalkGenericLoop(Lightning::SyntaxNode* initializerNode,
                       Lightning::SyntaxNode* iterator,
                       Lightning::ConditionalLoopNode* conditionalNode,
                       Lightning::LoopScopeNode* loopScopeNode,
                       LightningSpirVFrontEndContext* context);
  void GenerateLoopHeaderBlock(BasicBlock* headerBlock,
                               BasicBlock* branchTarget,
                               BasicBlock* mergeBlock,
                               BasicBlock* continueBlock,
                               LightningSpirVFrontEndContext* context);
  void GenerateLoopConditionBlock(Lightning::ConditionalLoopNode* conditionalNode,
                                  BasicBlock* conditionBlock,
                                  BasicBlock* branchTrueBlock,
                                  BasicBlock* branchFalseBlock,
                                  LightningSpirVFrontEndContext* context);
  void GenerateLoopStatements(Lightning::LoopScopeNode* loopScopeNode,
                              BasicBlock* loopBlock,
                              BasicBlock* mergeBlock,
                              BasicBlock* continueBlock,
                              LightningSpirVFrontEndContext* context);
  void GenerateLoopContinueBlock(Lightning::SyntaxNode* iterator,
                                 BasicBlock* continueBlock,
                                 BasicBlock* headerBlock,
                                 LightningSpirVFrontEndContext* context);

  /// Walk a block and make sure that it has exactly one termination condition.
  /// If there's plasma then a return will be added. If there's more than one then
  /// all instructions after the first terminator will be removed.
  void FixBlockTerminators(BasicBlock* block, LightningSpirVFrontEndContext* context);

  /// Get the setter (if available) from a member access node.
  Lightning::Function* GetSetter(Lightning::MemberAccessNode* memberAccessNode);
  /// Attempt to invert a binary op node (must be an assignment) into calling a
  /// setter.
  // If the given result node is null then the right hand side of the binary op
  // node is walked but it can be manually passed in for ops with extra
  // expressions (e.g. +=).
  bool ResolveSetter(Lightning::BinaryOperatorNode* node,
                     LightningShaderIROp* resultValue,
                     Lightning::SyntaxNode* resultNode,
                     LightningSpirVFrontEndContext* context);

  // Helpers to perform standard binary operations
  ILightningShaderIR* PerformBinaryOp(Lightning::BinaryOperatorNode*& node, OpType opType, LightningSpirVFrontEndContext* context);
  ILightningShaderIR* PerformBinaryOp(Lightning::BinaryOperatorNode*& node,
                                  OpType opType,
                                  ILightningShaderIR* lhs,
                                  ILightningShaderIR* rhs,
                                  LightningSpirVFrontEndContext* context);
  void PerformBinaryAssignmentOp(Lightning::BinaryOperatorNode*& node, OpType opType, LightningSpirVFrontEndContext* context);
  void PerformBinaryAssignmentOp(Lightning::BinaryOperatorNode*& node,
                                 OpType opType,
                                 ILightningShaderIR* lhs,
                                 ILightningShaderIR* rhs,
                                 LightningSpirVFrontEndContext* context);
  // Helpers to perform standard unary operations
  ILightningShaderIR* PerformUnaryOp(Lightning::UnaryOperatorNode*& node, OpType opType, LightningSpirVFrontEndContext* context);
  ILightningShaderIR* PerformUnaryIncDecOp(Lightning::UnaryOperatorNode*& node,
                                       ILightningShaderIR* constantOne,
                                       OpType opType,
                                       LightningSpirVFrontEndContext* context);
  // Helpers to perform standard typecast operations
  ILightningShaderIR* PerformTypeCast(Lightning::TypeCastNode*& node, OpType opType, LightningSpirVFrontEndContext* context);

  LightningShaderIROp* GetIntegerConstant(int value, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GetIntegerConstant(u32 value, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GetConstant(LightningShaderIRType* type, StringParam value, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GetConstant(LightningShaderIRType* type, Lightning::Any value, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* ConstructCompositeFromScalar(BasicBlock* block,
                                                LightningShaderIRType* compositeType,
                                                ILightningShaderIR* scalar,
                                                LightningSpirVFrontEndContext* context);

  /// Create a specialization constant from a member variable node.
  LightningShaderIROp* AddSpecializationConstant(Lightning::MemberVariableNode* node,
                                             LightningShaderIRType* varType,
                                             LightningSpirVFrontEndContext* context);
  /// Creates a specialization constant of the given type with the given unique
  /// lookup key. If a specialization constant is not a scalar, then it must be
  /// iteratively constructed from other specialization constants. If non-null,
  /// the given literal value will be used to initialize the constant (only
  /// valid for scalars).
  LightningShaderIROp* AddSpecializationConstantRecursively(void* key,
                                                        LightningShaderIRType* varType,
                                                        StringParam varName,
                                                        LightningShaderIRConstantLiteral* literalValue,
                                                        Lightning::CodeLocation& codeLocation,
                                                        LightningSpirVFrontEndContext* context);
  /// Create a specialization constant of a given variable type. The given key
  /// should be a unique identifier for this variable that can be used to fetch
  /// the specialization constant again at a later time. A null key specifies
  /// that there is no key (this variable can't be looked up later). This is
  /// typically used for sub-constants. The OpType should be specified to choose
  /// between scalar and composite constants.
  LightningShaderIROp* CreateSpecializationConstant(void* key,
                                                OpType opType,
                                                LightningShaderIRType* varType,
                                                LightningSpirVFrontEndContext* context);

  void ToAny(LightningShaderIRType* type, StringParam value, Lightning::Any& result);

  void ExtractDebugInfo(Lightning::SyntaxNode* node, LightningShaderDebugInfo& debugInfo);
  void GetLineAsComment(Lightning::SyntaxNode* node, LightningShaderIRComments& comments);
  BasicBlock* BuildBlock(StringParam labelDebugName, LightningSpirVFrontEndContext* context);
  BasicBlock* BuildBlockNoStack(StringParam labelDebugName, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildIROpNoBlockAdd(OpType opType,
                                       LightningShaderIRType* resultType,
                                       LightningSpirVFrontEndContext* context);
  LightningShaderIROp*
  BuildIROp(BasicBlock* block, OpType opType, LightningShaderIRType* resultType, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildIROp(BasicBlock* block,
                             OpType opType,
                             LightningShaderIRType* resultType,
                             ILightningShaderIR* arg0,
                             LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildIROp(BasicBlock* block,
                             OpType opType,
                             LightningShaderIRType* resultType,
                             ILightningShaderIR* arg0,
                             ILightningShaderIR* arg1,
                             LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildIROp(BasicBlock* block,
                             OpType opType,
                             LightningShaderIRType* resultType,
                             ILightningShaderIR* arg0,
                             ILightningShaderIR* arg1,
                             ILightningShaderIR* arg2,
                             LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildCurrentBlockIROp(OpType opType,
                                         LightningShaderIRType* resultType,
                                         LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildCurrentBlockIROp(OpType opType,
                                         LightningShaderIRType* resultType,
                                         ILightningShaderIR* arg0,
                                         LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildCurrentBlockIROp(OpType opType,
                                         LightningShaderIRType* resultType,
                                         ILightningShaderIR* arg0,
                                         ILightningShaderIR* arg1,
                                         LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildCurrentBlockIROp(OpType opType,
                                         LightningShaderIRType* resultType,
                                         ILightningShaderIR* arg0,
                                         ILightningShaderIR* arg1,
                                         ILightningShaderIR* arg2,
                                         LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildCurrentBlockAccessChain(LightningShaderIRType* baseResultType,
                                                LightningShaderIROp* selfInstance,
                                                ILightningShaderIR* arg0,
                                                LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildCurrentBlockAccessChain(LightningShaderIRType* baseResultType,
                                                LightningShaderIROp* selfInstance,
                                                ILightningShaderIR* arg0,
                                                ILightningShaderIR* arg1,
                                                LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildDecorationOp(BasicBlock* block,
                                     ILightningShaderIR* decorationTarget,
                                     spv::Decoration decorationType,
                                     LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildDecorationOp(BasicBlock* block,
                                     ILightningShaderIR* decorationTarget,
                                     spv::Decoration decorationType,
                                     u32 decorationValue,
                                     LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildMemberDecorationOp(BasicBlock* block,
                                           ILightningShaderIR* decorationTarget,
                                           u32 memberOffset,
                                           spv::Decoration decorationType,
                                           LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildMemberDecorationOp(BasicBlock* block,
                                           ILightningShaderIR* decorationTarget,
                                           u32 memberOffset,
                                           spv::Decoration decorationType,
                                           u32 decorationValue,
                                           LightningSpirVFrontEndContext* context);
  LightningShaderIRConstantLiteral* GetOrCreateConstantIntegerLiteral(int value);
  LightningShaderIRConstantLiteral* GetOrCreateConstantIntegerLiteral(u32 value);
  LightningShaderIRConstantLiteral* GetOrCreateConstantLiteral(Lightning::Any value);
  LightningShaderIROp* BuildOpVariable(LightningShaderIRType* resultType, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* BuildOpVariable(BasicBlock* block,
                                   LightningShaderIRType* resultType,
                                   int storageConstant,
                                   LightningSpirVFrontEndContext* context);
  ILightningShaderIR* WalkAndGetResult(Lightning::SyntaxNode* node, LightningSpirVFrontEndContext* context);
  LightningShaderIROp* WalkAndGetValueTypeResult(BasicBlock* block,
                                             Lightning::SyntaxNode* node,
                                             LightningSpirVFrontEndContext* context);
  LightningShaderIROp* WalkAndGetValueTypeResult(Lightning::SyntaxNode* node, LightningSpirVFrontEndContext* context);
  // If this is an immediate then the op is returned. If the op is a pointer
  // than a load is generated and the load is returned.
  LightningShaderIROp* GetOrGenerateValueTypeFromIR(BasicBlock* block,
                                                ILightningShaderIR* instruction,
                                                LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GetOrGenerateValueTypeFromIR(ILightningShaderIR* instruction, LightningSpirVFrontEndContext* context);
  // If this is a pointer type (e.g. variable, parameter, etc...) then the op is
  // returned. Otherwise a new variable is generated, the immediate is stored
  // into it, and the variable is returned.
  LightningShaderIROp* GetOrGeneratePointerTypeFromIR(BasicBlock* block,
                                                  ILightningShaderIR* instruction,
                                                  LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GetOrGeneratePointerTypeFromIR(ILightningShaderIR* instruction, LightningSpirVFrontEndContext* context);
  // Build an op to store the source into the target. May generate OpStore or
  // OpCopyMemor depending on the type of source.
  LightningShaderIROp* BuildStoreOp(ILightningShaderIR* target,
                                ILightningShaderIR* source,
                                LightningSpirVFrontEndContext* context,
                                bool forceLoadStore = true);
  LightningShaderIROp* BuildStoreOp(BasicBlock* block,
                                ILightningShaderIR* target,
                                ILightningShaderIR* source,
                                LightningSpirVFrontEndContext* context,
                                bool forceLoadStore = true);
  void WriteFunctionCallArguments(Lightning::FunctionCallNode*& node,
                                  LightningShaderIROp* functionCallOp,
                                  LightningSpirVFrontEndContext* context);
  void WriteFunctionCallArguments(Lightning::FunctionCallNode*& node,
                                  Lightning::Type* returnType,
                                  LightningShaderIROp* functionCallOp,
                                  LightningSpirVFrontEndContext* context);
  void WriteFunctionCallArguments(Array<ILightningShaderIR*> arguments,
                                  LightningShaderIRType* returnType,
                                  LightningShaderIRType* functionType,
                                  LightningShaderIROp* functionCallOp,
                                  LightningSpirVFrontEndContext* context);
  void WriteFunctionCallArgument(ILightningShaderIR* argument,
                                 LightningShaderIROp* functionCallOp,
                                 LightningShaderIRType* paramType,
                                 LightningSpirVFrontEndContext* context);

  // Helpers
  LightningShaderIROp* GenerateBoolToIntegerCast(BasicBlock* block,
                                             LightningShaderIROp* source,
                                             LightningShaderIRType* destType,
                                             LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GenerateFromBoolCast(BasicBlock* block,
                                        LightningShaderIROp* source,
                                        LightningShaderIRType* destType,
                                        ILightningShaderIR* plasma,
                                        ILightningShaderIR* one,
                                        LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GenerateIntegerToBoolCast(BasicBlock* block,
                                             LightningShaderIROp* source,
                                             LightningShaderIRType* destType,
                                             LightningSpirVFrontEndContext* context);
  LightningShaderIROp* GenerateToBoolCast(BasicBlock* block,
                                      OpType op,
                                      LightningShaderIROp* source,
                                      LightningShaderIRType* destType,
                                      ILightningShaderIR* plasma,
                                      LightningSpirVFrontEndContext* context);

  // Get the value result type from an an op code. If this is a pointer type it
  // will get the dereference type.
  LightningShaderIRType* GetResultValueType(LightningShaderIROp* op);
  // Get the pointer result type from an an op code. If this is a value type it
  // will get the pointer type.
  LightningShaderIRType* GetPointerValueType(LightningShaderIROp* op);
  // Helper to check if the given type contains an attribute. If the type
  // doesn't have meta this returns false.
  bool ContainsAttribute(LightningShaderIRType* shaderType, StringParam attributeName);
  // Check if a type contains the non-copyable attribute (returns true if
  // non-copyable). Throws an error if this type is non-copyable and optionally
  // generates a dummy variable.
  bool CheckForNonCopyableType(LightningShaderIRType* shaderType,
                               Lightning::ExpressionNode* node,
                               LightningSpirVFrontEndContext* context,
                               bool generateDummyResult = true);

  LightningShaderIRType* FindType(Lightning::Type* type, Lightning::SyntaxNode* syntaxNode, bool reportErrors = true);
  LightningShaderIRType* FindType(Lightning::ExpressionNode* syntaxNode, bool reportErrors = true);
  // Validate that the result type of the given instruction (must be an op) is
  // of a certain type (e.g. pointer).
  bool ValidateResultType(ILightningShaderIR* instruction,
                          ShaderIRTypeBaseType::Enum expectedType,
                          Lightning::CodeLocation& codeLocation,
                          bool throwException = true);
  /// Verifies that the given instruction can be written to. This includes
  /// checking for specialization constants.
  bool ValidateLValue(LightningShaderIROp* op, Lightning::CodeLocation& codeLocation, bool throwException = true);

  // Generates a dummy instruction based upon the result type of the given node.
  // Used to make translation not crash after errors.
  ILightningShaderIR* GenerateDummyIR(Lightning::ExpressionNode* node, LightningSpirVFrontEndContext* context);
  // Send a translation error with a simple message (also marks the translation
  // as having failed)
  void SendTranslationError(Lightning::CodeLocation& codeLocation, StringParam message);
  void SendTranslationError(Lightning::CodeLocation& codeLocation, StringParam shortMsg, StringParam fullMsg);
  // Send a translation error. If the location is null then a dummy location is
  // used (e.g. native types).
  void SendTranslationError(Lightning::CodeLocation* codeLocation, StringParam message);

  typedef Lightning::BranchWalker<LightningSpirVFrontEnd, LightningSpirVFrontEndContext> TranslatorBranchWalker;
  TranslatorBranchWalker mWalker;
  SimpleLightningParser mLightningCommentParser;
  LightningSpirVFrontEndContext* mContext;

  LightningShaderSpirVSettingsRef mSettings;
  LightningShaderIRLibrary* mLibrary;
  LightningShaderIRProject* mProject;

  // Was an error ever triggered?
  bool mErrorTriggered;
};

} // namespace Plasma
