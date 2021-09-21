// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

const String cTagIcon = "Tag";

struct LibDataEntry
{
  LibDataEntry() : mResource(nullptr)
  {
  }
  Resource* mResource;
  String mTag;
};

// Helpers for Sprite Frame
inline bool LibDataEntrySort(LibDataEntry* left, LibDataEntry* right)
{
  return left->mTag < right->mTag;
}

class LibraryDataSource : public DataSource
{
public:
  LibraryDataSource()
  {
    mRoot.mTag = "Root";
  }

  ~LibraryDataSource()
  {
    Clear();
  }

  LibDataEntry mRoot;
  Array<LibDataEntry*> mEntries;

  void AddResource(Resource* resource)
  {
    LibDataEntry* entry = new LibDataEntry();
    entry->mResource = resource;
    mEntries.PushBack(entry);
  }

  void AddTag(StringParam tag)
  {
    LibDataEntry* entry = new LibDataEntry();
    entry->mTag = tag;
    mEntries.PushBack(entry);
  }

  void SortByTagName()
  {
    Plasma::Sort(mEntries.All(), LibDataEntrySort);
  }

  void Clear()
  {
    DeleteObjectsInContainer(mEntries);
  }

  DataEntry* GetRoot() override
  {
    return &mRoot;
  }

  DataEntry* ToEntry(DataIndex index) override
  {
    if (index == cRootIndex || (uint)index.Id >= mEntries.Size())
      return &mRoot;
    return mEntries[(uint)index.Id];
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    if (dataEntry == &mRoot)
      return cRootIndex;
    LibDataEntry* entry = (LibDataEntry*)dataEntry;
    uint index = mEntries.FindIndex(entry);
    return DataIndex(index);
  }

  DataIndex GetResourceIndex(Resource* resource)
  {
    for (uint i = 0; i < mEntries.Size(); ++i)
    {
      if (resource == mEntries[i]->mResource)
        return DataIndex(i);
    }

    return DataIndex((u64)-1);
  }

  Handle ToHandle(DataEntry* dataEntry) override
  {
    LibDataEntry* entry = (LibDataEntry*)dataEntry;
    return entry->mResource;
  }

  DataEntry* Parent(DataEntry* dataEntry) override
  {
    if (dataEntry == &mRoot)
      return nullptr;
    return &mRoot;
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    if (dataEntry == &mRoot)
      return mEntries.Size();
    return 0;
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    if (dataEntry == &mRoot)
      return mEntries[index];
    return nullptr;
  }

  bool IsExpandable(DataEntry* dataEntry) override
  {
    return false;
  }
  
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    LibDataEntry* entry = (LibDataEntry*)dataEntry;

    // Resource entry
    if (entry->mResource)
    {
      Resource* resource = entry->mResource;
      BoundType* resourceType = LightningVirtualTypeId(resource);
      if (column == CommonColumns::Name)
        variant = resource->Name;
      else if (column == CommonColumns::Type)
        variant = resourceType->Name;
      else if (column == CommonColumns::Icon)
        variant = resource->mResourceIconName;
    }
    // Icon entry
    else
    {
      if (column == CommonColumns::Name)
        variant = entry->mTag;
      else if (column == CommonColumns::Type)
        variant = cTagIcon;
      else if (column == CommonColumns::Icon)
        variant = cTagIcon;
    }
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    if (PL::gEngine->IsReadOnly())
    {
      DoNotifyWarning("Resources", "Cannot rename resources in read-only mode");
      return false;
    }

    LibDataEntry* entry = (LibDataEntry*)dataEntry;
    Resource* resource = entry->mResource;

    if (resource == nullptr)
      return false;

    if (!column.Empty())
    {
      if (column == CommonColumns::Name)
      {
        String newName = variant.Get<String>();
        Status status;
        if (IsValidFilename(newName, status))
          return RenameResource(resource, newName);
        else
          DoNotifyWarning("Invalid resource name.", status.Message);
      }
    }

    return false;
  }
};

class TagTileWidget : public TileViewWidget
{
public:
  TagTileWidget(Composite* parent, TileView* tileView, PreviewWidget* tileWidget, DataIndex dataIndex) :
      TileViewWidget(parent, tileView, tileWidget, dataIndex)
  {
    // Create the tag icon
    mTagIcon = CreateAttached<Element>(cTagIcon);
  }

  /// Widget Interface.
  void UpdateTransform() override
  {
    TileViewWidget::UpdateTransform();
    // mTagIcon->SetTranslation(Pixels(2, 2, 0));

    float tagWidth = mTagIcon->mSize.x;

    // Place the tag to the left of the text
    Vec3 tagTranslation(mEditableText->mTranslation.x - tagWidth - Pixels(2), Pixels(2), 0);

    // The amount the tag would be negative on the left
    float overflow = tagTranslation.x - Pixels(2);
    overflow = -Math::Min(overflow, 0.0f);

    tagTranslation.x += overflow;
    mTagIcon->SetTranslation(tagTranslation);

    // Push the text to the right to account for the tag being stuck too far on
    // the left
    mEditableText->mTranslation.x += overflow;

    // Make sure the size isn't too large
    mEditableText->mSize.x = Math::Min(mEditableText->mSize.x, mSize.x - tagWidth - Pixels(2));

    // We need to update our children again to let the ellipses (...) on mText
    // process
    Composite::UpdateTransform();
  }

  Element* mTagIcon;
};

LibraryTileView::LibraryTileView(LibraryView* parent) : TileView(parent), mLibraryView(parent)
{
}

TileViewWidget* LibraryTileView::CreateTileViewWidget(
    Composite* parent, StringParam name, HandleParam instance, DataIndex index, PreviewImportance::Enum minImportance)
{
  PreviewWidget* previewWidget = nullptr;

  if (instance.IsNull())
    previewWidget = mLibraryView->CreatePreviewGroup(parent, name, 4);

  // If the instance isn't valid, it's a tag
  if (instance.IsNotNull() || previewWidget == nullptr)
    previewWidget = ResourcePreview::CreatePreviewWidget(parent, name, instance, minImportance);

  if (previewWidget == nullptr)
    return nullptr;

  if (instance.IsNull())
    return new TagTileWidget(parent, this, previewWidget, index);
  else
    return new TileViewWidget(parent, this, previewWidget, index);
}

