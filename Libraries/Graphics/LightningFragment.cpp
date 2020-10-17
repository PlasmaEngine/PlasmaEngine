// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(LightningFragment, builder, type)
{
}

LightningFragment::LightningFragment()
{
}

void LightningFragment::ReloadData(StringRange data)
{
  LightningDocumentResource::ReloadData(data);

  mResourceLibrary->FragmentsModified();

  ResourceEvent event;
  event.Manager = LightningFragmentManager::GetInstance();
  event.EventResource = this;
  event.Manager->DispatchEvent(Events::ResourceModified, &event);
}

void AddResourceLibraries(Array<Lightning::LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange (ResourceLibrary* dependency, library->Dependencies.All())
    AddResourceLibraries(libraries, dependency);

  libraries.PushBack(library->mSwapFragment.mCurrentLibrary);
}

void LightningFragment::GetKeywords(Array<Completion>& keywordsOut)
{
  LightningBase::GetKeywords(keywordsOut);

  GraphicsEngine* graphicsEngine = PL::gEngine->has(GraphicsEngine);
  LightningShaderGenerator* shaderGenerator = graphicsEngine->mShaderGenerator;
  SpirVNameSettings& nameSettings = shaderGenerator->mSpirVSettings->mNameSettings;

  // Create a map of special keywords that we can't use in shaders
  HashSet<String> badKeywords;
  badKeywords.Insert("any");
  badKeywords.Insert("class");
  badKeywords.Insert("debug");
  badKeywords.Insert("delegate");
  badKeywords.Insert("delete");
  badKeywords.Insert("destructor");
  badKeywords.Insert("do");
  badKeywords.Insert("enum");
  badKeywords.Insert("foreach");
  badKeywords.Insert("loop");
  badKeywords.Insert("memberid");
  badKeywords.Insert("new");
  badKeywords.Insert("null");
  badKeywords.Insert("sends");
  badKeywords.Insert("set");
  badKeywords.Insert("typeid");
  badKeywords.Insert("typeof");
  badKeywords.Insert("event");

  AddKeywords(keywordsOut, Grammar::GetUsedKeywords(), badKeywords);
  AddKeywords(keywordsOut, Grammar::GetSpecialKeywords(), badKeywords);

  AddKeywords(keywordsOut, nameSettings.mAllowedClassAttributes);
  AddKeywords(keywordsOut, nameSettings.mAllowedFunctionAttributes);
  AddKeywords(keywordsOut, nameSettings.mAllowedFieldAttributes);
}

void LightningFragment::AddKeywords(Array<Completion>& keywordsOut,
                                const Array<String>& keyswords,
                                HashSet<String>& keywordsToSkip)
{
  forRange (String& keyword, keyswords.All())
  {
    if (!keywordsToSkip.Contains(keyword))
      keywordsOut.PushBack(keyword);
  }
}

void LightningFragment::AddKeywords(Array<Completion>& keywordsOut, const HashMap<String, AttributeInfo>& keyswordsToTest)
{
  typedef HashMap<String, AttributeInfo> AttributeMap;
  forRange (AttributeMap::pair& pair, keyswordsToTest.All())
  {
    if (!pair.second.mHidden)
      keywordsOut.PushBack(pair.first);
  }
}

void LightningFragment::GetLibraries(Array<Lightning::LibraryRef>& libraries)
{
  // Add the core library so we get auto-completion on things like Console
  Lightning::Core& core = Core::GetInstance();
  libraries.Append(core.GetLibrary());
  // Also add the intrinsics library (to get the 'Shader' type).
  libraries.Append(ShaderIntrinsicsLibrary::GetInstance().GetLibrary());

  GetLibrariesRecursive(libraries, mResourceLibrary);
}

void LightningFragment::GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange (ResourceLibrary* dependency, library->Dependencies.All())
    GetLibrariesRecursive(libraries, dependency);

  if (library->mSwapFragment.mCurrentLibrary != nullptr)
  {
    GraphicsEngine* graphicsEngine = PL::gEngine->has(GraphicsEngine);
    Lightning::LibraryRef wrapperLibrary = library->mSwapFragment.mCurrentLibrary;
    LightningShaderIRLibrary* internalFragmentLibrary =
        graphicsEngine->mShaderGenerator->GetInternalLibrary(wrapperLibrary);
    ErrorIf(internalFragmentLibrary == nullptr, "Didn't find an internal library for a wrapper library");
    libraries.PushBack(internalFragmentLibrary->mLightningLibrary);
  }
}

void LightningFragment::AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition)
{
  LightningDocumentResource::AttemptGetDefinition(editor, cursorPosition, definition);
}

HandleOf<Resource> LightningFragmentLoader::LoadFromFile(ResourceEntry& entry)
{
  LightningFragmentManager* manager = LightningFragmentManager::GetInstance();
  LightningFragment* fragment = new LightningFragment();
  fragment->DocumentSetup(entry);
  fragment->mText = ReadFileIntoString(entry.FullPath);
  manager->AddResource(entry, fragment);
  return fragment;
}

