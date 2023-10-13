// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

LightningDefineType(EditorMain, builder, type)
{
}

EditorMain::EditorMain(Composite* parent, OsWindow* window) : Editor(parent)
{
  this->SetName("EditorMain");
  mTimeSinceEscape = 100.0f;
  mDisableInput = false;
  mGamePending = false;
  ConnectThisTo(parent, Events::Closing, OnClosing);
  ConnectThisTo(this, Events::MouseDown, OnMouseDown);
  ConnectThisTo(PL::gContentSystem, Events::PackageBuilt, OnPackagedBuilt);
}

EditorMain::~EditorMain()
{
  SafeDelete(mManager);
}

void EditorMain::OnPackagedBuilt(ContentSystemEvent* event)
{
  LoadPackage(mProject, event->mLibrary, event->mPackage);
}

bool EditorMain::LoadPackage(Cog* projectCog, ContentLibrary* library, ResourcePackage* package)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);

  ResourceSystem* resourceSystem = PL::gResources;

  ContentLibrary* originalProjectLibrary = mProjectLibrary;

  // Set the content library so Loading may try to create new content for
  // fixing old content elements.
  mProjectLibrary = library;

  Status status;
  project->ProjectResourceLibrary = resourceSystem->LoadPackage(status, package);
  if (!status)
    DoNotifyError("Failed to load resource package.", status.Message);

  DoEditorSideImporting(package, nullptr);
  PL::gEditor->SetExploded(false, true);

  // restore original mProjectLibrary value
  mProjectLibrary = originalProjectLibrary;

  return true;
}

void EditorMain::OnEngineUpdate(UpdateEvent* event)
{
  // Call base behavior for game management.
  Editor::OnEngineUpdate(event);

  mTimeSinceEscape += event->RealDt;
  Update();

  // We should only ever set the pending flag if we previously tried to create a
  // single instance
  if (mGamePending)
    PlayGame(PlayGameOptions::SingleInstance);
}

void EditorMain::OnClosing(HandleableEvent* event)
{
  // Prevent the window from closing
  // the main window automatically
  event->Handled = true;

  // Run editor quit logic
  RequestQuit(false);
}

void EditorMain::OnMouseDown(MouseEvent* mouseEvent)
{
  if (mouseEvent->Handled)
    return;

  if (mouseEvent->Button == MouseButtons::XOneBack)
  {
    mMainPropertyView->GetHistory()->Previous();
    if (mouseEvent->CtrlPressed)
      FocusOnSelectedObjects();
  }

  if (mouseEvent->Button == MouseButtons::XTwoForward)
  {
    mMainPropertyView->GetHistory()->Next();
    if (mouseEvent->CtrlPressed)
      FocusOnSelectedObjects();
  }
}

void EditorMain::OnCutCopyPaste(ClipboardEvent* event)
{
  if (event->mHandled)
    return;

  CommandManager* commands = CommandManager::GetInstance();
  if (commands->TestCommandCopyPasteShortcuts(event))
    return;
}

void EditorMain::OnKeyDown(KeyboardEvent* keyEvent)
{
  if (keyEvent->Handled || keyEvent->HandledEventScript || mDisableInput)
    return;

  // Tools
  uint keyPressed = keyEvent->Key;
  if (keyPressed >= '0' && keyPressed <= '9' && keyEvent->GetModifierPressed() == false)
  {
    if (keyPressed == '0')
      keyPressed += 10;
    uint toolIndex = keyPressed - '0' - 1;
    PL::gEditor->Tools->SelectToolIndex(toolIndex, ShowToolProperties::Auto);
  }

  CommandManager* commands = CommandManager::GetInstance();
  if (commands->TestCommandKeyboardShortcuts(keyEvent))
    return;

  if (keyEvent->CtrlPressed && keyEvent->AltPressed && mConfig->has(Plasma::DeveloperConfig) && keyEvent->Key == 'F')
    this->ShowWindow("Find/Replace Objects");

  // Global special hot keys
  if (keyEvent->ShiftPressed && keyEvent->CtrlPressed)
  {
    switch (keyEvent->Key)
    {
    case Keys::Tilde:
    {
      Editor::ToggleConsole();
      break;
    }

    case Keys::Space:
    {
      // Moved this to using Ctrl+Shift because shift represents anything to do
      // with selection, Ctrl + Space should bring up auto-complete, and Shift +
      // Space is really easy to accidentally type.
      OpenSearchWindow(nullptr);
      break;
    }

    case Keys::G:
    {
      SelectGame();
      break;
    }

    case Keys::S:
    {
      SelectSpace();
      break;
    }
    default:
      break;
    }
  }

  // Special keys
  Mouse* mouse = Mouse::GetInstance();

  switch (keyEvent->Key)
  {
  case Keys::Escape:
  {
    mouse->SetTrapped(false);

    if (DeveloperConfig* devConfig = mConfig->has(DeveloperConfig))
    {
      if (devConfig->mDoubleEscapeQuit && mTimeSinceEscape < 0.4f)
        PL::gEngine->Terminate();
    }
    mTimeSinceEscape = 0.0f;
    break;
  }

  case Keys::F10:
  {
    mouse->ToggleTrapped();
    break;
  }
  default:
    break;
  }
}

void EditorMain::ShowTools(CommandEvent* event)
{
  this->ShowWindow("Tools");
}

