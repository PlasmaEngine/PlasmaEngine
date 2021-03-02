// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

StageRequirementsGatherer::StageRequirementsGatherer(LightningShaderSpirVSettings* settings)
{
  mErrors = nullptr;
  mSettings = settings;
}

bool StageRequirementsGatherer::Run(Lightning::SyntaxTree& syntaxTree,
                                    LightningShaderIRLibrary* library,
                                    ShaderCompilationErrors* errors)
{
  mErrors = errors;
  StageRequirementsGathererContext context;
  context.mErrors = mErrors;
  context.mCurrentLibrary = library;

  // Walk all dependency libraries to build a mapping of lightning libraries to
  // shader libraries. This could potentially be done faster but this will never
  // be a bottleneck.
  BuildLibraryMap(library, &context);

  // Do a pre-walk in order to build a map of nodes to lightning objects. This is
  // needed during that actual pass as we'll have to recurse from a function
  // call node into a function and all of its statements. Only a function node
  // has the actual function statements though so we have to build a map here.
  Lightning::BranchWalker<StageRequirementsGatherer, StageRequirementsGathererContext> preWalker;
  preWalker.Register(&StageRequirementsGatherer::PreWalkClassNode);
  preWalker.Register(&StageRequirementsGatherer::PreWalkConstructor);
  preWalker.Register(&StageRequirementsGatherer::PreWalkClassFunction);
  preWalker.Register(&StageRequirementsGatherer::PreWalkClassMemberVariable);
  preWalker.Walk(this, syntaxTree.Root, &context);

  Lightning::BranchWalker<StageRequirementsGatherer, StageRequirementsGathererContext> walker;
  walker.Register(&StageRequirementsGatherer::WalkClassNode);
  walker.Register(&StageRequirementsGatherer::WalkClassConstructor);
  walker.Register(&StageRequirementsGatherer::WalkClassFunction);
  walker.Register(&StageRequirementsGatherer::WalkClassMemberVariable);
  walker.Register(&StageRequirementsGatherer::WalkMemberAccessNode);
  walker.Register(&StageRequirementsGatherer::WalkStaticTypeNode);
  walker.Walk(this, syntaxTree.Root, &context);

  return mErrors->mErrorTriggered;
}

void StageRequirementsGatherer::PreWalkClassNode(Lightning::ClassNode*& node, StageRequirementsGathererContext* context)
{
  // Map the type to its node
  context->mTypeMap[node->Type] = node;
  // Walk all functions, constructors, and variables to do the same
  context->Walker->Walk(this, node->Functions, context);
  context->Walker->Walk(this, node->Constructors, context);
  context->Walker->Walk(this, node->Variables, context);
}

void StageRequirementsGatherer::PreWalkConstructor(Lightning::ConstructorNode*& node,
                                                   StageRequirementsGathererContext* context)
{
  context->mConstructorMap[node->DefinedFunction] = node;
}

void StageRequirementsGatherer::PreWalkClassFunction(Lightning::FunctionNode*& node,
                                                     StageRequirementsGathererContext* context)
{
  context->mFunctionMap[node->DefinedFunction] = node;
}

void StageRequirementsGatherer::PreWalkClassMemberVariable(Lightning::MemberVariableNode*& node,
                                                           StageRequirementsGathererContext* context)
{
  context->mVariableMap[node->CreatedProperty] = node;
  // Additionally handle get/set functions.
  if (node->Get != nullptr)
    context->mFunctionMap[node->Get->DefinedFunction] = node->Get;
  if (node->Set != nullptr)
    context->mFunctionMap[node->Set->DefinedFunction] = node->Set;
}

void StageRequirementsGatherer::WalkClassNode(Lightning::ClassNode*& node, StageRequirementsGathererContext* context)
{
  // Recursively walk all functions
  WalkClassPreconstructor(node, context);
  context->Walker->Walk(this, node->Constructors, context);
  context->Walker->Walk(this, node->Functions, context);
}