HandleOf<Resource> LightningFragmentLoader::LoadFromBlock(ResourceEntry& entry)
{
  LightningFragmentManager* manager = LightningFragmentManager::GetInstance();
  LightningFragment* fragment = new LightningFragment();
  fragment->DocumentSetup(entry);
  fragment->mText = String((cstr)entry.Block.Data, entry.Block.Size);
  manager->AddResource(entry, fragment);
  return fragment;
}

void LightningFragmentLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  ((LightningFragment*)resource)->ReloadData(ReadFileIntoString(entry.FullPath));
}

ImplementResourceManager(LightningFragmentManager, LightningFragment);

LightningFragmentManager::LightningFragmentManager(BoundType* resourceType) :
    ResourceManager(resourceType),
    mLastExceptionVersion(-1)
{
  mCategory = "Code";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("Lightning Fragment (*.lightningfrag)", "*.lightningfrag"));
  mNoFallbackNeeded = true;
  mCanCreateNew = true;
  mCanDuplicate = true;
  mSearchable = true;
  mExtension = FileExtensionManager::GetLightningFragmentTypeEntry()->GetDefaultExtensionNoDot();
  mCanReload = true;

  AddLoader("LightningFragment", new LightningFragmentLoader());
}

LightningFragmentManager::~LightningFragmentManager()
{
}

void LightningFragmentManager::ValidateNewName(Status& status, StringParam name, BoundType* optionalType)
{
  // Check all shader types
  GraphicsEngine* graphicsEngine = PL::gEngine->has(GraphicsEngine);
  LightningShaderIRLibrary* lastFragmentLibrary = graphicsEngine->mShaderGenerator->GetCurrentInternalProjectLibrary();
  if (lastFragmentLibrary)
  {
    LightningShaderIRType* shaderType = lastFragmentLibrary->FindType(name, true);
    if (shaderType != nullptr)
    {
      status.SetFailed(String::Format("Type '%s' is already a fragment type", name.c_str()));
      return;
    }
  }

  LightningDocumentResource::ValidateNewScriptName(status, name);
}

void LightningFragmentManager::ValidateRawName(Status& status, StringParam name, BoundType* optionalType)
{
  LightningDocumentResource::ValidateRawScriptName(status, name);
}

String LightningFragmentManager::GetTemplateSourceFile(ResourceAdd& resourceAdd)
{
  LightningFragment* fragmentTemplate = Type::DynamicCast<LightningFragment*, Resource*>(resourceAdd.Template);

  ReturnIf(fragmentTemplate == nullptr, String(), "Invalid resource given to create template.");

  // Get the correct template file name
  String templateFile = BuildString("Template", fragmentTemplate->Name);

  // Replace the fragment name where needed
  Replacements replacements;
  Replacement& nameReplacement = replacements.PushBack();
  nameReplacement.MatchString = "RESOURCE_NAME_";
  nameReplacement.ReplaceString = resourceAdd.Name;

  // Replace the tabs with spaces
  Replacement& tabReplacement = replacements.PushBack();
  tabReplacement.MatchString = "\t";
  tabReplacement.ReplaceString = "    ";

  // Two spaces if specified
  TextEditorConfig* config = PL::gEngine->GetConfigCog()->has(TextEditorConfig);
  if (config && config->TabWidth == TabWidth::TwoSpaces)
    tabReplacement.ReplaceString = "  ";

  String fileData = Replace(replacements, fragmentTemplate->mText);

  // Get template data off of resource
  String sourceFile = FilePath::Combine(GetTemporaryDirectory(), resourceAdd.FileName);
  WriteStringRangeToFile(sourceFile, fileData);
  return sourceFile;
}

void LightningFragmentManager::DispatchScriptError(StringParam eventId,
                                               StringParam shortMessage,
                                               StringParam fullMessage,
                                               const Lightning::CodeLocation& location)
{
  // This should only happen when a composite has a lightning error. Figure out how
  // to report later?
  if (location.CodeUserData == nullptr)
    return;

  LightningDocumentResource* resource = (LightningDocumentResource*)location.CodeUserData;

  if (mLastExceptionVersion != LightningManager::GetInstance()->mVersion)
  {
    mLastExceptionVersion = LightningManager::GetInstance()->mVersion;
    mDuplicateExceptions.Clear();
  }

  bool isDuplicate = mDuplicateExceptions.Contains(fullMessage);
  mDuplicateExceptions.Insert(fullMessage);

  if (!isDuplicate)
  {
    ScriptEvent e;
    e.Script = resource;
    e.Message = shortMessage;
    e.Location = location;
    PL::gResources->DispatchEvent(eventId, &e);
  }

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

} // namespace Plasma
