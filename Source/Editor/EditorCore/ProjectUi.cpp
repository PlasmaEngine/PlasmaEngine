#include "Precompiled.hpp"

namespace Plasma
{

    namespace ProjectUi
    {
        const cstr cLocation = "EditorUi/ProjectDialog";
        Tweakable(Vec4, TabAreaColor, Vec4(1, 1, 1, 1), cLocation);
        Tweakable(Vec4, ProjectAreaColor, Vec4(1, 1, 1, 1), cLocation);
        Tweakable(Vec4, MouseOverTabColor, Vec4(1, 1, 1, 1), cLocation);
        Tweakable(Vec4, BorderColor, Vec4(1, 1, 1, 1), cLocation);
    }

    namespace Events
    {
        DefineEvent(TabSelected);
    }//namespace Event

    String GetDefaultProjectFolder()
    {
        return FilePath::Combine(GetUserDocumentsDirectory(), "PlasmaProjects");
    }

    ProjectTab::ProjectTab(Composite* parent, StringParam text, ProjectTabs::Type tabType)
        : Composite(parent), mTabType(tabType)
    {
        mBackground = CreateAttached<Element>(cWhiteSquare);
        mBackground->SetColor(Vec4(0, 0, 0, 0));
        mBorder = CreateAttached<Element>("TabHighlightBorder");
        mBorder->SetVisible(false);
        mBorder->SetInteractive(false);

        mText = new Label(this, cText);
        mText->SetText(text);
        mText->SetInteractive(false);

        mSelected = false;

        ConnectThisTo(mBackground, Events::MouseEnter, OnMouseEnter);
        ConnectThisTo(mBackground, Events::MouseExit, OnMouseExit);
        ConnectThisTo(mBackground, Events::LeftMouseUp, OnMouseUp);
    }

    void ProjectTab::UpdateTransform()
    {
        mBackground->SetSize(mSize);
        mBorder->SetSize(mSize);

        RenderFont* font = mText->GetFont();
        Vec3 textSize = ToVector3(font->MeasureText(mText->GetText()));

        Vec3 textPos = (ToVector3(mSize) * 0.5f) - textSize * 0.5f;
        mText->SetTranslation(SnapToPixels(textPos));
        mText->SetSize(mSize);

        Composite::UpdateTransform();
    }

    void ProjectTab::Select()
    {
        mSelected = true;
        mBackground->SetColor(ProjectUi::ProjectAreaColor);
        mBorder->SetColor(ProjectUi::BorderColor);
        mBorder->SetVisible(true);
    }

    void ProjectTab::DeSelect()
    {
        mSelected = false;
        mBackground->SetColor(Vec4(0, 0, 0, 0));
        mBorder->SetVisible(false);
    }

    void ProjectTab::OnMouseEnter(MouseEvent* e)
    {
        if (!mSelected)
        {
            mBackground->SetColor(ProjectUi::MouseOverTabColor);
            mBorder->SetColor(ProjectUi::MouseOverTabColor);
            mBorder->SetVisible(true);
        }
    }

    void ProjectTab::OnMouseExit(MouseEvent* e)
    {
        if (!mSelected)
        {
            mBackground->SetColor(Vec4(0, 0, 0, 0));
            mBorder->SetColor(ProjectUi::BorderColor);
            mBorder->SetVisible(false);
        }
    }

    void ProjectTab::OnMouseUp(MouseEvent* e)
    {
        Select();

        ObjectEvent eventToSend(this);
        GetDispatcher()->Dispatch(Events::TabSelected, &eventToSend);
    }

    ProjectOption::ProjectOption(ProjectTabArea* parent)
        : Composite(parent), mArea(parent)
    {
        ConnectThisTo(this, Events::LeftMouseUp, OnMouseUp);
    }

    bool ProjectOption::IsSelected()
    {
        return mArea->mSelectedOption == this;
    }

