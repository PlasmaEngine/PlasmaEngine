// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{  
// When importing Plasma Engine resource files we strip the resource extension
// from the filename to get the original resource name from the project it was
// imported from if we do not do this then the file
// PlasmaEngineResource.ResourceType.data becomes
// PlasmaEngineResourceResourceType.ResourceType.data instead of
// PlasmaEngineResource.ResourceType.data
String StripResourceExtension(StringParam filename)
{
  // Count the periods in the filename
  int periodCount = 0;
  forRange (Rune rune, filename.All())
  {
    if (rune == '.')
      ++periodCount;
  }
  // If it contains more than 2 periods this is not one of Plasma Engine's data
  // files as at most a file would be named PlasmaEngineResource.ResourceType.data
  // so anymore than 2 is a way of identifying a user named external file
  // and not accidentally turning a user file like User.Custom.Font.ttf
  // into UserCustom as a font instead of UserCustomFont
  if (periodCount > 2)
    return filename;

  // Find the resource extensions begin and end position in the string
  StringRange resourceExtensionStart = filename.FindFirstOf('.');
  StringRange resourceExtensionEnd = filename.FindLastOf('.');
  if (resourceExtensionStart.Begin() != resourceExtensionEnd.Begin())
  {
    // Get the resource extension without including the beginning and end
    // periods
    String resourceExtension = filename.SubString(resourceExtensionStart.End(), resourceExtensionEnd.Begin());
    // Check if the included middle extension is a Plasma Engine resource
    MetaDatabase* metaDatabase = MetaDatabase::GetInstance();
    BoundType* type = metaDatabase->FindType(resourceExtension);
    // If it is a Plasma Engine resource strip it from the filename
    if (type->IsA(LightningTypeId(Resource)))
    {
      // Replace the .ResourceType with nothing and return that filename to
      // import Plasma Engine created resources files from another project and get
      // the same name
      return filename.Replace(BuildString(".", resourceExtension), "");
    }
  }
  // This file is not a Plasma Engine resource so return as is
  return filename;
}

void RunGroupImport(ImportOptions& options)
{
  if (PL::gEngine->IsReadOnly())
  {
    DoNotifyWarning("Content", "Cannot add content items in read-only mode");
    return;
  }

  ContentLibrary* library = options.mLibrary;

  Array<ContentItem*> contentToBuild;

  Array<String> filesToExport(options.mFiles);
  if (options.mConflictOptions && options.mConflictOptions->GetAction() == ConflictAction::Replace)
    filesToExport.Append(options.mConflictedFiles.All());

  // For every file imported
  for (uint fileIndex = 0; fileIndex < filesToExport.Size(); ++fileIndex)
  {
    String fullPath = filesToExport[fileIndex];
    String filename = StripResourceExtension(FilePath::GetFileName(fullPath));
    filename = SanitizeContentFilename(filename);
    String storedfilename = FilePath::Combine(library->SourcePath, filename);

    // Add the content item
    AddContentItemInfo addContent;
    addContent.FileName = filename;
    addContent.Library = library;
    addContent.ExternalFile = fullPath;
    addContent.OnContentFileConflict = ContentFileConflict::Replace;
    addContent.Options = &options;

    Status addItem;

    ContentItem* newContentItem = PL::gContentSystem->AddContentItemToLibrary(addItem, addContent);

    // If the content add succeeded
    if (addItem.Succeeded())
      contentToBuild.PushBack(newContentItem);
    else
      DoNotifyError("Failed Import", addItem.Message);
  }

  // Now that all the content has been added. Build and load them for use.

  // Build all new content items
  ResourceLibrary* resourceLibrary = PL::gResources->GetResourceLibrary(library->Name); 

  ImportJobProperties jobProperties;
  jobProperties.mLibrary = library;
  jobProperties.mOptions = &options;
  jobProperties.mResourceLibrary = resourceLibrary;
  jobProperties.mContentToBuild = contentToBuild;
  
  ImportJob* job = new ImportJob(jobProperties);
  BackgroundTask* task = PL::gBackgroundTasks->CreateTask(job);
  task->mName = "Import Asset";
  PL::gJobs->AddJob(job);
}

void GroupImport()
{
  ImportCallback* importCallback = new ImportCallback();
  importCallback->Open();
}

void OpenGroupImport(Array<String>& files)
{
  ContentLibrary* library = PL::gEditor->mLibrary->GetLibrary();
  ImportOptions* options = new ImportOptions();
  options->Initialize(files, library);

  if (options->ShouldAutoImport())
  {
    RunGroupImport(*options);
  }
  else
  {
    // Create the import options window
    Editor* editor = PL::gEditor;
    Window* window = new Window(PL::gEditor);
    window->SetTitle("Group Import Options");

    GroupImportWindow* import = new GroupImportWindow(window, options);

    window->SetSize(Pixels(540, 500.0f));
    window->SetTranslation(Pixels(10, 10, 0));

    CenterToWindow(PL::gEditor, window, false);
  }
}