void StageRequirementsGatherer::WalkClassPreconstructor(Lightning::ClassNode*& node,
                                                        StageRequirementsGathererContext* context)
{
  // Only walk the pre-constructor if it exists and we haven't already processed
  // it
  Lightning::Function* preConstructor = node->PreConstructor;
  if (preConstructor == nullptr || context->mProcessedObjects.Contains(preConstructor))
    return;
  context->mProcessedObjects.Insert(preConstructor);

  // Push a new requirements object onto the stack
  StageRequirementsData prevRequirements = context->mCurrentRequirements;
  context->mCurrentRequirements = StageRequirementsData();

  // Walk all member variables (which walks their initializers) to get their
  // requirements
  context->Walker->Walk(this, node->Variables, context);

  // Now we need to actually merge each instance member variable's requirements
  // into the pre-constructor's requirements. Static member variable
  // initializers are not invoked by our pre-constructor, only on reference to
  // the static variable itself.
  LightningShaderIRLibrary* shaderLibrary = context->mCurrentLibrary;
  for (size_t i = 0; i < node->Variables.Size(); ++i)
  {
    Lightning::MemberVariableNode* varNode = node->Variables[i];
    if (varNode->CreatedProperty->HasAttribute(Lightning::StaticAttribute))
      continue;

    // Merge the result from the member variable into our current context
    MergeCurrent(shaderLibrary, varNode->CreatedProperty, node, context);
  }

  // If we found any stage requirements then cache them on the library
  if (context->mCurrentRequirements.mRequiredStages != ShaderStage::None)
    shaderLibrary->mStageRequirementsData[preConstructor] = context->mCurrentRequirements;
  // Now check if this symbol has any mis-matched requirements
  CheckAndDispatchErrors(preConstructor, preConstructor->Owner, context);

  // Pop the stack for the requirements we're building
  context->mCurrentRequirements = prevRequirements;
}

void StageRequirementsGatherer::WalkClassPreconstructor(Lightning::Function* preConstructor,
                                                        StageRequirementsGathererContext* context)
{
  // Pre-constructors require walking the class node.
  Lightning::ClassNode* classNode = context->mTypeMap.FindValue(preConstructor->Owner, nullptr);
  if (classNode != nullptr)
    WalkClassPreconstructor(classNode, context);
}

void StageRequirementsGatherer::WalkClassConstructor(Lightning::ConstructorNode*& node,
                                                     StageRequirementsGathererContext* context)
{
  // Only walk a constructor once
  Lightning::Function* lightningFunction = node->DefinedFunction;
  if (context->mProcessedObjects.Contains(lightningFunction))
    return;
  context->mProcessedObjects.Insert(lightningFunction);

  LightningShaderIRLibrary* shaderLibrary = context->mCurrentLibrary;

  // Push a new requirements object onto the stack
  StageRequirementsData prevRequirements = context->mCurrentRequirements;
  context->mCurrentRequirements = StageRequirementsData();

  // If there's a preconstructor then merge in its results (instance variable
  // initializers)
  Lightning::Function* preConstructor = lightningFunction->Owner->PreConstructor;
  if (preConstructor != nullptr)
  {
    WalkClassPreconstructor(preConstructor, context);
    MergeCurrent(shaderLibrary, preConstructor, node, context);
  }
  // Now walk all statements in the constructor
  context->Walker->Walk(this, node->Statements, context);

  // If we found any stage requirements then cache them on the library
  if (context->mCurrentRequirements.mRequiredStages != ShaderStage::None)
    shaderLibrary->mStageRequirementsData[lightningFunction] = context->mCurrentRequirements;
  // Now check if this symbol has any mis-matched requirements
  CheckAndDispatchErrors(lightningFunction, lightningFunction->Owner, context);

  // Pop the stack for the requirements we're building
  context->mCurrentRequirements = prevRequirements;
}

void StageRequirementsGatherer::WalkClassFunction(Lightning::FunctionNode*& node, StageRequirementsGathererContext* context)
{
  // Only walk a function once
  Lightning::Function* lightningFunction = node->DefinedFunction;
  if (context->mProcessedObjects.Contains(lightningFunction))
    return;
  context->mProcessedObjects.Insert(lightningFunction);

  // Push a new requirements object onto the stack
  StageRequirementsData prevRequirements = context->mCurrentRequirements;
  context->mCurrentRequirements = StageRequirementsData();

  // Simply walk all statements in the function to check for stage requirements
  LightningShaderIRLibrary* shaderLibrary = context->mCurrentLibrary;
  context->Walker->Walk(this, node->Statements, context);

  // If we found any stage requirements then cache them on the library
  if (context->mCurrentRequirements.mRequiredStages != ShaderStage::None)
    shaderLibrary->mStageRequirementsData[lightningFunction] = context->mCurrentRequirements;
  // Now check if this symbol has any mis-matched requirements
  CheckAndDispatchErrors(lightningFunction, lightningFunction->Owner, context);

  // Pop the stack for the requirements we're building
  context->mCurrentRequirements = prevRequirements;
}

