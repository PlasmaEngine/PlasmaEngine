// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

class BuildItem : public Composite
{
public:
  /// Typedefs.
  typedef BuildItem LightningSelf;

  /// Constructor.
  BuildItem(Composite* parent, PlasmaBuild* version, BuildsMenu* buildsMenu);

  /// Special handle when we're being destroyed to disconnect events
  void OnDestroy() override;

  /// Composite Interface.
  void UpdateTransform() override;

  /// Installs the build.
  void Install();
  void OnInstallStarted(Event* e);

  /// Uninstall the build.
  void Uninstall();
  /// Internal uninstall function that actually does the uninstalling (shouldn't
  /// be called directly unless all relevant modals have already been passed).
  void UninstallBuildInternal();
  void OnUninstallModalResult(ModalConfirmEvent* e);
  /// Create a modal notifying the user that a build has instances running.
  void CreateBuildsRunningModal();
  /// Get the result from the user about how they want to deal with instances of
  /// a build running.
  void OnBuildRunningModalResult(ModalButtonEvent* e);

  /// Download Event Response.
  void OnDownloadUpdate(BackgroundTaskEvent* e);
  void OnDownloadCompleted(BackgroundTaskEvent* e);
  void OnUninstallCompleted(BackgroundTaskEvent* e);
  void OnDownloadFailed(BackgroundTaskEvent* e);

  /// Mouse Event Response.
  void OnMouseEnter(MouseEvent* e);
  void OnLeftClick(MouseEvent* e);
  void OnRightClick(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);

  void SetSelected(bool state);

  /// Updates the installed state text based on whether or not we're installed.
  void UpdateInstallState();

  Text* mBuildVersion;
  Text* mInstallState;
  Text* mTags;
 // Text* mReleaseDate;

  /// Whether or not this build version is selected.
  bool mSelected;

  /// Displays selection state and mouse interaction.
  Element* mBackground;
  Vec4 mBackgroundColor;

  /// The progress bar that appears when downloading the build.
  ProgressBar* mDownloadProgress;

  /// The build version we're representing.
  PlasmaBuild* mVersion;
  VersionSelector* mVersionSelector;
  BuildsMenu* mBuildsMenu;
  HandleOf<ToolTip> mToolTip;
};

class ReleaseNotes : public Composite
{
public:
  /// Typedefs.
  typedef ReleaseNotes LightningSelf;

  /// Constructor.
  ReleaseNotes(Composite* parent);

  /// Composite Interface.
  void UpdateTransform() override;

  /// Displays the data of the given build.
  void DisplayReleaseNotes(PlasmaBuild* build);

  /// When the build install state changes, we want to update the buttons.
  void OnSelectedBuildChanged(Event*);

  /// Updates whether or not the download and uninstall buttons are visible
  /// based on the selected builds install state.
  void UpdateBuildButtons();

  Text* mTitle;
  Text* mBuildVersion;
  Text* mNote;
  TextEditor* mReleaseNotes;
  IconButton* mDownloadButton;
  IconButton* mUninstallButton;
  PlasmaBuild* mSelectedBuild;
};

class BuildsMenu : public Composite
{
public:
  /// Typedefs.
  typedef BuildsMenu LightningSelf;

  /// Constructor.
  BuildsMenu(Composite* parent, LauncherWindow* launcher);

  /// Composite Interface.
  void UpdateTransform() override;
  /// Delete all old BuildItems and then create all new ones (with the current
  /// filtering)
  void CreateBuildItems();
  /// Event Response.
  void OnVersionListLoaded(Event* e);
  void OnBuildListUpdated(Event* e);
  void OnShowDevelopChanged(Event* e);
  void OnCheckForUpdates(Event* e);
  void OnBuildSelected(ObjectEvent* e);
  void SelectBuild(BuildItem* build);

  /// When this menu is activated / deactivated, we want to change the
  /// launch buttons functionality.
  void OnMenuDisplayed(Event* e);
  void OnMenuHidden(Event* e);

  /// When the install button is pressed, we want to install the currently
  /// selected build.
  void OnInstallPressed(Event* e);
  void OnUninstallPressed(Event* e);
  void OnSearchChanged(Event* e);

  /// Updates the state of the install button based on the selected build.
  void UpdateInstallButton();

  /// Finds the build item for the associated build if it exists.
  BuildItem* FindBuildItem(BuildId& buildId);

  /// All displayed build items.
  Array<BuildItem*> mBuildItems;

  /// The currently selected build.
  HandleOf<BuildItem> mSelectedBuild;

  /// Used to display all the available builds.
  ScrollArea* mBuildList;

  /// Displays information about the currently selected build.
  ReleaseNotes* mReleaseNotes;

  /// We need access back to the main launcher for the main button.
  LauncherWindow* mLauncher;

  /// Handle to the modal so that we can remove it if the page switches.
  HandleOf<Modal> mUninstallModal;
};

} // namespace Plasma