void EditorMain::ShowLibrary(CommandEvent* event)
{
  // If the library window is hidden, show it
  if (mManager->InactiveWidgets.ContainsKey("Library"))
  {
    this->ShowWindow("Library");
    return;
  }

  this->CreateLibraryView(PL::gEditor->mConfig->has(DeveloperConfig));
}

void EditorMain::ShowCoreLibrary(CommandEvent* event)
{
  this->CreateLibraryView(true);
}

LibraryView* EditorMain::CreateLibraryView(bool showCore, bool autoDock)
{
  Vec2 rootSize = this->GetRootWidget()->GetSize();
  float dockWidth = Math::Min(rootSize.x * 0.15f, 280.0f);

  // Create a new one library view
  LibraryView* library = new LibraryView(this);
  library->SetName("Library");
  library->SetHideOnClose(false);
  library->SetSize(Pixels(dockWidth, 280));
  library->SetTagEditorSize(SizeAxis::X, dockWidth);

  // Hide core libraries
  if (!showCore)
  {
    library->AddHiddenLibrary("Loading");
    library->AddHiddenLibrary("PlasmaCore");
    library->AddHiddenLibrary("UiWidget");
    library->AddHiddenLibrary("Editor");
    library->AddHiddenLibrary("EditorUi");
    library->AddHiddenLibrary("FragmentCore");
  }
  else
  {
    library->SetName("Library");
  }

  // Initially select the loaded project's library
  Editor* editor = PL::gEditor;
  Cog* projectCog = editor->mProject;

  // If no project is loaded, select the 0th element in the dropdown
  if (projectCog == nullptr)
  {
    library->View();
  }
  else
  {
    ProjectSettings* project = projectCog->has(ProjectSettings);
    library->View(editor->mProjectLibrary, project->ProjectResourceLibrary);
  }

  if (autoDock)
  {
    // Dock it to the layout
    LayoutInfo info;
    info.Area = DockArea::Right;
    info.Size = library->GetSize();
    info.Name = library->GetName();
    info.Visible = true;
    info.ActiveWidget = library;

    this->AddWidget(library, info);
  }

  return library;
}

void EditorMain::ToggleConsole(CommandEvent* event)
{
  Editor::ToggleConsole();
}

void EditorMain::ShowConsole(CommandEvent* event)
{
  Editor::ShowConsole();
}

void EditorMain::HideConsole(CommandEvent* event)
{
  Editor::HideConsole();
}

void EditorMain::ShowMarket(CommandEvent* event)
{
  Editor::ShowMarket();
}

void EditorMain::ShowChat(CommandEvent* event)
{
  Editor::ShowChat();
}

void EditorMain::ShowObjects(CommandEvent* event)
{
  this->ShowWindow("Objects");
}

void EditorMain::ShowAnimator(CommandEvent* event)
{
  Widget* widget = ShowWindow("Animator");
  widget->TakeFocus();
}

void EditorMain::ShowHotKeyEditor(CommandEvent* event)
{
  // Widget* widget = ShowWindow("CommandListViewer");
  ////widget->SetSize(Pixels(850, 500));
  // widget->TakeFocus();

  Widget* widget = PL::gEditor->mManager->ShowWidget("Commands");
  ((HotKeyEditor*)widget)->DisplayResource();
}

void EditorMain::ShowOperationHistroy(CommandEvent* event)
{
  if (mManager->FindWidget("HistoryWindow"))
    return;

  GameSession* editorGameSession = mEditGame;
  Space* newSpace =
      editorGameSession->CreateNamedSpace("HistoryWindow", ArchetypeManager::FindOrNull(CoreArchetypes::Space));

  Level* level = LevelManager::FindOrNull("HistoryWindowLevel");
  if (level != nullptr)
    newSpace->LoadLevel(level);
  else
    Error("HistoryWindowLevel not found.");

  Cog* cameraCog = newSpace->FindObjectByName("Camera");
  CameraViewport* cameraViewport = cameraCog->has(CameraViewport);

  CreateDockableWindow("HistoryWindow", cameraViewport, Vec2(300, 450), true);
}

void EditorMain::ShowBroadPhaseTracker(CommandEvent* event)
{
  this->ShowWindow("BroadPhaseTracker");
}

void EditorMain::ShowProperties(CommandEvent* event)
{
  this->ShowWindow("Properties");
}

void EditorMain::ShowConfig(CommandEvent* event)
{
  ShowProperties(event);
  mSelection->SelectOnly(mConfig);
  mSelection->FinalSelectionChanged();
}

void EditorMain::ShowProject(CommandEvent* event)
{
  ShowProperties(event);
  mSelection->SelectOnly(mProject);
  mSelection->FinalSelectionChanged();
}

void EditorMain::SelectTweakables(CommandEvent* event)
{
  ShowProperties(event);
  if (PL::gEngine->GetConfigCog()->has(DeveloperConfig))
    mMainPropertyView->EditObject(PL::gTweakables, true);
}

void EditorMain::ShowFindNext(CommandEvent* event)
{
  this->ShowWindow("Find/Replace Text");
  mFindTextDialog->DefaultFindNextSettings();
}

void EditorMain::ShowFindAll(CommandEvent* event)
{
  this->ShowWindow("Find/Replace Text");
  mFindTextDialog->DefaultFindAllSettings();
}

void EditorMain::ShowReplaceNext(CommandEvent* event)
{
  this->ShowWindow("Find/Replace Text");
  mFindTextDialog->DefaultReplaceNextSettings();
}

void EditorMain::ShowReplaceAll(CommandEvent* event)
{
  this->ShowWindow("Find/Replace Text");
  mFindTextDialog->DefaultReplaceAllSettings();
}

