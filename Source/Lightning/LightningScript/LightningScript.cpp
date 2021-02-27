// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void PlasmaLightningExceptionCallback(ExceptionEvent* e)
{
  // Get the first non-native stack for debugging
  Exception* exception = e->ThrownException;
  CodeLocation location = exception->Trace.GetMostRecentNonNativeLocation();

  String shortMessage = exception->Message;
  String fullMessage = exception->GetFormattedMessage(MessageFormat::Python);
  LightningScriptManager::DispatchScriptError(Events::UnhandledException, shortMessage, fullMessage, location);
}

void PlasmaLightningFatalErrorCallback(FatalErrorEvent* e)
{
  if (e->ErrorCode == FatalError::OutOfMemory)
    FatalEngineError("Lightning Fatal Error: Out of Memory");
  else if (e->ErrorCode == FatalError::StackReserveOverflow)
    FatalEngineError("Lightning Fatal Error: Stack Reserve Overflow");
  else
    FatalEngineError("Lightning Fatal Error: Error Code '%d'", (int)e->ErrorCode);
}

// Called when an error occurs in compilation
void PlasmaLightningErrorCallback(Lightning::ErrorEvent* e)
{
  // If plugins are currently compiling, let the user know that the error
  // *might* be because of that
  LightningPluginSourceManager* manager = LightningPluginSourceManager::GetInstance();
  if (manager->IsCompilingPlugins())
  {
    e->ExactError = BuildString(e->ExactError,
                                "\nThis may be because we're currently compiling Lightning"
                                " plugins (once finished, scripts will recompile)");
  }

  String shortMessage = e->ExactError;
  String fullMessage = e->GetFormattedMessage(MessageFormat::Python);

  LightningScriptManager::DispatchScriptError(Events::SyntaxError, shortMessage, fullMessage, e->Location);
}

void OnDebuggerPauseUpdate(DebuggerEvent* event)
{
  Event toSend;
  PL::gEngine->DispatchEvent(Events::DebuggerPauseUpdate, &toSend);

  // We assume the graphical rendering will not change the state of the program,
  // so its safe to do during a breakpoint This also generally draws some sort
  // of 'debugging' overlay
  // PL::gGraphics->PerformRenderTasks(0.0f);
}

void OnDebuggerPause(DebuggerEvent* event)
{
  Event toSend;
  PL::gEngine->DispatchEvent(Events::DebuggerPause, &toSend);
}

void OnDebuggerResume(DebuggerEvent* event)
{
  Event toSend;
  PL::gEngine->DispatchEvent(Events::DebuggerResume, &toSend);
}

// LightningScript Resource
LightningDefineType(LightningScript, builder, type)
{
  PlasmaBindDocumented();
}

void LightningScript::ReloadData(StringRange data)
{
  LightningDocumentResource::ReloadData(data);

  mResourceLibrary->ScriptsModified();
}

void LightningScript::GetKeywords(Array<Completion>& keywordsOut)
{
  LightningBase::GetKeywords(keywordsOut);

  LightningScriptManager* manager = LightningScriptManager::GetInstance();
  keywordsOut.Append(Grammar::GetUsedKeywords().All());
  keywordsOut.Append(Grammar::GetSpecialKeywords().All());

  AttributeExtensions* attributeExtensions = AttributeExtensions::GetInstance();
  keywordsOut.Append(attributeExtensions->mClassExtensions.Keys());
  keywordsOut.Append(attributeExtensions->mPropertyExtensions.Keys());
  keywordsOut.Append(attributeExtensions->mFunctionExtensions.Keys());
}

void LightningScript::GetLibraries(Array<LibraryRef>& libraries)
{
  MetaDatabase* metaDatabase = MetaDatabase::GetInstance();
  libraries.Insert(libraries.End(), metaDatabase->mNativeLibraries.All());

  // Add the core library so we get auto-completion on things like Console
  Lightning::Core& core = Core::GetInstance();
  libraries.Append(core.GetLibrary());

  GetLibrariesRecursive(libraries, mResourceLibrary);
}