    void ProjectOption::OnMouseUp(MouseEvent* e)
    {
        forRange(ProjectOption * item, mArea->mOptions.All())
        {
            item->DeSelected();
        }

        this->Selected();
        mArea->mSelectedOption = this;
        mArea->TakeFocus();
    }

    ProjectTabArea::ProjectTabArea(ProjectDialog* projectDialog)
        : Composite(projectDialog)
    {
        mBackground = CreateAttached<Element>(cWhiteSquare);
        mBorder = CreateAttached<Element>(cWhiteSquareBorder);
        mSelectedOption = NULL;
        mProjectDialog = projectDialog;

        ConnectThisTo(this, Events::KeyDown, OnKeyDown);
    }

    void ProjectTabArea::RegisterOption(ProjectOption* item)
    {
        if (mSelectedOption == NULL)
        {
            mSelectedOption = item;
            mSelectedOption->Selected();
        }
        mOptions.PushBack(item);
    }

    void ProjectTabArea::RemoveOption(ProjectOption* item)
    {
        mOptions.EraseValueError(item);

        if (mSelectedOption == item)
        {
            if (mOptions.Size() == 0)
            {
                mSelectedOption = NULL;
            }
            else
            {
                mSelectedOption = mOptions.Front();
                mSelectedOption->Selected();
            }
        }

        MarkAsNeedsUpdate();
    }

    ProjectOption* ProjectTabArea::GetSelectedItem()
    {
        return mSelectedOption;
    }

    void ProjectTabArea::UpdateTransform()
    {
        mBackground->SetSize(mSize);
        mBackground->SetColor(ProjectUi::ProjectAreaColor);

        mBorder->SetSize(mSize);
        mBorder->SetColor(ProjectUi::BorderColor);

        TileLayout tiledLayout(mOptionSize, mSize, int(mOptions.Size()), Pixels(5.0f));

        for (uint i = 0; i < mOptions.Size(); ++i)
        {
            LayoutResult result = tiledLayout.ComputeTileLayout(i);

            mOptions[i]->SetTranslation(result.Translation);
            mOptions[i]->SetSize(result.Size);
        }

        Composite::UpdateTransform();
    }

    bool ProjectTabArea::TakeFocusOverride()
    {
        this->HardTakeFocus();
        return true;
    }

    void ProjectTabArea::OnKeyDown(KeyboardEvent* e)
    {
        TileLayout tiledLayout(mOptionSize, mSize, int(mOptions.Size()), Pixels(5.0f));

        IntVec2 direction = IntVec2::cZero;
        if (e->Key == Keys::Up)
            direction.y += -1;
        else if (e->Key == Keys::Down)
            direction.y += 1;
        else if (e->Key == Keys::Right)
            direction.x += 1;
        else if (e->Key == Keys::Left)
            direction.x += -1;

        int currentIndex = (int)mOptions.FindIndex(mSelectedOption);
        int newTile = tiledLayout.GetTileInDirection(currentIndex, direction);

        if (newTile >= 0 && newTile < (int)mOptions.Size())
        {
            mSelectedOption->DeSelected();
            mSelectedOption = mOptions[newTile];
            mSelectedOption->Selected();
        }
    }

    IconOption::IconOption(ProjectTabArea* parent, StringParam name)
        : ProjectOption(parent)
    {
        mHighlight = CreateAttached<Element>("Highlight");
        mHighlight->SetColor(Vec4(0, 0, 0, 0));
        mText = new Label(this, cText);
        mText->SetText(name);
        mIcon = CreateAttached<Element>("Item");
        mIcon->SetInteractive(false);

        ConnectThisTo(this, Events::DoubleClick, OnDoubleClick);
        ConnectThisTo(mHighlight, Events::MouseEnterHierarchy, OnMouseEnter);
        ConnectThisTo(mHighlight, Events::MouseExitHierarchy, OnMouseExit);
    }

    void IconOption::Selected()
    {
        mHighlight->ChangeDefinition(mDefSet->GetDefinition("Highlight"));
        mHighlight->SetColor(Vec4(1, 1, 1, 0.85f));
        MarkAsNeedsUpdate();
    }