AddLibraryUI::AddLibraryUI(Composite* parent, LibraryView* libraryView): Composite(parent), mLibraryView(libraryView)
{
    this->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 2), Thickness::cZero));
    new Label(this, "Text", "Library Name:");
    mNewLibraryName = new TextBox(this);
    mNewLibraryName->SetEditable(true);
    mNewLibraryName->SetText("NewLibrary");  
    mNewLibraryName->TakeFocus();
    new Label(this, "Text", "Library Path:");

    Composite* pathRow = new Composite(this);
    pathRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
  
    mLibraryPath = new TextBox(pathRow);
    mLibraryPath->SetEditable(false);
    mLibraryPath->SetSizing(SizeAxis::X,SizePolicy::Flex, Pixels(200));
    mLibraryPath->SetInteractive(false);
  
    mPathSelectButton = new TextButton(pathRow);
    mPathSelectButton->SetText("...");
    mPathSelectButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
    mPathSelectButton->SetInteractive(false);
    ConnectThisTo(mPathSelectButton, Events::ButtonPressed, OnSelectPath);

    Composite* pathCheckboxRow = new Composite(this);
    pathCheckboxRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));

    mSetIndependentPathCheckbox = new TextCheckBox(pathCheckboxRow);
    mSetIndependentPathCheckbox->SetCheckedDirect(false);
    mSetIndependentPathCheckbox->SetText(" Enable Manual Library Path Editing");
    ConnectThisTo(mSetIndependentPathCheckbox, Events::LeftClick, OnToggleEditablePath);
  
    Composite* buttonBar = new Composite(this);

    buttonBar->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
    {
        Composite* space = new Composite(buttonBar);
        space->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

        TextButton* createButton = new  TextButton(buttonBar);
        createButton->SetText("Create");
        createButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(80));
    
        ConnectThisTo(createButton, Events::ButtonPressed, OnCreate);

        TextButton* cancelButton = new  TextButton(buttonBar);
        cancelButton->SetText("Cancel");
        cancelButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(80));
        ConnectThisTo(cancelButton, Events::ButtonPressed, OnCancel);
    }
  
    Cog* projectCog = PL::gEditor->mProject;

    if (projectCog != nullptr)
    {
        ProjectSettings* projectSettings = projectCog->has(ProjectSettings);

        if (projectSettings != nullptr)
        {
            mLibraryPath->SetText(FilePath::Combine(projectSettings->ProjectFolder, mNewLibraryName->GetText()));
        }
    }

    UpdateLibraryPath();

    ConnectThisTo(this, Events::KeyUp, OnKeyUp);

}

AddLibraryUI::~AddLibraryUI()
{
}

void AddLibraryUI::OnCreate(Event* e)
{
  if(!mCanCreateLibrary)
  {
    DoNotify("Error", "Library with that name already exists", "Warning");
    return;
  }
  Status s;
  ContentLibrary* library = PL::gContentSystem->LibraryFromDirectory(s, mNewLibraryName->GetText(), mLibraryPath->GetText());

  if(!library)
  {
    DoNotifyError("Failed to create library.", s.Message);
    CloseTabContaining(this);
    return;
  }

  Status buildStatus;
  ResourcePackage* resourcePackage = PL::gContentSystem->BuildLibrary(buildStatus, library, true);

  Status status;
  ResourceLibrary* lib = PL::gResources->LoadPackage(status, resourcePackage);
  if (!status)
  {
    DoNotifyError("Failed to load resource package.", status.Message);
  }

  Cog* projectCog = PL::gEditor->mProject;

  if (projectCog != nullptr)
  {
    ProjectSettings* projectSettings = projectCog->has(ProjectSettings);

    if (projectSettings != nullptr)
    {      
      projectSettings->SharedResourceLibraries.PushBack(lib);

      Status projectResourceStatus;
      projectSettings->ProjectResourceLibrary = PL::gResources->LoadPackage(projectResourceStatus, resourcePackage);

      if (!projectResourceStatus)
        DoNotifyError("Failed to load resource package.", projectResourceStatus.Message);

      DoEditorSideImporting(resourcePackage, nullptr);

      PL::gEditor->SetExploded(false, true);
      PL::gEditor->ProjectLoaded();
      
      projectSettings->AddLibrary(mNewLibraryName->GetText());
      projectSettings->Save();
    }
  }

  if(mLibraryView)
  {
    mLibraryView->SetSelectedByName(mNewLibraryName->GetText());
  }
  
  CloseTabContaining(this);
}

void AddLibraryUI::OnToggleEditablePath(Event* e)
{
    bool isChecked = mSetIndependentPathCheckbox->GetChecked();
    if (mLibraryPath->mInteractive != isChecked)
    {
        mLibraryPath->SetInteractive(isChecked);
        mLibraryPath->SetEditable(isChecked);
        mLibraryPath->MarkAsNeedsUpdate();
        mPathSelectButton->SetInteractive(isChecked);
        mPathSelectButton->MarkAsNeedsUpdate();

        if (!isChecked)
            UpdateLibraryPath();
    }
}

void AddLibraryUI::OnSelectPath(Event* e)
{
  // Set up the callback for when library path is selected
  const String CallBackEvent = "LibraryPathSelected";
  if (!GetDispatcher()->IsConnected(CallBackEvent, this))
    ConnectThisTo(this, CallBackEvent, OnFolderSelected);

  // Open the open file dialog
  FileDialogConfig* config = FileDialogConfig::Create();
  config->EventName = CallBackEvent;
  config->CallbackObject = this;
  config->Title = "Select a folder";
  config->AddFilter("Library Folder", "*.none");
  config->DefaultFileName = mLibraryPath->GetText();
  config->StartingDirectory = mLibraryPath->GetText();
  config->Flags |= FileDialogFlags::Folder;
  PL::gEngine->has(OsShell)->SaveFile(config);
}

void AddLibraryUI::OnFolderSelected(OsFileSelection* e)
{
  if (e->Files.Size() > 0)
  {
    String path = BuildString(FilePath::GetDirectoryPath(e->Files[0]), cDirectorySeparatorCstr);
    mLibraryPath->SetText(path);
  }

  bool valid = true;
  Cog* projectCog = PL::gEditor->mProject;

  if (projectCog != nullptr)
  {
    ProjectSettings* projectSettings = projectCog->has(ProjectSettings);

    if (projectSettings != nullptr)
    {
      forRange (String library, projectSettings->ExtraLibraries.All())
      {
        String name = mNewLibraryName->GetText().ToLower();
        if(name.CompareTo(library.ToLower()))
        {
          valid = false;
        }
      }
    }
  }

  mCanCreateLibrary = valid;
}

void AddLibraryUI::OnCancel(Event* e)
{
  CloseTabContaining(this);
}

void AddLibraryUI::OnKeyUp(KeyboardEvent* event) {

    if (event->Key == Keys::Enter && !mLibraryPath->GetText().Empty() && !mNewLibraryName->GetText().Empty())
    {
        OnCreate(nullptr);
    }
    else if (event->Key == Keys::Escape)
    {
        OnCancel(nullptr);
    }
    else if(!mSetIndependentPathCheckbox->GetChecked())
    {
        UpdateLibraryPath();
    }
}

void AddLibraryUI::UpdateLibraryPath()
{
    Cog* projectCog = PL::gEditor->mProject;

    if (projectCog != nullptr)
    {
        ProjectSettings* projectSettings = projectCog->has(ProjectSettings);

        if (projectSettings != nullptr)
        {
            mLibraryPath->SetText(FilePath::Combine(projectSettings->ProjectFolder, mNewLibraryName->GetText()));
            mLibraryPath->MarkAsNeedsUpdate();
        }
    }
}

LightningDefineType(LibraryView, builder, type)
{
}

void RegisterEditorTileViewWidgets();

