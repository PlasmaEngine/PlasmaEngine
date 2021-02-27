// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
// Ranges
LightningDefineRange(ContextMenuEntryChildren::range);

// Enums
LightningDefineEnum(VerticalAlignment);
LightningDefineEnum(HorizontalAlignment);
LightningDefineEnum(IndicatorSide);
LightningDefineEnum(ToolTipColorScheme);

LightningDefineStaticLibrary(WidgetLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  LightningInitializeRangeAs(ContextMenuEntryChildren::range, "ContextMenuEntryChildrenRange");

  // Enums
  LightningInitializeEnum(VerticalAlignment);
  LightningInitializeEnum(HorizontalAlignment);
  LightningInitializeEnum(IndicatorSide);
  LightningInitializeEnum(ToolTipColorScheme);

  // Events
  LightningInitializeType(FocusEvent);
  LightningInitializeType(MouseEvent);
  LightningInitializeType(CommandEvent);
  LightningInitializeType(CommandUpdateEvent);
  LightningInitializeType(CommandCaptureContextEvent);
  LightningInitializeType(HighlightBorderEvent);
  LightningInitializeType(TabModifiedEvent);
  LightningInitializeType(TabRenamedEvent);
  LightningInitializeType(QueryModifiedSaveEvent);
  LightningInitializeType(HandleableEvent);
  LightningInitializeType(WindowTabEvent);
  LightningInitializeType(MainWindowTransformEvent);
  LightningInitializeType(MouseDragEvent);
  LightningInitializeType(ModalConfirmEvent);
  LightningInitializeType(ModalButtonEvent);
  LightningInitializeType(SearchViewEvent);
  LightningInitializeType(AlternateSearchCompletedEvent);
  LightningInitializeType(TagEvent);
  LightningInitializeType(ContextMenuEvent);

  LightningInitializeType(LayoutArea);
  LightningInitializeType(Layout);
  LightningInitializeType(FillLayout);
  LightningInitializeType(StackLayout);
  LightningInitializeType(EdgeDockLayout);
  LightningInitializeType(DockLayout);
  LightningInitializeType(RatioLayout);
  LightningInitializeType(GridLayout);
  LightningInitializeType(SizePolicies);
  LightningInitializeType(Widget);
  LightningInitializeType(Composite);
  LightningInitializeType(MultiDock);
  LightningInitializeType(RootWidget);
  LightningInitializeType(MainWindow);
  LightningInitializeType(MouseManipulation);
  LightningInitializeType(BaseScrollArea);
  LightningInitializeType(ScrollArea);
  LightningInitializeType(ButtonBase);
  LightningInitializeType(TextButton);
  LightningInitializeType(IconButton);
  LightningInitializeType(ToggleIconButton);
  LightningInitializeType(ListBox);
  LightningInitializeType(ComboBox);
  LightningInitializeType(StringComboBox);
  LightningInitializeType(ToolTip);
  LightningInitializeType(TextureView);
  LightningInitializeType(EditText);
  LightningInitializeType(TextBox);
  LightningInitializeType(SearchView);
  LightningInitializeType(SearchViewElement);
  LightningInitializeType(Label);
  LightningInitializeType(ProgressBar);
  LightningInitializeType(Slider);
  LightningInitializeType(SelectorButton);
  LightningInitializeType(ImageWidget);
  LightningInitializeType(CheckBox);
  LightningInitializeType(TextCheckBox);
  LightningInitializeType(WidgetManager);
  LightningInitializeType(CommandExecuter);
  LightningInitializeType(CommandManager);
  LightningInitializeType(MetaScriptTagAttribute);
  LightningInitializeType(MetaScriptShortcutAttribute);
  LightningInitializeType(Spacer);
  LightningInitializeType(Splitter);
  LightningInitializeType(TabArea);
  LightningInitializeType(Window);
  LightningInitializeType(Viewport);
  LightningInitializeType(ContextMenuEntry);
  LightningInitializeType(ContextMenuEntryDivider);
  LightningInitializeType(ContextMenuEntryCommand);
  LightningInitializeType(ContextMenuEntryMenu);
  LightningInitializeType(MenuBarItem);
  LightningInitializeType(MenuBar);
  LightningInitializeType(MultiManager);
  LightningInitializeType(Modal);
  LightningInitializeType(Text);
  LightningInitializeType(MultiLineText);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void WidgetLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  RegisterClassAttributeType(ObjectAttributes::cTags, MetaScriptTagAttribute)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cShortcut, MetaScriptShortcutAttribute)->TypeMustBe(Component);

  WidgetManager::Initialize();
  CommandManager::Initialize();
}

void WidgetLibrary::Shutdown()
{
  CommandManager::Destroy();
  WidgetManager::Destroy();

  GetLibrary()->ClearComponents();
}

} // namespace Plasma
