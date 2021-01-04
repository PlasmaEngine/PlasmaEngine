// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Ranges
LightningDefineRange(HierarchyNameRange);
LightningDefineRange(HierarchyListNameRange);
LightningDefineRange(HierarchyRange);
LightningDefineRange(CogNameRange);
LightningDefineRange(CogRootNameRange);
LightningDefineRange(HierarchyList::range);
LightningDefineRange(HierarchyList::reverse_range);
LightningDefineRange(Space::range);
LightningDefineRange(SpaceMap::valueRange);
LightningDefineRange(ObjectLinkRange);
LightningDefineRange(JoystickDeviceRange);
LightningDefineRange(CogHashSetRange);
LightningDefineRange(ResourceTableEntryList::range);
LightningDefineRange(OperationListRange);
LightningDefineRange(Engine::GameSessionArray::range);

// Enums
LightningDefineEnum(ActionExecuteMode);
LightningDefineEnum(ActionState);
LightningDefineEnum(AnimationBlendMode);
LightningDefineEnum(AnimationBlendType);
LightningDefineEnum(AnimationPlayMode);
LightningDefineEnum(Buttons);
LightningDefineEnum(CogPathPreference);
LightningDefineEnum(Cursor);
LightningDefineEnum(EaseType);
LightningDefineEnum(FlickedStick);
LightningDefineEnum(InputDevice);
LightningDefineEnum(KeyState);
LightningDefineEnum(LauncherAutoRunMode);
LightningDefineEnum(Math::CurveType);
LightningDefineEnum(MouseButtons);
LightningDefineEnum(ProgressType);
LightningDefineEnum(SplineType);
LightningDefineEnum(StoreResult);
LightningDefineEnum(StreamType);
LightningDefineEnum(TabWidth);
LightningDefineEnum(TimeMode);
LightningDefineEnum(WindowState);
LightningDefineEnum(WindowStyleFlags);
LightningDefineEnum(GeometryValue);

void LocationBind(LibraryBuilder& builder, BoundType* type)
{
  // We need to alias LightningSelf for the method binding macros
  namespace LightningSelf = Location;

  LightningBindMethod(IsCardinal);
  LightningBindMethod(GetCardinalAxis);
  LightningBindOverloadedMethod(GetDirection, LightningStaticOverload(Vec2, Location::Enum));
  LightningBindOverloadedMethod(GetDirection, LightningStaticOverload(Vec2, Location::Enum, Location::Enum));
  LightningBindMethod(GetOpposite);
}

LightningDefineExternalBaseType(Location::Enum, TypeCopyMode::ValueType, builder, type)
{
  LightningFullBindEnum(builder, type, SpecialType::Enumeration);
  LightningBindEnumValues(Location);

  LocationBind(builder, type);
}

// Arrays
PlasmaDefineArrayType(Array<ContentLibraryReference>);

// The keys enum has to be declared special since it skips values
LightningDefineExternalBaseType(Keys::Enum, TypeCopyMode::ValueType, builder, type)
{
  SetUpKeyNames();
  LightningFullBindEnum(builder, type, SpecialType::Enumeration);

  // For now, just iterate over all keys in the name map and if there was no
  // saved name then assume that the key doesn't exist (linear but whatever)
  for (size_t i = 0; i < Keys::Size; ++i)
  {
    if (KeyNames[i] == nullptr)
      continue;

    LightningFullBindEnumValue(builder, type, i, KeyNames[i]);
  }
}