void EditorMain::ShowBugReporter(CommandEvent* event)
{
  // For now the window is disabled and we link directly to the github page.
  // TODO: Fix the bug reporter to actually work with GitHub's API.
  Os::OpenUrl(Urls::cUserReportIssue);
  // this->ShowWindow("Bug Reporter");
  // mBugReporter->Reset();
}

void EditorMain::EditColorScheme(CommandEvent* event)
{
  this->ShowWindow("Properties");
  mMainPropertyView->EditObject(GetColorScheme(), true);
}

void EditorMain::ClearConsole(CommandEvent* event)
{
  mConsole->ClearAllReadOnly();
}

void EditorMain::OnNameActivated(TypeEvent* event)
{
  BoundType* boundType = event->mType;

  CodeDefinition definition;
  // If this is a native location, we need to generate stub code
  if (boundType->NameLocation.IsNative)
  {
    // Generate stub code for the library (if its already generated, this will
    // do nothing)
    Library* library = boundType->GetOwningLibrary();
    library->GenerateDefinitionStubCode();
  }

  // Copy out the needed information to the code definition (this looks like all
  // that's needed)
  definition.NameLocation = boundType->NameLocation;
  definition.Name = boundType->Name;
  definition.ElementLocation = boundType->Location;

  DisplayCodeDefinition(definition);
}

void EditorMain::ShowLibrary(StringParam libraryName)
{
  ContentLibrary* coreLibrary = PL::gContentSystem->Libraries.FindValue(libraryName, nullptr);
  ResourceLibrary* coreResourcSet = PL::gResources->LoadedResourceLibraries.FindValue(libraryName, nullptr);

  forRange (ContentItem* contentItem, coreLibrary->GetContentItems())
    contentItem->ShowInEditor = true;

  LibraryView* libraryView = new LibraryView(this);
  libraryView->View(coreLibrary, coreResourcSet);
  libraryView->SetName(libraryName);
  libraryView->SetHideOnClose(true);
  libraryView->SetSize(Pixels(280, 280));
  this->AddManagedWidget(libraryView, DockArea::Floating, true);
}

void EditorMain::ShowVolumeMeter(CommandEvent* event)
{
  if (mManager->FindWidget("VolumeMeterWindow"))
    return;

  GameSession* editorGameSession = mEditGame;
  Space* newSpace =
      editorGameSession->CreateNamedSpace("VolumeMeterWindow", ArchetypeManager::FindOrNull(CoreArchetypes::Space));

  Level* level = LevelManager::FindOrNull("VolumeMeterLevel");
  if (level != nullptr)
    newSpace->LoadLevel(level);
  else
    Error("VolumeMeterLevel not found.");

  Cog* cameraCog = newSpace->FindObjectByName("GameCamera");
  CameraViewport* cameraViewport = cameraCog->has(CameraViewport);

  CreateDockableWindow("VolumeMeterWindow", cameraViewport, Vec2(300, 330), true);
}

void EditorMain::ShowSoundNodeGraph(CommandEvent* event)
{
  if (mManager->FindWidget("SoundNodeGraphWindow"))
    return;

  GameSession* editorGameSession = mEditGame;
  Space* newSpace =
      editorGameSession->CreateNamedSpace("SoundNodeGraphWindow", ArchetypeManager::FindOrNull(CoreArchetypes::Space));

  Level* level = LevelManager::FindOrNull("SoundNodeGraphLevel");
  if (level != nullptr)
    newSpace->LoadLevel(level);
  else
    Error("SoundNodeGraphLevel not found.");

  Cog* cameraCog = newSpace->FindObjectByName("GameCamera");
  CameraViewport* cameraViewport = cameraCog->has(CameraViewport);

  CreateDockableWindow("SoundNodeGraphWindow", cameraViewport, Vec2(700, 500), true);
}

void EditorMain::ShowRenderGroupHierarchies(CommandEvent* event)
{
  // Just show current window if it already exists.
  if (Widget* widget = mManager->FindWidget("RenderGroupHierarchies"))
  {
    Plasma::ShowWidget(widget);
    return;
  }

  RenderGroupHierarchies* hierarchies = new RenderGroupHierarchies(this);
  hierarchies->SetName("RenderGroupHierarchies");
  hierarchies->SetSize(Pixels(280, 400));
  AddManagedWidget(hierarchies, DockArea::Floating, true);
}

void EditorMain::AttachDocumentEditor(StringParam name, DocumentEditor* docEditor)
{
  docEditor->SetName(name);
  PL::gEditor->AddManagedWidget(docEditor, DockArea::Center);

  Connect(this, Events::Save, docEditor, &DocumentEditor::OnSave);
  Connect(this, Events::SaveCheck, docEditor, &DocumentEditor::OnSaveCheck);
}

DocumentEditor* EditorMain::OpenTextString(StringParam name, StringParam text, StringParam extension)
{
  StringDocument* document = new StringDocument();
  String documentName = name;
  // Generate a name
  if (documentName.Empty())
    documentName = BuildString("Text", ToString(text.Hash()));

  Widget* widget = mManager->ShowWidget(documentName);
  if (widget)
    return Type::DynamicCast<DocumentEditor*>(widget);

  document->mName = documentName;
  document->mData = text;
  DocumentEditor* editor = CreateDocumentEditor(this, document);
  TypeExtensionEntry* lightningEntry = FileExtensionManager::GetLightningScriptTypeEntry();

  if (lightningEntry->IsValidExtensionNoDot(extension))
    editor->SetLexer(Lexer::Lightning);

  AttachDocumentEditor(document->mName, editor);
  // After the document is attached we need to re-layout the editor so that the
  // document's size is correctly updated before any further operations. This
  // was specifically added for text editors as the scintilla will get the wrong
  // size otherwise.
  this->UpdateTransform();

  return editor;
}