    void IconOption::DeSelected()
    {
        mHighlight->SetColor(Vec4(0, 0, 0, 0));
        MarkAsNeedsUpdate();
    }

    void IconOption::UpdateTransform()
    {
        const Vec2 cIconSize = Pixels(27, 31);
        mIcon->SetTranslation(Pixels(4, 5, 0));
        mIcon->SetSize(cIconSize);
        mHighlight->SetSize(mSize);
        mText->SetTranslation(Vec3(cIconSize.x + Pixels(9), Pixels(12), 0));
        mText->SetSize(mText->GetMinSize());

        Composite::UpdateTransform();
    }

    void IconOption::OnDoubleClick(MouseEvent* e)
    {
        mArea->mProjectDialog->Accept();
    }

    void IconOption::OnMouseEnter(MouseEvent* e)
    {
        if (!IsSelected())
        {
            mHighlight->ChangeDefinition(mDefSet->GetDefinition("SubHighlight"));
            mHighlight->SetColor(Vec4(1, 1, 1, 0.85f));
        }
    }

    void IconOption::OnMouseExit(MouseEvent* e)
    {
        if (!IsSelected())
            mHighlight->SetColor(Vec4(0, 0, 0, 0));
    }

    NewProjectArea::NewProjectArea(ProjectDialog* projectDialog)
        : ProjectTabArea(projectDialog)
    {
        mOptionSize = Pixels(200, 40);

        RegisterOption(new IconOption(this, "Empty Project (3D)"));
        RegisterOption(new IconOption(this, "Empty Project (2D)"));

        mName = "<ProjectName>";
        mLocation = GetDefaultProjectFolder();
    }

    void NewProjectArea::Enabled()
    {
        mProjectDialog->mNameBox->SetReadOnly(false);
        mProjectDialog->mNameBox->SetText(mName);
        mProjectDialog->mLocationBox->SetReadOnly(false);
        mProjectDialog->mLocationBox->SetText(mLocation);
        mProjectDialog->mNameBox->TakeFocus();
    }

    void NewProjectArea::Disabled()
    {
        mName = mProjectDialog->mNameBox->GetText();
        mLocation = mProjectDialog->mLocationBox->GetText();
    }

    bool NewProjectArea::Accept(StringParam projectName, StringParam location)
    {
        IconOption* item = static_cast<IconOption*>(GetSelectedItem());
        String name = item->mText->GetText();

        EditorMode::Enum editorMode = EditorMode::Mode3D;
        if (name == "Empty Project (3D)")
            editorMode = EditorMode::Mode3D;
        else if (name == "Empty Project (2D)")
            editorMode = EditorMode::Mode2D;
        else
        {
            DoNotifyWarning("Invalid template", String::Format("'%s' does not exist.", name.c_str()));
            return false;
        }

        Status status;
        if (!IsValidFilename(projectName, status))
        {
            DoNotifyWarning("Invalid Project name", status.Message);
            return false;
        }

        if (!DirectoryExists(location))
            CreateDirectory(location);

        String directory = FilePath::Combine(location, projectName);
        directory = FilePath::Normalize(directory);

        String file = FilePath::CombineWithExtension(directory, projectName, ".plasmaproj");
        file = FilePath::Normalize(file);

        if (FileExists(file))
        {
            DoNotifyWarning("Cannot create project", "Project already exists.");
            return false;
        }

        CreateProject(PL::gEditor, projectName, directory, editorMode);

        // Restart the editor
        if (!Os::IsDebuggerAttached())
        {
            // Store the new project in the config file
            EditorConfig* config = HasOrAdd<EditorConfig>(PL::gEditor->mConfig);
            config->EditingProject = file;
            config->EditingLevel = String();
            SaveConfig(/*PL::gEditor->mConfig*/);

            // Restart the editor
           PL::gEngine->Terminate();
            Os::ShellOpenFile(GetApplication().c_str());
        }

        return true;
    }
    
