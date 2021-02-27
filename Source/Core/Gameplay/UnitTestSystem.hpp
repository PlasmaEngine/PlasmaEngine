// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

// Widgets
class WidgetChildId
{
public:
  WidgetChildId();
  void Serialize(Serializer& stream);

  String mName;
  BoundType* mType;
  uint mIndex;
};

class WidgetPath
{
public:
  WidgetPath();
  WidgetPath(Widget* toWidget, RootWidget* fromRoot);

  void Serialize(Serializer& stream);
  Widget* Resolve(RootWidget* root);

  Array<WidgetChildId> mPath;
};

class UnitTestSystem;

// Unit Test Events
class UnitTestEvent : public ILightningObject
{
public:
  LightningDeclareType(UnitTestEvent, TypeCopyMode::ReferenceType);

  virtual void Serialize(Serializer& stream) = 0;
  virtual ~UnitTestEvent();
  virtual void Execute(UnitTestSystem* system) = 0;
};

class UnitTestEndFrameEvent : public UnitTestEvent
{
public:
  LightningDeclareType(UnitTestEndFrameEvent, TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;
};

class UnitTestBaseMouseEvent : public UnitTestEvent
{
public:
  LightningDeclareType(UnitTestBaseMouseEvent, TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;

  WidgetPath mWidgetPath;
  Vec2 mNormalizedWidgetOffset;
};

class UnitTestMouseEvent : public UnitTestBaseMouseEvent
{
public:
  LightningDeclareType(UnitTestMouseEvent, TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  OsMouseEvent mEvent;
};

class UnitTestMouseDropEvent : public UnitTestBaseMouseEvent
{
public:
  LightningDeclareType(UnitTestMouseDropEvent, TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  OsMouseDropEvent mEvent;
  uint mFileIndex;
};

class UnitTestKeyboardEvent : public UnitTestEvent
{
public:
  LightningDeclareType(UnitTestKeyboardEvent, TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  KeyboardEvent mEvent;
};

class UnitTestKeyboardTextEvent : public UnitTestEvent
{
public:
  LightningDeclareType(UnitTestKeyboardTextEvent, TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  KeyboardTextEvent mEvent;
};

class UnitTestWindowEvent : public UnitTestEvent
{
public:
  LightningDeclareType(UnitTestWindowEvent, TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  OsWindowEvent mEvent;
};

DeclareEnum5(UnitTestMode, Stopped, StartRecording, Recording, StartPlaying, Playing);

class UnitTestSystem : public System, public OsShellHook, public OsInputHook
{
public:
  LightningDeclareType(UnitTestSystem, TypeCopyMode::ReferenceType);

  UnitTestSystem();

  // System interface
  cstr GetName();
  void Initialize(SystemInitializer& initializer) override;

  // OsShellHook interface
  void HookUpdate();

  void RecordToPlasmaTestFile();
  void RecordToPlasmaTestFile(StringParam plasmaTestFile);

  void PlayFromPlasmaTestFile();
  void PlayFromPlasmaTestFile(StringParam plasmaTestFile);

  // These should only be called while systems are NOT updating, especially not
  // the OsShell systems Calling these during the OsShell would result in loss
  // of inputs / missed events
  void SubProcessRecord();
  void SubProcessPlay();
  OsWindow* SubProcessSetupWindow();

  static void DiffDirectories(StringParam dir1, StringParam dir2, StringParam diffProgram = "tortoisemerge");
  static void EnumerateFiles(StringParam directory, StringParam relativeParentPath, HashSet<String>* relativePaths);

  RootWidget* GetRootWidget();
  OsWindow* GetMainWindow();

  void OnUnitTestRecordFileSelected(OsFileSelection* event);
  void OnUnitTestPlayFileSelected(OsFileSelection* event);

  // Internals
  Widget* mEmulatedCursor;
  UnitTestMode::Enum mMode;
  Array<UnitTestEvent*> mEvents;

  void RecordBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event);
  void ExecuteBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event);

  void RecordFile(StringParam file);
  String GetRecordedFile(uint fileIndex, StringParam fileName);
  void RecordEvent(UnitTestEvent* e);
  void LoadRecordedEvents();

  // InputHook interface
  void HookKeyboardEvent(KeyboardEvent& event) override;
  void HookKeyboardTextEvent(KeyboardTextEvent& event) override;
  void HookMouseEvent(OsMouseEvent& event) override;
  void HookMouseDropEvent(OsMouseDropEvent& event) override;
  void HookWindowEvent(OsWindowEvent& event) override;

  // The file we store our recorded data
  String mRecordedEventsFile;
  String mRecordedFilesDirectory;
  uint mEventIndex;
  File mPlaybackFile;
  uint mFilesIndex;
};

UnitTestSystem* CreateUnitTestSystem();

} // namespace Plasma
