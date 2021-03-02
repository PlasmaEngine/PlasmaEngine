// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

TypeResolvers::TypeResolvers()
{
  mBackupFieldResolver = nullptr;
  mDefaultConstructorResolver = nullptr;
  mBackupConstructorResolver = nullptr;
  mBackupSetterResolver = nullptr;
  mExpressionInitializerListResolver = nullptr;
}

void TypeResolvers::RegisterFieldResolver(Lightning::Field* field, MemberAccessResolverIRFn fieldResolver)
{
  ErrorIf(field == nullptr, "Cannot register a field resolver on a null field");
  mFieldResolvers.InsertOrError(field, fieldResolver);
}

void TypeResolvers::RegisterBackupFieldResolver(MemberAccessResolverIRFn backupResolver)
{
  ErrorIf(mBackupFieldResolver != nullptr, "Backup field resolver cannot be registered twice.");
  mBackupFieldResolver = backupResolver;
}

MemberAccessResolverIRFn TypeResolvers::FindFieldResolver(Lightning::Field* field)
{
  MemberAccessResolverIRFn resolver = mFieldResolvers.FindValue(field, nullptr);
  if (resolver == nullptr)
    resolver = mBackupFieldResolver;
  return resolver;
}

void TypeResolvers::RegisterConstructorResolver(Lightning::Function* lightningFunction, ConstructorCallResolverIRFn resolverFn)
{
  ErrorIf(lightningFunction == nullptr, "Trying to register a null function resolver");
  ErrorIf(mConstructorResolvers.ContainsKey(lightningFunction), "Constructor resolver was already registered");
  mConstructorResolvers[lightningFunction] = resolverFn;
}

ConstructorCallResolverIRFn TypeResolvers::FindConstructorResolver(Lightning::Function* lightningFunction)
{
  ConstructorCallResolverIRFn resolver = mConstructorResolvers.FindValue(lightningFunction, nullptr);
  if (resolver != nullptr)
    return resolver;
  return mBackupConstructorResolver;
}

void TypeResolvers::RegisterFunctionResolver(Lightning::Function* function, MemberFunctionResolverIRFn functionResolver)
{
  ErrorIf(function == nullptr, "Cannot register a function resolver on a null function");
  mFunctionResolvers.InsertOrError(function, functionResolver);
}

MemberFunctionResolverIRFn TypeResolvers::FindFunctionResolver(Lightning::Function* function)
{
  return mFunctionResolvers.FindValue(function, nullptr);
}

void TypeResolvers::RegisterSetterResolver(Lightning::Function* function, MemberPropertySetterResolverIRFn functionResolver)
{
  ErrorIf(function == nullptr, "Cannot register a function resolver on a null function");
  mSetterResolvers.InsertOrError(function, functionResolver);
}

void TypeResolvers::RegisterBackupSetterResolver(MemberPropertySetterResolverIRFn backupResolver)
{
  mBackupSetterResolver = backupResolver;
}

MemberPropertySetterResolverIRFn TypeResolvers::FindSetterResolver(Lightning::Function* function)
{
  MemberPropertySetterResolverIRFn resolver = mSetterResolvers.FindValue(function, nullptr);
  if (resolver == nullptr)
    resolver = mBackupSetterResolver;
  return resolver;
}

void OperatorResolvers::RegisterBinaryOpResolver(Lightning::Type* lhsType,
                                                 Lightning::Type* rhsType,
                                                 Lightning::Grammar::Enum op,
                                                 BinaryOpResolverIRFn resolver)
{
  BinaryOperatorKey opId(lhsType, rhsType, op);
  mBinaryOpResolvers.InsertOrError(opId, resolver);
}

BinaryOpResolverIRFn OperatorResolvers::FindOpResolver(BinaryOperatorKey& opId)
{
  return mBinaryOpResolvers.FindValue(opId, nullptr);
}

void OperatorResolvers::RegisterUnaryOpResolver(Lightning::Type* type,
                                                Lightning::Grammar::Enum op,
                                                UnaryOpResolverIRFn resolver)
{
  UnaryOperatorKey opId(type, op);
  mUnaryOpResolvers.InsertOrError(opId, resolver);
}

UnaryOpResolverIRFn OperatorResolvers::FindOpResolver(UnaryOperatorKey& opId)
{
  return mUnaryOpResolvers.FindValue(opId, nullptr);
}

void OperatorResolvers::RegisterTypeCastOpResolver(Lightning::Type* fromType,
                                                   Lightning::Type* toType,
                                                   TypeCastResolverIRFn resolver)
{
  TypeCastKey opId(fromType, toType);
  mTypeCastResolvers.InsertOrError(opId, resolver);
}