void StageRequirementsGatherer::WalkClassMemberVariable(Lightning::MemberVariableNode*& node,
                                                        StageRequirementsGathererContext* context)
{
  // Only walk members once
  Lightning::Property* lightningProperty = node->CreatedProperty;
  if (context->mProcessedObjects.Contains(lightningProperty))
    return;
  context->mProcessedObjects.Insert(lightningProperty);

  // Push a new requirements object onto the stack
  StageRequirementsData prevRequirements = context->mCurrentRequirements;
  context->mCurrentRequirements = StageRequirementsData();

  LightningShaderIRLibrary* shaderLibrary = context->mCurrentLibrary;
  // Walk the initializer for the variable if it exists
  if (node->InitialValue != nullptr)
    context->Walker->Walk(this, node->InitialValue, context);
  // Merge the initializer results into the variable's results
  MergeCurrent(shaderLibrary, lightningProperty, node, context);

  // If we found any stage requirements then cache them on the library
  if (context->mCurrentRequirements.mRequiredStages != ShaderStage::None)
    shaderLibrary->mStageRequirementsData[lightningProperty] = context->mCurrentRequirements;
  // Now check if this symbol has any mis-matched requirements
  CheckAndDispatchErrors(lightningProperty, lightningProperty->Owner, context);

  // Pop the stack for the requirements we're building
  context->mCurrentRequirements = prevRequirements;
}

void StageRequirementsGatherer::WalkMemberAccessNode(Lightning::MemberAccessNode*& node,
                                                     StageRequirementsGathererContext* context)
{
  // Find the actual accessed function/property
  Lightning::Function* lightningFunction = nullptr;
  Lightning::Property* lightningProperty = nullptr;
  // If this is actually a getter/setter, then find the called get/set function
  if (node->AccessedGetterSetter != nullptr)
  {
    if (node->IoUsage & Lightning::IoMode::ReadRValue)
      lightningFunction = node->AccessedGetterSetter->Get;
    else if (node->IoUsage & Lightning::IoMode::WriteLValue)
      lightningFunction = node->AccessedGetterSetter->Set;
  }
  else if (node->AccessedFunction != nullptr)
    lightningFunction = node->AccessedFunction;
  else if (node->AccessedProperty != nullptr)
    lightningProperty = node->AccessedProperty;

  // If we're calling a function (including getter/setters)
  if (lightningFunction != nullptr)
  {
    // Deal with [Implements]. If an implements is registered we'll find a
    // shader function with a different lightning function then the one we started
    // with. This is the one that should actually be walked to find
    // dependencies.
    LightningShaderIRFunction* shaderFunction = context->mCurrentLibrary->FindFunction(lightningFunction);
    if (shaderFunction != nullptr)
      lightningFunction = shaderFunction->mMeta->mLightningFunction;

    // Recursively walk the function we're calling if it's in the current
    // library (to build up it's requirements)
    Lightning::FunctionNode* fnNode = context->mFunctionMap.FindValue(lightningFunction, nullptr);
    if (fnNode != nullptr)
      context->Walker->Walk(this, fnNode, context);

    // Merge the requirements from the constructor into the current object's
    // requirements.
    LightningShaderIRLibrary* shaderLibrary = context->mLightningLibraryToShaderLibraryMap[lightningFunction->SourceLibrary];
    MergeCurrent(shaderLibrary, lightningFunction, node, context);
  }
  // Otherwise, deal with member variables
  else if (lightningProperty != nullptr)
  {
    // Recursively walk the member we're referencing if it's in the current
    // library (to build up it's requirements)
    Lightning::MemberVariableNode* varNode = context->mVariableMap.FindValue(lightningProperty, nullptr);
    if (varNode != nullptr)
      context->Walker->Walk(this, varNode, context);

    // Merge the requirements from the constructor into the current object's
    // requirements.
    LightningShaderIRLibrary* shaderLibrary = context->mLightningLibraryToShaderLibraryMap[lightningProperty->Owner->SourceLibrary];
    MergeCurrent(shaderLibrary, lightningProperty, node, context);
  }
}