DocumentEditor* EditorMain::OpenTextFile(StringParam filename)
{
  String smallFileName = FilePath::GetFileName(filename);
  String name = BuildString("File: ", smallFileName);
  Widget* widget = mManager->ShowWidget(name);
  if (widget)
  {
    return Type::DynamicCast<DocumentEditor*>(widget);
  }
  else
  {
    if (FileExists(filename))
    {
      FileDocument* document = new FileDocument(smallFileName, filename);
      DocumentEditor* editor = CreateDocumentEditor(this, document);

      String extension = FilePath::GetExtension(filename);
      TypeExtensionEntry* lightningEntry = FileExtensionManager::GetLightningScriptTypeEntry();

      if (lightningEntry->IsValidExtensionNoDot(extension))
        editor->SetLexer(Lexer::Lightning);

      AttachDocumentEditor(name, editor);
      return editor;
    }
  }
  return nullptr;
}

DocumentEditor* EditorMain::OpenDocumentResource(DocumentResource* docResource)
{
  // Is the window already open?
  Widget* widget = mManager->ShowWidgetWith(docResource);

  if (widget)
  {
    return Type::DynamicCast<DocumentEditor*>(widget);
  }
  else
  {
    DocumentManager* docManager = DocumentManager::GetInstance();
    ResourceDocument* document =
        (ResourceDocument*)docManager->Documents.FindValue((u64)docResource->mResourceId, nullptr);
    if (document == nullptr)
      document = new ResourceDocument(docResource);

    Window* mainWindow = this->GetCenterWindow();

    DocumentEditor* editor = nullptr;

    String format = docResource->GetFormat();

    if (format == "Text")
      editor = CreateDocumentEditor(this, document);
    else
      editor = CreateScriptEditor(this, document);

    AttachDocumentEditor(document->GetDisplayName(), editor);

    return editor;
  }
}

DocumentEditor* EditorMain::OpenTextFileAuto(StringParam file)
{
  // Attempt to get the resource via loaded file lookup
  ResourceId resourceId = PL::gResources->TextResources.FindValue(file, 0);
  DocumentResource* resource = (DocumentResource*)PL::gResources->GetResource(resourceId);

  if (resource)
    return OpenDocumentResource(resource);

  // Generic file
  return OpenTextFile(file);
}

void EditorMain::OnScriptError(ScriptEvent* event)
{
  // At the moment we always pause due to a syntax error or exception
  // If we are live editing, we really want to continue (live edit may need to
  // be a mode)
  SetGamePaused(true);

  if (event->Script)
  {
    // debug exception needs the full file path, so set the filename now to the
    // full path
    event->Location.Origin = event->Script->LoadPath;
    DocumentEditor* editor = OpenDocumentResource(event->Script);
    if (!editor)
      return;

    editor->ScriptError(event);
  }
  // If there was no valid script to display an error message on then just
  // do-notify the warning message.
  else
  {
    DoNotifyWarning("Script Error", event->Message);
  }
}

void EditorMain::OnDebuggerPaused(ScriptEvent* event)
{
  // JC or TS commented that we should remove these lines in a code review,
  // but the reason was unclear. Investigate this!
  forRange (DocumentEditor* otherEditor, DocumentManager::GetInstance()->Instances)
    otherEditor->ClearMarker(-1, TextEditor::InstructionMarker);

  if (event->Script == nullptr)
    return;

  DocumentEditor* editor = OpenDocumentResource(event->Script);
  if (!editor)
    return;

  // CodeLocations use 1 based indices
  editor->SetMarker(event->Location.StartLine - 1, TextEditor::InstructionMarker);
  editor->GoToLine(event->Location.StartLine - 1);
}

void EditorMain::OnDebuggerResumed(ScriptEvent* event)
{
  forRange (DocumentEditor* otherEditor, DocumentManager::GetInstance()->Instances)
    otherEditor->ClearMarker(-1, TextEditor::InstructionMarker);
}

void EditorMain::OnBlockingTaskStart(BlockingTaskEvent* event)
{
  mDisableInput = true;
  mLoading->Activate(event->mTaskName);
}

void EditorMain::OnBlockingTaskFinish(Event* event)
{
  mDisableInput = false;
  mLoading->Deactivate();
}

void EditorMain::OnNotifyEvent(NotifyEvent* event)
{
  if (event->Type == NotifyType::Error)
    Editor::ShowConsole();

  DoNotifyPopup(this->GetRootWidget()->GetPopUp(), event);
}

void EditorMain::StressTest(CommandEvent* event)
{
  Widget* widget = ShowWindow("Stress Test");
  if (widget != nullptr)
  {
    StressTestDialog* dialog = Type::DynamicCast<StressTestDialog*>(widget);
    if (dialog != nullptr)
      dialog->Refresh();
  }
}

void EditorMain::OnMainClick(MouseEvent* event)
{
  Composite* searchWindow = PL::gEditor->OpenSearchWindow(nullptr, true);
  searchWindow->SetTranslation(Vec3(0, 0, 0));
}

