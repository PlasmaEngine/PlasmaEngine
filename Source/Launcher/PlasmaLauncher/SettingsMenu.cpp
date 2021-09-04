// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(LauncherConfigChanged);
} // namespace Events

namespace SettingsUi
{
const cstr cLocation = "LauncherUi/Settings";
Tweakable(Vec4, BackgroundColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(float, SlideInTime, 0.25f, cLocation);
Tweakable(Vec4, SettingsColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, SettingsHoverColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, SettingsClickedColor, Vec4(1, 1, 1, 1), cLocation);

Tweakable(Vec4, TextBoxButtonHover, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, TextBoxButtonClicked, Vec4(1, 1, 1, 1), cLocation);
} // namespace SettingsUi

SettingsMenu::SettingsMenu(Modal* parent, LauncherWindow* launcher) :
    Composite(parent),
    mLauncher(launcher),
    mParentModal(parent)
{
  PlasmaPrint("Settings Menu Displayed\n");
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 5), Thickness(32, 52, 32, 22)));

  // Create the background and slide it in
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(SettingsUi::BackgroundColor);
  mBackground->SetTranslation(Pixels(-500, 0, 0));

  ActionGroup* group = new ActionGroup(this);

  LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);

  Composite* topRow = new Composite(this);
  topRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(23, 0), Thickness(-2, -3, 0, 0)));
  {
    IconButton* settingsButton = new IconButton(topRow);
    settingsButton->SetIcon("OptionsIcon");
    settingsButton->mBackground->SetVisible(false);
    settingsButton->mBorder->SetVisible(false);
    settingsButton->mIconColor = ToByteColor(SettingsUi::SettingsColor);
    settingsButton->mIconHoverColor = ToByteColor(SettingsUi::SettingsHoverColor);
    settingsButton->mIconClickedColor = ToByteColor(SettingsUi::SettingsClickedColor);
    settingsButton->SetColor(Vec4(1, 1, 1, 0));
    ConnectThisTo(settingsButton, Events::ButtonPressed, OnSettingsPressed);

    group->Add(Fade(settingsButton, Vec4(1, 1, 1, 1), SettingsUi::SlideInTime));

    Text* settingsText = new Text(topRow, "MainTabText");
    settingsText->SetText("SETTINGS");
    settingsText->SizeToContents();
    settingsText->SetColor(Vec4(1, 1, 1, 0));

    group->Add(Fade(settingsText, SettingsUi::SettingsColor, SettingsUi::SlideInTime * 2.0f));
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 28));

  // AutoRunMode
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("PROJECT SETTINGS");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    StringSource* source = new StringSource();
    source->Strings.Resize(2);
    source->Strings[LauncherAutoRunMode::None] = "Always open launcher";
    source->Strings[LauncherAutoRunMode::IfInstalled] = "Auto run project if build installed";
    // Don't do this option for now, maybe later
    // source->Strings[LauncherAutoRunMode::InstallAndRun] = "Auto run project";
    mAutoRunMode = new ComboBox(this);
    mAutoRunMode->SetListSource(source);
    mAutoRunMode->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(200));
    // mAutoRunMode->mBackgroundColor =
    // ToByteColor(ModernTextBoxUi::BackgroundColor);
    ConnectThisTo(mAutoRunMode, Events::ItemSelected, OnAutoRunModeSelected);
    mWidgetsToAnimate.PushBack(mAutoRunMode);
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 10));

  // build common style struct for FolderLocation objects
  FolderLocationStyle style;
  style.mFont = mLauncherRegularFont;
  style.mTextColor = SettingsUi::SettingsColor;
  style.mBackgroundColor = SettingsUi::BackgroundColor;
  style.mHoverColor = SettingsUi::SettingsHoverColor;
  style.mClickedColor = SettingsUi::SettingsClickedColor;
  style.mButtonHoverColor = SettingsUi::TextBoxButtonHover;
  style.mButtonClickedColor = SettingsUi::TextBoxButtonClicked;

  // Default Project Location
  {
      mDefaultProjectLocation = new FolderLocation(this);
      mDefaultProjectLocation->mStyle = style;
      mDefaultProjectLocation->mLabel = "DEFAULT PROJECT LOCATION";
      mDefaultProjectLocation->mFileDialogFilterDescription = "Project Folder";
      mDefaultProjectLocation->GetConfigValue = []() {
          LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
          return config->mDefaultProjectSaveLocation;
      };
      mDefaultProjectLocation->SetConfigValue = [](String value) {
          LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
          config->mDefaultProjectSaveLocation = value;
      };
      mDefaultProjectLocation->mConfigSavedEventLabel = Events::LauncherConfigChanged;
      ConnectThisTo(mDefaultProjectLocation, Events::FolderLocationUpdated, OnDefaultProjectLocationChange);
      mDefaultProjectLocation->Create(mWidgetsToAnimate);
  }

  // Download location
  {
      mDownloadLocation = new FolderLocation(this);
      mDownloadLocation->mStyle = style;
      mDownloadLocation->mLabel = "DEFAULT DOWNLOAD LOCATION";
      mDownloadLocation->mFileDialogFilterDescription = "Download Folder";
      mDownloadLocation->GetConfigValue = []() {
          LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
          return config->mDownloadPath;
      };
      mDownloadLocation->SetConfigValue = [](String value) {
          LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
          config->mDownloadPath = value;
      };
      mDownloadLocation->mConfigSavedEventLabel = Events::LauncherConfigChanged;
      ConnectThisTo(mDownloadLocation, Events::FolderLocationUpdated, OnDownloadLocationTextSubmit);
      ConnectThisTo(mDownloadLocation, Events::ConfigChangeApplied, MoveDownloadLocation);
      mDownloadLocation->Create(mWidgetsToAnimate);
  }

  // ContentOutput location
  {
    mContentOutputLocation = new FolderLocation(this);
    mContentOutputLocation->mStyle = style;
    mContentOutputLocation->mLabel = "DEFAULT CACHE LOCATION";
    mContentOutputLocation->mFileDialogFilterDescription = "Content Cache";
    mContentOutputLocation->GetConfigValue = [](){ 
        ContentConfig* config = PL::gEngine->GetConfigCog()->has(ContentConfig);
        return config->ContentOutput;
    };
    mContentOutputLocation->SetConfigValue = [](String value) {
        ContentConfig* config = PL::gEngine->GetConfigCog()->has(ContentConfig);
        config->ContentOutput = value;
    };
    mContentOutputLocation->mConfigSavedEventLabel = Events::LauncherConfigChanged;
    ConnectThisTo(mContentOutputLocation, Events::FolderLocationUpdated, OnContentOutputLocationTextSubmit);
    ConnectThisTo(mContentOutputLocation, Events::ConfigChangeApplied, MoveContentOutputLocation);
    mContentOutputLocation->Create(mWidgetsToAnimate);
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 10));

  // Max recent projects
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("MAX RECENT PROJECTS");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mMaxNumberOfRecentProjects = new TextBox(this);
    mMaxNumberOfRecentProjects->SetEditable(true);
    mMaxNumberOfRecentProjects->SetStyle(TextBoxStyle::Modern);
    mMaxNumberOfRecentProjects->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(160));
    ConnectThisTo(mMaxNumberOfRecentProjects, Events::TextSubmit, OnMaxRecentProjectsModified);
    mWidgetsToAnimate.PushBack(mMaxNumberOfRecentProjects);
  }

  // Auto check for major updates
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("AUTO CHECK FOR MAJOR UPDATES");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mAutoCheckForLauncherUpdatesCheckBox = new CheckBox(this);
    mAutoCheckForLauncherUpdatesCheckBox->SetChecked(config->mAutoCheckForLauncherUpdates);
    mAutoCheckForLauncherUpdatesCheckBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(12));
    ConnectThisTo(mAutoCheckForLauncherUpdatesCheckBox, Events::ValueChanged, OnAutoCheckForMajorUpdatesModified);
    mWidgetsToAnimate.PushBack(mAutoCheckForLauncherUpdatesCheckBox);
  }

  // Show development builds
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("SHOW DEVELOPMENT BUILDS");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mShowDevelopCheckBox = new CheckBox(this);
    mShowDevelopCheckBox->SetChecked(true);
    mShowDevelopCheckBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(12));
    ConnectThisTo(mShowDevelopCheckBox, Events::ValueChanged, OnShowDevelopModified);
    mWidgetsToAnimate.PushBack(mShowDevelopCheckBox);
  }

  // Show experimental branches
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("SHOW EXPERIMENTAL BRANCHES");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mShowExperimentalBranchesCheckBox = new CheckBox(this);
    mShowExperimentalBranchesCheckBox->SetChecked(true);
    mShowExperimentalBranchesCheckBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(12));
    ConnectThisTo(mShowExperimentalBranchesCheckBox, Events::ValueChanged, OnShowExperimentalBranchesModified);
    mWidgetsToAnimate.PushBack(mShowExperimentalBranchesCheckBox);
  }

  new Spacer(this, SizePolicy::Flex, Vec2(1, 1));

  // Check for updates (builds/etc...)
  TextButton* checkForUpdatesButton = new TextButton(this);
  checkForUpdatesButton->SetText("Check for Updates");
  ConnectThisTo(checkForUpdatesButton, Events::ButtonPressed, OnCheckForUpdates);

  mWidgetsToAnimate.PushBack(checkForUpdatesButton);
  if (PL::gEngine->GetConfigCog()->has(DeveloperConfig))
  {
    // Check for updates (might go away)
    TextButton* checkForLauncherUpdatesButton = new TextButton(this);
    checkForLauncherUpdatesButton->SetText("Check for Launcher Updates");
    ConnectThisTo(checkForLauncherUpdatesButton, Events::ButtonPressed, OnCheckForLauncherUpdates);
    mWidgetsToAnimate.PushBack(checkForLauncherUpdatesButton);
  }

  // Revert settings
  TextButton* revertSettingsButton = new TextButton(this);
  revertSettingsButton->SetText("Revert to default");
  revertSettingsButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(160));
  ConnectThisTo(revertSettingsButton, Events::ButtonPressed, OnRevertToDefaults);
  mWidgetsToAnimate.PushBack(revertSettingsButton);

  ConnectThisTo(this, Events::Activated, OnActivated);
  ConnectThisTo(this, Events::Deactivated, OnDeactivated);

  // Update all of the ui to display the current config values
  LoadFromConfig();

  ConnectThisTo(mParentModal, Events::ModalClosed, OnModalClosed);

  AnimateIn();
}

