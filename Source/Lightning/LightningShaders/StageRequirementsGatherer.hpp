// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class StageRequirementsGatherer;

/// Context for stage requirements gathering. Used to store state needed during
/// iteration.
class StageRequirementsGathererContext
    : public Lightning::WalkerContext<StageRequirementsGatherer, StageRequirementsGathererContext>
{
public:
  HashMap<Lightning::Type*, Lightning::ClassNode*> mTypeMap;
  HashMap<Lightning::Function*, Lightning::FunctionNode*> mFunctionMap;
  HashMap<Lightning::Function*, Lightning::ConstructorNode*> mConstructorMap;
  HashMap<Lightning::Property*, Lightning::MemberVariableNode*> mVariableMap;
  HashSet<void*> mProcessedObjects;

  LightningShaderIRLibrary* mCurrentLibrary;
  StageRequirementsData mCurrentRequirements;

  /// Lightning library to shader library map. Needed during recursion to find the
  /// shader library for a symbol to check for cached stage requirements.
  HashMap<Lightning::Library*, LightningShaderIRLibrary*> mLightningLibraryToShaderLibraryMap;

  ShaderCompilationErrors* mErrors;
};

/// Helper lightning AST walker to find what symbols have various stage requirements
/// and emit errors when invalid combinations are found (e.g. a vertex calling a
/// pixel only function).
class StageRequirementsGatherer
{
public:
  StageRequirementsGatherer(LightningShaderSpirVSettings* settings);

  /// Run the requirements gatherer to find if any invalid shader stage
  /// combinations are found. Returns false if an error was found.
  bool Run(Lightning::SyntaxTree& syntaxTree, LightningShaderIRLibrary* library, ShaderCompilationErrors* errors);

private:
  void PreWalkClassNode(Lightning::ClassNode*& node, StageRequirementsGathererContext* context);
  void PreWalkConstructor(Lightning::ConstructorNode*& node, StageRequirementsGathererContext* context);
  void PreWalkClassFunction(Lightning::FunctionNode*& node, StageRequirementsGathererContext* context);
  void PreWalkClassMemberVariable(Lightning::MemberVariableNode*& node, StageRequirementsGathererContext* context);

  void WalkClassNode(Lightning::ClassNode*& node, StageRequirementsGathererContext* context);
  void WalkClassPreconstructor(Lightning::ClassNode*& node, StageRequirementsGathererContext* context);
  void WalkClassPreconstructor(Lightning::Function* preConstructor, StageRequirementsGathererContext* context);
  void WalkClassConstructor(Lightning::ConstructorNode*& node, StageRequirementsGathererContext* context);
  void WalkClassFunction(Lightning::FunctionNode*& node, StageRequirementsGathererContext* context);
  void WalkClassMemberVariable(Lightning::MemberVariableNode*& node, StageRequirementsGathererContext* context);

  void WalkMemberAccessNode(Lightning::MemberAccessNode*& node, StageRequirementsGathererContext* context);
  void WalkStaticTypeNode(Lightning::StaticTypeNode*& node, StageRequirementsGathererContext* context);

  void MergeCurrent(LightningShaderIRLibrary* shaderLibrary,
                    Lightning::Member* lightningMember,
                    Lightning::SyntaxNode* node,
                    StageRequirementsGathererContext* context);

  void BuildLibraryMap(LightningShaderIRLibrary* library, StageRequirementsGathererContext* context);

  ShaderStage::Enum GetRequiredStages(Lightning::Member* lightningObject, Lightning::ReflectionObject* owner);
  String GetStageName(ShaderStage::Enum stage);
  void CheckAndDispatchErrors(Lightning::Member* lightningObject,
                              Lightning::ReflectionObject* owner,
                              StageRequirementsGathererContext* context);

  ShaderCompilationErrors* mErrors;
  LightningShaderSpirVSettings* mSettings;
};

} // namespace Plasma
