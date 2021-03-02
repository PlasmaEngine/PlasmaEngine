// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{
ContentItem* MakeLightningPluginContent(ContentInitializer& initializer);

class LightningPluginBuilder : public DataBuilder
{
public:
  LightningDeclareType(LightningPluginBuilder, TypeCopyMode::ReferenceType);
  LightningPluginBuilder();

  ResourceId mSharedLibraryResourceId;

  void Rename(StringParam newName) override;

  // Gets a platform and Plasma revision dependent extension (ends with
  // lightningPlugin)
  static String GetSharedLibraryPlatformName();
  static String GetSharedLibraryPlatformBuildName();
  static String GetSharedLibraryExtension(bool includeDot);

  String GetSharedLibraryFileName();

  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer) override;
  void BuildContent(BuildOptions& buildOptions) override;
  void BuildListing(ResourceListing& listing) override;
};

void CreateLightningPluginContent(ContentSystem* system);
ContentItem* MakeLightningPluginContent(ContentInitializer& initializer);

} // namespace Plasma