void SettingsMenu::UpdateTransform()
{
  Vec2 parentSize = GetParent()->GetSize();
  SetSize(Vec2(parentSize.x * 0.4f, parentSize.y));

  mBackground->SetSize(mSize);
  mBackground->SetColor(SettingsUi::BackgroundColor);
  Composite::UpdateTransform();
}

void SettingsMenu::OnActivated(Event* e)
{
  mLauncher->mMainButton->SetEnabled(false);
  mLauncher->mSearch->SetVisible(false);

  // The config settings could've been changed (via a project loading for
  // example) so reload the current settings from the config
  LoadFromConfig();
  MarkAsNeedsUpdate();
}

void SettingsMenu::OnDeactivated(Event* e)
{
  ModalConfirmAction* modal = mConfirmModal;
  // If we had an active modal dialog then cancel it
  // (cancel so we revert the settings back to what they are supposed to be)
  if (modal)
    modal->Cancel();
}


void SettingsMenu::OnDefaultProjectLocationChange(StringChangeEvent* e)
{
    PlasmaPrint("Settings: Changing default project location from '%s' to '%s'\n", e->OldValue.c_str(), e->NewValue.c_str());
    mDefaultProjectLocation->ApplyChange();
}

void SettingsMenu::OnDownloadLocationTextSubmit(StringChangeEvent* e)
{
  String& oldDownloadPath = e->OldValue;
  String& newDownloadPath = e->NewValue;

  // Since we're just moving the folder to the new location, we can't move to a
  // sub-folder of the current location, to detect this we're just seeing if the
  // starting paths are the same (by adding a trailing '\' right now, hopefully
  // that's sufficient) and if they are we're just showing a tooltip to the user
  String oldPathWithSeparator = BuildString(oldDownloadPath, cDirectorySeparatorCstr);
  String newPathWithSeparator = BuildString(FilePath::Normalize(newDownloadPath), cDirectorySeparatorCstr);
  if (newPathWithSeparator.StartsWith(oldPathWithSeparator))
  {
    mDownloadLocation->ResetWithError("Cannot move to sub-directory");
    return;
  }

  // Make a confirmation dialog to see if the user whats to move
  // the downloads folder and contents to the new location
  ModalConfirmAction* modal = new ModalConfirmAction(mParentModal, "Move downloads to new location?");
  ConnectThisTo(modal, Events::ModalConfirmResult, OnConfirmChangeDownloadLocation);
  mConfirmModal = modal;
}