void LoadDroppedFiles(Array<HandleOfString>& files)
{
  if (files.Empty())
    return;

  {
    // Check for project file load and load if true
    String fileName = files[0];
    String extension = FilePath::GetExtension(fileName);
    if (extension == "plasmaproj")
    {
      OpenProjectFile(fileName);
      return;
    }
    else if (extension == "data")
    {
      PL::gEditor->OpenTextFile(fileName);
      return;
    }
    else if (extension == "plasmapack")
    {
      ContentImporter::OpenImportWindow(fileName);
      return;
    }
  }

  // Attempt to add files as resources to the project's library
  ContentLibrary* library = PL::gEditor->mLibrary->GetLibrary();

  if (library == NULL)
  {
    DoNotifyError("Failed", "No valid content library to add content");
    return;
  }

  // The user could've dragged a file in. Recursively find all files in the
  // given path. Might need to be updated later to deal with multiple files of
  // the same name in different directories...
  Array<String> allFiles;
  for (size_t i = 0; i < files.Size(); ++i)
    FindFilesRecursively(files[i], allFiles);

  OpenGroupImport(allFiles);
}

// GroupImportWindow
GroupImportWindow::GroupImportWindow(Composite* parent, ImportOptions* options) : Composite(parent)
{
  mOptions = options;

  mParentWindow = parent;
  this->SetLayout(CreateStackLayout());

  Composite* top = new Composite(this);
  top->SetLayout(CreateRowLayout());
  top->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  {
    {
      Composite* left = new Composite(top);
      left->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
      left->SetLayout(CreateStackLayout());
      mPropertyView = new PropertyView(left);
      mPropertyView->SetObject(mOptions);
      mPropertyView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
      mPropertyView->Rebuild();
    }

    {
      Composite* right = new Composite(top);
      right->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
      right->SetLayout(CreateStackLayout());

      Label* label = new Label(right);
      label->SetText("Files to import...");

      mListBox = new ListBox(right);
      mListBox->SetDataSource(&mStrings);
      mListBox->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
    }
  }

  Composite* buttonRow = new Composite(this);
  buttonRow->SetLayout(CreateRowLayout());
  {
    mImportButton = new TextButton(buttonRow);
    mImportButton->SetText("Import All");
    mCancelButton = new TextButton(buttonRow);
    mCancelButton->SetText("Cancel All");
  }

  ConnectThisTo(mImportButton, Events::ButtonPressed, OnPressed);
  ConnectThisTo(mCancelButton, Events::ButtonPressed, OnCancel);
  ConnectThisTo(mOptions, Events::ImportOptionsModified, OnOptionsModified);
  UpdateListBoxSource();
}

float GroupImportWindow::GetPropertyGridHeight()
{
  uint propertyCount = 0;

  // METAREFACTOR - Confirm AllProperties.Size() will have the same results as
  // the old Properties array Adding 1 to each for the
  if (mOptions->mImageOptions)
    propertyCount += LightningVirtualTypeId(mOptions->mImageOptions)->AllProperties.Size() + 1;
  if (mOptions->mGeometryOptions)
    propertyCount += LightningVirtualTypeId(mOptions->mGeometryOptions)->AllProperties.Size() + 1;
  if (mOptions->mAudioOptions)
    propertyCount += LightningVirtualTypeId(mOptions->mAudioOptions)->AllProperties.Size() + 1;
  if (mOptions->mConflictOptions)
    propertyCount += LightningVirtualTypeId(mOptions->mConflictOptions)->AllProperties.Size() + 1;

  return (float)propertyCount * 20.0f;
}

void GroupImportWindow::OnOptionsModified(Event* event)
{
  UpdateListBoxSource();
  return;
  mOptions->BuildOptions();

  float time = 0.2f;

  float height = GetPropertyGridHeight();
  Vec2 windowSize = Pixels(540, height + 60.0f);

  Vec2 propertyGridSize = Pixels(200, GetPropertyGridHeight() + 9.0f);

  AnimateToSize(mPropertyView, propertyGridSize, time);
  AnimateToSize(mParentWindow, windowSize, time);

  if (mOptions->mConflictOptions->mAction == ConflictAction::Replace)
  {
    ActionSequence* sequence = new ActionSequence(this);
    sequence->Add(new ActionDelay(time));
    sequence->Add(new CallAction<GroupImportWindow, &GroupImportWindow::RebuildTree>(this));
  }
  else
  {
    RebuildTree();
  }
}

void GroupImportWindow::RebuildTree()
{
  mPropertyView->Invalidate();
}

void GroupImportWindow::UpdateListBoxSource()
{
  mStrings.Strings.Clear();
  for (uint i = 0; i < mOptions->mFiles.Size(); ++i)
  {
    String file = FilePath::GetFileName(mOptions->mFiles[i]);
    mStrings.Strings.PushBack(String::Format("(Add) %s", file.c_str()));
  }

  for (uint i = 0; i < mOptions->mConflictedFiles.Size(); ++i)
  {
    String file = FilePath::GetFileName(mOptions->mConflictedFiles[i]);
    mStrings.Strings.PushBack(String::Format("(Update) %s", file.c_str()));
    mListBox->HighlightItem(mOptions->mFiles.Size() + i);
  }
}