void OnExportTypeList(Editor* editor)
{
  MetaDatabase* instance = MetaDatabase::GetInstance();
  // Append all libraries into one list to make searching easier
  Lightning::LibraryArray allLibraries;
  allLibraries.Append(instance->mLibraries.All());
  allLibraries.Append(instance->mNativeLibraries.All());

  Array<BoundType*> baseTypesToFind;
  baseTypesToFind.PushBack(LightningTypeId(Collider));
  baseTypesToFind.PushBack(LightningTypeId(Joint));
  baseTypesToFind.PushBack(LightningTypeId(PhysicsEffect));
  baseTypesToFind.PushBack(LightningTypeId(Graphical));
  baseTypesToFind.PushBack(LightningTypeId(ParticleAnimator));
  baseTypesToFind.PushBack(LightningTypeId(ParticleEmitter));
  baseTypesToFind.PushBack(LightningTypeId(Component));
  baseTypesToFind.PushBack(LightningTypeId(Resource));
  baseTypesToFind.PushBack(LightningTypeId(Event));
  baseTypesToFind.PushBack(LightningTypeId(Tool));
  baseTypesToFind.PushBack(LightningTypeId(Enum));
  baseTypesToFind.PushBack(LightningTypeId(MetaComposition));
  baseTypesToFind.PushBack(LightningTypeId(Widget));
  baseTypesToFind.PushBack(LightningTypeId(ContentComponent));

  typedef OrderedHashMap<BoundType*, HashSet<String>> MapType;
  // Build a map of base type we're searching for to a set of all names that
  // derive from that type
  MapType namesPerBaseType;
  for (size_t i = 0; i < baseTypesToFind.Size(); ++i)
  {
    namesPerBaseType[baseTypesToFind[i]];
  }
  // Add one extra set for the null type (one we didn't search for)
  namesPerBaseType[nullptr] = HashSet<String>();

  // Check all libraries
  for (size_t libraryIndex = 0; libraryIndex < allLibraries.Size(); ++libraryIndex)
  {
    // Check all types in that library
    LibraryRef& library = allLibraries[libraryIndex];
    forRange (BoundType* boundType, library->BoundTypes.Values())
    {
      BoundType* foundBaseType = nullptr;
      // Check all of our potential base types. If we don't find one then resort
      // to null (backup)
      for (size_t typeIndex = 0; typeIndex < baseTypesToFind.Size(); ++typeIndex)
      {
        BoundType* baseType = baseTypesToFind[typeIndex];
        // If this isn't the current base type skip this
        if (!boundType->IsA(baseType))
          continue;

        foundBaseType = baseType;
        break;
      }

      namesPerBaseType[foundBaseType].Insert(boundType->Name);
    }
  }

  // Output all of the types into one file
  StringBuilder builder;
  forRange (MapType::PairType& pair, namesPerBaseType.All())
  {
    // Sort by name all of the types
    Array<String> sortedNames;
    sortedNames.Append(pair.second.All());
    Sort(sortedNames.All());

    // Figure out the category name. If the type is null then we
    // didn't categorize this type so mark it as unknown
    String name = "Unknown";
    if (pair.first != nullptr)
      name = pair.first->Name;

    // Write the category name
    builder.AppendFormat("%s:\n", name.c_str());
    // Write out the type name in phabricator's check-box format
    for (size_t i = 0; i < sortedNames.Size(); ++i)
      builder.AppendFormat("  [ ] %s\n", sortedNames[i].c_str());
    builder.Append("\n");
  }

  // Write out the results to the project directory
  ProjectSettings* projectSettings = editor->mProject->has(ProjectSettings);
  String outDir = projectSettings->ProjectFolder;

  String outFilePath = FilePath::Combine(outDir, "TypeList.txt");
  WriteStringRangeToFile(outFilePath, builder.ToString());
}

void OnExportCommandsList(Editor* editor)
{
  StringBuilder builder;
  CommandManager* instance = CommandManager::GetInstance();

  // Sort all commands by name
  Array<String> sortedCommandNames;
  forRange (Command* command, instance->mCommands)
    sortedCommandNames.PushBack(command->Name);
  Sort(sortedCommandNames.All());

  // Print out the commands
  builder.AppendFormat("Commands:\n");
  for (size_t i = 0; i < sortedCommandNames.Size(); ++i)
    builder.AppendFormat("  [ ] %s\n", sortedCommandNames[i].c_str());

  // Write out the results to the project directory
  ProjectSettings* projectSettings = editor->mProject->has(ProjectSettings);
  String outDir = projectSettings->ProjectFolder;

  String outFilePath = FilePath::Combine(outDir, "CommandsList.txt");
  WriteStringRangeToFile(outFilePath, builder.ToString());
}

void OnResaveAllResources(Editor* editor)
{
  forRange (ContentLibrary* library, PL::gContentSystem->Libraries.Values())
  {
    library->SaveAllContentItemMeta();
    library->Save();

    forRange (ContentItem* contentItem, library->GetContentItems())
    {
      contentItem->SaveContent();
    }
  }
}

void EditorRescueCall(void* userData)
{
  // Get the error context printed
  // DoNotifyErrorWithContext("Crashing");

  // Make sure the editor is valid
  if (PL::gEditor != nullptr)
  {
    // Get the target space from the editor
    Space* space = PL::gEditor->GetEditSpace();

    //// Make sure the space is valid
    if (space != nullptr && space->GetModified())
    {
      if (Level* level = space->mLevelLoaded)
      {
        ContentLibrary* library = PL::gEditor->mProjectLibrary;
        if (library)
        {
          String path = PL::gContentSystem->GetHistoryPath(library);
          // Build a string for the crashed level that places it in the library.
          String fileName = BuildString("Recovered", GetTimeAndDateStamp(), ".data");
          String backupFile = FilePath::Combine(path, fileName);
          // Attempt to save the space
          space->SaveLevelFile(backupFile);
        }
      }
    }
  }
}

