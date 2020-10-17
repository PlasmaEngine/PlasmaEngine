// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningShaderIRProject::LightningShaderIRProject(StringParam projectName)
{
  mProjectName = projectName;
  mCompiledSuccessfully = false;
}

void LightningShaderIRProject::AddCodeFromString(StringParam code, StringParam codeLocation, void* userData)
{
  CodeEntry& entry = mCodeEntries.PushBack();
  entry.mCode = code;
  entry.mCodeLocation = codeLocation;
  entry.mUserData = userData;
}

void LightningShaderIRProject::AddCodeFromFile(StringParam filePath, void* userData)
{
  File file;
  Status status;
  file.Open(filePath, FileMode::Read, FileAccessPattern::Sequential, FileShare::Read, &status);

  String code = ReadFileIntoString(filePath);
  AddCodeFromString(code, filePath, userData);
}

void LightningShaderIRProject::Clear()
{
  mCodeEntries.Clear();
  UserData = nullptr;
  ComplexUserData.Clear();
}

bool LightningShaderIRProject::CompileTree(Lightning::Module& lightningDependencies,
                                       Lightning::LibraryRef& lightningLibrary,
                                       Lightning::SyntaxTree& syntaxTree,
                                       Lightning::Array<Lightning::UserToken>& tokensOut)
{
  // Add all of the source code to the lightning project
  Lightning::Project lightningProject;
  BuildLightningProject(lightningProject);
  // Listen for compilation errors on this lightning project (so we can forward them
  // back up)
  ListenForLightningErrors(lightningProject);
  ListenForTypeParsed(lightningProject);

  // Compile the source code into a syntax tree
  Lightning::LibraryBuilder libraryBuilder(mProjectName);
  bool compiledSuccessfully = lightningProject.CompileCheckedSyntaxTree(
      syntaxTree, libraryBuilder, tokensOut, lightningDependencies, Lightning::EvaluationMode::Project);

  // If it failed to compile then don't build the library
  if (!compiledSuccessfully)
    return compiledSuccessfully;

  // Always have to run the code generator to create the lightning library
  // (so we can build an executable state and run it to collect default values).
  Lightning::CodeGenerator codeGenerator;
  lightningLibrary = codeGenerator.Generate(syntaxTree, libraryBuilder);
  return compiledSuccessfully;
}

LightningShaderIRLibraryRef LightningShaderIRProject::CompileAndTranslate(LightningShaderIRModuleRef& dependencies,
                                                                  BaseShaderIRTranslator* translator)
{
  // Reset the error state
  mErrorTriggered = false;

  // Add all of the dependencies to the lightning module
  Lightning::Module lightningDependencies;
  // Module is defaulting with an entry that is already being accounted for
  lightningDependencies.Clear();
  PopulateLightningModule(lightningDependencies, dependencies);

  // Add all of the source code to the lightning project
  Lightning::Project lightningProject;
  BuildLightningProject(lightningProject);
  // Listen for compilation errors on this lightning project (so we can forward them
  // back up)
  ListenForLightningErrors(lightningProject);
  ListenForTypeParsed(lightningProject);

  // Compile the source code into a syntax tree
  Lightning::Array<Lightning::UserToken> tokensOut;
  Lightning::SyntaxTree syntaxTree;
  Lightning::LibraryBuilder libraryBuilder(mProjectName);
  mCompiledSuccessfully = lightningProject.CompileCheckedSyntaxTree(
      syntaxTree, libraryBuilder, tokensOut, lightningDependencies, Lightning::EvaluationMode::Project);

  // If it failed to compile then don't build the library
  if (!mCompiledSuccessfully)
    return nullptr;

  // Otherwise make a new library ref
  LightningShaderIRLibrary* library = new LightningShaderIRLibrary();
  library->mDependencies = dependencies;
  // Flatten all dependents from the dependency parents
  library->FlattenModuleDependents();

  // Always have to run the code generator to create the lightning library
  // (so we can build an executable state and run it to collect default values).
  Lightning::CodeGenerator codeGenerator;
  library->mLightningLibrary = codeGenerator.Generate(syntaxTree, libraryBuilder);

  LightningShaderIRLibraryRef libraryRef = library;

  // Add the lightning library that was just built to the lightning module so any
  // calls such as searching for a type will search the new library
  lightningDependencies.PushBack(library->mLightningLibrary);

  // Now actually translate the types of this library
  library->mTranslated = translator->Translate(syntaxTree, this, libraryRef);
  // If translation failed then return then don't return a valid library
  if (library->mTranslated == false)
    return nullptr;

  // For all types in this library, collect the default values of all properties
  // (so they can be stored in meta or wherever)
  CollectLibraryDefaultValues(libraryRef, lightningDependencies);

  return libraryRef;
}

// LightningShaderLibraryRef CompileAndTranslate(LightningShaderModuleRef& dependencies,
// BaseShaderTranslator* translator, LightningShaderSettingsRef& settings, bool test
// = false);
void LightningShaderIRProject::BuildLightningProject(Lightning::Project& lightningProject)
{
  for (size_t i = 0; i < mCodeEntries.Size(); ++i)
    lightningProject.AddCodeFromString(mCodeEntries[i].mCode, mCodeEntries[i].mCodeLocation, mCodeEntries[i].mUserData);
  lightningProject.UserData = UserData;
  lightningProject.ComplexUserData = ComplexUserData;
}

