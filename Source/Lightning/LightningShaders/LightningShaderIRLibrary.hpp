// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class LightningShaderIRLibrary;
class LightningShaderIRModule;
typedef Lightning::Ref<LightningShaderIRLibrary> LightningShaderIRLibraryRef;
typedef Lightning::Ref<LightningShaderIRModule> LightningShaderIRModuleRef;

class LightningSpirVFrontEnd;
class LightningSpirVFrontEndContext;

typedef Pair<LightningShaderIRType*, Lightning::Any> ConstantOpKeyType;

typedef void (*DefaultConstructorResolverFn)(LightningSpirVFrontEnd* translator,
                                             Lightning::Type* resultType,
                                             LightningSpirVFrontEndContext* context);
typedef void (*ConstructorCallResolverIRFn)(LightningSpirVFrontEnd* translator,
                                            Lightning::FunctionCallNode* fnCallNode,
                                            Lightning::StaticTypeNode* staticTypeNode,
                                            LightningSpirVFrontEndContext* context);
typedef void (*MemberAccessResolverIRFn)(LightningSpirVFrontEnd* translator,
                                         Lightning::MemberAccessNode* memberAccessNode,
                                         LightningSpirVFrontEndContext* context);
typedef void (*MemberFunctionResolverIRFn)(LightningSpirVFrontEnd* translator,
                                           Lightning::FunctionCallNode* functionCallNode,
                                           Lightning::MemberAccessNode* memberAccessNode,
                                           LightningSpirVFrontEndContext* context);
typedef void (*MemberPropertySetterResolverIRFn)(LightningSpirVFrontEnd* translator,
                                                 Lightning::MemberAccessNode* memberAccessNode,
                                                 LightningShaderIROp* resultValue,
                                                 LightningSpirVFrontEndContext* context);
typedef void (*BinaryOpResolverIRFn)(LightningSpirVFrontEnd* translator,
                                     Lightning::BinaryOperatorNode* binaryOpNode,
                                     LightningSpirVFrontEndContext* context);
typedef void (*UnaryOpResolverIRFn)(LightningSpirVFrontEnd* translator,
                                    Lightning::UnaryOperatorNode* binaryOpNode,
                                    LightningSpirVFrontEndContext* context);
typedef void (*TypeCastResolverIRFn)(LightningSpirVFrontEnd* translator,
                                     Lightning::TypeCastNode* binaryOpNode,
                                     LightningSpirVFrontEndContext* context);
typedef void (*TemplateTypeIRResloverFn)(LightningSpirVFrontEnd* translator, Lightning::BoundType* boundType);
typedef void (*ExpressionInitializerIRResolverFn)(LightningSpirVFrontEnd* translator,
                                                  Lightning::ExpressionInitializerNode*& node,
                                                  LightningSpirVFrontEndContext* context);

/// A collection of member/function/etc... resolvers for specific library
/// translations. Some examples include Math.Dot and samplers.
class TypeResolvers
{
public:
  TypeResolvers();

  /// Register a resolver for a field.
  void RegisterFieldResolver(Lightning::Field* field, MemberAccessResolverIRFn fieldResolver);
  /// Register a resolver to use when a field-specific resolver isn't found.
  /// Used to handle things like swizzles.
  void RegisterBackupFieldResolver(MemberAccessResolverIRFn backupResolver);
  /// Finds the resolver for the given field. Returns the backup resolver if no
  /// match is found.
  MemberAccessResolverIRFn FindFieldResolver(Lightning::Field* field);

  void RegisterConstructorResolver(Lightning::Function* lightningFunction, ConstructorCallResolverIRFn resolverFn);
  ConstructorCallResolverIRFn FindConstructorResolver(Lightning::Function* lightningFunction);

  /// Register a resolver for a function call.
  void RegisterFunctionResolver(Lightning::Function* function, MemberFunctionResolverIRFn functionResolver);
  MemberFunctionResolverIRFn FindFunctionResolver(Lightning::Function* function);

  /// Register a resolver for a setter function call.
  void RegisterSetterResolver(Lightning::Function* function, MemberPropertySetterResolverIRFn functionResolver);
  void RegisterBackupSetterResolver(MemberPropertySetterResolverIRFn backupResolver);
  MemberPropertySetterResolverIRFn FindSetterResolver(Lightning::Function* function);