void StageRequirementsGatherer::WalkStaticTypeNode(Lightning::StaticTypeNode*& node,
                                                   StageRequirementsGathererContext* context)
{
  // If there's a constructor function then walk its dependencies
  Lightning::Function* constructor = node->ConstructorFunction;
  if (constructor != nullptr)
  {
    // Find the node for this constructor so we can walk the statements within.
    // If this node is null then the constructor call is from a different
    // library
    Lightning::ConstructorNode* constructorNode = context->mConstructorMap.FindValue(constructor, nullptr);
    if (constructorNode != nullptr)
      context->Walker->Walk(this, constructorNode, context);

    // Merge the requirements from the constructor into the current object's
    // requirements.
    LightningShaderIRLibrary* shaderLibrary = context->mLightningLibraryToShaderLibraryMap[constructor->SourceLibrary];
    MergeCurrent(shaderLibrary, constructor, node, context);
  }
  // Otherwise this is a constructor call to a class with an implicit
  // constructor. In this case we have to traverse the pre-constructor.
  else
  {
    Lightning::Function* preConstructor = node->ReferencedType->PreConstructor;
    if (preConstructor != nullptr)
    {
      // Walk the pre-constructor to collect all its requirements
      WalkClassPreconstructor(preConstructor, context);

      // Merge the requirements from the pre-constructor into the current
      // object's requirements.
      LightningShaderIRLibrary* shaderLibrary = context->mLightningLibraryToShaderLibraryMap[preConstructor->SourceLibrary];
      MergeCurrent(context->mCurrentLibrary, preConstructor, node, context);
    }
  }
}

void StageRequirementsGatherer::MergeCurrent(LightningShaderIRLibrary* shaderLibrary,
                                             Lightning::Member* lightningMember,
                                             Lightning::SyntaxNode* node,
                                             StageRequirementsGathererContext* context)
{
  // If the given member has any stage requirements then merge them into the
  // context's requirements
  StageRequirementsData* newState = shaderLibrary->mStageRequirementsData.FindPointer(lightningMember);
  if (newState != nullptr)
    context->mCurrentRequirements.Combine(lightningMember, node->Location, newState->mRequiredStages);
}

void StageRequirementsGatherer::BuildLibraryMap(LightningShaderIRLibrary* library,
                                                StageRequirementsGathererContext* context)
{
  // Map the lightning library to the shader library
  context->mLightningLibraryToShaderLibraryMap[library->mLightningLibrary] = library;

  // Walk all dependencies recursively
  if (library->mDependencies == nullptr)
    return;

  for (auto range = library->mDependencies->All(); !range.Empty(); range.PopFront())
    BuildLibraryMap(range.Front(), context);
}

ShaderStage::Enum StageRequirementsGatherer::GetRequiredStages(Lightning::Member* lightningObject,
                                                               Lightning::ReflectionObject* owner)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;
  ShaderStage::Enum stage = (ShaderStage::Enum)ShaderStage::None;

  // Check all shader stage types to see if this object or its owner contain
  // either a stage declaration or a stage requirement declaration.
  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    String stageName = nameSettings.mFragmentTypeAttributes[i];
    String requiresName = nameSettings.mRequiresAttributes[i];

    bool hasStageName = (lightningObject->HasAttribute(stageName) != nullptr) || owner->HasAttribute(stageName);
    bool hasRequiresName = false;
    // Currently, not all [Requires...] attributes exist. Only check non-empty
    // strings
    if (!requiresName.Empty())
      hasRequiresName = (lightningObject->HasAttribute(requiresName) != nullptr) || owner->HasAttribute(requiresName);
    if (hasStageName || hasRequiresName)
      stage = FragmentTypeToShaderStage((FragmentType::Enum)i);
  }

  return stage;
}

String StageRequirementsGatherer::GetStageName(ShaderStage::Enum stage)
{
  SpirVNameSettings& nameSettings = mSettings->mNameSettings;

  // Find the first non-plasma shader stage set and return that as the name.
  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    ShaderStage::Enum testStage = FragmentTypeToShaderStage((FragmentType::Enum)i);
    if (stage & testStage)
      return nameSettings.mFragmentTypeAttributes[i];
  }
  return String();
}

