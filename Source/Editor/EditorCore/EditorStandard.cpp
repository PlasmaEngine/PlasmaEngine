// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Enums
LightningDefineEnum(GizmoGrab);
LightningDefineEnum(GizmoBasis);
LightningDefineEnum(GizmoPivot);
LightningDefineEnum(UpdateMode);
LightningDefineEnum(IncludeMode);
LightningDefineEnum(GizmoDragMode);
LightningDefineEnum(GizmoGrabMode);
LightningDefineEnum(GizmoSnapMode);
LightningDefineEnum(DockArea);
LightningDefineEnum(EditorMode);
LightningDefineEnum(MultiConvexMeshDrawMode);
LightningDefineEnum(MultiConvexMeshSnappingMode);
LightningDefineEnum(MultiConvexMeshAutoComputeMode);
LightningDefineEnum(MultiConvexMeshAutoComputeMethod);
LightningDefineEnum(ControlMode);
LightningDefineEnum(CameraDragMode);
LightningDefineEnum(Placement);
LightningDefineEnum(ArrowHeadType);
LightningDefineEnum(JointToolTypes);
LightningDefineEnum(TimeDisplay);
LightningDefineEnum(HeightTool);
LightningDefineEnum(HeightTextureSelect);
LightningDefineEnum(CellIndexType);
LightningDefineEnum(SpriteOrigin);
LightningDefineEnum(TileEditor2DSubToolType);
LightningDefineEnum(SpringSubTools);
LightningDefineEnum(ImportMode);
LightningDefineEnum(PlayGameOptions);
LightningDefineEnum(ImportFrames);

LightningDefineType(MetaCompositionWrapper, builder, type)
{
}