// @TrevorS: Isn't this the same logic as AddDependencies on
// ResourceLibrary/LightningManager?
void LightningScript::GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange (ResourceLibrary* dependency, library->Dependencies.All())
    GetLibrariesRecursive(libraries, dependency);

  forRange (SwapLibrary& swapPlugin, library->mSwapPlugins.Values())
  {
    if (swapPlugin.mCurrentLibrary != nullptr)
      libraries.PushBack(swapPlugin.mCurrentLibrary);
  }

  if (library->mSwapScript.mCurrentLibrary != nullptr)
    libraries.PushBack(library->mSwapScript.mCurrentLibrary);
  if (library->mSwapFragment.mCurrentLibrary != nullptr)
    libraries.PushBack(library->mSwapFragment.mCurrentLibrary);
}

// LightningScriptLoader
HandleOf<Resource> LightningScriptLoader::LoadFromFile(ResourceEntry& entry)
{
  LightningScript* script = new LightningScript();
  script->DocumentSetup(entry);
  LightningScriptManager::GetInstance()->AddResource(entry, script);
  script->mText = ReadFileIntoString(entry.FullPath);
  return script;
}

HandleOf<Resource> LightningScriptLoader::LoadFromBlock(ResourceEntry& entry)
{
  LightningScript* script = new LightningScript();
  script->DocumentSetup(entry);
  LightningScriptManager::GetInstance()->AddResource(entry, script);
  script->mText = String((cstr)entry.Block.Data, entry.Block.Size);
  return script;
}

void LightningScriptLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  ((LightningScript*)resource)->ReloadData(ReadFileIntoString(entry.FullPath));
}

// LightningScriptManager
ImplementResourceManager(LightningScriptManager, LightningScript);

LightningScriptManager::LightningScriptManager(BoundType* resourceType) :
    ResourceManager(resourceType),
    mLastExceptionVersion(-1)
{
  mCategory = "Code";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("All Lightning Scripts", "*.lightningscript;*.z"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.lightningscript"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.z"));
  // We want LightningScript to be the first thing that shows up in the "Code"
  // category in the add window
  mAddSortWeight = 0;
  mNoFallbackNeeded = true;
  mCanCreateNew = true;
  mSearchable = true;
  mExtension = FileExtensionManager::GetLightningScriptTypeEntry()->GetDefaultExtensionNoDot();
  mCanReload = true;

  AddLoader("LightningScript", new LightningScriptLoader());

  // listen for when we should compile
  Lightning::EventConnect(ExecutableState::CallingState, Lightning::Events::PreUnhandledException, PlasmaLightningExceptionCallback);
  Lightning::EventConnect(ExecutableState::CallingState, Lightning::Events::FatalError, PlasmaLightningFatalErrorCallback);

  ConnectThisTo(PL::gResources, Events::ResourceLibraryConstructed, OnResourceLibraryConstructed);
}

void LightningScriptManager::ValidateNewName(Status& status, StringParam name, BoundType* optionalType)
{
  LightningDocumentResource::ValidateNewScriptName(status, name);
}

void LightningScriptManager::ValidateRawName(Status& status, StringParam name, BoundType* optionalType)
{
  if (!optionalType || optionalType->IsA(LightningTypeId(Component)))
  {
    // Because we do component access off of Cogs using the . operator, then it
    // might conflict with an actual member of Cog (name a component 'Destroy',
    // what is Owner.Destroy?) We must do this for Space and GameSession also
    // (technically GameSession and Space doubly hit Cog, but that's fine).
    bool hasMember = LightningTypeId(Cog)->GetMember(name) || LightningTypeId(GameSession)->GetMember(name) ||
                     LightningTypeId(Space)->GetMember(name) || LightningTypeId(CogPath)->GetMember(name);

    if (hasMember)
    {
      String message = String::Format("Components cannot have the same name as a property/method on "
                                      "Cog/Space/GameSession (this.Owner.%s would conflict)",
                                      name.c_str());
      status.SetFailed(message);
      return;
    }
  }

  LightningDocumentResource::ValidateRawScriptName(status, name);
}