void SettingsMenu::OnConfirmChangeDownloadLocation(ModalConfirmEvent* e)
{
  if (e->mConfirmed == true)
  {
    mDownloadLocation->ApplyChange();
  }
  else
  {
    // They hit cancel, set the download location text back to the config's
    // settings
    mDownloadLocation->Reset();
  }
}

void SettingsMenu::MoveDownloadLocation(StringChangeEvent* e)
{
    String oldPath = e->OldValue;
    String newPath = e->NewValue;

    PlasmaPrint("Settings: Changing downloads from '%s' to '%s'\n", oldPath.c_str(), newPath.c_str());

    // Move the old folder to the new location
    MoveFolderContents(newPath, oldPath);
    // Currently Move doesn't delete the old directory, so delete it...
    if (oldPath != newPath)
        DeleteDirectory(oldPath);
}


void SettingsMenu::OnContentOutputLocationTextSubmit(StringChangeEvent* e)
{
    String& oldPath = e->OldValue;
    String& newPath = e->NewValue;

    // Since we're just moving the folder to the new location, we can't move to a
    // sub-folder of the current location, to detect this we're just seeing if the
    // starting paths are the same (by adding a trailing '\' right now, hopefully
    // that's sufficient) and if they are we're just showing a tooltip to the user
    String oldPathWithSeparator = BuildString(oldPath, cDirectorySeparatorCstr);
    String newPathWithSeparator = BuildString(FilePath::Normalize(newPath), cDirectorySeparatorCstr);
    if (newPathWithSeparator.StartsWith(oldPathWithSeparator))
    {
        mContentOutputLocation->ResetWithError("Cannot move to sub-directory");
        return;
    }

    // Make a confirmation dialog to see if the user whats to move
    // the downloads folder and contents to the new location
    ModalConfirmAction* modal = new ModalConfirmAction(mParentModal, "Move content cache to new location?");
    ConnectThisTo(modal, Events::ModalConfirmResult, OnConfirmChangeDownloadLocation);
    mConfirmModal = modal;
}