void OnTweakablesModified()
{
  PL::gEditor->MarkAsNeedsUpdate();

  if (PL::gEditor->GetEditSpace())
    PL::gEditor->GetEditSpace()->MarkModified();
}

void AutoVersionCheck();
void SetupTools(Editor* editor);

#define BindCommand(commandName, memberFunction)                                                                       \
  Connect(commands->GetCommand(commandName), Events::CommandExecute, editorMain, &EditorMain::memberFunction);

void CreateEditor2(OsWindow* mainWindow, StringParam projectFile, StringParam newProjectName)
{
    ZoneScoped;


}

void CreateEditor(OsWindow* mainWindow, StringParam projectFile, StringParam newProjectName)
{
  ZoneScoped;
  ProfileScopeFunction();

  // Set the tweakables modified callback so that we can update the Ui
  Tweakables::sModifiedCallback = &OnTweakablesModified;

  CrashHandler::SetupRescueCallback(EditorRescueCall, nullptr);

  MainWindow* rootWidget = new MainWindow(mainWindow);
  EditorMain* editorMain = new EditorMain(rootWidget, mainWindow);
  Cog* config = PL::gEngine->GetConfigCog();
  MainConfig* mainConfig = config->has(MainConfig);
  editorMain->mConfig = config;
  editorMain->mOsWindow = mainWindow;
  editorMain->mMainWindow = rootWidget;

  String dataDirectory = mainConfig->DataDirectory;
  CommandManager* commands = CommandManager::GetInstance();
  editorMain->mCommands = commands;
  commands->LoadCommands(FilePath::Combine(dataDirectory, "Commands.data"));
  commands->LoadMenu(FilePath::Combine(dataDirectory, "Menus.data"));
  commands->LoadMenu(FilePath::Combine(dataDirectory, "Toolbars.data"));

  SetupTools(editorMain);

  commands->GetContext()->Add(editorMain, LightningTypeId(Editor));
  rootWidget->LoadMenu("Main");

  Connect(PL::gEngine, Events::Notify, editorMain, &EditorMain::OnNotifyEvent);

  BindEditorCommands(config, commands);
  BindAppCommands(config, commands);
  BindCodeTranslatorCommands(config, commands);
  SetupGraphCommands(config, commands);
  BindArchiveCommands(config, commands);
  BindGraphicsCommands(config, commands);
  BindCreationCommands(config, commands);
  BindDocumentationCommands(config, commands);
  BindProjectCommands(config, commands);
  BindContentCommands(config, commands);

  // Listen to the resource system if any unhandled exception or syntax error
  // occurs
  Connect(PL::gResources, Events::UnhandledException, editorMain, &EditorMain::OnScriptError);
  Connect(PL::gResources, Events::SyntaxError, editorMain, &EditorMain::OnScriptError);
  Connect(PL::gResources, Events::DebuggerPaused, editorMain, &EditorMain::OnDebuggerPaused);
  Connect(PL::gResources, Events::DebuggerResumed, editorMain, &EditorMain::OnDebuggerResumed);

  // For setting the default docked windows' width to a percentage
  // to make a better initial layout on smaller resolutions
  Vec2 rootSize = editorMain->GetRootWidget()->GetSize();
  float dockWidth = Math::Min(rootSize.x * 0.15f, 280.0f);

  MultiManager* manager = new MultiManager(rootWidget, editorMain);
  editorMain->mManager = manager;

  Connect(manager, Events::OsKeyDown, editorMain, &EditorMain::OnKeyDown);
  Connect(manager, Events::Cut, editorMain, &EditorMain::OnCutCopyPaste);
  Connect(manager, Events::Copy, editorMain, &EditorMain::OnCutCopyPaste);
  Connect(manager, Events::Paste, editorMain, &EditorMain::OnCutCopyPaste);

  {
    // Create a persistent Library instance so that the rest of the engine can
    // manipulate it regardless of how many instances there are existing (See
    // ContentPackageImporter)
    editorMain->mLibrary = editorMain->CreateLibraryView(true, false);
    editorMain->mLibrary->SetHideOnClose(true);
    editorMain->AddManagedWidget(editorMain->mLibrary, DockArea::Right, true);
  }

  {
    ObjectView* objects = new ObjectView(editorMain);
    objects->SetName("Objects");
    objects->SetHideOnClose(true);
    objects->SetSize(Pixels(dockWidth, 280));
    editorMain->AddManagedWidget(objects, DockArea::Right, true);
    editorMain->mObjectView = objects;
  }

  {
    HotKeyEditor* hotkeyEditor = new HotKeyEditor(editorMain);
    hotkeyEditor->SetName("Commands");
    hotkeyEditor->SetHideOnClose(true);
    hotkeyEditor->SetSize(Pixels(850, 500));
    editorMain->AddManagedWidget(hotkeyEditor, DockArea::Floating, false);
  }

  {
    BindCommand("Commands", ShowHotKeyEditor);
    BindCommand("OperationHistory", ShowOperationHistroy);
    BindCommand("Animator", ShowAnimator);
    BindCommand("FindNext", ShowFindNext);
    BindCommand("FindAll", ShowFindAll);
    BindCommand("ReplaceNext", ShowReplaceNext);
    BindCommand("ReplaceAll", ShowReplaceAll);
    BindCommand("ReportBug", ShowBugReporter);
    BindCommand("Tools", ShowTools);
    BindCommand("Properties", ShowProperties);
    BindCommand("SelectEditorConfig", ShowConfig);
    BindCommand("SelectProject", ShowProject);
    BindCommand("Library", ShowLibrary);
    BindCommand("Console", ToggleConsole);
    BindCommand("ShowConsole", ShowConsole);
    BindCommand("HideConsole", HideConsole);
    BindCommand("Market", ShowMarket);
    BindCommand("Chat", ShowChat);
    BindCommand("Objects", ShowObjects);
    BindCommand("BroadPhaseTracker", ShowBroadPhaseTracker);
    BindCommand("VolumeMeter", ShowVolumeMeter);
    BindCommand("SoundNodeGraph", ShowSoundNodeGraph);
    BindCommand("RenderGroupHierarchies", ShowRenderGroupHierarchies);
    BindCommand("EditColorScheme", EditColorScheme);
    BindCommand("ClearConsole", ClearConsole);
    BindCommand("ShowCoreLibrary", ShowCoreLibrary);

    Connect(PL::gEngine, Events::BlockingTaskStart, editorMain, &EditorMain::OnBlockingTaskStart);
    Connect(PL::gEngine, Events::BlockingTaskFinish, editorMain, &EditorMain::OnBlockingTaskFinish);

    BindCommand("StressTest", StressTest);
    // Add a command to write out all bound types in the engine
    DeveloperConfig* devConfig = config->has(DeveloperConfig);
    if (devConfig != nullptr)
    {
      commands->AddCommand("ExportTypeList", BindCommandFunction(OnExportTypeList));
      commands->AddCommand("ResaveAllResources", BindCommandFunction(OnResaveAllResources));
      commands->AddCommand("OnExportCommandsList", BindCommandFunction(OnExportCommandsList));
    }

    {
      // All tools exist under this toolbar area
      ToolBarArea* toolBarArea = new ToolBarArea(editorMain);
      toolBarArea->SetTranslation(Vec3(0, Pixels(-40), 0));
      toolBarArea->SetSize(Pixels(1, 38));
      toolBarArea->SetDockArea(DockArea::TopTool);

      Spacer* spacer = new Spacer(toolBarArea);
      spacer->SetSize(Pixels(3, 0));
      spacer->SetDockMode(DockMode::DockLeft);

      rootWidget->mMainMenu->SetActive(true);
      Connect(rootWidget->mMainMenu, Events::LeftClick, editorMain, &EditorMain::OnMainClick);

      // Save, copy/cut/paste, undo/redo
      ToolBar* primaryActions = new ToolBar(toolBarArea);
      primaryActions->LoadMenu("PrimaryToolbar");
      primaryActions->SetDockMode(DockMode::DockLeft);

      // Primary Tools
      ToolBar* primaryTools = new ToolBar(toolBarArea);
      primaryTools->LoadMenu("PrimaryToolsToolbar");
      primaryTools->SetDockMode(DockMode::DockLeft);

      // Edit Tools
      ToolBar* editTools = new ToolBar(toolBarArea);
      editTools->LoadMenu("EditToolsToolbar");
      editTools->SetDockMode(DockMode::DockLeft);

      ToolBar* addBar = new ToolBar(toolBarArea);
      addBar->LoadMenu("ResourceToolbar");
      addBar->SetDockMode(DockMode::DockLeft);

      // Extra windows
      ToolBar* windowsGroup = new ToolBar(toolBarArea);
      editTools->LoadMenu("WindowsToolbar");
      windowsGroup->SetDockMode(DockMode::DockLeft);

      ToolBar* gameToolbar = new ToolBar(toolBarArea);
      gameToolbar->SetTranslation(Pixels(825, 0, 0));
      gameToolbar->SetDockMode(DockMode::Enum(DockMode::DockLeft | DockMode::DockRight));
      gameToolbar->LoadMenu("GameToolbar");

      ToolBar* helpToolbar = new ToolBar(toolBarArea);
      helpToolbar->SetTranslation(Pixels(1856, 0, 0));
      helpToolbar->SetDockMode(DockMode::DockRight);
      new BackgroundTaskButton(helpToolbar);
      helpToolbar->LoadMenu("HelpToolbar");

      ToolBar* debuggerToolbar = new ToolBar(toolBarArea);
      debuggerToolbar->SetDockMode(DockMode::DockRight);
      debuggerToolbar->LoadMenu("DebuggerToolbar");
    }
  }

  MetaSelection* selection = editorMain->GetSelection();

  // Tool Area
  editorMain->Tools->SetSize(Pixels(dockWidth, 280));
  Window* sideBar = editorMain->AddManagedWidget(editorMain->Tools, DockArea::Left, true);

  MainPropertyView* propertyViewArea = new MainPropertyView(editorMain, selection, editorMain->mQueue);

  propertyViewArea->SetHideOnClose(true);
  editorMain->mMainPropertyView = propertyViewArea;

  sideBar->AttachAsTab(propertyViewArea, false);

  editorMain->Tools->SelectToolIndex(0);

  Connect(propertyViewArea->GetPropertyView(), Events::NameActivated, editorMain, &EditorMain::OnNameActivated);

  {
    // Create the console window
    ConsoleUi* console = new ConsoleUi(editorMain);
    console->SetName("Console");
    console->SetHideOnClose(true);
    console->SetSize(Pixels(800, 220));

    editorMain->AddManagedWidget(console, DockArea::Bottom, false);
    editorMain->mConsole = console;
  }

  {
    // Create the find text dialog
    FindTextDialog* findText = new FindTextDialog(editorMain);
    findText->SetName("Find/Replace Text");
    findText->SetHideOnClose(true);
    editorMain->AddManagedWidget(findText, DockArea::Floating, false);
    editorMain->mFindTextDialog = findText;
  }

  {
    // Create the bug report dialog
    BugReporter* dialog = new BugReporter(editorMain);
    dialog->SetName("Bug Reporter");
    dialog->SetHideOnClose(true);
    editorMain->AddManagedWidget(dialog, DockArea::Floating, false);
    editorMain->mBugReporter = dialog;
  }

  {
    // Create the stress test dialog
    StressTestDialog* stress = new StressTestDialog(editorMain);
    stress->SetName("Stress Test");
    stress->SetHideOnClose(true);
    editorMain->AddManagedWidget(stress, DockArea::Floating, false);
    editorMain->mStressTestDialog = stress;
  }

  {
    // Create the Broad Phase Editor window
    BroadPhaseEditor* bpEditor = new BroadPhaseEditor(editorMain);
    bpEditor->SetName("BroadPhaseTracker");
    bpEditor->SetHideOnClose(true);
    bpEditor->SetSize(Pixels(320, 360));
    editorMain->AddManagedWidget(bpEditor, DockArea::Floating, false);
  }

  {
    // Create the desync window
    Window* desyncWindow = new Window(editorMain);
    desyncWindow->SetTitle("Desync");
    desyncWindow->SetTranslationAndSize(Pixels(0, 0, 0), Pixels(800, 800));
    desyncWindow->SetHideOnClose(true);
    desyncWindow->SetActive(false);
    editorMain->mDesyncWindow = desyncWindow;
  }

  if (DeveloperConfig* devConfig = config->has(DeveloperConfig))
  {
    // editorMain->Tools->AddTool(new ClothAnchorTool());
    // editorMain->Tools->AddTool(new ClothCutterTool());
  }

  {
    AnimationEditor* animator = new AnimationEditor(editorMain);
    animator->SetPropertyView(editorMain->mMainPropertyView);
    animator->SetName("Animator");
    animator->SetHideOnClose(true);
    animator->SetSize(Pixels(500, 95));
    animator->SetMinSize(Pixels(300, 200));
    editorMain->AddManagedWidget(animator, DockArea::Bottom, false);
  }

  // This sets the size of the editor to the correct full size of the screen
  rootWidget->Refresh();

  // Moves everything off the screen
  editorMain->SetExploded(true, false);

  PlasmaPrint("Welcome to the Plasma Editor.\n");

  editorMain->mLoading = new LoadingWindow(rootWidget);
  editorMain->mLoading->SetActive(false);

  // Compile once before trying to load a project so that the engine can render
  LightningManager::GetInstance()->TriggerCompileExternally();

  // If we have a file to be loaded
  if (!projectFile.Empty())
  {
    // Project extension
    String extension = FilePath::GetExtension(projectFile);

    // If the file passed in is a project file...
    if (extension == "plasmaproj")
    {
      // Open the project
      OpenProjectFile(projectFile);
    }
    else
    {
      Event event;
      PL::gEngine->DispatchEvent(Events::NoProjectLoaded, &event);
      DoNotifyError("Unknown file type", "Unknown file type must be a valid plasma project");
    }
  }
  else
  {
    bool projectSuccessfullyLoaded = false;
    // Open cached project in user config
    String startingProject = HasOrAdd<EditorConfig>(PL::gEditor->mConfig)->EditingProject;
    // if the user has requested to create a new project then don't open the
    // last edited project
    if (newProjectName.Empty() && FileExists(startingProject))
      projectSuccessfullyLoaded = OpenProjectFile(startingProject);

    // If loading failed for some reason (either it didn't exist, the project
    // was corrupted, etc...) then send out the failure event so we stop
    // blocking and shell out to the launcher.
    if (!projectSuccessfullyLoaded)
    {
      ProjectDialog* dialog = OpenNewProjectDialog(editorMain);
      //If the name of the new project is true then there was a command line argument to
      //make a new project but no name was specified (hence the string we got was true).
      //In this case we should bring up the new project dialog but not override the name
      //of the project. This does however mean that no one can specify via command line a
      //project whos name is "true", but I don't care...
      if (newProjectName != "true")
      {
          dialog->mNameBox->SetText(newProjectName);
          dialog->mNameBox->TakeFocus();
      }

      editorMain->mProjectLibrary = PL::gContentSystem->Libraries["Scratch"];
      DoNotify("No Project", "Open a project or create a new project", "Disk");
    }
  }

  HasOrAdd<TextEditorConfig>(PL::gEditor->mConfig);

  editorMain->ShowWindow("Tools");

  // If the debugger is attached add a simple listener component to listen on
  // a tcp socket for the launcher telling us to open a project file
  if (Os::IsDebuggerAttached())
  {
    PL::gEditor->mSimpleDebuggerListener = new SimpleDebuggerListener();
  }

  CommandManager::GetInstance()->ValidateCommands();

  // Copy commands to the HotKeyCommands DataSource.
  HotKeyCommands::GetInstance()->CopyCommandData(commands->mCommands);
}
} // namespace Plasma