LibraryView::LibraryView(Composite* parent) : Composite(parent), mResourceLibrary(nullptr)
{
  mSearch = nullptr;
  mPrimaryCommandIndex = 0;
  mTagEditorCloseButton = nullptr;
  mTagEditor = nullptr;
  mSource = nullptr;
  mDataSelection = nullptr;
  mContentLibrary = nullptr;
  mResourceLibrary = nullptr;
  mIgnoreEditorSelectionChange = false;
  mTagEditorCurrentHeight = 0;
  mTagEditorFinalHeight = 0;

  this->SetLayout(CreateStackLayout());

  mLibrariesRow = new Composite(this);
  mLibrariesRow->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(16));
  mLibrariesRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(5, 0), Thickness(1, 0, 2, 0)));
  {
    mAddNewLibrary = new IconButton(mLibrariesRow);
    mAddNewLibrary->SetIcon("NewResource");

    mAddNewLibrary->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    ConnectThisTo(mAddNewLibrary, Events::ButtonPressed, OnCreateLibraryPress);
    mContentLibraries = new StringComboBox(mLibrariesRow);

    ConnectThisTo(mContentLibraries, Events::ItemSelected, OnContentLibrarySelected);

    mContentLibraries->SetSizing(SizeAxis::X, SizePolicy::Flex, 18);

    BuildContentLibraryList();
  }

  Spacer* spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(4));

  ConnectThisTo(PL::gContentSystem, Events::PackageBuilt, OnPackageBuilt);

  Composite* topRow = new Composite(this);
  topRow->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(16));
  topRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(1, 0, 2, 0)));
  {
    mSearchBox = new TagChainTextBox(topRow);
    mSearchBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
    mSearchBox->mAddTagsOnEnter = false;
    ConnectThisTo(mSearchBox, Events::SearchDataModified, OnSearchDataModified);
    ConnectThisTo(mSearchBox, Events::KeyDown, OnSearchKeyDown);
    ConnectThisTo(mSearchBox, Events::KeyPreview, OnSearchKeyPreview);
    ConnectThisTo(mSearchBox, Events::KeyRepeated, OnSearchKeyRepeated);
    mSearch = &mSearchBox->mSearch;
    mSearch->ActiveTags.Insert("Resources");

    mToggleViewButton = new ToggleIconButton(topRow);
    mToggleViewButton->SetEnabledIcon("GridIcon");
    mToggleViewButton->SetDisabledIcon("TreeIcon");
    ConnectThisTo(mToggleViewButton, Events::ButtonPressed, OnToggleViewButtonPressed);
  }

  spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(4));

  RegisterEditorTileViewWidgets();

  mTreeView = new TreeView(this);
  mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mTreeView->SetFormatNameAndType();

  ConnectThisTo(mTreeView, Events::TreeRightClick, OnTreeRightClick);
  ConnectThisTo(mTreeView, Events::KeyDown, OnKeyDown);
  ConnectThisTo(mTreeView, Events::MouseEnterRow, OnMouseEnterTreeRow);
  ConnectThisTo(mTreeView, Events::MouseExitRow, OnMouseExitTreeRow);

  mActiveView = mTreeView;

  mTileView = new LibraryTileView(this);
  mTileView->SetActive(false);
  mTileView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  ConnectThisTo(mTileView, Events::TileViewRightClick, OnTileViewRightClick);
  ConnectThisTo(mTileView, Events::KeyDown, OnKeyDown);
  ConnectThisTo(mTileView, Events::ScrolledAllTheWayOut, OnTilesScrolledAllTheWayOut);

  mTagEditor = new ResourceTagEditor(this);
  mTagEditor->SetSizing(SizeAxis::Y, SizePolicy::Fixed, 20);

  ConnectThisTo(mTagEditor, Events::TagsModified, OnTagEditorModified);

  mTagEditorCloseButton = mTagEditor->CreateAttached<Element>("Minimize");
  ConnectThisTo(mTagEditorCloseButton, Events::LeftMouseDown, OnTagEditorClose);
  ConnectThisTo(mTagEditorCloseButton, Events::MouseHover, OnTagEditorCloseHover);

  ConnectThisTo(PL::gResources, Events::ResourcesLoaded, OnResourcesModified);
  ConnectThisTo(PL::gResources, Events::ResourcesUnloaded, OnResourcesModified);
  ConnectThisTo(PL::gResources, Events::ResourceTagsModified, OnResourcesModified);
  ConnectThisTo(PL::gResources, Events::ResourceModified, OnResourcesModified);

  ConnectThisTo(mTreeView, Events::MouseScroll, OnMouseScroll);

  ConnectThisTo(PL::gEditor, Events::ProjectLoaded, OnProjectLoaded);

  MetaSelection* selection = PL::gEditor->GetSelection();
  ConnectThisTo(selection, Events::SelectionFinal, OnEditorSelectionChanged);

  ConnectThisTo(this, Events::RightMouseUp, OnRightMouseUp);
}

LibraryView::~LibraryView()
{
  SafeDelete(mSource);
}

void LibraryView::UpdateTransform()
{
  mTagEditorCloseButton->SetTranslation(Vec3(mTagEditor->mSize.x - Pixels(18), Pixels(2), 0));

  Composite::UpdateTransform();
}

void LibraryView::View(ContentLibrary* contentLibrary, ResourceLibrary* resourceLibrary)
{
  if (mResourceLibrary)
    DisconnectAll(mResourceLibrary, this);

  mContentLibrary = contentLibrary;
  mResourceLibrary = resourceLibrary;

  // These can be set to NULL
  if (mContentLibrary == nullptr || mResourceLibrary == nullptr)
    return;

  ConnectThisTo(resourceLibrary, Events::ResourceAdded, OnResourcesModified);

  mSearch->ClearSearchProviders();
  // We want to show all hidden resources in this resource library. The reason
  // hidden exists is because we don't want them to show up when setting
  // resource properties, but we do want them to show up when viewing the
  // resource library
  mSearch->SearchProviders.PushBack(GetResourceSearchProvider(mResourceLibrary, true));

  // Refresh the search with the new library
  mSearchBox->Refresh();

  // The data source represents the visible objects in the tree and tile view
  if (mSource == nullptr)
  {
    mSource = new LibraryDataSource();
    mTreeView->SetDataSource(mSource);
    mTileView->SetDataSource(mSource);
    ConnectThisTo(mSource, Events::DataActivated, OnDataActivated);
  }

  // The selected objects
  if (mDataSelection == nullptr)
  {
    mDataSelection = new HashDataSelection();
    mTreeView->SetSelection(mDataSelection);
    mTileView->SetSelection(mDataSelection);
    ConnectThisTo(mDataSelection, Events::DataSelectionModified, OnDataSelectionModified);
    ConnectThisTo(mDataSelection, Events::DataSelectionFinal, OnDataSelectionFinal);
  }

  UpdateVisibleResources();

  uint index = mContentLibraries->GetIndexOfItem(contentLibrary->Name);
  mContentLibraries->SetSelectedItem((int)index, false);
}

void LibraryView::View()
{
  if (mContentLibraries->GetCount() > 0)
  {
    String selectedLibrary = mContentLibraries->GetItem(0);
    ResourceLibrary* resourceLibrary = PL::gResources->GetResourceLibrary(selectedLibrary);
    ContentLibrary** contentLibrary = PL::gContentSystem->Libraries.FindPointer(selectedLibrary);

    View(*contentLibrary, resourceLibrary);
  }
}

void LibraryView::SwitchToTreeView()
{
  if (mActiveView == mTileView)
  {
    mTileView->SetActive(false);
    mTreeView->SetActive(true);
    mTreeView->MarkAsNeedsUpdate();
    mActiveView = mTreeView;
  }
}

