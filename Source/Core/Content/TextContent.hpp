// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Text content item. Text content is content that is loaded
/// from plain text files. The editor can edit them with various
/// text editors.
class TextContent : public ContentComposition
{
public:
  LightningDeclareType(TextContent, TypeCopyMode::ReferenceType);

  TextContent();
};

class BaseTextBuilder : public DirectBuilderComponent
{
public:
  LightningDeclareType(BaseTextBuilder, TypeCopyMode::ReferenceType);

  void Generate(ContentInitializer& initializer) override;
  BaseTextBuilder(uint order, StringParam extension, StringParam resourceName) :
      DirectBuilderComponent(order, extension, resourceName)
  {
  }
};

const String TextResourceName = "Text";
const String TextExtension = ".txt";

/// Text Builder for generic text.
class TextBuilder : public BaseTextBuilder
{
public:
  LightningDeclareType(TextBuilder, TypeCopyMode::ReferenceType);

  void SetDefaults()
  {
  }
  TextBuilder() : BaseTextBuilder(15, TextExtension, TextResourceName)
  {
  }
};

const String LightningScriptResourceName = "LightningScript";

/// Lightning Script File builder.
class LightningScriptBuilder : public BaseTextBuilder
{
public:
  LightningDeclareType(LightningScriptBuilder, TypeCopyMode::ReferenceType);

  void SetDefaults()
  {
  }

  LightningScriptBuilder() :
      BaseTextBuilder(
          15, FileExtensionManager::GetLightningScriptTypeEntry()->GetDefaultExtensionWithDot(), LightningScriptResourceName)
  {
  }
};

class LightningFragmentBuilder : public BaseTextBuilder
{
public:
  LightningDeclareType(LightningFragmentBuilder, TypeCopyMode::ReferenceType);

  void SetDefaults()
  {
  }

  LightningFragmentBuilder()
      // Increase the load order to 9 so that these load before materials
      // (since a material may need the block created from this fragment)
      :
      BaseTextBuilder(
          9, FileExtensionManager::GetLightningFragmentTypeEntry()->GetDefaultExtensionWithDot(), "LightningFragment")
  {
  }
};

} // namespace Plasma