TypeCastResolverIRFn OperatorResolvers::FindOpResolver(TypeCastKey& opId)
{
  return mTypeCastResolvers.FindValue(opId, nullptr);
}

GlobalVariableData::GlobalVariableData()
{
  mInstance = nullptr;
  mInitializerFunction = nullptr;
}

GlobalVariableData::~GlobalVariableData()
{
  delete mInstance;
}

StageRequirementsData::StageRequirementsData()
{
  mRequiredStages = (ShaderStage::Enum)ShaderStage::None;
  mDependency = nullptr;
}

void StageRequirementsData::Combine(Lightning::Member* dependency,
                                    const Lightning::CodeLocation& location,
                                    ShaderStage::Enum requiredStage)
{
  // Only set the dependency and call location on the first non-empty stage
  // requirement.
  if (mDependency == nullptr && requiredStage != ShaderStage::None)
  {
    mDependency = dependency;
    mCallLocation = location;
  }
  mRequiredStages |= requiredStage;
}

LightningShaderIRType* LightningShaderIRModule::FindType(const String& typeName, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    LightningShaderIRType* type = library->FindType(typeName, checkDependencies);
    if (type != nullptr)
      return type;
  }
  return nullptr;
}

GlobalVariableData* LightningShaderIRModule::FindGlobalVariable(Lightning::Field* lightningField, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    GlobalVariableData* result = library->FindGlobalVariable(lightningField, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

GlobalVariableData* LightningShaderIRModule::FindGlobalVariable(LightningShaderIROp* globalInstance, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    GlobalVariableData* result = library->FindGlobalVariable(globalInstance, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

TemplateTypeIRResloverFn LightningShaderIRModule::FindTemplateResolver(const TemplateTypeKey& templateKey,
                                                                   bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    TemplateTypeIRResloverFn resolver = library->FindTemplateResolver(templateKey, checkDependencies);
    if (resolver != nullptr)
      return resolver;
  }
  return nullptr;
}

TypeResolvers* LightningShaderIRModule::FindTypeResolver(Lightning::Type* lightningType, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    TypeResolvers* result = library->FindTypeResolver(lightningType, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

LightningShaderIRFunction* LightningShaderIRModule::FindFunction(Lightning::Function* lightningFunction, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    LightningShaderIRFunction* irFunction = library->FindFunction(lightningFunction, checkDependencies);
    if (irFunction != nullptr)
      return irFunction;
  }
  return nullptr;
}

SpirVExtensionInstruction* LightningShaderIRModule::FindExtensionInstruction(Lightning::Function* lightningFunction,
                                                                         bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    SpirVExtensionInstruction* result = library->FindExtensionInstruction(lightningFunction, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

LightningShaderExtensionImport* LightningShaderIRModule::FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                                            bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    LightningShaderExtensionImport* result = library->FindExtensionLibraryImport(extensionLibrary, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

LightningShaderIRConstantLiteral* LightningShaderIRModule::FindConstantLiteral(Lightning::Any& literalValue, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    LightningShaderIRConstantLiteral* result = library->FindConstantLiteral(literalValue, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

LightningShaderIROp* LightningShaderIRModule::FindConstantOp(ConstantOpKeyType& key, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    LightningShaderIROp* result = library->FindConstantOp(key, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

LightningShaderIROp* LightningShaderIRModule::FindEnumConstantOp(void* key, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    LightningShaderIROp* result = library->FindEnumConstantOp(key, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

LightningShaderIROp* LightningShaderIRModule::FindSpecializationConstantOp(void* key, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    LightningShaderIROp* result = library->FindSpecializationConstantOp(key, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

template <typename OpIdType, typename OpResolverType>
OpResolverType LightningShaderIRModule::FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    LightningShaderIRLibrary* library = (*this)[i];
    OpResolverType result = library->FindOperatorResolver(opId, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

LightningShaderIRLibrary::LightningShaderIRLibrary()
{
  mTranslated = false;
}

LightningShaderIRLibrary::~LightningShaderIRLibrary()
{
  DeleteObjectsIn(mOwnedTypes.All());
  DeleteObjectsIn(mOwnedFunctions.All());
  DeleteObjectsIn(mOwnedFieldMeta.All());
  DeleteObjectsIn(mOwnedFunctionMeta.All());
  DeleteObjectsIn(mOwnedTypeMeta.All());
  DeleteObjectsIn(mOwnedSpecializationConstants.All());

  DeleteObjectsIn(mExtensionInstructions.Values());
  DeleteObjectsIn(mExtensionLibraries.All());
  DeleteObjectsIn(mConstantLiterals.Values());
  DeleteObjectsIn(mConstantOps.Values());
  DeleteObjectsIn(mExtensionLibraryImports.Values());
  DeleteObjectsIn(mLightningFieldToGlobalVariable.Values());
}

void LightningShaderIRLibrary::AddType(StringParam typeName, LightningShaderIRType* shaderType)
{
  mTypes.InsertOrError(typeName, shaderType);
  mOwnedTypes.PushBack(shaderType);
}

LightningShaderIRType* LightningShaderIRLibrary::FindType(const String& typeName, bool checkDependencies)
{
  // Try to find the type in this library
  LightningShaderIRType* type = mTypes.FindValue(typeName, nullptr);
  if (type != nullptr)
    return type;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindType(typeName);
}

LightningShaderIRType* LightningShaderIRLibrary::FindType(Lightning::Type* lightningType, bool checkDependencies)
{
  Lightning::BoundType* boundType = Lightning::BoundType::GetBoundType(lightningType);
  return FindType(boundType->Name, checkDependencies);
}

GlobalVariableData* LightningShaderIRLibrary::FindGlobalVariable(Lightning::Field* lightningField, bool checkDependencies)
{
  GlobalVariableData* result = mLightningFieldToGlobalVariable.FindValue(lightningField, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindGlobalVariable(lightningField, checkDependencies);
}

GlobalVariableData* LightningShaderIRLibrary::FindGlobalVariable(LightningShaderIROp* globalInstance, bool checkDependencies)
{
  Lightning::Field* lightningField = mGlobalVariableToLightningField.FindValue(globalInstance, nullptr);
  GlobalVariableData* result = mLightningFieldToGlobalVariable.FindValue(lightningField, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindGlobalVariable(globalInstance, checkDependencies);
}

void LightningShaderIRLibrary::RegisterTemplateResolver(const TemplateTypeKey& templateKey,
                                                    TemplateTypeIRResloverFn resolver)
{
  mTemplateResolvers.InsertOrError(templateKey, resolver);
}

TemplateTypeIRResloverFn LightningShaderIRLibrary::FindTemplateResolver(const TemplateTypeKey& templateKey,
                                                                    bool checkDependencies)
{
  // Try to find the type in this library
  TemplateTypeIRResloverFn resolver = mTemplateResolvers.FindValue(templateKey, nullptr);
  if (resolver != nullptr)
    return resolver;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindTemplateResolver(templateKey);
}

void LightningShaderIRLibrary::FlattenModuleDependents()
{
  // Bring all dependents from our dependency modules into ourself
  LightningShaderIRModule* module = mDependencies;
  for (size_t i = 0; i < module->Size(); ++i)
  {
    LightningShaderIRLibrary* parentLibrary = (*module)[i];

    // For each type in the parent library's reverse dependencies, copy all
    // dependents into the current library
    AutoDeclare(pairRange, parentLibrary->mTypeDependents.All());
    for (; !pairRange.Empty(); pairRange.PopFront())
    {
      AutoDeclare(pair, pairRange.Front());
      HashSet<LightningShaderIRType*>& parentLibraryDependents = pair.second;
      HashSet<LightningShaderIRType*>& currentLibraryDependents = mTypeDependents[pair.first];

      // Copy all dependents into the current library
      HashSet<LightningShaderIRType*>::range dependentRange = parentLibraryDependents.All();
      for (; !dependentRange.Empty(); dependentRange.PopFront())
        currentLibraryDependents.Insert(dependentRange.Front());
    }
  }
}

void LightningShaderIRLibrary::GetAllDependents(LightningShaderIRType* shaderType, HashSet<LightningShaderIRType*>& finalDependents)
{
  // Find if the current shader type has any dependents (things that depend
  // on it). If it doesn't then there's nothing more to do.
  HashSet<LightningShaderIRType*>* currentTypeDependents = mTypeDependents.FindPointer(shaderType);
  if (currentTypeDependents == nullptr)
    return;

  // Iterate over all dependent types
  HashSet<LightningShaderIRType*>::range range = currentTypeDependents->All();
  for (; !range.Empty(); range.PopFront())
  {
    // Don't visit a dependent that's already been visited
    LightningShaderIRType* dependent = range.Front();
    if (finalDependents.Contains(dependent))
      continue;

    // Mark that we've visited this dependent and get all of its dependents
    // recursively
    finalDependents.Insert(dependent);
    GetAllDependents(dependent, finalDependents);
  }
}

TypeResolvers* LightningShaderIRLibrary::FindTypeResolver(Lightning::Type* lightningType, bool checkDependencies)
{
  TypeResolvers* result = mTypeResolvers.FindPointer(lightningType);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindTypeResolver(lightningType, checkDependencies);
}

BinaryOpResolverIRFn LightningShaderIRLibrary::FindOperatorResolver(BinaryOperatorKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<BinaryOperatorKey, BinaryOpResolverIRFn>(opId, checkDependencies);
}

UnaryOpResolverIRFn LightningShaderIRLibrary::FindOperatorResolver(UnaryOperatorKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<UnaryOperatorKey, UnaryOpResolverIRFn>(opId, checkDependencies);
}

TypeCastResolverIRFn LightningShaderIRLibrary::FindOperatorResolver(TypeCastKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<TypeCastKey, TypeCastResolverIRFn>(opId, checkDependencies);
}

MemberAccessResolverIRFn LightningShaderIRLibrary::FindFieldResolver(Lightning::Type* lightningType,
                                                                 Lightning::Field* lightningField,
                                                                 bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(lightningType, checkDependencies);
  if (typeResolver != nullptr)
    return typeResolver->FindFieldResolver(lightningField);
  return nullptr;
}

MemberFunctionResolverIRFn LightningShaderIRLibrary::FindFunctionResolver(Lightning::Type* lightningType,
                                                                      Lightning::Function* lightningFunction,
                                                                      bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(lightningType, checkDependencies);
  if (typeResolver != nullptr)
    return typeResolver->FindFunctionResolver(lightningFunction);
  return nullptr;
}

MemberPropertySetterResolverIRFn LightningShaderIRLibrary::FindSetterResolver(Lightning::Type* lightningType,
                                                                          Lightning::Function* lightningFunction,
                                                                          bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(lightningType, checkDependencies);
  if (typeResolver != nullptr)
    return typeResolver->FindSetterResolver(lightningFunction);
  return nullptr;
}

ConstructorCallResolverIRFn LightningShaderIRLibrary::FindConstructorResolver(Lightning::Type* lightningType,
                                                                          Lightning::Function* lightningFunction,
                                                                          bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(lightningType, checkDependencies);
  if (typeResolver != nullptr)
    return typeResolver->FindConstructorResolver(lightningFunction);
  return nullptr;
}

LightningShaderIRFunction* LightningShaderIRLibrary::FindFunction(Lightning::Function* lightningFunction, bool checkDependencies)
{
  // Try to find the type in this library
  LightningShaderIRFunction* irFunction = mFunctions.FindValue(lightningFunction, nullptr);
  if (irFunction != nullptr)
    return irFunction;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindFunction(lightningFunction, checkDependencies);
}

SpirVExtensionInstruction* LightningShaderIRLibrary::FindExtensionInstruction(Lightning::Function* lightningFunction,
                                                                          bool checkDependencies)
{
  SpirVExtensionInstruction* result = mExtensionInstructions.FindValue(lightningFunction, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindExtensionInstruction(lightningFunction, checkDependencies);
}

LightningShaderExtensionImport* LightningShaderIRLibrary::FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                                             bool checkDependencies)
{
  LightningShaderExtensionImport* result = mExtensionLibraryImports.FindValue(extensionLibrary, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindExtensionLibraryImport(extensionLibrary, checkDependencies);
}

LightningShaderIRConstantLiteral* LightningShaderIRLibrary::FindConstantLiteral(Lightning::Any& literalValue,
                                                                        bool checkDependencies)
{
  // Try to find the type in this library
  LightningShaderIRConstantLiteral* result = mConstantLiterals.FindValue(literalValue, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindConstantLiteral(literalValue, checkDependencies);
}

LightningShaderIROp* LightningShaderIRLibrary::FindConstantOp(ConstantOpKeyType& key, bool checkDependencies)
{
  // Try to find the type in this library
  LightningShaderIROp* result = mConstantOps.FindValue(key, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindConstantOp(key, checkDependencies);
}

LightningShaderIROp* LightningShaderIRLibrary::FindEnumConstantOp(void* key, bool checkDependencies)
{
  // Try to find the type in this library
  LightningShaderIROp* result = mEnumContants.FindValue(key, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindEnumConstantOp(key, checkDependencies);
}

LightningShaderIROp* LightningShaderIRLibrary::FindSpecializationConstantOp(void* key, bool checkDependencies)
{
  // Try to find the type in this library
  LightningShaderIROp* result = mSpecializationConstantMap.FindValue(key, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindSpecializationConstantOp(key, checkDependencies);
}

template <typename OpIdType, typename OpResolverType>
OpResolverType LightningShaderIRLibrary::FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies)
{
  OpResolverType result = mOperatorResolvers.FindOpResolver(opId);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindOperatorResolverTemplate<OpIdType, OpResolverType>(opId, checkDependencies);
}

String GetOverloadedName(StringParam functionName, Lightning::Function* lightningFunction)
{
  Lightning::GuidType thisHash = lightningFunction->This ? lightningFunction->This->ResultType->Hash() : 0;
  return BuildString(functionName, ToString(lightningFunction->FunctionType->Hash() ^ thisHash));
}

} // namespace Plasma