void LibraryView::SwitchToTileView()
{
  if (mActiveView == mTreeView)
  {
    mTreeView->SetActive(false);
    mTileView->SetActive(true);
    mActiveView = mTileView;
  }
}

void LibraryView::SetSearchTags(TagList& tags)
{
  mSearchBox->ClearTags();
  forRange (String tag, tags.All())
  {
    mSearchBox->AddTag(tag, true, false);
  }
  mSearchBox->Refresh();
}

void PopulateGroup(PreviewWidgetGroup* group, SearchData* searchData, uint maxResults)
{
  searchData->Search();

  uint count = searchData->Results.Size();
  count = Math::Min(count, maxResults);
  uint found = 0;
  for (uint i = 0; i < searchData->Results.Size() && found < count; ++i)
  {
    SearchViewResult& result = searchData->Results[i];

    String resultType = result.Interface->GetElementType(result);
    if (resultType != "Tag")
    {
      Resource* resource = (Resource*)result.Data;

      String name = resource->Name;
      PreviewWidget* widget = group->AddPreviewWidget(name, resource, PreviewImportance::High);
      if (widget)
        ++found;
    }
  }
}

PreviewWidgetGroup* LibraryView::CreatePreviewGroup(Composite* parent, StringParam tag, uint max)
{
  PreviewWidgetGroup* group = new PreviewWidgetGroup(parent, tag);

  // Create a search to search for items with the given tag
  SearchData temporarySearch;
  temporarySearch.SearchProviders.PushBack(GetResourceSearchProvider(mResourceLibrary, true));
  forRange (String currTag, mSearch->ActiveTags.All())
  {
    temporarySearch.ActiveTags.Insert(currTag);
  }
  temporarySearch.ActiveTags.Insert(tag);

  // Fill out the group
  PopulateGroup(group, &temporarySearch, max);

  if (group->mPreviewWidgets.Size() == 0)
  {
    group->Destroy();
    return nullptr;
  }

  return group;
}

void LibraryView::AddHiddenLibrary(StringParam libraryName)
{
  mHiddenLibraries.Insert(libraryName);
  BuildContentLibraryList();
}

void LibraryView::UpdateVisibleResources()
{
  if (mSource == nullptr || mResourceLibrary == nullptr)
    return;

  mSource->Clear();

  // If there's only the default active tag (Resources), and they haven't
  // typed anything yet, we want to just show tags for the resources
  // they have in their project
  if (mSearch->ActiveTags.Size() == 1 && mSearch->SearchString.Empty())
  {
    HashSet<String> resourceTypes;
    forRange (HandleOf<Resource> resourceHandle, mResourceLibrary->Resources.All())
    {
      Resource* resource = resourceHandle;
      if (resource == nullptr)
      {
        Error("This should never be the case. "
              "Somehow we have a reference to a resource that doesn't exist.");
        continue;
      }

      // Add the type name
      resourceTypes.Insert(LightningVirtualTypeId(resource)->Name);

      // Add a filter tag as well
      String filterTag = resource->FilterTag;
      if (!filterTag.Empty())
        resourceTypes.Insert(filterTag);
    }

    forRange (String& resourceType, resourceTypes.All())
    {
      mSource->AddTag(resourceType);
    }

    // Sort by resource name
    mSource->SortByTagName();
  }
  // Otherwise, we want to show the results
  else
  {
    forRange (SearchViewResult& result, mSearch->Results.All())
    {
      String resultType = result.Interface->GetElementType(result);
      if (resultType != "Tag")
      {
        Resource* resource = (Resource*)result.Data;

        // Only add it if it's the set we're editing
        // if(resource->mSet == mResourceLibrary)
        mSource->AddResource(resource);
      }
      else
      {
        mSource->AddTag(result.Name);
      }
    }
  }

  mTreeView->ClearAllRows();
  mTreeView->SetDataSource(mSource);
  mTileView->SetDataSource(mSource);
}

void LibraryView::BuildContentLibraryList()
{
  mContentLibraries->ClearItems();
  mLibrariesRow->SetActive(false);

  forRange (ContentLibrary* library, PL::gContentSystem->Libraries.Values())
  {
    if (library == nullptr || mHiddenLibraries.Contains(library->Name))
      continue;

    // Only show the library if a resource library was built from it
    if (PL::gResources->GetResourceLibrary(library->Name))
      mContentLibraries->AddItem(library->Name);
  }

  // Enable dropdown selection menu for multiple libraries
  if (mContentLibraries->GetCount() > 1)
  {
    mLibrariesRow->SetActive(true);
  }
}

void LibraryView::OnPackageBuilt(ContentSystemEvent* e)
{
  String libraryName = e->mLibrary->Name;
  if (mHiddenLibraries.Contains(libraryName))
    return;

  mContentLibraries->AddItem(libraryName);
  if (mContentLibraries->GetCount() > 1)
    mLibrariesRow->SetActive(true);

  // If nothing is selected, attempt to select the library that was initially
  // viewed

  
 if(!initialized)
 {
   uint index = mContentLibraries->GetIndexOfItem(mContentLibrary->Name);
   mContentLibraries->SetSelectedItem((int)index, false);
   SetSelected(index);
   initialized = true;
 }
}

void LibraryView::SetSelected(int selectedIndex)
{
  String selectedItem = mContentLibraries->GetItem(selectedIndex);
  SetSelectedByName(selectedItem);
}

void LibraryView::OnContentLibrarySelected(Event* e)
{
  int selectedIndex = mContentLibraries->GetSelectedItem();
  if (selectedIndex == -1)
    return;

  SetSelected(selectedIndex);
}

void LibraryView::OnResourcesModified(ResourceEvent* event)
{
  // If an archetype has a RunInEditor script that creates a runtime resource
  // we don't want the preview to cause itself to be recreated
  if (event->EventResource != nullptr && event->EventResource->IsRuntime())
    return;

  // Whenever a resource is modified, added/removed, etc close the tag editor to
  // clean up all handles to now potentially invalid data that results in an odd
  // crash that hasn't been able to be reproduced. T941.
  if (event->EventId != Events::ResourceTagsModified)
    CloseTagEditor();

  mSearchBox->Refresh();

  // If there are no results (likely because resources have been removed),
  // clear the active tags and restart from the beginning
  if (mSearch->Results.Empty())
  {
    mSearchBox->ClearTags();
    mSearchBox->Refresh();
  }

  UpdateVisibleResources();
}

void LibraryView::OnTreeRightClick(TreeEvent* event)
{
  DataIndex index = event->Row->mIndex;
  OnRightClickObject(event->Row, index);
}

void LibraryView::OnTileViewRightClick(TileViewEvent* event)
{
  DataIndex index = event->mTile->GetDataIndex();
  OnRightClickObject(event->mTile, index);
}