LightningDefineStaticLibrary(EditorLibrary)
{
  builder.CreatableInScriptDefault = false;

  // External Event Bindings
  PlasmaBindExternalEvent(Events::ToolActivate, Event, Cog);
  PlasmaBindExternalEvent(Events::ToolDeactivate, Event, Cog);
  PlasmaBindExternalEvent(Events::ToolDraw, Event, Cog);

  // Enums
  LightningInitializeEnum(GizmoGrab);
  LightningInitializeEnum(GizmoBasis);
  LightningInitializeEnum(GizmoPivot);
  LightningInitializeEnum(UpdateMode);
  LightningInitializeEnum(IncludeMode);
  LightningInitializeEnum(GizmoDragMode);
  LightningInitializeEnum(GizmoGrabMode);
  LightningInitializeEnum(GizmoSnapMode);
  LightningInitializeEnum(DockArea);
  LightningInitializeEnum(EditorMode);
  LightningInitializeEnum(MultiConvexMeshDrawMode);
  LightningInitializeEnum(MultiConvexMeshSnappingMode);
  LightningInitializeEnum(MultiConvexMeshAutoComputeMode);
  LightningInitializeEnum(MultiConvexMeshAutoComputeMethod);
  LightningInitializeEnum(ControlMode);
  LightningInitializeEnum(CameraDragMode);
  LightningInitializeEnum(Placement);
  LightningInitializeEnum(ArrowHeadType);
  LightningInitializeEnum(JointToolTypes);
  LightningInitializeEnum(TimeDisplay);
  LightningInitializeEnum(HeightTool);
  LightningInitializeEnum(HeightTextureSelect);
  LightningInitializeEnum(CellIndexType);
  LightningInitializeEnum(SpriteOrigin);
  LightningInitializeEnum(TileEditor2DSubToolType);
  LightningInitializeEnum(SpringSubTools);
  LightningInitializeEnum(ImportMode);
  LightningInitializeEnum(PlayGameOptions);
  LightningInitializeEnum(ImportFrames);

  // Structs
  LightningInitializeType(ImportJobProperties);

  // Events
  LightningInitializeType(LauncherCommunicationEvent);
  LightningInitializeType(BackgroundTaskEvent);
  LightningInitializeType(EditorEvent);
  LightningInitializeType(MetaDropEvent);
  LightningInitializeType(PostAddResourceEvent);
  LightningInitializeType(TreeEvent);
  LightningInitializeType(TreeViewHeaderAddedEvent);
  LightningInitializeType(ValueEvent);
  LightningInitializeType(ContextMenuEvent);
  LightningInitializeType(TileViewEvent);
  LightningInitializeType(CurveEvent);
  LightningInitializeType(TextUpdatedEvent);
  LightningInitializeType(ConsoleTextEvent);
  LightningInitializeType(MessageBoxEvent);
  LightningInitializeType(ColorEvent);
  LightningInitializeType(TextEditorEvent);
  LightningInitializeType(ObjectPollEvent);
  LightningInitializeType(GizmoEvent);
  LightningInitializeType(GizmoUpdateEvent);
  LightningInitializeType(GizmoRayTestEvent);
  LightningInitializeType(RingGizmoEvent);
  LightningInitializeType(TranslateGizmoUpdateEvent);
  LightningInitializeType(ScaleGizmoUpdateEvent);
  LightningInitializeType(RotateGizmoUpdateEvent);
  LightningInitializeType(ObjectTransformGizmoEvent);
  LightningInitializeType(RotationBasisGizmoAabbQueryEvent);
  LightningInitializeType(ToolGizmoEvent);
  LightningInitializeType(ManipulatorToolEvent);
  LightningInitializeType(SelectToolFrustumEvent);

  LightningInitializeType(BugReporter);
  LightningInitializeType(MetaPropertyEditor);
  LightningInitializeType(MetaCompositionWrapper);
  LightningInitializeType(BackgroundTasks);
  LightningInitializeType(StressTest);
  LightningInitializeType(GeneralSearchView);
  LightningInitializeTypeAs(CurveEditing::Draggable, "CurveDraggable");
  LightningInitializeTypeAs(CurveEditing::ControlPoint, "CurveControlPoint");
  LightningInitializeTypeAs(CurveEditing::Tangent, "CurveTangent");
  LightningInitializeType(Document);
  LightningInitializeType(DocumentManager);

  // Commands
  LightningInitializeType(Command);
  LightningInitializeType(CogCommand);
  LightningInitializeTypeAs(EditorScriptObjects<CogCommand>, "EditorScriptObjectsCogCommand");
  LightningInitializeType(CogCommandManager);

  // Data Editors
  LightningInitializeType(PropertyView);
  LightningInitializeType(FormattedInPlaceText);
  LightningInitializeType(InPlaceTextEditor);
  LightningInitializeType(ValueEditorFactory);
  LightningInitializeType(PreviewWidget);
  LightningInitializeType(PreviewWidgetFactory);
  LightningInitializeType(TileViewWidget);
  LightningInitializeType(TileView);
  LightningInitializeType(ItemList);
  LightningInitializeType(WeightedComposite);
  LightningInitializeType(ItemGroup);
  LightningInitializeType(Item);
  LightningInitializeType(ImportButton);

  // Content Importing
  LightningInitializeType(ContentPackage);

  // Editor Core
  LightningInitializeType(Editor);
  LightningInitializeType(EditorMain);
  LightningInitializeType(LauncherOpenProjectComposite);
  LightningInitializeType(LauncherSingletonCommunication);
  LightningInitializeType(LauncherDebuggerCommunication);
  LightningInitializeType(SimpleDebuggerListener);
  LightningInitializeType(MainPropertyView);

  // Editor Core
  LightningInitializeType(ColorScheme);
  LightningInitializeType(TextEditor);

  // Editor3D
  LightningInitializeType(GridDraw);
  LightningInitializeType(EditorCameraController);
  LightningInitializeType(EditorViewport);

  // Gizmos
  LightningInitializeType(Gizmo);
  LightningInitializeType(GizmoSpace);
  LightningInitializeType(GizmoDrag);
  LightningInitializeType(SimpleGizmoBase);
  LightningInitializeType(SquareGizmo);
  LightningInitializeType(ArrowGizmo);
  LightningInitializeType(RingGizmo);
  LightningInitializeType(TranslateGizmo);
  LightningInitializeType(ScaleGizmo);
  LightningInitializeType(RotateGizmo);
  LightningInitializeType(ObjectTransformGizmo);
  LightningInitializeType(ObjectTranslateGizmo);
  LightningInitializeType(ObjectScaleGizmo);
  LightningInitializeType(ObjectRotateGizmo);
  // LightningInitializeType(CogSizerGizmo);
  // LightningInitializeType(SizerGizmoEvent);
  // LightningInitializeType(SizerGizmo);
  LightningInitializeType(RotationBasisGizmoMetaTransform);
  LightningInitializeType(RotationBasisGizmoInitializationEvent);
  LightningInitializeType(RotationBasisGizmo);
  LightningInitializeType(OrientationBasisGizmo);
  LightningInitializeType(PhysicsCarWheelBasisGizmo);
  LightningInitializeType(RevoluteBasisGizmo);

  // Resource Editors
  LightningInitializeType(ResourceEditors);
  LightningInitializeType(SpritePreview);
  LightningInitializeType(SpriteSourceEditor);
  LightningInitializeType(MultiConvexMeshPoint);
  LightningInitializeType(MultiConvexMeshPropertyViewInfo);
  LightningInitializeType(MultiConvexMeshEditor);

  LightningInitializeType(HeightMapDebugDrawer);
  LightningInitializeType(HeightMapAabbChecker);

  LightningInitializeType(SpriteSheetImporter);
  LightningInitializeType(HeightMapImporter);

  // Tools
  LightningInitializeType(Tool);
  LightningInitializeType(SelectTool);
  LightningInitializeType(CreationTool);
  LightningInitializeType(ObjectConnectingTool);
  LightningInitializeType(ParentingTool);
  LightningInitializeType(ToolUiEvent);
  LightningInitializeType(ToolControl);
  LightningInitializeType(ManipulatorTool);
  LightningInitializeType(GizmoCreator);
  LightningInitializeType(ObjectTransformTool);
  LightningInitializeType(ObjectTranslateTool);
  LightningInitializeType(ObjectScaleTool);
  LightningInitializeType(ObjectRotateTool);
  LightningInitializeType(JointTool);
  LightningInitializeType(SpringSubTool);
  LightningInitializeType(DragSelectSubTool);
  LightningInitializeType(SelectorSpringSubTool);
  LightningInitializeType(PointMassSelectorSubTool);
  LightningInitializeType(AnchoringSubTool);
  LightningInitializeType(PointSelectorSubTool);
  LightningInitializeType(SpringSelectorSubTool);
  LightningInitializeType(SpringCreatorSubTool);
  LightningInitializeType(RopeCreatorSubTool);
  LightningInitializeType(SpringPointProxy);
  LightningInitializeType(SpringPointProxyProperty);
  LightningInitializeType(SpringTools);

  LightningInitializeType(HeightMapSubTool);
  LightningInitializeType(HeightManipulationTool);
  LightningInitializeType(RaiseLowerTool);
  LightningInitializeType(SmoothSharpenTool);
  LightningInitializeType(FlattenTool);
  LightningInitializeType(CreateDestroyTool);
  LightningInitializeType(WeightPainterTool);
  LightningInitializeType(HeightMapTool);
  LightningInitializeType(ViewportTextWidget);

  LightningInitializeType(SpriteFrame);

  // TileMap
  LightningInitializeType(TileEditor2DSubTool);
  LightningInitializeType(TileEditor2DDrawTool);
  LightningInitializeType(TileEditor2DSelectTool);
  LightningInitializeType(TileEditor2D);
  LightningInitializeType(TilePaletteSource);
  LightningInitializeType(TilePaletteView);
  LightningInitializeType(TilePaletteSprite);

  // Editor Ui
  LightningInitializeType(ObjectView);
  LightningInitializeType(HotKeyEditor);
  LightningInitializeType(LibraryView);
  LightningInitializeType(FloatingComposite);
  LightningInitializeType(PopUp);
  LightningInitializeType(AutoCompletePopUp);
  LightningInitializeType(CallTipPopUp);
  LightningInitializeType(RemovedEntry);
  LightningInitializeType(ConsoleUi);
  LightningInitializeType(DocumentEditor);
  LightningInitializeType(AddResourceWindow);
  LightningInitializeType(ResourceTypeSearch);
  LightningInitializeType(ResourceTemplateSearch);
  LightningInitializeType(ResourceTemplateDisplay);
  LightningInitializeType(TreeView);
  LightningInitializeType(TreeRow);
  LightningInitializeType(PropertyWidget);
  LightningInitializeType(PropertyWidgetObject);
  LightningInitializeType(AddObjectWidget);
  LightningInitializeType(UiLegacyToolTip);
  LightningInitializeType(RenderGroupHierarchies);

  LightningInitializeType(DirectProperty);

  // Animator
  LightningInitializeType(AnimationEditorData);
  LightningInitializeType(AnimationSettings);
  LightningInitializeType(AnimationTrackView);
  LightningInitializeType(AnimationEditor);

  // Stress test
  LightningInitializeType(StressTestDialog);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void EditorLibrary::Initialize()
{
  BuildStaticLibrary();

  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  BackgroundTasks::Initialize();
  DocumentManager::Initialize();
  Exporter::Initialize();
  PreviewWidgetFactory::Initialize();
  ResourceEditors::Initialize();
  ValueEditorFactory::Initialize();
  ColorScheme::Initialize();
  HotKeyCommands::Initialize();

  RegisterGeneralEditors();
  RegisterEngineEditors();
  RegisterObjectViewEditors();
  RegisterHotKeyEditors();
  RegisterAnimationTrackViewEditors();

  // Raycaster should start expanded when opening the property grid
  PropertyWidgetObject::mExpandedTypes.Insert("Raycaster");

  InitializeResourceManager(TilePaletteSourceManager);
}

void EditorLibrary::Shutdown()
{
  HotKeyCommands::Destroy();
  ColorScheme::Destroy();
  ValueEditorFactory::Destroy();
  ResourceEditors::Destroy();
  PreviewWidgetFactory::Destroy();
  DocumentManager::Destroy();
  Exporter::Destroy();
  BackgroundTasks::Destroy();

  GetLibrary()->ClearComponents();
}

} // namespace Plasma