  HashMap<Lightning::Field*, MemberAccessResolverIRFn> mFieldResolvers;
  MemberAccessResolverIRFn mBackupFieldResolver;

  DefaultConstructorResolverFn mDefaultConstructorResolver;
  ConstructorCallResolverIRFn mBackupConstructorResolver;
  MemberPropertySetterResolverIRFn mBackupSetterResolver;
  /// Library translations for constructors of a type (e.g. Real3 splat
  /// constructor)
  HashMap<Lightning::Function*, ConstructorCallResolverIRFn> mConstructorResolvers;

  HashMap<Lightning::Function*, MemberFunctionResolverIRFn> mFunctionResolvers;
  HashMap<Lightning::Function*, MemberPropertySetterResolverIRFn> mSetterResolvers;
  /// Some types need to override how expression initializers work (e.g. fixed
  /// array).
  ExpressionInitializerIRResolverFn mExpressionInitializerListResolver;
};

/// A collection of operators (unary, binary, type cast) resolvers for a
/// specific library.
class OperatorResolvers
{
public:
  /// Binary operators
  void RegisterBinaryOpResolver(Lightning::Type* lhsType,
                                Lightning::Type* rhsType,
                                Lightning::Grammar::Enum op,
                                BinaryOpResolverIRFn resolver);
  BinaryOpResolverIRFn FindOpResolver(BinaryOperatorKey& opId);

  /// Unary operators
  void RegisterUnaryOpResolver(Lightning::Type* type, Lightning::Grammar::Enum op, UnaryOpResolverIRFn resolver);
  UnaryOpResolverIRFn FindOpResolver(UnaryOperatorKey& opId);

  /// Type Cast operators
  void RegisterTypeCastOpResolver(Lightning::Type* fromType, Lightning::Type* toType, TypeCastResolverIRFn resolver);
  TypeCastResolverIRFn FindOpResolver(TypeCastKey& opId);

private:
  HashMap<BinaryOperatorKey, BinaryOpResolverIRFn> mBinaryOpResolvers;
  HashMap<UnaryOperatorKey, UnaryOpResolverIRFn> mUnaryOpResolvers;
  HashMap<TypeCastKey, TypeCastResolverIRFn> mTypeCastResolvers;
};

/// Data about global variables.
class GlobalVariableData
{
public:
  GlobalVariableData();
  ~GlobalVariableData();

  /// The instance of the global variable. This instance is owned by this class.
  LightningShaderIROp* mInstance;
  /// A function that initializes this instance. Can be null.
  LightningShaderIRFunction* mInitializerFunction;
};

/// Used to store if a symbol requires a certain stage (e.g. [RequiresPixel]).
/// If a symbol does have a requirement, this also stores the dependency that
/// causes this requirement as well as the location that references the
/// dependency (e.g. a the function call location).
struct StageRequirementsData
{
  StageRequirementsData();

  /// Merges the given dependency into this object. Only updates the dependency
  /// and location if this is the first time a non-empty stage requirement is
  /// being set.
  void Combine(Lightning::Member* dependency, const Lightning::CodeLocation& location, ShaderStage::Enum requiredStage);

  /// What stage this symbol requires.
  ShaderStage::Enum mRequiredStages;
  /// The first symbol that causes the stage requirement. Used to generate a
  /// call graph on error.
  Lightning::Member* mDependency;
  /// The location that the dependency is called.
  Lightning::CodeLocation mCallLocation;
};

/// A module represents a collection of libraries, typically used to
/// express what dependencies another library has. All libraries in this
/// module should be fully compiled (and hence locked) if they're in a module.
class LightningShaderIRModule : public Array<LightningShaderIRLibraryRef>
{
public:
  /// Find a type in any of the contained libraries
  LightningShaderIRType* FindType(const String& typeName, bool checkDependencies = true);

  /// Find the global variable data associate with the given lightning field.
  GlobalVariableData* FindGlobalVariable(Lightning::Field* lightningField, bool checkDependencies = true);
  /// Find the global variable data associate with the given instance variable
  /// op.
  GlobalVariableData* FindGlobalVariable(LightningShaderIROp* globalInstance, bool checkDependencies = true);