    void NewProjectArea::OnKeyDown(KeyboardEvent* e)
    {
        if (e->Key == Keys::Enter)
            mProjectDialog->mNameBox->TakeFocus();
        else
            ProjectTabArea::OnKeyDown(e);
    }

    RecentProject::RecentProject(ProjectTabArea* parent, StringParam projectFile)
        : IconOption(parent, FilePath::GetFileNameWithoutExtension(projectFile))
    {
        mProjectFile = projectFile;
        ConnectThisTo(this, Events::DoubleClick, OnMouseDoubleClick);
        ConnectThisTo(this, Events::RightMouseUp, OnRightMouseUp);
        mIcon->ChangeDefinition(mDefSet->GetDefinition("Level"));
    }

    void RecentProject::UpdateTransform()
    {
        const Vec2 cIconSize = Pixels(32, 32);
        mIcon->SetTranslation(Pixels(4, 4, 0));
        mIcon->SetSize(cIconSize);

        mHighlight->SetSize(mSize);
        mText->SetTranslation(Vec3(cIconSize.x + Pixels(11), Pixels(13), 0));

        IconOption::UpdateTransform();
    }

    void RecentProject::Load()
    {
        // Load the project
        mArea->mProjectDialog->OpenProject(mProjectFile);
    }

    void RecentProject::Selected()
    {
        // We want to update the name and location text boxes to show details
        // of this project
        String name = FilePath::GetFileNameWithoutExtension(mProjectFile);
        String location = FilePath::GetDirectoryPath(mProjectFile);

        mArea->mProjectDialog->mNameBox->SetText(name);
        mArea->mProjectDialog->mLocationBox->SetText(location);

        IconOption::Selected();
    }

    void RecentProject::OnMouseDoubleClick(MouseEvent* e)
    {
        // Load the project only if it's the left button
        if (e->IsButtonDown(MouseButtons::Left))
            Load();
    }

    void RecentProject::OnRightMouseUp(MouseEvent* e)
    {
        ContextMenu* menu = new ContextMenu(this);
        Mouse* mouse = PL::gMouse;
        menu->SetBelowMouse(mouse, Pixels(0, 0));

        ConnectMenu(menu, "Remove", OnRemove, false);
    }

    void RecentProject::OnRemove(MouseEvent* e)
    {
        // Erase this project from the config file
        RecentProjects* recent = HasOrAdd<RecentProjects>(PL::gEditor->mConfig);
        recent->RemoveRecentProject(mProjectFile);
        SaveConfig(/*pl::gEditor->mConfig*/);

        // Remove ourselves from our parent and destroy us
        mArea->RemoveOption(this);
        this->Destroy();
    }

    RecentProjectArea::RecentProjectArea(ProjectDialog* projectDialog)
        : ProjectTabArea(projectDialog)
    {
        mOptionSize = Pixels(200, 40);

        Array<String> projects;
        RecentProjects* recent = HasOrAdd<RecentProjects>(PL::gEditor->mConfig);
        recent->GetProjectsByDate(projects);

        // Register them
        for (uint i = 0; i < projects.Size(); ++i)
            RegisterOption(new RecentProject(this, projects[i]));
    }

    void RecentProjectArea::OnKeyDown(KeyboardEvent* e)
    {
        if (e->Key == Keys::Enter)
        {
            RecentProject* project = static_cast<RecentProject*>(GetSelectedItem());
            project->Load();
            mProjectDialog->Close();
        }
        else
        {
            ProjectTabArea::OnKeyDown(e);
        }
    }

    void RecentProjectArea::Enabled()
    {
        mProjectDialog->mNameBox->SetReadOnly(true);
        mProjectDialog->mLocationBox->SetReadOnly(true);
        if (mSelectedOption)
            mSelectedOption->Selected();
        TakeFocus();
    }