void LibraryView::OnRightClickObject(Composite* objectToAttachTo, DataIndex index)
{
    mCommandIndices.Clear();

    ContextMenu* menu = new ContextMenu(objectToAttachTo);
    Mouse* mouse = PL::gMouse;

    mDataSelection->GetSelected(mCommandIndices);
    mPrimaryCommandIndex = index;
    LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);

    Resource* resource = entry->mResource;
    if (resource)
    {
        ConnectMenu(menu, "Edit", OnEdit, true);
        ConnectMenu(menu, "Rename", OnRename, false);
        ConnectMenu(menu, "Edit Content Meta", OnEditMeta, false);
        ConnectMenu(menu, "Edit Tags", OnEditTags, false);
        
        if (mContentLibrary->GetWritable())
        {
            menu->AddDivider();
            
            if (resource) {
                ConnectMenu(menu, "Move", OnMoveResourcePress, false);
            }
            
            if (resource && resource->mManager->mCanDuplicate)
            {
                ConnectMenu(menu, "Duplicate", OnDuplicate, false);
            }
            
            ConnectMenu(menu, "Remove", OnRemove, false);
        }

        BoundType* resourceType = LightningVirtualTypeId(resource);

        // Add composing and translation test functions for materials
        if (resourceType->IsA(LightningTypeId(Material)))
        {
            ConnectMenu(menu, "ComposeLightningMaterial", OnComposeLightningMaterial, false);
            ConnectMenu(menu, "TranslateLightningPixelMaterial", OnTranslateLightningPixelMaterial, false);
            ConnectMenu(menu, "TranslateLightningGeometryMaterial", OnTranslateLightningGeometryMaterial, false);
            ConnectMenu(menu, "TranslateLightningVertexMaterial", OnTranslateLightningVertexMaterial, false);
        }
        // Add a translation tests function for fragments
        if (resourceType->IsA(LightningTypeId(LightningFragment)))
        {
            ConnectMenu(menu, "TranslateFragment", OnTranslateFragment, false);
        }

        AddResourceOptionsToMenu(menu, resourceType->Name, true);
    }
    else
    {
        // When right clicking on a resource tag show an "Add 'resourceType'" option
        // if the user can add this type of resource
        if (AddResourceOptionsToMenu(menu, entry->mTag))
            menu->AddDivider();
        ConnectMenu(menu, "Add Tag To Search", OnAddTagToSearch, true);
    }

    menu->SizeToContents();
    menu->ShiftOntoScreen(ToVector3(mouse->GetClientPosition()));
}

void LibraryView::OnRightMouseUp(MouseEvent* event)
{
  if (event->Handled)
    return;

  ContextMenu* menu = new ContextMenu(this);
  menu->mRootEntry->mContext.Add(mContentLibrary);

  // When in the context of a specific resource search show an "Add
  // 'resourceType'" option
  forRange (String& tag, mSearchBox->mSearch.ActiveTags.All())
  {
    if (AddResourceOptionsToMenu(menu, tag))
    {
      menu->ShiftOntoScreen(ToVector3(event->Position));
      return;
    }
  }

  // If a specific resource is not found and the current library is writeable, pop up the generic add resources menu
  if (mContentLibrary->GetWritable())
  {
      menu->AddPlasmaContextMenu("Resources");
      menu->ShiftOntoScreen(ToVector3(event->Position));
  }
}

void LibraryView::OnKeyDown(KeyboardEvent* event)
{
  if (event->Handled)
    return;

  // Delete selected objects
  if (event->Key == Keys::Delete)
  {
    mCommandIndices.Clear();
    mDataSelection->GetSelected(mCommandIndices);
    if (!mCommandIndices.Empty())
    {
      mPrimaryCommandIndex = mCommandIndices.Front();
      OnRemove(nullptr);
    }
  }
  else if (event->Key == Keys::Enter)
  {
    Array<DataIndex> indices;
    mDataSelection->GetSelected(indices);

    if (indices.Size() == 1)
    {
      LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(indices.Front());
      if (entry->mResource)
      {
        PL::gEditor->EditResource(entry->mResource);
      }
      else
      {
        mSearchBox->AddTag(entry->mTag, true, false);
      }
    }
  }

  if (event->Key == Keys::F2)
  {
    if (PL::gEngine->IsReadOnly())
    {
      DoNotifyWarning("Resources", "Cannot rename resources while in read-only mode");
      return;
    }

    DataSelection* selection = mTreeView->GetSelection();
    // check if we have something currently selected
    if (selection)
    {
      Array<DataIndex> selectedIndices;
      mDataSelection->GetSelected(selectedIndices);
      // make sure we have anything actively selected
      if (selectedIndices.Size())
      {
        // get the data index of the selected item and edit its name
        RenameAtIndex(selectedIndices.Front());
      }
    }
  }

  if (event->CtrlPressed && event->Key == Keys::A)
    SelectAll();
}

void LibraryView::OnMouseEnterTreeRow(TreeEvent* event)
{
  mResourcePreview.SafeDestroy();
  DataIndex index = event->Row->mIndex;
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(index);
  if (entry->mResource)
    CreateResourceToolTip(entry->mResource, event->Row);
  else
    CreateTagToolTip(entry->mTag, event->Row);
}

void LibraryView::OnMouseExitTreeRow(TreeEvent* event)
{
  mResourcePreview.SafeDestroy();
}

void LibraryView::CreateResourceToolTip(Resource* resource, TreeRow* row)
{
  // Create the tooltip
  ToolTip* toolTip = new ToolTip(row);
  toolTip->mContentPadding = Thickness(2, 2, 2, 2);
  toolTip->SetColorScheme(ToolTipColorScheme::Gray);

  // Create the resource widget and attach it to the tooltip
  String name = resource->Name;
  PreviewWidget* tileWidget = ResourcePreview::CreatePreviewWidget(toolTip, name, resource, PreviewImportance::High);
  if (tileWidget == nullptr)
  {
    toolTip->Destroy();
    return;
  }
  tileWidget->SetSize(Pixels(200, 200));
  toolTip->SetContent(tileWidget);

  // Position the tooltip
  ToolTipPlacement placement;
  placement.SetScreenRect(row->GetScreenRect());
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, IndicatorSide::Bottom, IndicatorSide::Top);
  toolTip->SetArrowTipTranslation(placement);

  tileWidget->AnimatePreview(PreviewAnimate::Always);

  mResourcePreview = toolTip;
}

void LibraryView::CreateTagToolTip(StringParam tagName, TreeRow* row)
{
  // Create the tooltip
  ToolTip* toolTip = new ToolTip(row);
  toolTip->mContentPadding = Thickness(2, 2, 2, 2);
  toolTip->SetColorScheme(ToolTipColorScheme::Gray);

  PreviewWidgetGroup* group = CreatePreviewGroup(toolTip, tagName, 9);
  if (group == nullptr)
  {
    toolTip->Destroy();
    return;
  }

  group->SetSize(group->GetMinSize());
  toolTip->SetContent(group);

  // Position the tooltip
  ToolTipPlacement placement;
  placement.SetScreenRect(row->GetScreenRect());
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, IndicatorSide::Bottom, IndicatorSide::Top);
  toolTip->SetArrowTipTranslation(placement);

  group->AnimatePreview(PreviewAnimate::Always);

  mResourcePreview = toolTip;
}

void LibraryView::OnDataActivated(DataEvent* event)
{
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(event->Index);
  if (entry->mResource)
  {
    PL::gEditor->EditResource(entry->mResource);
  }
  else
  {
    mSearchBox->AddTag(entry->mTag, true, false);
  }
}

