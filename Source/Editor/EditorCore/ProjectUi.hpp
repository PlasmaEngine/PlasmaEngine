#pragma once

namespace Plasma
{

    String GetDefaultProjectFolder();

    class OsFileSelection;


    namespace Events
    {
        DeclareEvent(TabSelected);
    }//namespace Event

    DeclareEnum3(ProjectTabs, NewProject, Recent, Examples);

    //------------------------------------------------------------------ Project Tab
    class ProjectTab : public Composite
    {
    public:
        /// Typedefs.
        typedef ProjectTab LightningSelf;

        /// Constructor.
        ProjectTab(Composite* parent, StringParam text, ProjectTabs::Type tabType);

        /// Composite Interface.
        void UpdateTransform() override;

        /// Selection.
        void Select();
        void DeSelect();

        /// Event response.
        void OnMouseEnter(MouseEvent* e);
        void OnMouseExit(MouseEvent* e);
        void OnMouseUp(MouseEvent* e);

        /// Whether or not the tab is currently selected.
        bool mSelected;

        /// Which tab this is.
        ProjectTabs::Type mTabType;

        Element* mBackground;
        /// The highlight for when the mouse is over or it's selected.
        Element* mBorder;

        /// The name of the tab.
        Label* mText;
    };

    class ProjectTabArea;
    class ProjectDialog;

    //--------------------------------------------------------------- Project Option
    class ProjectOption : public Composite
    {
    public:
        /// Needed to connect to events.
        typedef ProjectOption LightningSelf;

        /// Constructor.
        ProjectOption(ProjectTabArea* parent);

        /// Whether or not the option is selected.
        bool IsSelected();

        /// Used for custom logic when an option is selected.
        virtual void Selected() {}
        virtual void DeSelected() {}

    protected:
        /// When it's clicked, we want to select it.
        void OnMouseUp(MouseEvent* e);

        /// The area that owns us.
        ProjectTabArea* mArea;
    };

    //------------------------------------------------------------- Project Tab Area
    class ProjectTabArea : public Composite
    {
    public:
        /// Needed to connect to events.
        typedef ProjectTabArea LightningSelf;

        /// Constructor.
        ProjectTabArea(ProjectDialog* projectDialog);

        /// Custom logic for when this area is enabled / disabled.
        virtual void Enabled() {}
        virtual void Disabled() {}

        /// Called when the 'Ok' button is pressed.
        virtual bool Accept(StringParam projectName, StringParam location) = 0;

        /// Registers an option to be displayed and managed in the selection.
        void RegisterOption(ProjectOption* item);
        void RemoveOption(ProjectOption* item);

        /// Returns the selected item.
        ProjectOption* GetSelectedItem();

        /// Composite Interface.
        void UpdateTransform() override;
        bool TakeFocusOverride() override;

        /// Used for moving around the options with the arrow keys.
        virtual void OnKeyDown(KeyboardEvent* e);

        /// The size of the options.
        Vec2 mOptionSize;

        /// All options for this tab.
        Array<ProjectOption*> mOptions;

        /// The currently selected option.
        ProjectOption* mSelectedOption;

        /// Access back to the main dialog.
        ProjectDialog* mProjectDialog;

        /// A background for the entire area.
        Element* mBackground;
        Element* mBorder;
    };

    //------------------------------------------------------------------ Icon Option
    class IconOption : public ProjectOption
    {
    public:
        /// Needed to connect to events.
        typedef IconOption LightningSelf;

        /// Constructor.
        IconOption(ProjectTabArea* parent, StringParam name);

        /// ProjectItem Interface.
        void Selected() override;
        void DeSelected() override;
        void UpdateTransform() override;

        /// Event response.
        void OnDoubleClick(MouseEvent* e);
        void OnMouseEnter(MouseEvent* e);
        void OnMouseExit(MouseEvent* e);

        /// The name of this option.
        Label* mText;

        /// An icon on the left side.
        Element* mIcon;

        /// The highlight for when the mouse is over or it's selected.
        Element* mHighlight;
    };

    //------------------------------------------------------------- New Project Area
    class NewProjectArea : public ProjectTabArea
    {
    public:
        /// Constructor.
        NewProjectArea(ProjectDialog* projectDialog);