    bool RecentProjectArea::Accept(StringParam projectName, StringParam location)
    {
        // Load the selected project
        RecentProject* project = static_cast<RecentProject*>(GetSelectedItem());
        if (project)
        {
            project->Load();
            return true;
        }

        return false;
    }

    ExamplesArea::ExamplesArea(ProjectDialog* projectDialog)
        : ProjectTabArea(projectDialog)
    {
        mComingSoonText = new Label(this, cText);
        mComingSoonText->SetText("Coming Soon!");
        mComingSoonText->SizeToContents();
    }

    void ExamplesArea::UpdateTransform()
    {
        Vec2 textSize = mComingSoonText->GetSize();
        Vec3 position = ToVector3((mSize * 0.5f) - (textSize * 0.5f));
        mComingSoonText->SetTranslation(SnapToPixels(position) - Pixels(15, 5, 0));
        ProjectTabArea::UpdateTransform();
    }

    ProjectDialog::ProjectDialog(Composite* parent)
        : Composite(parent)
    {
        const String cDefinitionSet = "ProjectDialog";
        mDefSet = mDefSet->GetDefinitionSet(cDefinitionSet);

        mTabAreaBackground = CreateAttached<Element>(cWhiteSquare);
        mTabAreaBorder = CreateAttached<Element>(cWhiteSquareBorder);

        mNameText = new Label(this, cText);
        mNameText->SetText("Name:");
        mNameText->SetSize(Pixels(100, 25));

        mLocationText = new Label(this, cText);
        mLocationText->SetText("Location:");
        mLocationText->SetSize(Pixels(100, 25));

        mNameBox = new TextBox(this);
        mNameBox->SetEditable(true);
        mNameBox->SetText("<ProjectName>");
        ConnectThisTo(mNameBox, Events::KeyDown, OnKeyDown);

        mLocationBox = new TextBox(this);
        mLocationBox->SetText(GetDefaultProjectFolder());
        mLocationBox->SetEditable(true);
        ConnectThisTo(mLocationBox, Events::KeyDown, OnKeyDown);

        mBrowseButton = new TextButton(this);
        mBrowseButton->SetText("Browse...");
        mBrowseButton->SetToolTip("Set the location for a new project.");
        ConnectThisTo(mBrowseButton, Events::ButtonPressed, OnBrowse);

        mOkButton = new TextButton(this);
        mOkButton->SetText("Ok");
        ConnectThisTo(mOkButton, Events::ButtonPressed, OnOk);

        mCancelButton = new TextButton(this);
        mCancelButton->SetText("Cancel");
        ConnectThisTo(mCancelButton, Events::ButtonPressed, OnCancel);

        mOpenButton = new TextButton(this);
        mOpenButton->SetText("Open...");
        mOpenButton->SetToolTip("Open existing project");
        ConnectThisTo(mOpenButton, Events::ButtonPressed, OnOpen);

        // Create the tab areas corresponding to the tabs
        mTabAreas[ProjectTabs::NewProject] = new NewProjectArea(this);
        mTabAreas[ProjectTabs::NewProject]->SetActive(false);
        mTabAreas[ProjectTabs::Recent] = new RecentProjectArea(this);
        mTabAreas[ProjectTabs::Recent]->SetActive(false);
        mTabAreas[ProjectTabs::Examples] = new ExamplesArea(this);
        mTabAreas[ProjectTabs::Examples]->SetActive(false);

        // Create the tabs
        mTabs[ProjectTabs::NewProject] = new ProjectTab(this, "New Project", ProjectTabs::NewProject);
        mTabs[ProjectTabs::Recent] = new ProjectTab(this, "Recent Projects", ProjectTabs::Recent);
        mTabs[ProjectTabs::Examples] = new ProjectTab(this, "Examples", ProjectTabs::Examples);

        for (uint i = 0; i < ProjectTabs::Size; ++i)
            ConnectThisTo(mTabs[i], Events::TabSelected, OnTabSelected);

        mSelectedTab = ProjectTabs::NewProject;
        SetSelectedTab(ProjectTabs::NewProject);

        mNameBox->TakeFocus();
        PL::gEditor->SetExploded(true, true);
    }