void LibraryView::OnDataSelectionModified(ObjectEvent* event)
{
  if (TagEditorIsOpen())
    EditTags(mDataSelection);

  // Get the selected indices
  Array<DataIndex> selectedIndices;
  mDataSelection->GetSelected(selectedIndices);

  if (mDataSelection->Size() != 0)
    mSearchBox->mSearchIndex = (uint)selectedIndices.Front().Id;

  // Add all selected resources to the editors selection
  MetaSelection* editorSelection = PL::gEditor->GetSelection();

  // We only want to clear the editors selection if we're selecting a resource
  bool cleared = false;
  forRange (DataIndex dataIndex, selectedIndices.All())
  {
    // Only add it if it's a resource
    LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(dataIndex);
    if (entry->mResource)
    {
      // Clear the editor selection first
      if (!cleared)
      {
        editorSelection->Clear();
        cleared = true;
      }

      // Ignore all documents (scripts, shader fragments, etc...)
      if (Type::DynamicCast<DocumentResource*>(entry->mResource))
        continue;

      editorSelection->Add(entry->mResource, SendsEvents::False);
    }
  }

  if (cleared)
    editorSelection->SelectionChanged();
}

void LibraryView::OnDataSelectionFinal(ObjectEvent* event)
{
  OnDataSelectionModified(event);

  MetaSelection* editorSelection = PL::gEditor->GetSelection();

  mIgnoreEditorSelectionChange = true;
  editorSelection->FinalSelectionChanged();
  mIgnoreEditorSelectionChange = false;
}

void LibraryView::OnEditorSelectionChanged(SelectionChangedEvent* event)
{
  if (mIgnoreEditorSelectionChange || mDataSelection == nullptr)
    return;

  mDataSelection->SelectNone();

  forRange (Resource* resource, event->Selection->AllOfType<Resource>())
  {
    DataIndex index = mSource->GetResourceIndex(resource);
    if (index.Id != (u64)-1)
      mDataSelection->Select(index);
  }
  mTreeView->MarkAsNeedsUpdate();
  mTileView->MarkAsNeedsUpdate();
}

void LibraryView::SelectAll()
{
  mTreeView->SelectAll();
}

void LibraryView::OnRemove(ObjectEvent* event)
{
  if (PL::gEngine->IsReadOnly())
  {
    DoNotifyWarning("Resources", "Cannot remove resources while in read-only mode");
    return;
  }

  String message, title;

  // This isn't the resource count, because it may contain rows that are tags
  uint indexCount = mCommandIndices.Size();
  if (indexCount > 1)
  {
    // We're removing multiple resources
    title = "Remove Resources";

    String names;
    uint resourceCount = 0;
    for (uint i = 0; i < mCommandIndices.Size(); ++i)
    {
      DataIndex currIndex = mCommandIndices[i];

      LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(currIndex);
      if (Resource* resource = treeNode->mResource)
      {
        // Add this resource name to the list of names
        names = BuildString(names, resource->Name);

        // Only add a comma after if it's not the last resource
        if (i != mCommandIndices.Size() - 1)
          names = BuildString(names, ", ");

        ++resourceCount;
      }
    }

    message = String::Format("Are you sure you want to remove %i resources [%s]"
                             " from the content library?",
                             resourceCount,
                             names.c_str());
  }
  else
  {
    // We're removing a single resource
    title = "Remove Resource";

    LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
    if (Resource* resource = treeNode->mResource)
    {
      message = String::Format("Are you sure you want to remove the resource '%s' "
                               " from the content library?",
                               resource->Name.c_str());
    }
    else
    {
      // If it's not a resource, there's nothing we can do
      return;
    }
  }

  MessageBox* box = MessageBox::Show(title, message, MBConfirmCancel);
  ConnectThisTo(box, Events::MessageBoxResult, OnMessageBox);
}

void LibraryView::OnRename(ObjectEvent* event)
{
  RenameAtIndex(mPrimaryCommandIndex);
}

void LibraryView::OnEdit(ObjectEvent* event)
{
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);

  if (Resource* resource = entry->mResource)
    PL::gEditor->EditResource(resource);
}

void LibraryView::OnEditMeta(ObjectEvent* event)
{
  LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);

  if (Resource* resource = entry->mResource)
  {
    ContentItem* contentItem = resource->mContentItem;

    MetaSelection* selection = PL::gEditor->GetSelection();
    selection->SelectOnly(contentItem);
    selection->FinalSelectionChanged();
  }
}

void LibraryView::OnEditTags(ObjectEvent* event)
{
  EditTags(mDataSelection);
}

void LibraryView::OnMessageBox(MessageBoxEvent* event)
{
  if (event->ButtonIndex == MessageResult::Yes)
  {
    Array<Resource*> resourcesToRemove = GetSelectedResources();

    forRange (Resource* resource, resourcesToRemove.All())
    {
      RemoveResource(resource);
    }

    ResourceEvent eventToSend;
    eventToSend.RemoveMode = RemoveMode::Unloading;
    PL::gResources->DispatchEvent(Events::ResourcesUnloaded, &eventToSend);

    mPrimaryCommandIndex = 0;
    mCommandIndices.Clear();
  }
}

void LibraryView::OnDuplicate(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if (Resource* resource = treeNode->mResource)
    DuplicateResource(resource);
}

void LibraryView::OnMoveResourcePress(Event* e)
{
    Window* window = new Window(PL::gEditor);
    MoveItemUI* moveItemUI = new MoveItemUI(window, this);

    window->SizeToContents();
    window->SetTitle("Move Resource");
    window->SetDockMode(DockMode::DockNone);

    CenterToWindow(PL::gEditor, window, false);
    window->MoveToFront();
}

void LibraryView::OnComposeLightningMaterial(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if (Resource* resource = treeNode->mResource)
    if (Resource* resource = treeNode->mResource)
    {
      ObjectEvent toSend(resource);
      PL::gEditor->DispatchEvent("ComposeLightningMaterial", &toSend);
    }
}

void LibraryView::OnTranslateLightningPixelMaterial(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if (Resource* resource = treeNode->mResource)
  {
    ObjectEvent toSend(resource);
    PL::gEditor->DispatchEvent("TranslateLightningPixelMaterial", &toSend);
  }
}

void LibraryView::OnTranslateLightningGeometryMaterial(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if (Resource* resource = treeNode->mResource)
  {
    ObjectEvent toSend(resource);
    PL::gEditor->DispatchEvent("TranslateLightningGeometryMaterial", &toSend);
  }
}

void LibraryView::OnTranslateLightningVertexMaterial(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if (Resource* resource = treeNode->mResource)
  {
    ObjectEvent toSend(resource);
    PL::gEditor->DispatchEvent("TranslateLightningVertexMaterial", &toSend);
  }
}

void LibraryView::OnTranslateFragment(Event* event)
{
  LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(mPrimaryCommandIndex);
  if (Resource* resource = treeNode->mResource)
  {
    ObjectEvent toSend(resource);
    PL::gEditor->DispatchEvent("TranslateLightningFragment", &toSend);
  }
}

