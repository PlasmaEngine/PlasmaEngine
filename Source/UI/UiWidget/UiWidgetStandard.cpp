// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// Ranges
LightningDefineRange(UiWidgetComponentHierarchy::ChildListRange);

// Enums
LightningDefineEnum(UiSizePolicy);
LightningDefineEnum(UiVerticalAlignment);
LightningDefineEnum(UiHorizontalAlignment);
LightningDefineEnum(UiDockMode);
LightningDefineEnum(UiFocusDirection);
LightningDefineEnum(UiStackLayoutDirection);
LightningDefineEnum(UiButtonState);
LightningDefineEnum(UiButtonMouseDetectionMode);

LightningDefineStaticLibrary(UiWidgetLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  LightningInitializeRangeAs(UiWidgetComponentHierarchy::ChildListRange, "UiWidgetRange");

  // Enums
  LightningInitializeEnum(UiSizePolicy);
  LightningInitializeEnum(UiVerticalAlignment);
  LightningInitializeEnum(UiHorizontalAlignment);
  LightningInitializeEnum(UiDockMode);
  LightningInitializeEnum(UiFocusDirection);
  LightningInitializeEnum(UiStackLayoutDirection);
  LightningInitializeEnum(UiButtonState);
  LightningInitializeEnum(UiButtonMouseDetectionMode);

  // Events
  LightningInitializeType(UiFocusEvent);
  LightningInitializeType(UiTransformUpdateEvent);

  // Widgets
  LightningInitializeType(UiWidgetCastResultsRange);
  LightningInitializeType(UiWidgetComponentHierarchy);
  LightningInitializeType(UiWidget);
  LightningInitializeType(UiButton);
  LightningInitializeType(UiRootWidget);
  LightningInitializeType(UiLayout);
  LightningInitializeType(UiStackLayout);
  LightningInitializeType(UiFillLayout);
  LightningInitializeType(UiDockLayout);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void UiWidgetLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void UiWidgetLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Plasma