    ProjectDialog::~ProjectDialog()
    {

    }

    void ProjectDialog::UpdateTransform()
    {
        const Vec2 cTabSize = Pixels(119, 40);
        const float cFileAreaHeight = Pixels(87);
        const Vec2 cButtonSize = Pixels(65, 22);

        for (uint i = 0; i < ProjectTabs::Size; ++i)
        {
            ProjectTab* tab = mTabs[i];

            tab->SetSize(cTabSize);
            tab->SetTranslation(Vec3(Pixels(1), cTabSize.y * float(i), 0));
        }

        mTabAreaBackground->SetSize(Vec2(cTabSize.x, mSize.y - cFileAreaHeight));
        mTabAreaBackground->SetColor(ProjectUi::TabAreaColor);

        mTabAreaBorder->SetSize(mTabAreaBackground->GetSize());
        mTabAreaBorder->SetColor(ProjectUi::BorderColor);

        Vec2 tabAreaSize(mSize.x - cTabSize.x, mSize.y - cFileAreaHeight + Pixels(1));
        mTabAreas[mSelectedTab]->SetSize(tabAreaSize);
        mTabAreas[mSelectedTab]->SetTranslation(Vec3(cTabSize.x, 0, 0));

        const float cFileAreaStart = mSize.y - cFileAreaHeight;

        mNameText->SetTranslation(Vec3(Pixels(11), cFileAreaStart + Pixels(8), 0));
        mLocationText->SetTranslation(Vec3(Pixels(11), cFileAreaStart + Pixels(35), 0));

        mNameBox->SetTranslation(Vec3(Pixels(70), cFileAreaStart + Pixels(7), 0));
        mNameBox->SetSize(Vec2(mSize.x - Pixels(80), Pixels(20)));

        mLocationBox->SetTranslation(Vec3(Pixels(70), cFileAreaStart + Pixels(32), 0));
        mLocationBox->SetSize(Vec2(mSize.x - Pixels(150), Pixels(20)));

        mBrowseButton->SetTranslation(Vec3(mSize.x - Pixels(75), cFileAreaStart + Pixels(31), 0));
        mBrowseButton->SetSize(cButtonSize);

        mOkButton->SetTranslation(Vec3(mSize.x - Pixels(145), cFileAreaStart + Pixels(58), 0));
        mOkButton->SetSize(cButtonSize);

        mCancelButton->SetTranslation(Vec3(mSize.x - Pixels(75), cFileAreaStart + Pixels(58), 0));
        mCancelButton->SetSize(cButtonSize);

        mOpenButton->SetTranslation(Vec3(Pixels(11), cFileAreaStart + Pixels(58), 0));
        mOpenButton->SetSize(cButtonSize);

        Composite::UpdateTransform();
    }

    void ProjectDialog::OnDestroy()
    {
        Composite::OnDestroy();
        PL::gEditor->SetExploded(false, true);
    }

    void ProjectDialog::SetSelectedTab(ProjectTabs::Type tab)
    {
        if (tab != mSelectedTab)
        {
            mTabs[mSelectedTab]->DeSelect();
            mTabAreas[mSelectedTab]->SetActive(false);
            mTabAreas[mSelectedTab]->Disabled();
        }

        mTabs[tab]->Select();
        mTabAreas[tab]->SetActive(true);
        mTabAreas[tab]->Enabled();

        mSelectedTab = tab;
    }

    void ProjectDialog::OpenProject(StringParam file)
    {
        // If the debugger is attached, restarting the process would detach
        // the debugger. So for now, just open the project until we find another
        // way around it
        if (Os::IsDebuggerAttached())
        {
            OpenProjectFile(file);
            Close();
        }
        else
        {
            // Store the new project in the config file
            EditorConfig* config = HasOrAdd<EditorConfig>(PL::gEditor->mConfig);
            config->EditingProject = file;
            config->EditingLevel = String();
            SaveConfig(/*PL::gEditor->mConfig*/);

            // Restart the engine
            PL::gEngine->Terminate();

            // Otherwise run zero again with the given project
            Os::ShellOpenFile(GetApplication().c_str());
        }
    }