void LibraryView::OnAddTagToSearch(ObjectEvent* event)
{
  Array<DataIndex> selected;
  mDataSelection->GetSelected(selected);

  Array<String> tagsToAdd;
  forRange (DataIndex& index, selected.All())
  {
    LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(index);
    if (entry->mResource == nullptr)
      tagsToAdd.PushBack(entry->mTag);
  }

  forRange (String& tag, tagsToAdd.All())
  {
    mSearchBox->AddTag(tag, true, false);
  }
}

bool LibraryView::AddResourceOptionsToMenu(ContextMenu* menu, StringParam resouceTypeName, bool addDivider)
{
  BoundType* boundType = MetaDatabase::GetInstance()->FindType(resouceTypeName);

  // Attempt to get bound type for the tag and if we have a type is it a
  // resource
  if (boundType && boundType->IsA(LightningTypeId(Resource)))
  {
    ResourceManager* manager = PL::gResources->GetResourceManager(boundType);
    if ((manager->mCanAddFile || manager->mCanCreateNew) && mContentLibrary->GetWritable())
    {
      if (addDivider)
        menu->AddDivider();

      // We have a resource so add the option to create a new resource of the
      // viewed type
      StringBuilder buttonTitle;
      buttonTitle.Append("Add ");
      buttonTitle.Append(boundType->Name);

      ContextMenuEntry* entry = menu->AddEntry(buttonTitle.ToString());
      ConnectThisTo(entry, Plasma::Events::MenuItemSelected, OnAddResource);
      entry->mContext.Add(boundType);
      return true;
    }
  }

  return false;
}

void LibraryView::OnAddResource(ObjectEvent* event)
{
  ContextMenuItem* item = (ContextMenuItem*)event->Source;
  ContextMenuEntry* entry = item->mEntry;

  AddResourceWindow* resourceWindow = OpenAddWindow(nullptr);
  resourceWindow->SetLibrary(mContentLibrary);
  if (BoundType* resourceType = entry->mContext.Get<BoundType>())
  {
    resourceWindow->SelectResourceType(resourceType);

    TagList tags = mSearch->ActiveTags;
    // The library view has an implicit "Resources" tag and we don't want to add
    // this to all the newly created resources as a custom tag
    tags.Erase("Resources");
    resourceWindow->AddTags(tags);
  }
  resourceWindow->TemplateSearchTakeFocus();
}

void LibraryView::OnToggleViewButtonPressed(Event* e)
{
  if (mToggleViewButton->GetEnabled())
    SwitchToTreeView();
  else
    SwitchToTileView();
}

void LibraryView::OnSearchDataModified(Event* e)
{
  if (mSearchBox->mSearchBar->HasFocus())
  {
    if (mTreeView->GetActive())
    {
      mTreeView->SelectFirstRow();
      mTreeView->ShowRow(mPrimaryCommandIndex);
    }
    else if (mTileView->GetActive())
    {
      mTileView->SelectFirstTile();
    }
  }
  else if (mDataSelection)
  {
    mDataSelection->SelectNone();
  }

  UpdateVisibleResources();
}

void LibraryView::OnSearchKeyDown(KeyboardEvent* e)
{
  HandleSearchKeyLogic(e);
}

void LibraryView::OnSearchKeyPreview(KeyboardEvent* e)
{
  if (e->Handled)
    return;

  if (e->Key == Keys::Enter)
  {
    Array<DataIndex> indices;
    mDataSelection->GetSelected(indices);

    if (indices.Size() == 1)
    {
      LibDataEntry* entry = (LibDataEntry*)mSource->ToEntry(indices.Front());

      if (entry->mResource)
        PL::gEditor->EditResource(entry->mResource);
      else
        mSearchBox->AddTag(entry->mTag, true, false);

      e->Handled = true;
    }
  }
}

void LibraryView::OnSearchKeyRepeated(KeyboardEvent* e)
{
  HandleSearchKeyLogic(e);
}

void LibraryView::HandleSearchKeyLogic(KeyboardEvent* e)
{
  if (e->Key == Keys::Down || e->Key == Keys::Up || e->Key == Keys::Left || e->Key == Keys::Right)
  {
    e->Handled = false;
    if (mTreeView->GetActive())
      mTreeView->GetScrollArea()->GetClientWidget()->DispatchEvent(Events::KeyDown, e);
    else if (mTileView->GetActive())
      mTileView->GetScrollArea()->GetClientWidget()->DispatchEvent(Events::KeyDown, e);
  }
}

void LibraryView::OnMouseScroll(MouseEvent* e)
{
  if (e->CtrlPressed)
  {
    if (e->Scroll.y > 0)
    {
      SwitchToTileView();
      mTileView->SetItemSizePercent(0);
    }
  }
}

void LibraryView::OnTilesScrolledAllTheWayOut(Event* e)
{
  SwitchToTreeView();
}

void LibraryView::OnProjectLoaded(Event* e)
{
  mSearchBox->ClearTags();
  mSearchBox->Refresh();
}

void LibraryView::OnTagEditorModified(Event* e)
{
  OpenTagEditor();
}

void LibraryView::OnTagEditorClose(MouseEvent* e)
{
  CloseTagEditor();
}

void LibraryView::OnTagEditorCloseHover(MouseEvent* e)
{
  // new ToolTip(this, "Hide");
}

float LibraryView::GetTagEditorSize(SizeAxis::Enum axis)
{
  return mTagEditor->GetAxisSize(axis);
}

void LibraryView::SetTagEditorSize(SizeAxis::Enum axis, float size)
{
  mTagEditor->SetAxisSize(axis, size);
}

void LibraryView::SetSelectedByName(String name)
{
  ContentLibrary* contentLibrary = PL::gContentSystem->Libraries.FindValue(name, nullptr);
  ResourceLibrary* resourceLibrary = PL::gResources->LoadedResourceLibraries.FindValue(name, nullptr);

  if (contentLibrary && resourceLibrary)
    View(contentLibrary, resourceLibrary);
}
  
float LibraryView::GetTagEditorCurrentHeight()
{
  return mTagEditorCurrentHeight;
}

void LibraryView::SetTagEditorCurrentHeight(float height)
{
  if (height < Pixels(1))
  {
    mTagEditor->SetActive(false);
  }
  else
  {
    mTagEditorCloseButton->SetTranslation(Vec3(mTagEditor->mSize.x - Pixels(18), Pixels(2), 0));
    mTagEditor->SetSize(Vec2(mTagEditor->mSize.x, height));
    mTagEditor->SetSizing(SizeAxis::Y, mTagEditor->GetSizePolicy().YPolicy, height);
    mTagEditor->SetActive(true);
  }

  mTagEditorCurrentHeight = height;

  // If the difference is less than 1 pixel, then the animation has finished.
  if (Math::Abs(mTagEditorFinalHeight - height) < Pixels(1))
    mTagEditor->SetIsAnimating(false);

  MarkAsNeedsUpdate();
}

void LibraryView::RenameAtIndex(DataIndex& dataIndex)
{
  if (PL::gEngine->IsReadOnly())
  {
    DoNotifyWarning("Resources", "Cannot rename resources in read-only mode");
    return;
  }

  if (mTreeView->GetActive())
  {
    TreeRow* row = mTreeView->FindRowByIndex(dataIndex);
    row->Edit(CommonColumns::Name);
  }
  else if (mTileView->GetActive())
  {
    TileViewWidget* tile = mTileView->FindTileByIndex(dataIndex);
    tile->Edit();
  }
}