  /// Find a resolver for a template type
  TemplateTypeIRResloverFn FindTemplateResolver(const TemplateTypeKey& templateKey, bool checkDependencies = true);

  TypeResolvers* FindTypeResolver(Lightning::Type* lightningType, bool checkDependencies = true);

  // Constructor library replacements
  LightningShaderIRFunction* FindFunction(Lightning::Function* lightningFunction, bool checkDependencies = true);
  SpirVExtensionInstruction* FindExtensionInstruction(Lightning::Function* lightningFunction, bool checkDependencies = true);
  LightningShaderExtensionImport* FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                         bool checkDependencies = true);

  LightningShaderIRConstantLiteral* FindConstantLiteral(Lightning::Any& literalValue, bool checkDependencies = true);
  LightningShaderIROp* FindConstantOp(ConstantOpKeyType& key, bool checkDependencies = true);
  LightningShaderIROp* FindEnumConstantOp(void* key, bool checkDependencies = true);
  LightningShaderIROp* FindSpecializationConstantOp(void* key, bool checkDependencies = true);

  // An intrusive reference count for memory handling
  LightningRefLink(LightningShaderIRModule);

private:
  friend LightningShaderIRLibrary;

  // Helper template to make finding operators easier
  template <typename OpIdType, typename OpResolverType>
  OpResolverType FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies);
};

/// A library built during shader translation. Mostly an internal type that
/// stores all necessary lookup information to build and generate a spir-v
/// shader. Also contains what types were created during translation.
class LightningShaderIRLibrary
{
public:
  LightningShaderIRLibrary();
  ~LightningShaderIRLibrary();

  void AddType(StringParam typeName, LightningShaderIRType* shaderType);
  LightningShaderIRType* FindType(const String& typeName, bool checkDependencies = true);
  LightningShaderIRType* FindType(Lightning::Type* lightningType, bool checkDependencies = true);

  /// Find the global variable data associate with the given lightning field.
  GlobalVariableData* FindGlobalVariable(Lightning::Field* lightningField, bool checkDependencies = true);
  /// Find the global variable data associate with the given instance variable
  /// op.
  GlobalVariableData* FindGlobalVariable(LightningShaderIROp* globalInstance, bool checkDependencies = true);

  // Resolvers for template types (e.g. FixedArray)
  void RegisterTemplateResolver(const TemplateTypeKey& templateKey, TemplateTypeIRResloverFn resolver);
  TemplateTypeIRResloverFn FindTemplateResolver(const TemplateTypeKey& templateKey, bool checkDependencies = true);

  /// Pulls all reverse dependencies from all the dependent modules into this
  /// library (flattens the list)
  void FlattenModuleDependents();
  /// Fills out a list of all types that depend on the given type
  void GetAllDependents(LightningShaderIRType* shaderType, HashSet<LightningShaderIRType*>& finalDependents);

  // Library replacements on a type
  TypeResolvers* FindTypeResolver(Lightning::Type* lightningType, bool checkDependencies = true);

  // Operator resolvers
  BinaryOpResolverIRFn FindOperatorResolver(BinaryOperatorKey& opId, bool checkDependencies = true);
  UnaryOpResolverIRFn FindOperatorResolver(UnaryOperatorKey& opId, bool checkDependencies = true);
  TypeCastResolverIRFn FindOperatorResolver(TypeCastKey& opId, bool checkDependencies = true);

  MemberAccessResolverIRFn FindFieldResolver(Lightning::Type* lightningType,
                                             Lightning::Field* lightningField,
                                             bool checkDependencies = true);
  MemberFunctionResolverIRFn FindFunctionResolver(Lightning::Type* lightningType,
                                                  Lightning::Function* lightningFunction,
                                                  bool checkDependencies = true);
  MemberPropertySetterResolverIRFn FindSetterResolver(Lightning::Type* lightningType,
                                                      Lightning::Function* lightningFunction,
                                                      bool checkDependencies = true);
  ConstructorCallResolverIRFn FindConstructorResolver(Lightning::Type* lightningType,
                                                      Lightning::Function* lightningFunction,
                                                      bool checkDependencies = true);