    void ProjectDialog::Close()
    {
        this->GetParent()->GetParent()->Destroy();
    }

    bool ProjectDialog::Accept()
    {
        String projectName = mNameBox->GetText();
        String location = mLocationBox->GetText();

        // Attempt to see if the selected tab area will accept the name and location
        if (mTabAreas[mSelectedTab]->Accept(projectName, location))
        {
            // If it does, close the window
            Close();
            return true;
        }

        return false;
    }

    void ProjectDialog::OnTabSelected(ObjectEvent* e)
    {
        // Set the selected tab
        ProjectTab* tab = static_cast<ProjectTab*>(e->Source);
        SetSelectedTab(tab->mTabType);
    }

    void ProjectDialog::OnOk(ObjectEvent* e)
    {
        Accept();
    }

    void ProjectDialog::OnCancel(ObjectEvent* e)
    {
        Close();
    }

    void ProjectDialog::OnOpen(ObjectEvent* e)
    {
        //Set up the callback for when project file is selected
        const String cCallBackEvent = "OpenProjectCallback";
        if (!GetDispatcher()->IsConnected(cCallBackEvent, this))
            ConnectThisTo(this, cCallBackEvent, OnOpenProjectFile);

        //Open the open file dialog
        FileDialogConfig* config = FileDialogConfig::Create();
        config->EventName = cCallBackEvent;
        config->CallbackObject = this;
        config->Title = "Open a project";
        config->AddFilter("Plasma Project File", "*.plasmaproj");
        config->StartingDirectory = GetDefaultProjectFolder();
        PL::gEngine->has(OsShell)->OpenFile(config);
    }

    void ProjectDialog::OnOpenProjectFile(OsFileSelection* e)
    {
        if (e->Success)
            OpenProject(e->Files[0]);
    }

    void ProjectDialog::OnBrowse(ObjectEvent* e)
    {
        //Set up the callback for when project file is selected
        const String CallBackEvent = "FolderCallback";
        if (!GetDispatcher()->IsConnected(CallBackEvent, this))
            ConnectThisTo(this, CallBackEvent, OnBrowseSelected);

        //Open the open file dialog
        FileDialogConfig* config = FileDialogConfig::Create();
        config->EventName = CallBackEvent;
        config->CallbackObject = this;
        config->Title = "Select a folder";
        config->AddFilter("Plasma Project Folder", "*.none");
        config->Flags |= FileDialogFlags::Folder;
        PL::gEngine->has(OsShell)->SaveFile(config);
    }

    void ProjectDialog::OnBrowseSelected(OsFileSelection* e)
    {
        if (e->Files.Size() > 0)
        {
            String path = FilePath::GetDirectoryPath(e->Files[0]);
            mLocationBox->SetText(path);
        }
    }

    void ProjectDialog::OnKeyDown(KeyboardEvent* e)
    {
        // If enter was pressed, we want to attempt to create / load the project
        if (e->Key == Keys::Enter)
        {
            if (!Accept())
                mNameText->TakeFocus();
            e->Handled = true;
        }

        if (e->Key == Keys::Escape)
        {
            Close();
        }
    }

    ProjectDialog* OpenNewProjectDialog(Composite* main)
    {
        Window* newWindow = new Window(main);
        ProjectDialog* dialog = new ProjectDialog(newWindow);
        newWindow->SetTitle("Projects");
        newWindow->SetSize(Pixels(600, 397));

        Vec3 center = GetCenterPosition(main, newWindow);
        center.y = Pixels(-397);
        newWindow->SetTranslation(center);
        newWindow->SetHideOnClose(false);
        newWindow->SetActive(true);
        CenterToWindow(main, newWindow, true);

        return dialog;
    }

}//namespace Plasma