void GroupImportWindow::OnPressed(Event* event)
{
  RunGroupImport(*mOptions);
  float time = 0.5f;
  AnimateTo(mParentWindow, Pixels(2000.0f, 200.0f, 0), mParentWindow->GetSize() * 0.5f, time);
  
  ActionSequence* sequence = new ActionSequence(mParentWindow);
  sequence->Add(new ActionDelay(time));
  sequence->Add(DestroyAction(mParentWindow));
}

void GroupImportWindow::OnCancel(Event* event)
{
  mParentWindow->Destroy();
}

// ImportCallback
void ImportCallback::Open()
{
  // Open the open file dialog
  FileDialogConfig* config = FileDialogConfig::Create();
  config->EventName = "OnFileSelected";
  config->CallbackObject = this;
  config->Title = "Select resource";
  config->AddFilter("ResourceFile", "*.*");
  config->Flags |= FileDialogFlags::MultiSelect;

  ConnectThisTo(this, config->EventName, OnFilesSelected);
  PL::gEngine->has(OsShell)->OpenFile(config);
}

void ImportCallback::OnFilesSelected(OsFileSelection* fileSelection)
{
  OpenGroupImport(fileSelection->Files);
  delete this;
}

 LightningDefineType(ImportJobProperties, builder, type)
{
  LightningBindFieldProperty(mLibrary);
  LightningBindFieldProperty(mResourceLibrary);
  LightningBindFieldProperty(mOptions);
  PlasmaBindExpanded();
}
  
ImportJobProperties::ImportJobProperties()
{
}

ImportJob::ImportJob(ImportJobProperties jobProperties) : mJobProperties(jobProperties)
{
  ConnectThisTo(PL::gBackgroundTasks, Events::PostImport, OnImportFinished);
}

void ImportJob::Execute()
{
    

  Status status;
  HandleOf<ResourcePackage> packageHandle = BuildContentItems(status, mJobProperties.mContentToBuild, mJobProperties.mLibrary);
  
  ResourcePackage* package = packageHandle;
  DoNotifyStatus(status);

  UpdateTaskProgress(1.0, "Finished Importing");
  
  Event* e = new PostImportEvent (mJobProperties.mResourceLibrary, package, mJobProperties.mContentToBuild, status, mJobProperties.mOptions);
  PL::gDispatch->Dispatch(PL::gBackgroundTasks, Events::PostImport, e);
}

int ImportJob::Cancel()
{
  return 0;
}

void ImportJob::UpdateTaskProgress(float percentComplete, StringParam progressText)
{
  UpdateProgress("Import Asset", percentComplete, progressText);
}

 struct SortByLoadOrder
{
  bool operator()(ResourceEntry& left, ResourceEntry& right)
  {
    // First sort by load order, then name for determinism.
    return left.LoadOrder < right.LoadOrder || (left.LoadOrder == right.LoadOrder && left.Name < right.Name);
  }
};
  
HandleOf<ResourcePackage> ImportJob::BuildContentItems(Status& status, ContentItemArray& toBuild, ContentLibrary* library)
{
  ZoneScoped;
  ProfileScopeFunctionArgs(library->Name);

  ResourcePackage* package = new ResourcePackage();
  package->Name = library->Name;

  package->Location = library->GetOutputPath();
  CreateDirectoryAndParents(package->Location);

  BuildOptions buildOptions(library);

  bool allBuilt = true;

  for (uint i = 0; i < toBuild.Size(); ++i)
  {
    // Process from this contentItem down.
    ContentItem* contentItem = toBuild[i];
    static const String cProcessing(String::Format("Processing : %s", contentItem->Filename));
    
    UpdateTaskProgress((float)(i + 1) / toBuild.Size(), "Processing : " + contentItem->Filename);

    contentItem->BuildContentItem(false);

    if (buildOptions.Failure)
    {
      PlasmaPrint("Content Build Failed, %s\n", buildOptions.Message.c_str());
      buildOptions.Failure = false;
      buildOptions.Message = String();
      allBuilt = false;
    }

    contentItem->BuildListing(package->Resources);

    
    // Don't do this in the thread (do it after).
    if (contentItem->mNeedsEditorProcessing)
      package->EditorProcessing.PushBack(contentItem);
  }

  Sort(package->Resources.All(), SortByLoadOrder());

  if (!allBuilt)
    status.SetFailed(String::Format("Failed to build content library '%s'", library->Name.c_str()));

  return package;
}

  
void ImportJob::OnImportFinished(PostImportEvent* e)
{
  // Load all resource generated into the active resource library
  PL::gResources->ReloadPackage(e->library, e->package);

  // Do editor side importing
  
  DoEditorSideImporting(e->package, e->mOptions);

  // Compile all scripts
  LightningManager::GetInstance()->TriggerCompileExternally();

  if (!e->contentToBuild.Empty() && e->status.Succeeded())
    DoNotify("Content Imported", "Content has been added to the project", "BigPlus");
  else if (e->status.Failed())
    DoNotify("Content Import", "Content failed to be added to the project", "Error");
}
} // namespace Plasma