void SettingsMenu::OnConfirmChangeContentOutputLocation(ModalConfirmEvent* e)
{
    if (e->mConfirmed == true)
    {
        mContentOutputLocation->ApplyChange();
    }
    else
    {
        // They hit cancel, set the download location text back to the config's settings
        mContentOutputLocation->Reset();
    }
}

void SettingsMenu::MoveContentOutputLocation(StringChangeEvent* e)
{
    String oldPath = e->OldValue;
    String newPath = e->NewValue;

    PlasmaPrint("Settings: Changing content cache from '%s' to '%s'\n", oldPath.c_str(), newPath.c_str());

    // Move the old folder to the new location
    MoveFolderContents(newPath, oldPath);
    // Currently Move doesn't delete the old directory, so delete it...
    if (oldPath != newPath)
        DeleteDirectory(oldPath);

    PlasmaTodo("Update DefaultEditorConfiguration.data with new ContentOutput cache location");
}


void SettingsMenu::OnAutoRunModeSelected(Event* e)
{
  LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
  config->mAutoRunMode = mAutoRunMode->GetSelectedItem();
  SaveConfig();
}

void SettingsMenu::OnMaxRecentProjectsModified(Event* e)
{
  RecentProjects* recentProjects = PL::gEngine->GetConfigCog()->has(RecentProjects);

  uint maxRecentProjects;
  ToValue(mMaxNumberOfRecentProjects->GetText(), maxRecentProjects);
  // For now, ToValue doesn't return if the parsing failed, it just will return
  // the default value of plasma. In that case just return revert back to the
  // config's settings (even if they typed plasma) and ignore their changes.
  if (maxRecentProjects == 0)
    maxRecentProjects = recentProjects->mMaxRecentProjects;

  // We still need some logical extremes for the max projects...
  maxRecentProjects = Math::Clamp(maxRecentProjects, 1u, 50u);

  PlasmaPrint(
      "Settings: Changing max recent projects from %d to %d\n", recentProjects->mMaxRecentProjects, maxRecentProjects);

  // If we clamped the value then the text also has to be updated
  mMaxNumberOfRecentProjects->SetText(ToString(maxRecentProjects));
  // Make sure we remove projects if we now have too many
  recentProjects->UpdateMaxNumberOfProjects(maxRecentProjects, true);
  // Since we may have changed what projects were in recent make sure to save
  SaveConfig();
}

void SettingsMenu::OnAutoCheckForMajorUpdatesModified(Event* e)
{
  LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
  config->mAutoCheckForLauncherUpdates = mAutoCheckForLauncherUpdatesCheckBox->GetChecked();

  // Save the config now with the new settings
  SaveConfig();

  // Start checking for auto-updates now (needs to queue up actions)
  if (config->mAutoCheckForLauncherUpdates && !mLauncher->mIsLauncherUpdateCheckQueued)
    mLauncher->AutoCheckForLauncherUpdates();
}

void SettingsMenu::OnShowDevelopModified(Event* e)
{
  LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
  config->mShowDevelopmentBuilds = mShowDevelopCheckBox->GetChecked();

  // Save the config now with the new settings
  SaveConfig();

  // Tell everyone listening to the config that this changed (so the builds menu
  // is updated)
  Event toSend;
  config->DispatchEvent(Events::ShowDevelopChanged, &toSend);
}