LightningDefineStaticLibrary(EngineLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  LightningInitializeRange(HierarchyNameRange);
  LightningInitializeRange(HierarchyListNameRange);
  LightningInitializeRange(HierarchyRange);
  LightningInitializeRange(CogNameRange);
  LightningInitializeRange(CogRootNameRange);
  LightningInitializeRangeAs(HierarchyList::range, "HierarchyListRange");
  LightningInitializeRangeAs(HierarchyList::reverse_range, "HierarchyListReverseRange");
  LightningInitializeRangeAs(Space::range, "SpaceRange");
  LightningInitializeRangeAs(SpaceMap::valueRange, "SpaceMapValueRange");
  LightningInitializeRange(ObjectLinkRange);
  LightningInitializeRangeAs(JoystickDeviceRange, "JoystickRange");
  LightningInitializeRange(CogHashSetRange);
  LightningInitializeRangeAs(ResourceTableEntryList::range, "ResourceTableEntryRange");
  LightningInitializeRange(OperationListRange);
  LightningInitializeRangeAs(Engine::GameSessionArray::range, "GameSessionRange");

  // Enums
  LightningInitializeEnum(ActionExecuteMode);
  LightningInitializeEnum(ActionState);
  LightningInitializeEnum(AnimationBlendMode);
  LightningInitializeEnum(AnimationBlendType);
  LightningInitializeEnum(AnimationPlayMode);
  LightningInitializeEnum(Buttons);
  LightningInitializeEnum(CogPathPreference);
  LightningInitializeEnum(Cursor);
  LightningInitializeEnumAs(EaseType, "Ease");
  LightningInitializeEnum(FlickedStick);
  LightningInitializeEnum(InputDevice);
  LightningInitializeEnum(Keys);
  LightningInitializeEnum(KeyState);
  LightningInitializeEnum(LauncherAutoRunMode);
  LightningInitializeEnum(Location);
  LightningInitializeEnumAs(Math::CurveType, "CurveType");
  LightningInitializeEnum(MouseButtons);
  LightningInitializeEnum(ProgressType);
  LightningInitializeEnum(SplineType);
  LightningInitializeEnum(StoreResult);
  LightningInitializeEnum(StreamType);
  LightningInitializeEnum(TabWidth);
  LightningInitializeEnum(TimeMode);
  LightningInitializeEnum(WindowState);
  LightningInitializeEnum(WindowStyleFlags);
  LightningInitializeEnum(GeometryValue);

  // Arrays
  PlasmaInitializeArrayTypeAs(Array<ContentLibraryReference>, "ContentLibraryReferenceArray");
  LightningInitializeType(DataSource);

  LightningInitializeType(System);

  // Meta Components
  LightningInitializeType(TransformMetaTransform);
  LightningInitializeType(CogMetaComposition);
  LightningInitializeType(CogMetaDataInheritance);
  LightningInitializeType(CogMetaDisplay);
  LightningInitializeType(CogMetaSerialization);
  LightningInitializeType(CogMetaOperations);
  LightningInitializeType(CogMetaTransform);
  LightningInitializeType(CogArchetypeExtension);
  LightningInitializeType(CogSerializationFilter);
  LightningInitializeType(CogPathMetaSerialization);
  LightningInitializeType(ComponentMetaDataInheritance);
  LightningInitializeType(DataResourceInheritance);
  LightningInitializeType(ResourceMetaSerialization);
  LightningInitializeType(EngineMetaComposition);
  LightningInitializeType(HierarchyComposition);
  LightningInitializeType(MetaResource);
  LightningInitializeType(ComponentMetaOperations);
  LightningInitializeType(ResourceMetaOperations);
  LightningInitializeType(CogArchetypePropertyFilter);
  LightningInitializeType(CogPathMetaComposition);
  LightningInitializeType(MetaEditorScriptObject);
  LightningInitializeType(MetaDependency);
  LightningInitializeType(MetaInterface);
  LightningInitializeType(RaycasterMetaComposition);

  // Events
  LightningInitializeType(CogPathEvent);
  LightningInitializeType(UpdateEvent);
  LightningInitializeType(ResourceEvent);
  LightningInitializeType(InputDeviceEvent);
  LightningInitializeType(GameEvent);
  LightningInitializeType(AnimationGraphEvent);
  LightningInitializeType(KeyboardEvent);
  LightningInitializeType(FileEditEvent);
  LightningInitializeType(KeyboardTextEvent);
  LightningInitializeType(OsMouseEvent);
  LightningInitializeType(HierarchyEvent);
  LightningInitializeType(JoystickEvent);
  LightningInitializeType(CogInitializerEvent);
  LightningInitializeType(ObjectLinkEvent);
  LightningInitializeType(ObjectLinkPointChangedEvent);
  LightningInitializeType(HeightMapEvent);
  LightningInitializeType(AreaEvent);
  LightningInitializeType(GamepadEvent);
  LightningInitializeType(OperationQueueEvent);
  LightningInitializeType(OsWindowEvent);
  LightningInitializeType(OsWindowBorderHitTest);
  LightningInitializeType(OsMouseDropEvent);
  LightningInitializeType(SavingEvent);
  LightningInitializeType(ScriptEvent);
  LightningInitializeType(DataEvent);
  LightningInitializeType(DataReplaceEvent);
  LightningInitializeType(CogReplaceEvent);
  LightningInitializeType(TextEvent);
  LightningInitializeType(TextErrorEvent);
  LightningInitializeType(ProgressEvent);
  LightningInitializeType(OsFileSelection);
  LightningInitializeType(ClipboardEvent);
  LightningInitializeType(LightningCompiledEvent);
  LightningInitializeType(LightningCompileFragmentEvent);
  LightningInitializeType(LightningCompileEvent);
  LightningInitializeType(SplineEvent);
  LightningInitializeType(DebugViewEvent);
  LightningInitializeType(BlockingTaskEvent);
  LightningInitializeType(AsyncProcessEvent);

  // Components
  LightningInitializeType(Component);
  LightningInitializeType(Transform);
  LightningInitializeType(Hierarchy);
  LightningInitializeType(TimeSpace);
  LightningInitializeType(ObjectLink);
  LightningInitializeType(ObjectLinkAnchor);
  LightningInitializeType(Hierarchy);
  LightningInitializeType(AnimationGraph);
  LightningInitializeType(SimpleAnimation);
  LightningInitializeType(HeightMap);
  LightningInitializeType(Area);
  LightningInitializeType(AnimationGraph);
  LightningInitializeType(ProjectSettings);
  LightningInitializeType(ProjectDescription);

  LightningInitializeType(Cog);
  LightningInitializeType(Space);
  LightningInitializeType(ResourceDisplayFunctions);
  LightningInitializeType(Resource);
  LightningInitializeType(DataResource);
  LightningInitializeType(ResourceSystem);
  LightningInitializeType(ResourcePackageDisplay);
  LightningInitializeType(ResourcePackage);
  LightningInitializeType(ResourceLibrary);

  LightningInitializeType(CogPath);

  LightningInitializeType(Engine);
  LightningInitializeType(GameSession);

  LightningInitializeType(AnimationNode);
  LightningInitializeType(PoseNode);
  LightningInitializeType(BasicAnimation);
  LightningInitializeType(DualBlend<DirectBlend>);
  LightningInitializeType(DirectBlend);
  LightningInitializeType(DualBlend<CrossBlend>);
  LightningInitializeType(CrossBlend);
  LightningInitializeType(DualBlend<SelectiveNode>);
  LightningInitializeType(SelectiveNode);
  LightningInitializeType(DualBlend<ChainNode>);
  LightningInitializeType(ChainNode);
  LightningInitializeType(ObjectTrack);
  LightningInitializeType(Animation);
  LightningInitializeType(Environment);

  LightningInitializeType(Keyboard);

  LightningInitializeType(System);
  LightningInitializeType(TimeSystem);
  LightningInitializeType(OsShell);
  LightningInitializeType(OsWindow);
  LightningInitializeType(Mouse);
  LightningInitializeType(Factory);
  LightningInitializeType(Operation);
  LightningInitializeType(OperationQueue);
  LightningInitializeType(OperationBatch);
  LightningInitializeType(PropertyOperation);
  LightningInitializeType(Tracker);
  LightningInitializeType(Spline);
  LightningInitializeType(SplineSampleData);
  LightningInitializeType(SplineBakedPoints);
  LightningInitializeType(SplineBakedPoint);
  LightningInitializeType(SplineControlPoints);
  LightningInitializeType(SplineControlPoint);
  LightningInitializeType(AsyncProcess);

  LightningInitializeType(Action);
  LightningInitializeType(ActionSet);
  LightningInitializeType(Actions);
  LightningInitializeType(ActionGroup);
  LightningInitializeType(ActionSequence);
  LightningInitializeType(ActionSpace);
  LightningInitializeType(ActionDelay);

  LightningInitializeType(CogInitializer);

  LightningInitializeType(Thickness);
  LightningInitializeType(Rectangle);

  LightningInitializeType(LinkId);
  LightningInitializeType(Named);
  LightningInitializeType(EditorFlags);
  LightningInitializeType(SpaceObjects);
  LightningInitializeType(Archetype);
  LightningInitializeType(Archetyped);
  LightningInitializeType(ArchetypeInstance);

  LightningInitializeType(MetaSelection);

  LightningInitializeType(ObjectLinkEdge);
  LightningInitializeType(ObjectLinkAnchor);
  LightningInitializeType(ObjectLink);

  LightningInitializeType(Level);

  LightningInitializeType(DebugDraw);

  LightningInitializeType(DocumentResource);
  LightningInitializeType(TextBlock);

  LightningInitializeType(MainConfig);
  LightningInitializeType(EditorConfig);
  LightningInitializeType(WindowLaunchSettings);
  LightningInitializeType(FrameRateSettings);
  LightningInitializeType(DebugSettings);
  LightningInitializeType(ExportSettings);
  LightningInitializeType(ContentConfig);
  LightningInitializeType(UserConfig);
  LightningInitializeType(DeveloperConfig);
  LightningInitializeType(LightningPluginConfig);
  LightningInitializeType(TextEditorConfig);
  LightningInitializeType(RecentProjects);
  LightningInitializeType(EditorSettings);
  LightningInitializeType(LauncherConfig);
  LightningInitializeType(LauncherLegacySettings);

  LightningInitializeType(HierarchySpline);

  LightningInitializeType(ObjectStore);
  LightningInitializeType(ResourceTable);
  LightningInitializeType(ResourceTableEntry);

  LightningInitializeType(SampleCurve);

  LightningInitializeType(HeightMap);
  LightningInitializeType(HeightPatch);
  LightningInitializeType(HeightMapSource);

  LightningInitializeType(SceneGraphSource);

  LightningInitializeType(ColorGradient);

  LightningInitializeType(Area);

  LightningInitializeType(ProjectSettings);
  LightningInitializeType(ContentLibraryReference);
  LightningInitializeType(SharedContent);
  LightningInitializeType(ProjectDescription);

  LightningInitializeType(RaycastProvider);
  LightningInitializeType(Raycaster);

  LightningInitializeType(Gamepad);
  LightningInitializeType(Gamepads);

  LightningInitializeType(Tweakables);

  LightningInitializeType(RawControlMapping);
  LightningInitializeType(Joystick);
  LightningInitializeType(Joysticks);

  LightningInitializeType(EventDirectoryWatcher);
  LightningInitializeType(Job);
  LightningInitializeType(DocumentationLibrary);
  LightningInitializeType(Shortcuts);
  LightningInitializeTypeAs(ProxyObject<Component>, "ComponentProxy");

  if (GetApplicationName() != sLauncherName)
    LightningInitializeTypeAs(LauncherProjectInfoProxy, "LauncherProjectInfo");

  LightningInitializeType(LightningLibraryResource);
  LightningInitializeType(LightningDocumentResource);

  BindActionFunctions(builder);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

bool EngineLibrary::Initialize()
{
  // Build meta
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  RegisterClassAttribute(ObjectAttributes::cRunInEditor)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cCommand, MetaEditorScriptObject)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cTool, MetaEditorScriptObject)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cGizmo, MetaEditorGizmo)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cComponentInterface, MetaInterface)->TypeMustBe(Component);

  RegisterPropertyAttributeType(PropertyAttributes::cDependency, MetaDependency)->TypeMustBe(Component);
  RegisterPropertyAttributeType(PropertyAttributes::cResourceProperty, MetaEditorResource)->TypeMustBe(Resource);

  PlasmaPrintFilter(Filter::DefaultFilter, "Engine Initialize...\n");

  EngineObject::sEngineHeap = new Memory::Heap("Engine", Memory::GetRoot());

  UndoMap::Initialize();

  StartSystemObjects();

  // Could be moved to Platform
  Environment* environment = Environment::GetInstance();
  CrashHandler::SetRestartCommandLine(environment->mCommandLine);

  // Allow the user to specify an extra log file (use 2 different log files so
  // the crash reporter works and because we need to start system objects for
  // this one in order to get the log file)
  String logFile = environment->GetParsedArgument("log");
  FileListener extraListener;
  if (!logFile.Empty())
  {
    extraListener.OverrideLogFile(logFile);
    Plasma::Console::Add(&extraListener);
  }

  // Uncomment out this line to disable the fpu exceptions
  // FpuControlSystem::Active = false;

  // Load the debug drawer.
  Debug::DebugDraw::Initialize();

  // Resource System
  ResourceSystem::Initialize();
  PL::gSystemObjects->Add(PL::gResources, ObjectCleanup::None);

  // Core resource Managers
  InitializeResourceManager(ArchetypeManager);
  InitializeResourceManager(LevelManager);
  InitializeResourceManager(AnimationManager);
  InitializeResourceManager(CurveManager);
  InitializeResourceManager(ResourceTableManager);
  InitializeResourceManager(ColorGradientManager);
  InitializeResourceManager(TextBlockManager);
  InitializeResourceManager(HeightMapSourceManager);

  // Create the engine.
  Engine* engine = new Engine();

  // This must be called right after the engine is created because it connects
  // to the engine.
  StartThreadSystem();

  Keyboard::Initialize();
  Mouse::Initialize();
  Gamepads::Initialize();
  Joysticks::Initialize();
  LocalModifications::Initialize();
  ObjectStore::Initialize();
  // Need to initialize lightning here as it can be used in the factory below.
  LightningManager::Initialize();

  Space* engineSpace = new Space();
  engineSpace->SetName("EngineSpace");
  CogInitializer init(nullptr);
  engineSpace->Initialize(init);
  init.AllCreated();

  engine->mEngineSpace = engineSpace;

  // Create the factory and Tracker for object creation.
  Tracker* tracker = Tracker::StaticInitialize();
  engine->AddSystemInterface(LightningTypeId(Tracker), tracker);

  Factory* factory = Factory::StaticInitialize(engine, tracker);
  engine->AddSystemInterface(LightningTypeId(Factory), factory);

  MetaDatabase::GetInstance()->AddAlternateName("Project", LightningTypeId(ProjectSettings));
  return true;
}

void EngineLibrary::Shutdown()
{
  PlasmaPrintFilter(Filter::DefaultFilter, "Shutdown Engine...\n");

  Engine* engine = PL::gEngine;

  // Clear map and all handles before final deletion of objects
  UndoMap::Shutdown();

  // Clear references to any default resources held by meta properties
  // Must be done before shutting down the resource system
  MetaDatabase::GetInstance()->ReleaseDefaults();

  // Shutdown background workers and threads
  // before all systems since most code assume
  // systems are never deleted.
  ShutdownThreadSystem();

  ShutdownContentSystem();

  ObjectStore::Destroy();
  LocalModifications::Destroy();
  Joysticks::Destroy();
  Gamepads::Destroy();
  Mouse::Destroy();
  Keyboard::Destroy();

  // Unload all resource and destroy all resource managers
  PL::gResources->UnloadAll();
  delete PL::gResources;

  CleanUpSystemObjects();

  CleanUpErrorContext();

  Debug::DebugDraw::Shutdown();
  engine->DestroySystems();
  delete engine;

  SafeDelete(PL::gTracker);
  SafeDelete(PL::gFactory);
  SafeDelete(PL::gTweakables);

  GetLibrary()->ClearComponents();
}

} // namespace Plasma