        /// ProjectArea Interface.
        void Enabled() override;
        void Disabled() override;
        bool Accept(StringParam projectName, StringParam location) override;

        /// When Enter is pressed, we want to move focus back to the name text box.
        void OnKeyDown(KeyboardEvent* e) override;

        /// The name that was last in the text box when this tab area was enabled.
        String mName;

        /// The location that was last in the text box when this tab area was enabled.
        String mLocation;
    };

    //--------------------------------------------------------------- Recent Project
    class RecentProject : public IconOption
    {
    public:
        /// Needed to connect to events.
        typedef RecentProject LightningSelf;

        /// Constructor.
        RecentProject(ProjectTabArea* parent, StringParam projectFile);

        /// Composite Interface.
        void UpdateTransform();

        /// Loads the project.
        void Load();

        /// IconOption Interface.
        void Selected() override;

        /// We want to load the project when it's double clicked.
        void OnMouseDoubleClick(MouseEvent* e);

        /// We want to add a remove option when it's right clicked.
        void OnRightMouseUp(MouseEvent* e);

        /// When the remove option is clicked in the context menu.
        void OnRemove(MouseEvent* e);

        /// The full path to the file of the project being displayed.
        String mProjectFile;
    };

    //---------------------------------------------------------- Recent Project Area
    class RecentProjectArea : public ProjectTabArea
    {
    public:
        /// Constructor.
        RecentProjectArea(ProjectDialog* projectDialog);

        /// ProjectArea Interface.
        void Enabled() override;
        bool Accept(StringParam projectName, StringParam location);

        /// When Enter is pressed, we want to load the project.
        void OnKeyDown(KeyboardEvent* e) override;
    };

    //---------------------------------------------------------------- Examples Area
    class ExamplesArea : public ProjectTabArea
    {
    public:
        /// Constructor
        ExamplesArea(ProjectDialog* projectDialog);

        /// ProjectTabArea Interface.
        void UpdateTransform() override;
        bool Accept(StringParam projectName, StringParam location) { return false; }

        Label* mComingSoonText;
    };

    //--------------------------------------------------------------- Project Dialog
    class ProjectDialog : public Composite
    {
    public:
        /// Needed to connect to events.
        typedef ProjectDialog LightningSelf;

        /// Constructor / Destructor.
        ProjectDialog(Composite* parent);
        ~ProjectDialog();

        /// Composite Interface.
        void UpdateTransform() override;
        void OnDestroy() override;

        /// Sets the currently selected tab.
        void SetSelectedTab(ProjectTabs::Type tab);

        /// Opens the project at the given file.
        void OpenProject(StringParam file);

        /// Closes the window.
        void Close();

        /// When the okay button was pressed.
        bool Accept();

    private:
        friend class ProjectTabArea;

        /// Sent when a tab is clicked on.
        void OnTabSelected(ObjectEvent* e);

        /// Event response.
        void OnOk(ObjectEvent* e);
        void OnCancel(ObjectEvent* e);
        void OnOpen(ObjectEvent* e);
        void OnOpenProjectFile(OsFileSelection* e);
        void OnBrowse(ObjectEvent* e);
        void OnBrowseSelected(OsFileSelection* e);
        void OnKeyDown(KeyboardEvent* e);

        /// Tabs along the left side of the window.
        ProjectTab* mTabs[ProjectTabs::Size];
        ProjectTabArea* mTabAreas[ProjectTabs::Size];
        ProjectTabs::Type mSelectedTab;

        /// Separators.
        Element* mHorizontalSeparator;
        Element* mVerticalSeparator;

    public:
        /// File Area.
        Label* mNameText;
        Label* mLocationText;
        TextBox* mNameBox;
        TextBox* mLocationBox;
        TextButton* mBrowseButton;
        TextButton* mOkButton;
        TextButton* mCancelButton;
        TextButton* mOpenButton;

        Element* mTabAreaBackground;
        Element* mTabAreaBorder;
    };

    ProjectDialog* OpenNewProjectDialog(Composite* main);

}//namespace Plasma