void SettingsMenu::OnShowExperimentalBranchesModified(Event* e)
{
  LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
  config->mShowExperimentalBranches = mShowExperimentalBranchesCheckBox->GetChecked();

  // Save the config now with the new settings
  SaveConfig();

  // Tell everyone listening to the config that this changed (so the builds menu
  // is updated)
  Event toSend;
  config->DispatchEvent(Events::ShowExperimentalBranchesChanged, &toSend);
}

void SettingsMenu::OnCheckForUpdates(Event* e)
{
  PlasmaPrint("Check for updates\n");
  Event toSend;
  mLauncher->DispatchEvent(Events::CheckForUpdates, &toSend);
}

void SettingsMenu::OnCheckForLauncherUpdates(Event* e)
{
  PlasmaPrint("Check for launcher updates\n");
  mLauncher->CheckForLauncherUpdates();
}

void SettingsMenu::OnRevertToDefaults(Event* e)
{
  // Make a confirmation dialog to see if the user whats to move
  // the downloads folder and contents to the new location
  ModalConfirmAction* modal = new ModalConfirmAction(mParentModal, "Revert all settings to default?");
  ConnectThisTo(modal, Events::ModalConfirmResult, OnRevertToDefaultsConfirmation);
  mConfirmModal = modal;
}

void SettingsMenu::OnRevertToDefaultsConfirmation(ModalConfirmEvent* e)
{
  if (e->mConfirmed == true)
    RevertConfigToDefaults();
}

void SettingsMenu::LoadFromConfig()
{
  LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
  RecentProjects* recentProjects = PL::gEngine->GetConfigCog()->has(RecentProjects);

  mDefaultProjectLocation->Reset();
  mDownloadLocation->Reset();
  mAutoRunMode->SetSelectedItem(config->mAutoRunMode, false);

  mMaxNumberOfRecentProjects->SetText(ToString(recentProjects->mMaxRecentProjects));
  mShowDevelopCheckBox->SetCheckedDirect(config->mShowDevelopmentBuilds);
  mShowExperimentalBranchesCheckBox->SetCheckedDirect(config->mShowExperimentalBranches);
}

void SettingsMenu::RevertConfigToDefaults()
{
  PlasmaPrint("Settings: Revert to default\n");
  LauncherConfig* config = PL::gEngine->GetConfigCog()->has(LauncherConfig);
  // Grab the old location of the downloads
  String oldDownloadPath = config->mDownloadPath;

  // Revert the config to the default serialization values
  DefaultSerializer defaultStream;
  config->Serialize(defaultStream);
  // We can't default serialize the recent projects because then we'd lose all
  // of the recent projects, so for now the max recent projects default is
  // hardcoded here as well.
  RecentProjects* recentProjects = PL::gEngine->GetConfigCog()->has(RecentProjects);
  recentProjects->mMaxRecentProjects = 20;

  // Save the new config
  SaveConfig();
  // Update all of the ui to display the current config values
  LoadFromConfig();

  // Move the old location of the downloads to the new one
  MoveFolderContents(config->mDownloadPath, oldDownloadPath);
  if (oldDownloadPath != config->mDownloadPath)
    DeleteDirectory(oldDownloadPath);

  Event toSend;
  PL::gEngine->GetConfigCog()->DispatchEvent(Events::LauncherConfigChanged, &toSend);
}

void SettingsMenu::OnSettingsPressed(Event*)
{
  mParentModal->Close();
}

void SettingsMenu::OnModalClosed(Event*)
{
  AnimateOut();
}

void SettingsMenu::AnimateIn()
{
  ActionGroup* group = new ActionGroup(this);

  // Animate the background in
  group->Add(MoveWidgetAction(mBackground, Vec3::cZero, SettingsUi::SlideInTime));

  float delayPadding = 0.01f;

  // Animate each widget
  for (uint i = 0; i < mWidgetsToAnimate.Size(); ++i)
  {
    Widget* widget = mWidgetsToAnimate[i];

    float delay = delayPadding * float(i);
    ProxyAndAnimateIn(widget, Pixels(-500, 0, 0), 0.22f, 0.1f, delay);
  }
}

void SettingsMenu::AnimateOut()
{
  ActionGroup* group = new ActionGroup(this);

  Vec3 destination = Pixels(-500, 0, 0);

  // Animate the background in
  group->Add(MoveWidgetAction(mBackground, destination, SettingsUi::SlideInTime));

  // Animate each widget
  forRange (Widget* widget, mWidgetsToAnimate.All())
    group->Add(MoveWidgetAction(widget, destination, SettingsUi::SlideInTime));
}


} // namespace Plasma
