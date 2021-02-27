// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

// LightningPluginBuilder
LightningDefineType(LightningPluginBuilder, builder, type)
{
  PlasmaBindComponent();
  PlasmaBindSetup(SetupMode::CallSetDefaults);
}

String GenerateGuid()
{
  // This isn't really 'cryptographically random' or anything safe, but it works
  Math::Random rand((uint)GenerateUniqueId64());
  char data[] = "{11111111-1111-1111-1111-111111111111}";
  for (size_t i = 0; i < sizeof(data); ++i)
  {
    if (data[i] == '1')
    {
      char letterIndex = (char)rand.IntRangeInEx(0, 16);
      if (letterIndex < 10)
        data[i] = '0' + letterIndex;
      else
        data[i] = 'A' + (letterIndex - 10);
    }
  }

  return String(data, sizeof(data) - 1);
}

String GetContentDirectory(ContentLibrary* library)
{
  return library->SourcePath;
}

String GetCodeDirectory(ContentLibrary* library, StringParam name)
{
  return FilePath::Combine(GetContentDirectory(library), name);
}

LightningPluginBuilder::LightningPluginBuilder()
{
  mSharedLibraryResourceId = 0;
}

void LightningPluginBuilder::Rename(StringParam newName)
{
  DataBuilder::Rename(newName);
  DoNotifyWarning("Lightning Plugin",
                  "You must rename the plugin directory and all the contents "
                  "(projects, solutions, make files, etc)");
}

String LightningPluginBuilder::GetSharedLibraryPlatformName()
{
  StringBuilder builder;

  // This is not currently used because we ensured that Debug/Release builds
  // of the plugins are compatible with both Debug/Release builds of Plasma
  // Note that this relates to linked libraries within the generated project
  // files (plugin templates) The template projects also generate dynamic
  // libraries with extensions that could include Debug/Release
  // builder.Append(GetConfigurationString());
  // builder.Append('-');

  // Append the operating system name (or some grouped name for all OSes that
  // support this shared library)
  builder.Append(PlasmaTargetOsName);
  builder.Append('-');

  // Append the target machine architecture
  builder.Append(PlasmaArchitectureName);
  String pluginFileName = builder.ToString();
  return pluginFileName;
}

String LightningPluginBuilder::GetSharedLibraryPlatformBuildName()
{
  String platformName = GetSharedLibraryPlatformName();
  String platformBuildName = BuildString(platformName, "-", GetRevisionNumberString());
  return platformBuildName;
}

String LightningPluginBuilder::GetSharedLibraryExtension(bool includeDot)
{
  String extension;
  if (includeDot)
    extension = BuildString(".", GetSharedLibraryPlatformBuildName(), "-lightningPlugin");
  else
    extension = BuildString(GetSharedLibraryPlatformBuildName(), "-lightningPlugin");
  return extension;
}

String LightningPluginBuilder::GetSharedLibraryFileName()
{
  return BuildString(Name, GetSharedLibraryExtension(true));
}

void LightningPluginBuilder::Serialize(Serializer& stream)
{
  DataBuilder::Serialize(stream);
  SerializeNameDefault(mSharedLibraryResourceId, ResourceId(0));
}

void LightningPluginBuilder::Generate(ContentInitializer& initializer)
{
  DataBuilder::Generate(initializer);

  if (mSharedLibraryResourceId == 0)
    mSharedLibraryResourceId = GenerateUniqueId64();
}

void LightningPluginBuilder::BuildContent(BuildOptions& buildOptions)
{
  // This must run before the template early out so that we output the data file
  // to the content output directory and the template plugin stops thinking it
  // needs to build every time we run Plasma.
  DataBuilder::BuildContent(buildOptions);

  // Don't build the resource template plugin.
  // This stops a stub dll from getting into our Editor resources directory.
  if (mOwner->has(ResourceTemplate))
    return;

  String sharedLibraryFileName = GetSharedLibraryFileName();
  String destFile = FilePath::Combine(buildOptions.OutputPath, sharedLibraryFileName);
  String sourceFile = FilePath::Combine(buildOptions.SourcePath, sharedLibraryFileName);

  // We output a dummy empty shared library so that content won't get mad at us
  if (!FileExists(sourceFile))
  {
    byte value = 0;
    WriteToFile(sourceFile.c_str(), &value, 0);
  }

  CopyFile(destFile, sourceFile);

  Resource* resource = PL::gResources->GetResource(mOwner->mRuntimeResource);
  if (resource)
  {
    resource->SendModified();
  }
}

void LightningPluginBuilder::BuildListing(ResourceListing& listing)
{
  DataBuilder::BuildListing(listing);

  String destFile = GetSharedLibraryFileName();
  static const String LibraryLoaderType("LightningPluginLibrary");
  uint order = PL::gContentSystem->LoadOrderMap.FindValue(LibraryLoaderType, 10);
  listing.PushBack(
      ResourceEntry(order, LibraryLoaderType, Name, destFile, mSharedLibraryResourceId, this->mOwner, this));
}

ContentItem* MakeLightningPluginContent(ContentInitializer& initializer)
{
  DataContent* content = new DataContent();
  content->mIgnoreMultipleResourcesWarning = true;
  content->Filename = initializer.Filename;
  LightningPluginBuilder* builder = new LightningPluginBuilder();
  builder->Generate(initializer);
  builder->LoaderType = "LightningPluginSource";
  content->AddComponent(builder);

  String pluginName = initializer.Name;
  String codeDir = GetCodeDirectory(initializer.Library, pluginName);

  // If the directory already exists
  if (DirectoryExists(codeDir))
  {
    DoNotifyWarning("Lightning Plugin",
                    String::Format("The directory for the Lightning plugin '%s' already exists, "
                                   "therefore we will not create the template project:\n  '%s'\n",
                                   pluginName.c_str(),
                                   codeDir.c_str()));
    return content;
  }

  Cog* configCog = PL::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  String templateDir = FilePath::Combine(mainConfig->DataDirectory, "LightningCustomPluginTemplate");

  // Some templates require generated guids which we will create and replace
  static const String ReplaceGuid("{11111111-1111-1111-1111-111111111111}");
  static const String ReplaceName("RESOURCE_NAME_");

  String includeGuard = pluginName.ToUpper();
  String guid = GenerateGuid();

  CreateDirectoryAndParents(codeDir);
  for (FileRange files(templateDir); !files.Empty(); files.PopFront())
  {
    String templateFileName = files.Front();
    String templateFile = FilePath::Combine(templateDir, templateFileName);
    String outputFileName = templateFileName.Replace(ReplaceName, pluginName);
    String outputFile = FilePath::Combine(codeDir, outputFileName);

    // Everything we're writing in this template is string data that required
    // replacements
    String data = ReadFileIntoString(templateFile.c_str());

    // Replace any guids and names inside the template text files
    // This could probably be done more efficiently
    data = data.Replace(ReplaceName, pluginName);
    data = data.Replace(ReplaceGuid, guid);

    WriteToFile(outputFile.Data(), (byte*)data.Data(), data.SizeInBytes());
  }

  return content;
}

void CreateLightningPluginContent(ContentSystem* system)
{
  AddContentComponent<LightningPluginBuilder>(system);
}

} // namespace Plasma
