// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
LightningDefineType(TextContent, builder, type)
{
}

TextContent::TextContent()
{
  EditMode = ContentEditMode::ResourceObject;
}

ContentItem* MakeTextContent(ContentInitializer& initializer)
{
  TextContent* content = new TextContent();

  content->Filename = initializer.Filename;

  DirectBuilderComponent* builder = nullptr;

  if (initializer.Extension == "txt")
    builder = new TextBuilder();

  TypeExtensionEntry* lightningTypeEntry = FileExtensionManager::GetLightningScriptTypeEntry();
  if (lightningTypeEntry->IsValidExtensionNoDot(initializer.Extension))
    builder = new LightningScriptBuilder();

  TypeExtensionEntry* fragmentTypeEntry = FileExtensionManager::GetLightningFragmentTypeEntry();
  // at the moment the extension always comes through as
  // lower case so add both cases to cover any future changes
  if (fragmentTypeEntry->IsValidExtensionNoDot(initializer.Extension))
    builder = new LightningFragmentBuilder();

  builder->Generate(initializer);

  content->AddComponent(builder);

  return content;
}

LightningDefineType(BaseTextBuilder, builder, type)
{
  PlasmaBindDependency(TextContent);
}

void BaseTextBuilder::Generate(ContentInitializer& initializer)
{
  Name = initializer.Name;
  mResourceId = GenerateUniqueId64();
}

LightningDefineType(TextBuilder, builder, type)
{
}
LightningDefineType(LightningScriptBuilder, builder, type)
{
}
LightningDefineType(LightningFragmentBuilder, builder, type)
{
}

void CreateScriptContent(ContentSystem* system)
{
  AddContent<TextContent>(system);
  AddContentComponent<TextBuilder>(system);
  AddContentComponent<LightningScriptBuilder>(system);
  AddContentComponent<LightningFragmentBuilder>(system);

  ContentTypeEntry text(LightningTypeId(TextContent), MakeTextContent);
  system->CreatorsByExtension["txt"] = text;

  TypeExtensionEntry* lightningExtensions = FileExtensionManager::GetInstance()->GetLightningScriptTypeEntry();
  for (size_t i = 0; i < lightningExtensions->mExtensions.Size(); ++i)
    system->CreatorsByExtension[lightningExtensions->mExtensions[i]] = text;

  TypeExtensionEntry* fragmentExtensions = FileExtensionManager::GetInstance()->GetLightningFragmentTypeEntry();
  for (size_t i = 0; i < fragmentExtensions->mExtensions.Size(); ++i)
    system->CreatorsByExtension[fragmentExtensions->mExtensions[i]] = text;
}

} // namespace Plasma
