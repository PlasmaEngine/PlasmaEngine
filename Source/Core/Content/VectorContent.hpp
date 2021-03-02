// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

/// Font content file are true type font files. Loading into
/// vector or bitmap formats.
class FontContent : public ContentComposition
{
public:
  LightningDeclareType(FontContent, TypeCopyMode::ReferenceType);

  FontContent();
};

// Normal font builder just copies the ttf for direct loading.
class FontBuilder : public DirectBuilderComponent
{
public:
  LightningDeclareType(FontBuilder, TypeCopyMode::ReferenceType);
  FontBuilder();
  void Generate(ContentInitializer& initializer) override;
};

} // namespace Plasma
