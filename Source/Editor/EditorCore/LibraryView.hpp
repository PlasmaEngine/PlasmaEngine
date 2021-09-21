// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
  // Forward declarations
  class TagChainTextBox;
  class TreeView;
  class TileView;
  class IconButton;
  class ResourceTagEditor;

  class ContentLibrary;
  class ResourceLibrary;
  class LibraryDataSource;

  class TreeEvent;
  class TreeRow;
  class DataEvent;
  class KeyboardEvent;
  class MessageBoxEvent;
  class TagEvent;
  class PreviewWidgetGroup;
  class LibraryView;
  struct SelectionChangedEvent;

  class LibraryTileView : public TileView
  {
  public:
    LibraryTileView(LibraryView* parent);

    /// TileView interface.
    TileViewWidget* CreateTileViewWidget(Composite* parent,
                                         StringParam name,
                                         HandleParam instance,
                                         DataIndex index,
                                         PreviewImportance::Enum minImportance) override;

    LibraryView* mLibraryView;
  };

  /// UI for creating libraries.
  class AddLibraryUI : public Composite
  {
  public:
    typedef AddLibraryUI LightningSelf;
    
    AddLibraryUI(Composite* parent, LibraryView* libraryView);
    ~AddLibraryUI();

    /// Called when we are typing in the library name text box.
    void UpdateLibraryPath();

    /// Called when on create button is pressed.
    void OnCreate(Event* e);

    /// Called when library path is selected.
    void OnSelectPath(Event* e);

    /// Called when the checkbox is checked
    void OnToggleEditablePath(Event* e);

    /// Called when folder is selected and validates selection.
    void OnFolderSelected(OsFileSelection* e);

    /// Called when 
    void OnKeyUp(KeyboardEvent* event);

    /// Called when the cancel button is pressed.
    void OnCancel(Event* e);

  
  private:
    TextBox* mNewLibraryName;
    TextBox* mLibraryPath;
    TextButton* mPathSelectButton;
    TextCheckBox* mSetIndependentPathCheckbox;
    LibraryView* mLibraryView;

    bool mCanCreateLibrary = true;
  };

  class MoveItemUI : public Composite
  {
  public:
    typedef MoveItemUI LightningSelf;

    MoveItemUI(Composite* parent, LibraryView* libraryView);
    ~MoveItemUI();

    void BuildContentLibraryList();

    void OnMove(Event* e);

    void OnCancel(Event* e);

  private:
     StringComboBox* mContentLibraries;
     LibraryView* mLibraryView;
     Composite* mLibrariesRow;

  };
  
class LibraryView : public Composite
{
public:
  /// Meta Initialization.
  LightningDeclareType(LibraryView, TypeCopyMode::ReferenceType);

  /// Constructor.
  LibraryView(Composite* parent);
  ~LibraryView();

  /// Widget interface.
  void UpdateTransform() override;

  /// Views the given content library.
  void View(ContentLibrary* contentLibrary, ResourceLibrary* resourceLibrary);

  /// Views the library at the 0th index (when no project is available)
  void View();

  /// Changes the display to the tree view.
  void SwitchToTreeView();

  /// Changes the display to the tile view.
  void SwitchToTileView();

  /// Sets the current search to the given tags.
  void SetSearchTags(TagList& tags);

  /// Creates a preview group of the given tag with the current search as
  /// extra tags.
  PreviewWidgetGroup* CreatePreviewGroup(Composite* parent, StringParam tag, uint max);

  void AddHiddenLibrary(StringParam libraryName);

  float GetTagEditorSize(SizeAxis::Enum axis);
  void SetTagEditorSize(SizeAxis::Enum axis, float size);

  void SetSelectedByName(String name);

  /// Returns current selected library in view
  ContentLibrary* GetLibrary() { return mContentLibrary; }

  Array<Resource*> GetSelectedResources();

private:
  void UpdateVisibleResources();

  void BuildContentLibraryList();
  void OnPackageBuilt(ContentSystemEvent* e);
  void SetSelected(int selectedIndex);
  void OnContentLibrarySelected(Event* e);

  /// Resource event response.
  void OnResourcesModified(ResourceEvent* event);