  LightningShaderIRFunction* FindFunction(Lightning::Function* lightningFunction, bool checkDependencies = true);
  SpirVExtensionInstruction* FindExtensionInstruction(Lightning::Function* lightningFunction, bool checkDependencies = true);
  LightningShaderExtensionImport* FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                         bool checkDependencies = true);

  LightningShaderIRConstantLiteral* FindConstantLiteral(Lightning::Any& literalValue, bool checkDependencies = true);

  LightningShaderIROp* FindConstantOp(ConstantOpKeyType& key, bool checkDependencies = true);
  LightningShaderIROp* FindEnumConstantOp(void* key, bool checkDependencies = true);
  /// Finds a specialization constant given a key. The key is normally a lightning
  /// field, but can also be special global keys (like languageId).
  LightningShaderIROp* FindSpecializationConstantOp(void* key, bool checkDependencies = true);

  Lightning::LibraryRef mLightningLibrary;
  LightningShaderIRModuleRef mDependencies;
  // An intrusive reference count for memory handling
  LightningRefLink(LightningShaderIRLibrary);

  // Data owned by this library. Needed for memory management.
  PodBlockArray<LightningShaderIRType*> mOwnedTypes;
  PodBlockArray<LightningShaderIRFunction*> mOwnedFunctions;
  PodBlockArray<ShaderIRFieldMeta*> mOwnedFieldMeta;
  PodBlockArray<ShaderIRFunctionMeta*> mOwnedFunctionMeta;
  PodBlockArray<ShaderIRTypeMeta*> mOwnedTypeMeta;
  PodBlockArray<LightningShaderIROp*> mOwnedSpecializationConstants;

  /// Map of all types by name (Lightning type name). Contains all types generated
  /// (including function pointer types, Real*, etc...), not just bound types.
  /// A type is a bound type is the meta is non-null.
  HashMap<String, LightningShaderIRType*> mTypes;

  /// All constant literals.
  HashMap<Lightning::Any, LightningShaderIRConstantLiteral*> mConstantLiterals;
  /// Map of all constants by type/value
  HashMap<ConstantOpKeyType, LightningShaderIROp*> mConstantOps;
  /// Lookup for enums based upon their lightning property.
  HashMap<void*, LightningShaderIROp*> mEnumContants;
  /// Map of all specialization constant by key (typically lightning field)
  HashMap<void*, LightningShaderIROp*> mSpecializationConstantMap;
  /// All globals declared in this library. Currently used for samplers.
  HashMap<Lightning::Field*, GlobalVariableData*> mLightningFieldToGlobalVariable;
  /// Mapping from the global variable ops to the owning field so that the
  /// global variable data can be looked up.
  HashMap<LightningShaderIROp*, Lightning::Field*> mGlobalVariableToLightningField;

  /// Map of all lightning functions to shader functions
  HashMap<Lightning::Function*, LightningShaderIRFunction*> mFunctions;
  HashMap<Lightning::Type*, TypeResolvers> mTypeResolvers;
  /// Operator resolvers for the current library
  OperatorResolvers mOperatorResolvers;

  Array<SpirVExtensionLibrary*> mExtensionLibraries;
  HashMap<Lightning::Function*, SpirVExtensionInstruction*> mExtensionInstructions;
  HashMap<SpirVExtensionLibrary*, LightningShaderExtensionImport*> mExtensionLibraryImports;
  HashMap<TemplateTypeKey, TemplateTypeIRResloverFn> mTemplateResolvers;

  /// Stores cached information about symbols in this library that have
  /// certain stage requirements. Used to detect and emit errors.
  HashMap<void*, StageRequirementsData> mStageRequirementsData;

  bool mTranslated;

  // A multi-map of all types to their dependents for this library (flattened to
  // include all module dependency data)
  typedef HashMap<LightningShaderIRType*, HashSet<LightningShaderIRType*>> TypeDependentMultiMap;
  TypeDependentMultiMap mTypeDependents;

private:
  // Helper template to make finding operators easier
  template <typename OpIdType, typename OpResolverType>
  OpResolverType FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies);
};

// Due to a bug in ANGLE, we can't rely on WebGL GLSL overloads on samplers.
// To fix this, we append the hash of the function signature to the name
// (different per overload).
String GetOverloadedName(StringParam functionName, Lightning::Function* lightningFunction);

} // namespace Plasma