void LightningShaderIRProject::PopulateLightningModule(Lightning::Module& lightningDependencies, LightningShaderIRModuleRef& dependencies)
{
  // Handle having no dependencies
  if (dependencies == nullptr)
    return;

  // The shader dependencies contains a whole bunch of top-level dependencies.
  // First add all of these to a stack of modules we need to walk.
  Array<LightningShaderIRLibrary*> dependencyStack;
  for (size_t i = 0; i < dependencies->Size(); ++i)
    dependencyStack.PushBack((*dependencies)[i]);

  // Now we need to iterate over all dependencies of dependencies but in a
  // breadth first order. This is not a "proper" dependency walker and can run
  // into errors in diamond situations, but I'm ignoring this for now.
  HashSet<Lightning::Library*> visitedLightningDependencies;
  for (size_t i = 0; i < dependencyStack.Size(); ++i)
  {
    // Add the lightning dependency of this library to the lightning module
    LightningShaderIRLibrary* dependencyLibrary = dependencyStack[i];
    lightningDependencies.PushBack(dependencyLibrary->mLightningLibrary);

    // @JoshD - Confirm this is correct
    visitedLightningDependencies.Insert(dependencyLibrary->mLightningLibrary);

    // If this shader library doesn't have any dependencies then stop
    if (dependencyLibrary->mDependencies == nullptr)
      continue;

    // Otherwise walk all dependent shader libraries
    for (size_t subIndex = 0; subIndex < dependencyLibrary->mDependencies->Size(); ++subIndex)
    {
      LightningShaderIRLibrary* subLibrary = (*dependencyLibrary->mDependencies)[subIndex];
      Lightning::Library* subLightningLibrary = subLibrary->mLightningLibrary;

      // Basic error Checking
      if (subLibrary == nullptr || subLightningLibrary == nullptr)
      {
        Error("Cannot have a null library as a dependency");
        continue;
      }
      ErrorIf(subLibrary->mTranslated == false, "Dependency was not already compiled somehow");

      // If we've already walked this library then ignore it
      if (visitedLightningDependencies.Contains(subLightningLibrary))
        continue;

      // Mark that we've now visited this lightning library and add it to the lightning
      // module
      visitedLightningDependencies.Insert(subLightningLibrary);
      dependencyStack.PushBack(subLibrary);
    }
  }
}

void LightningShaderIRProject::CollectLibraryDefaultValues(LightningShaderIRLibraryRef libraryRef, Lightning::Module& lightningModule)
{
  LightningShaderIRLibrary* library = libraryRef;
  // Link the module together to get an executable state we can run (to find
  // default values)
  Lightning::ExecutableState* state = lightningModule.Link();

  // Iterate over all the types in this library
  AutoDeclare(range, library->mTypes.All());
  for (; !range.Empty(); range.PopFront())
  {
    // Find the lightning type from the shader type
    LightningShaderIRType* shaderType = range.Front().second;
    ShaderIRTypeMeta* typeMeta = shaderType->mMeta;

    // Only get property values for fragment types (not helpers)
    if (typeMeta == nullptr || typeMeta->mFragmentType == FragmentType::None)
      continue;

    Lightning::BoundType* lightningType = lightningModule.FindType(typeMeta->mLightningName);

    // Default construct this type
    Lightning::ExceptionReport report;
    Lightning::Handle preconstructedObject =
        state->AllocateDefaultConstructedHeapObject(lightningType, report, Lightning::HeapFlags::NonReferenceCounted);

    for (size_t i = 0; i < typeMeta->mFields.Size(); ++i)
    {
      ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[i];

      // Check if this is a static field
      bool isStatic = fieldMeta->ContainsAttribute(Lightning::StaticAttribute);

      // Find the lightning property, properly setting the options
      // depending on if this is a static field or not
      Lightning::FindMemberOptions::Enum options = Lightning::FindMemberOptions::None;
      if (isStatic)
        options = Lightning::FindMemberOptions::Static;

      Lightning::Property* lightningProperty = lightningType->FindProperty(fieldMeta->mLightningName, options);
      // Validate that the property exists in lightning. This might not exist if the
      // property is entirely generated in shaders (such as the dummy)
      if (lightningProperty == nullptr)
        continue;

      // Invoke the property's Get function so we can find the return value
      Lightning::Call call(lightningProperty->Get, state);
      // Set the 'this' handle if the field is an instance field
      if (!isStatic)
        call.SetHandle(Lightning::Call::This, preconstructedObject);
      call.Invoke(report);

      if (report.HasThrownExceptions())
      {
        Error("Getting property default value from pre-constructed object "
              "failed");
        break;
      }

      // Extract the return value of the property's Get call and store it as an
      // Lightning::Any on our ShaderType
      fieldMeta->mDefaultValueVariant = Lightning::Any(call.GetReturnUnchecked(), lightningProperty->GetTypeOrNull());
    }
  }
  delete state;
}

} // namespace Plasma