void StageRequirementsGatherer::CheckAndDispatchErrors(Lightning::Member* lightningObject,
                                                       Lightning::ReflectionObject* owner,
                                                       StageRequirementsGathererContext* context)
{
  ShaderStage::Enum currentRequiredStages = context->mCurrentRequirements.mRequiredStages;
  // If no shader stages are required for the current context then there's
  // nothing to do.
  if (currentRequiredStages == ShaderStage::None)
    return;

  // Otherwise the current context has some requirements. Find what stages are
  // required on the current object (or its owner) to see if they're compatible.
  // If the current object doesn't have any requirements then there's no
  // possible errors so there's nothing more to check.
  ShaderStage::Enum specifiedStages = GetRequiredStages(lightningObject, owner);
  if (specifiedStages == ShaderStage::None)
    return;

  // Otherwise we might have an error. An error exists if there is a mismatch
  // between the current required and specified stages. We can test this by
  // seeing if the bit pattern matches between the stage specifications.
  bool isValid = (currentRequiredStages ^ specifiedStages) == 0;
  if (isValid)
    return;

  Array<StageRequirementsData*> stack;
  Lightning::Member* currentObj = lightningObject;
  // Trace back the error locations until we find the source of the requirement
  while (currentObj != nullptr)
  {
    // By default, assume the source library (where we trace cached
    // dependencies) is the same as the owner of the object (e.g. the function's
    // class' library).
    Lightning::Library* lightningSourceLibrary = lightningSourceLibrary = currentObj->Owner->SourceLibrary;

    // This assumption isn't always correct though. In particular, on extension
    // and implements the owning object can be in a different library from where
    // the object is declared (e.g. an extension function on Math has the owner
    // in the 'Core' library even though the cached information is in the user's
    // library). If this is a function then grab the source library from the
    // function itself.
    Lightning::Function* fn = Lightning::Type::DynamicCast<Lightning::Function*>(currentObj);
    if (fn != nullptr)
      lightningSourceLibrary = fn->SourceLibrary;

    // Lookup the object's shader source library from the lightning library
    LightningShaderIRLibrary* objectLibrary =
        context->mLightningLibraryToShaderLibraryMap.FindValue(lightningSourceLibrary, nullptr);

    // Find the cached requirements data for this object. If it doesn't exist or
    // this object has no dependencies then we've reached the end of the
    // dependency chain.
    StageRequirementsData* requirements = objectLibrary->mStageRequirementsData.FindPointer(currentObj);
    if (requirements == nullptr || requirements->mDependency == nullptr)
      break;

    // Otherwise, cache the requirements objects so we can build the full error
    // message
    stack.PushBack(requirements);
    // And then continue the recursion
    currentObj = requirements->mDependency;
  }

  // Get the names of the mismatched stages so we can write the error message
  String requiredStageName = GetStageName(currentRequiredStages);
  String currentStage = GetStageName(specifiedStages);

  // Build up the full error message which says where the error originated
  // from and who called it as well as what the error was.
  Lightning::Member* errorSource = stack[stack.Size() - 1]->mDependency;
  String currentObjName = BuildString(lightningObject->Owner->Name, ".", lightningObject->Name);
  String errrorSourceName = BuildString(errorSource->Owner->Name, ".", errorSource->Name);

  String fullMsg = String::Format("'%s' requires shader stage %s but "
                                  "references '%s' which requires stage %s\n",
                                  currentObjName.c_str(),
                                  currentStage.c_str(),
                                  errrorSourceName.c_str(),
                                  requiredStageName.c_str());

  ValidationErrorEvent toSend;
  toSend.mShortMessage = "Invalid shader stage combination";
  toSend.mFullMessage = fullMsg;
  toSend.mLocation = lightningObject->Location;
  // Copy over the call location stack trace
  for (size_t i = 0; i < stack.Size(); ++i)
    toSend.mCallStack.PushBack(stack[i]->mCallLocation);

  EventSend(context->mErrors, Events::ValidationError, &toSend);
  // Make sure to mark that we've triggered an error
  context->mErrors->mErrorTriggered = true;
}

} // namespace Plasma
