// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class CycleDetection;

/// Context class for detecting cycles in the static call graph.
class CycleDetectionContext : public Lightning::WalkerContext<CycleDetection, CycleDetectionContext>
{
public:
  CycleDetectionContext();

  HashMap<Lightning::Type*, Lightning::ClassNode*> mTypeMap;
  HashMap<Lightning::Function*, Lightning::FunctionNode*> mFunctionMap;
  HashMap<Lightning::Function*, Lightning::ConstructorNode*> mConstructorMap;
  HashMap<Lightning::Property*, Lightning::MemberVariableNode*> mVariableMap;

  /// All objects that have been processed. Prevents visiting a part of
  /// the call graph multiple times (this can happen even if no cycles exist).
  HashSet<void*> mAllProcessedObjects;
  /// Keeps track of all objects in the current call stack (a duplicate means a
  /// cycle was found).
  HashSet<void*> mProcessedObjectsStack;
  /// Used to report the call stack that creates a cycle.
  Array<Lightning::SyntaxNode*> mCallStack;

  ShaderCompilationErrors* mErrors;
  LightningShaderIRLibrary* mCurrentLibrary;
};

/// Helper class that manages pushing/popping an object from the current
/// call stack. Cleans up a lot of code in cycle detection.
class CycleDetectionObjectScope
{
public:
  CycleDetectionObjectScope(void* object, CycleDetectionContext* context);
  ~CycleDetectionObjectScope();

  void PushScope();
  void PopScope();

  CycleDetectionContext* mContext;
  void* mObject;
  /// Has the target object already been visited? Used to early
  /// out from the DFS. Set to true even if a cycle is detected.
  bool mAlreadyVisited;
  /// Did this object cause a cycle to be formed?
  bool mCycleDetected;
};

/// Helper class to detect cycles in the static call graph of a shader
/// library (which is illegal in shaders). Ideally this should be refactored to
/// two pieces: one that builds the static call graph which can be re-used for
/// multiple queries and then the actual cycle detection.
class CycleDetection
{
public:
  CycleDetection(LightningShaderSpirVSettings* settings);
  bool Run(Lightning::SyntaxTree& syntaxTree, LightningShaderIRLibrary* library, ShaderCompilationErrors* errors);

protected:
  void PreWalkClassNode(Lightning::ClassNode*& node, CycleDetectionContext* context);
  void PreWalkConstructor(Lightning::ConstructorNode*& node, CycleDetectionContext* context);
  void PreWalkClassFunction(Lightning::FunctionNode*& node, CycleDetectionContext* context);
  void PreWalkClassMemberVariable(Lightning::MemberVariableNode*& node, CycleDetectionContext* context);

  void WalkClassNode(Lightning::ClassNode*& node, CycleDetectionContext* context);
  void WalkClassPreconstructor(Lightning::ClassNode*& node, CycleDetectionContext* context);
  void WalkClassPreconstructor(Lightning::Function* preConstructor, CycleDetectionContext* context);
  void WalkClassConstructor(Lightning::ConstructorNode*& node, CycleDetectionContext* context);
  void WalkClassFunction(Lightning::FunctionNode*& node, CycleDetectionContext* context);
  void WalkClassMemberVariable(Lightning::MemberVariableNode*& node, CycleDetectionContext* context);
  void WalkMemberAccessNode(Lightning::MemberAccessNode*& node, CycleDetectionContext* context);
  void WalkStaticTypeNode(Lightning::StaticTypeNode*& node, CycleDetectionContext* context);

  void ReportError(CycleDetectionContext* context);

  ShaderCompilationErrors* mErrors;
  LightningShaderSpirVSettings* mSettings;
};

} // namespace Plasma
