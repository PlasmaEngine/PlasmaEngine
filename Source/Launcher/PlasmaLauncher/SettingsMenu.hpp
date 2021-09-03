// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

namespace Events
{
DeclareEvent(LauncherConfigChanged);
} // namespace Events

// fwd declare
class StringChangeEvent;
class FolderLocation;

/// The various configuration settings for the launcher.
class SettingsMenu : public Composite
{
public:
  /// Typedefs.
  typedef SettingsMenu LightningSelf;

  /// Constructor.
  SettingsMenu(Modal* parent, LauncherWindow* launcher);

  /// Composite Interface.
  void UpdateTransform() override;

  void OnActivated(Event* e);
  void OnDeactivated(Event* e);

  void OnDefaultProjectLocationChange(StringChangeEvent* e);

  void OnDownloadLocationTextSubmit(StringChangeEvent* e);
  void OnConfirmChangeDownloadLocation(ModalConfirmEvent* e);
  void MoveDownloadLocation(StringChangeEvent* e);

  void OnAutoRunModeSelected(Event* e);
  void OnMaxRecentProjectsModified(Event* e);
  void OnAutoCheckForMajorUpdatesModified(Event* e);
  void OnShowDevelopModified(Event* e);
  void OnShowExperimentalBranchesModified(Event* e);


  void OnCheckForUpdates(Event* e);
  void OnCheckForLauncherUpdates(Event* e);
  void OnRevertToDefaults(Event* e);
  void OnRevertToDefaultsConfirmation(ModalConfirmEvent* e);

  /// Update all of the ui to display the current config values
  void LoadFromConfig();
  void RevertConfigToDefaults();

  void OnSettingsPressed(Event*);	
  void OnModalClosed(Event*);

  void AnimateIn();
  void AnimateOut();

  /// All widgets, in top down order that need to be animated in.
  Array<Widget*> mWidgetsToAnimate;

  Element* mModalBackground;
  Element* mBackground;
  ComboBox* mAutoRunMode;
  FolderLocation* mDefaultProjectLocation;
  FolderLocation* mDownloadLocation;
  LauncherWindow* mLauncher;
  TextBox* mMaxNumberOfRecentProjects;
  CheckBox* mAutoCheckForLauncherUpdatesCheckBox;
  CheckBox* mShowDevelopCheckBox;
  CheckBox* mShowExperimentalBranchesCheckBox;
  /// Handle to the modal so that we can remove it if the page switches.
  HandleOf<ModalConfirmAction> mConfirmModal;
  HandleOf<ToolTip> mToolTip;
  Modal* mParentModal;
};

} // namespace Plasma