String LightningScriptManager::GetTemplateSourceFile(ResourceAdd& resourceAdd)
{
  LightningScript* scriptTemplate = Type::DynamicCast<LightningScript*, Resource*>(resourceAdd.Template);

  ReturnIf(scriptTemplate == nullptr, String(), "Invalid resource given to create template.");

  String templateFile = BuildString("TemplateLightning", scriptTemplate->Name);

  Replacements replacements;

  // Replace the component name
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

  String fileData = Replace(replacements, scriptTemplate->mText);

  // Get template data off of resource
  String sourceFile = FilePath::Combine(GetTemporaryDirectory(), resourceAdd.FileName);
  WriteStringRangeToFile(sourceFile, fileData);
  return sourceFile;
}

void LightningScriptManager::OnResourceLibraryConstructed(ObjectEvent* e)
{
  ResourceLibrary* library = (ResourceLibrary*)e->Source;
  EventConnect(&library->mScriptProject, Lightning::Events::CompilationError, PlasmaLightningErrorCallback);
}

void LightningScriptManager::DispatchScriptError(StringParam eventId,
                                             StringParam shortMessage,
                                             StringParam fullMessage,
                                             const CodeLocation& location)
{
  LightningScriptManager* instance = LightningScriptManager::GetInstance();
  Resource* resource = (Resource*)location.CodeUserData;

  if (instance->mLastExceptionVersion != LightningManager::GetInstance()->mVersion)
  {
    instance->mLastExceptionVersion = LightningManager::GetInstance()->mVersion;
    instance->mDuplicateExceptions.Clear();
  }

  bool isDuplicate = instance->mDuplicateExceptions.Contains(fullMessage);
  instance->mDuplicateExceptions.Insert(fullMessage);

  if (!isDuplicate)
  {
    ScriptEvent e;
    e.Script = Type::DynamicCast<DocumentResource*>(resource);
    e.Message = shortMessage;
    e.Location = location;
    PL::gResources->DispatchEvent(eventId, &e);
  }

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

void LightningScriptManager::DispatchPlasmaLightningError(const CodeLocation& location,
                                                StringParam message,
                                                Project* buildingProject)
{
  String shortMessage = BuildString("Error: ", message);
  String fullMessage = location.GetFormattedStringWithMessage(MessageFormat::Python, shortMessage);
  buildingProject->Raise(location, ErrorCode::GenericError, message.c_str());
}

void LightningScriptManager::OnMemoryLeak(MemoryLeakEvent* event)
{
  static const String UnknownType("<UnkownType>");
  static const String NullDump("null");

  String typeName = UnknownType;
  String dump = NullDump;
  bool isTypeNative = true;
  Handle* leakedObject = event->LeakedObject;
  if (leakedObject != nullptr)
  {
    BoundType* type = leakedObject->StoredType;
    typeName = type->ToString();

    StringBuilderExtended builder;
    Lightning::Console::DumpValue(builder, type, (const byte*)leakedObject, 5, 0);
    dump = builder.ToString();

    isTypeNative = type->IsTypeOrBaseNative();
  }

  String message = String::Format("* A memory leak was detected with the type %s. Make sure "
                                  "to avoid cycles "
                                  "of references, or explicitly invoke delete (typically "
                                  "within a destructor).\n* Memory Dump:\n%s",
                                  typeName.c_str(),
                                  dump.c_str());

  WarnIf(isTypeNative, "%s", message.c_str());
  PlasmaPrint("%s", message.c_str());
}

} // namespace Plasma