bool LibraryView::TagEditorIsOpen()
{
  // When it's closed the height should be 0
  return mTagEditorCurrentHeight != 0.0f;
}

void LibraryView::EditTags(DataSelection* dataSelection)
{
  Array<DataIndex> selection;
  dataSelection->GetSelected(selection);

  Array<Resource*> resources;
  for (uint i = 0; i < selection.Size(); ++i)
  {
    DataIndex dataIndex = selection[i];
    Resource* resource = ((LibDataEntry*)mSource->ToEntry(dataIndex))->mResource;
    if (resource)
      resources.PushBack(resource);
  }

  mTagEditor->EditResources(resources);
  OpenTagEditor();
}

void LibraryView::OpenTagEditor()
{
  mTagEditorFinalHeight = mTagEditor->GetDesiredHeight(mTagEditor->GetTagChain()->GetSize());

  ActionSequence* seq = new ActionSequence(this);
  seq->Add(
      AnimatePropertyGetSet(LightningSelf, TagEditorCurrentHeight, Ease::Quad::Out, this, 0.3f, mTagEditorFinalHeight));

  mTagEditor->SetIsAnimating(true);
}

void LibraryView::CloseTagEditor()
{
  // Clean up all references to resource that the tag editor is holding onto
  mTagEditor->CleanTagEditor();

  float mTagEditorFinalHeight = 0;

  ActionSequence* seq = new ActionSequence(this);
  seq->Add(
      AnimatePropertyGetSet(LightningSelf, TagEditorCurrentHeight, Ease::Quad::Out, this, 0.3f, mTagEditorFinalHeight));

  mTagEditor->SetIsAnimating(true);
}

void LibraryView::OnCreateLibraryPress(Event* e)
{
  Window* window = new Window(PL::gEditor);
  AddLibraryUI* libraryUI = new AddLibraryUI(window, this);

  window->SizeToContents();
  window->SetTitle("Create Library");
  window->SetDockMode(DockMode::DockNone);

  CenterToWindow(PL::gEditor, window, false);
  window->MoveToFront();
}

Array<Resource*> LibraryView::GetSelectedResources()
{
    Array<Resource*> selectedResources;
    // content items with an associated resource will be selected
    HashSet<ContentItem*> contentItems;

    for (uint i = 0; i < mCommandIndices.Size(); ++i)
    {
        DataIndex currIndex = mCommandIndices[i];
        LibDataEntry* treeNode = (LibDataEntry*)mSource->ToEntry(currIndex);
        if (Resource* resource = treeNode->mResource)
        {
            if (!contentItems.Contains(resource->mContentItem))
            {
                selectedResources.PushBack(resource);
                contentItems.Insert(resource->mContentItem);
            }
        }
    }

    return selectedResources;
}

String LibraryView::GetSelectedLibraryName()
{
    return mContentLibrary != nullptr ? mContentLibrary->Name : "";
}

MoveItemUI::MoveItemUI(Composite* parent, LibraryView* libraryView): Composite(parent), mLibraryView(libraryView)
{
    this->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(5, 2), Thickness::cZero));

    mLibrariesRow = new Composite(this);
    mLibrariesRow->SetSizing(SizeAxis::Y, SizePolicy::Flex, Pixels(0));
    mLibrariesRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(5, 0), Thickness(1, 0, 2, 0)));
    {
        new Label(mLibrariesRow, "Text", "Library: ");

        mContentLibraries = new StringComboBox(mLibrariesRow);
        mContentLibraries->SetSizing(SizeAxis::X, SizePolicy::Flex, Pixels(18));
        BuildContentLibraryList();

        mResourcesToMove = mLibraryView->GetSelectedResources();
    }

    Composite* textBar = new Composite(this);

    textBar->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(1,4,1,4)));
    {
        Text* moveText = new Text(textBar, "NotoSans-Regular", 11);
        moveText->SetText("Selected resources to move are:");

        Widget* divider1 = new ContextMenuDivider(textBar, MenuUi::GutterColor);

        int numberOfNamesPerLine = 5;
        // This breaks up the display of the names of resources so that if a user has like 30 items selected, it won't extend off the screen
        for (int i = 0; i < mResourcesToMove.Size(); i += numberOfNamesPerLine)
        {
            String resourcesToMove;
            int nextNumberToStopAt = mResourcesToMove.Size() - i >= numberOfNamesPerLine ? (i + numberOfNamesPerLine) : mResourcesToMove.Size();

            for (int j = i; j < nextNumberToStopAt; j++)
            {
                resourcesToMove = resourcesToMove + mResourcesToMove[j]->Name + ", ";
            }

            Text* resourceText = new Text(textBar, "NotoSans-Regular", 11);
            resourceText->SetText(resourcesToMove);
        }

        Widget* divider2 = new ContextMenuDivider(textBar, MenuUi::GutterColor);
        
        Text* warningText1 = new Text(textBar, "NotoSans-Regular", 11);
        warningText1->SetText("This operation cannot be undone, and may possibly break dependent assets.");
        
        Text* warningText2 = new Text(textBar, "NotoSans-Regular", 11); 
        warningText2->SetText("Please be sure to double check the selected resources.");


    }

    Composite* buttonBar = new Composite(this);

    buttonBar->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
    {
        Composite* space = new Composite(buttonBar);
        space->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

        TextButton* createButton = new  TextButton(buttonBar);
        createButton->SetText("Move");
        createButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(80));

        ConnectThisTo(createButton, Events::ButtonPressed, OnMove);

        TextButton* cancelButton = new  TextButton(buttonBar);
        cancelButton->SetText("Cancel");
        cancelButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(80));
        ConnectThisTo(cancelButton, Events::ButtonPressed, OnCancel);
    }
}

MoveItemUI::~MoveItemUI()
{
}

void MoveItemUI::BuildContentLibraryList()
{
    mContentLibraries->ClearItems();
    mLibrariesRow->SetActive(false);

    forRange(ContentLibrary * library, PL::gContentSystem->Libraries.Values())
    {
        if (library == nullptr || library->GetReadOnly())
            continue;

        // Only show the library if a resource library was built from it and if it's writeable
        if (PL::gResources->GetResourceLibrary(library->Name))
            mContentLibraries->AddItem(library->Name);

    }

    // Enable dropdown selection menu for multiple libraries
    if (mContentLibraries->GetCount() > 1)
    {
        mLibrariesRow->SetActive(true);
        mContentLibraries->SetSelectedItem(0, false);
    }
}

void MoveItemUI::OnMove(Event* e)
{
    if (PL::gEngine->IsReadOnly())
    {
        DoNotifyWarning("Resources", "Cannot move resources in read-only mode");
        return;
    }

    forRange(Resource* resource, mResourcesToMove)
    {
        // execute the move func
        if (MoveResource(resource, PL::gContentSystem->Libraries.FindValue(mContentLibraries->GetSelectedString(), nullptr), PL::gResources->GetResourceLibrary(mContentLibraries->GetSelectedString())))
        {
            mLibraryView->MarkAsNeedsUpdate();
        }
    }
    
    OnCancel(nullptr);
}
  
void MoveItemUI::OnCancel(Event* e)
{
    CloseTabContaining(this);
}

} // namespace Plasma
