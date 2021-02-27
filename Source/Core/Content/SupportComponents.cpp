// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
LightningDefineType(ContentCopyright, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);

  LightningBindFieldProperty(Owner);
  LightningBindFieldProperty(Date);
}

void ContentCopyright::Serialize(Serializer& stream)
{
  SerializeName(Owner);
  SerializeName(Date);
}

LightningDefineType(ContentHistory, builder, type)
{
  // LightningBindFieldProperty(mRevisions); // METAREFACTOR array

  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);

  LightningBindFieldProperty(mRevisions);
}

void ContentHistory::Initialize(ContentComposition* item)
{
  mOwner = item;

  SourceControl* sourceControl = GetSourceControl(mOwner->mLibrary->SourceControlType);

  // get the path to the meta file
  String path = mOwner->GetMetaFilePath();

  Status status;
  sourceControl->GetRevisions(status, path, mRevisions);
}

LightningDefineType(ContentNotes, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);

  LightningBindFieldProperty(Notes);
}

void ContentNotes::Serialize(Serializer& stream)
{
  SerializeName(Notes);
}

LightningDefineType(ContentEditorOptions, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);

  LightningBindFieldProperty(mShowInEditor);
}

void ContentEditorOptions::Serialize(Serializer& stream)
{
  SerializeNameDefault(mShowInEditor, false);
}

void ContentEditorOptions::Initialize(ContentComposition* item)
{
  mOwner = item;
  item->ShowInEditor = mShowInEditor;
}

LightningDefineType(ResourceTemplate, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);

  LightningBindFieldProperty(mDisplayName);
  LightningBindFieldProperty(mDescription);
  LightningBindFieldProperty(mSortWeight);
  LightningBindFieldProperty(mCategory);
  LightningBindFieldProperty(mCategorySortWeight);
}

void ResourceTemplate::Serialize(Serializer& stream)
{
  SerializeNameDefault(mDisplayName, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mSortWeight, 100u);
  SerializeNameDefault(mCategory, String());
  SerializeNameDefault(mCategorySortWeight, 100u);
}

void CreateSupportContent(ContentSystem* system)
{
  AddContentComponent<ContentCopyright>(system);
  AddContentComponent<ContentHistory>(system);
  AddContentComponent<ContentNotes>(system);
  AddContentComponent<ContentEditorOptions>(system);
  AddContentComponent<ResourceTemplate>(system);
}

} // namespace Plasma