  /// Tree event response.
  void OnTreeRightClick(TreeEvent* event);
  void OnTileViewRightClick(TileViewEvent* event);
  void OnRightClickObject(Composite* objectToAttachTo, DataIndex index);
  void OnRightMouseUp(MouseEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnMouseEnterTreeRow(TreeEvent* event);
  void OnMouseExitTreeRow(TreeEvent* event);
  void CreateResourceToolTip(Resource* resource, TreeRow* row);
  void CreateTagToolTip(StringParam tagName, TreeRow* row);

  /// Data Source event response.
  void OnDataActivated(DataEvent* event);
  void OnDataSelectionModified(ObjectEvent* event);
  void OnDataSelectionFinal(ObjectEvent* event);
  void OnEditorSelectionChanged(SelectionChangedEvent* event);
  void SelectAll();

  /// Context menu event response.
  void OnRemove(ObjectEvent* event);
  void OnRename(ObjectEvent* event);
  void OnEdit(ObjectEvent* event);
  void OnEditMeta(ObjectEvent* event);
  void OnEditTags(ObjectEvent* event);
  void OnMessageBox(MessageBoxEvent* event);
  void OnDuplicate(Event* event);

  /// Extra context menus for lightning fragment translation. These should
  /// eventually be moved to some external registration once it is possible.
  void OnComposeLightningMaterial(Event* event);
  void OnTranslateLightningPixelMaterial(Event* event);
  void OnTranslateLightningGeometryMaterial(Event* event);
  void OnTranslateLightningVertexMaterial(Event* event);
  void OnTranslateFragment(Event* event);

  void OnAddTagToSearch(ObjectEvent* event);

  bool AddResourceOptionsToMenu(ContextMenu* menu, StringParam resouceName, bool addDivider = false);
  void OnAddResource(ObjectEvent* event);

  /// Editor event response.
  void OnToggleViewButtonPressed(Event* e);
  void OnSearchDataModified(Event* e);
  void OnSearchKeyDown(KeyboardEvent* e);
  void OnSearchKeyPreview(KeyboardEvent* e);
  void OnSearchKeyRepeated(KeyboardEvent* e);
  void HandleSearchKeyLogic(KeyboardEvent* e);
  void OnMouseScroll(MouseEvent* e);
  void OnTilesScrolledAllTheWayOut(Event* e);
  void OnProjectLoaded(Event* e);

  /// Tag editor event response.
  void OnTagEditorModified(Event* e);
  void OnTagEditorClose(MouseEvent* e);
  void OnTagEditorCloseHover(MouseEvent* e);

  float GetTagEditorCurrentHeight();
  void SetTagEditorCurrentHeight(float height);

  void RenameAtIndex(DataIndex& dataIndex);

private:
  /// Tag editor functions.
  bool TagEditorIsOpen();
  void EditTags(DataSelection* dataSelection);
  void OpenTagEditor();
  void CloseTagEditor();

  /// Displays AddLibraryUI Window.
  void OnCreateLibraryPress(Event* e);
  void OnMoveResourcePress(Event* e);
  
  /// Used to hide
  HashSet<String> mHiddenLibraries;
  StringComboBox* mContentLibraries;
  IconButton* mAddNewLibrary;
  Composite* mLibrariesRow;

  SearchData* mSearch;
  HandleOf<ToolTip> mResourcePreview;

  ContentLibrary* mContentLibrary;
  ResourceLibrary* mResourceLibrary;
  bool mIgnoreEditorSelectionChange;

  /// Represents the objects in the tree and the selection.
  LibraryDataSource* mSource;
  HashDataSelection* mDataSelection;

  /// The index of the row that was clicked on.
  DataIndex mPrimaryCommandIndex;
  /// All indices in the selection, including the primary command index.
  Array<DataIndex> mCommandIndices;

  /// Button to switch between the tree view and grid view.
  ToggleIconButton* mToggleViewButton;

  TagChainTextBox* mSearchBox;

  Composite* mActiveView;
  TreeView* mTreeView;
  LibraryTileView* mTileView;

  float mTagEditorCurrentHeight;
  float mTagEditorFinalHeight;
  Element* mTagEditorCloseButton;
  ResourceTagEditor* mTagEditor;

  bool initialized = false;
};

} // namespace Plasma
