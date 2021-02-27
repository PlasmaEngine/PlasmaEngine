// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

namespace Events
{
DeclareEvent(BuildSelected);
DeclareEvent(BuildStateChanged);
} // namespace Events

class LauncherBuildEvent : public Event
{
public:
  LightningDeclareType(LauncherBuildEvent, TypeCopyMode::ReferenceType);

  PlasmaBuild* mBuild;
};

class BuildStatusView : public Composite
{
public:
  /// Typedefs.
  typedef BuildStatusView LightningSelf;

  /// Constructor.
  BuildStatusView(Composite* parent, PlasmaBuild* version = nullptr);

  void UpdateTransform() override;

  /// Sets the current version to display and connects to the
  /// build to automatically update the build state.
  void SetVersion(PlasmaBuild* version);
  /// Sets the current version to display. This should only be called when the
  /// build id is being set for a project without a backing version;
  void SetVersion(const BuildId& buildId);

  /// Updates the install state text.
  void UpdateInstallState();

  /// When the install state of the build has changed, we want to update the
  /// text.
  void OnBuildStateChanged(Event*);

  Text* mBuildText;
  Text* mInstallState;

  PlasmaBuild* mBuild;
};

class BuildList : public Composite
{
public:
  /// Typedefs.
  typedef BuildList LightningSelf;

  /// Constructor.
  BuildList(Composite* parent, VersionSelector* versionSelector, PlasmaBuild* selected, bool installedOnly);

  /// Composite Interface.
  void UpdateTransform() override;
  bool TakeFocusOverride() override;

  /// Updates the install state text.
  void OnMouseMove(MouseEvent* e);
  void OnMouseExitScrollArea(MouseEvent* e);

  /// Selects the displayed build.
  void OnLeftClick(MouseEvent* e);

  void OnInstallLatest(Event*);

  /// Returns the index of the entry that the mouse position is over.
  int IndexFromPosition(Vec2Param localPosition);

  /// All build items will be attached to the scroll area.
  ScrollArea* mScrollArea;

  /// The currently selected build.
  PlasmaBuild* mSelected;

  Element* mBorder;
  Element* mBackground;

  /// All builds to be selected from.
  typedef Pair<Element*, BuildStatusView*> Entry;
  Array<Entry> mEntries;

  VersionSelector* mVersionSelector;
};

class BuildSelector : public Composite
{
public:
  /// Typedefs.
  typedef BuildSelector LightningSelf;

  /// Constructor.
  BuildSelector(Composite* parent, VersionSelector* versionSelector, PlasmaBuild* version = nullptr);

  /// Composite Interface.
  void UpdateTransform() override;

  /// Sets the current version to display.
  void SetBuild(PlasmaBuild* version);
  /// This version should only be called when we're setting the build for a
  /// project where there's no backing build
  void SetBuild(const BuildId& buildId);

  /// Returns the currently selected version.
  PlasmaBuild* GetBuild();

  /// Event Response.
  void OnMouseEnter(Event*);
  void OnMouseExit(Event*);

  /// When the left mouse goes up, create the build list to select from.
  void OnLeftMouseDown(MouseEvent* e);

  /// Update the selected build from the build list.
  void OnBuildSelected(LauncherBuildEvent* e);
  void OnListFocusLost(FocusEvent* e);
  void OnListFocusReset(Event*);

  /// Closes the build list popup.
  void CloseBuildList();

  void SetHighlight(bool state);
  void ToColor0();
  void ToColor1();

  /// Allows only the selection of installed builds.
  bool mInstalledOnly;

  /// The currently displayed version.
  BuildStatusView* mCurrentBuild;

  /// An arrow to the right of the current build.
  Element* mArrow;

  /// When the build list is opened, there will be a background
  Element* mBackground;
  Element* mBorder;

  /// Handle to the list of builds that the user is selecting.
  HandleOf<BuildList> mBuildList;

  VersionSelector* mVersionSelector;
};

namespace BuildColors
{
DeclareTweakable(Vec4, Installed);
DeclareTweakable(Vec4, NotInstalled);
DeclareTweakable(Vec4, Deprecated);
} // namespace BuildColors

} // namespace Plasma
