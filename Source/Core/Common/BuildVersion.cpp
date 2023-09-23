// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

#define Stringify(s) InnerStringify(s)
#define InnerStringify(s) #s

#define WrapHex(a) InnerWrapHex(a)
#define InnerWrapHex(s) 0x##s##ULL

const String sPlasmaOrganization = "Plasma";
const String sEditorGuid = "51392222-AEDE-4530-8749-9DFAB5725FD7";
const String sEditorName = "Editor";
const String sLauncherGuid = "7489829B-8A03-4B26-B3AC-FDDC6668BAF7";
const String sLauncherName = "Launcher";


String GetEditorFullName()
{
  return sPlasmaOrganization + sEditorName;
}

String GetEditorExecutableFileName()
{
  return GetEditorFullName() + cExecutableExtensionWithDot;
}

String GetLauncherFullName()
{
  return sPlasmaOrganization + sLauncherName;
}

String GetLauncherExecutableFileName()
{
  return GetLauncherFullName() + cExecutableExtensionWithDot;
}

String GetRelativeExecutable(StringParam organization, StringParam name)
{
  String fullName = organization + name;
  return FilePath::Combine("..", "..", fullName, fullName + cExecutableExtensionWithDot);
}

uint gConfigVersion = 1;
String gAppGuid;
String gAppOrganization;
String gAppName;

void SetupApplication(uint configVersion, StringParam organization, StringParam guid, StringParam name)
{
  gConfigVersion = configVersion;
  gAppGuid = guid;
  gAppOrganization = organization;
  gAppName = name;
}

cstr GetGuidString()
{
  return gAppGuid.c_str();
}

StringParam GetApplicationName()
{
  return gAppName;
}

StringParam GetOrganization()
{
  return gAppOrganization;
}

String GetOrganizationApplicationName()
{
  return GetOrganization() + GetApplicationName();
}

uint GetConfigVersion()
{
  return gConfigVersion;
}

uint GetMajorVersion()
{
  return PLASMA_VERSION_MAJOR;
}

uint GetMinorVersion()
{
  return PLASMA_VERSION_MINOR;
}

uint GetPatchVersion()
{
  return PLASMA_VERSION_PATCH;
}

uint GetRevisionNumber()
{
  return PLASMA_REVISION_ID;
}

u64 GetShortChangeSet()
{
  return WrapHex(PLASMA_SHORT_CHANGE_SET);
}

cstr GetMajorVersionString()
{
  return Stringify(PLASMA_VERSION_MAJOR);
}

cstr GetMinorVersionString()
{
  return Stringify(PLASMA_VERSION_MINOR);
}

cstr GetPatchVersionString()
{
  return Stringify(PLASMA_VERSION_PATCH);
}

cstr GetRevisionNumberString()
{
  return Stringify(PLASMA_REVISION_ID);
}

String GetBuildIdString()
{
  StringBuilder builder;
  builder.Append(ToString(GetMajorVersion()));
  builder.Append('.');
  builder.Append(ToString(GetMinorVersion()));
  builder.Append('.');
  builder.Append(ToString(GetPatchVersion()));
  builder.Append('.');
  builder.Append(GetRevisionNumberString());
  String result = builder.ToString();
  return result;
}

cstr GetShortChangeSetString()
{
  return Stringify(PLASMA_SHORT_CHANGE_SET);
}

cstr GetChangeSetString()
{
  return Stringify(PLASMA_CHANGE_SET);
}

cstr GetChangeSetDateString()
{
  return Stringify(PLASMA_CHANGE_SET_DATE);
}

cstr GetConfigurationString()
{
  return PlasmaConfigName;
}

cstr GetPlatformString()
{
  return PLASMA_PLATFORM_NAME;
}

String GetBuildVersionName()
{
  /*
   * This needs to match
   * index.js:pack/Standalone.cpp:BuildId::Parse/BuildId::GetFullId/BuildVersion.cpp:GetBuildVersionName.
   * Application.Branch.Major.Minor.Patch.Revision.ShortChangeset.MsSinceEpoch.TargetOs.Architecture.Config.Extension
   * Example: PlasmaEditor.master.1.5.0.1501.fb02756c46a4.1574702096290.Windows.x86.Release.zip
   */
  StringBuilder builder;
  builder.AppendFormat("%s.", GetApplicationName().c_str()); // Application [PlasmaEditor]
  builder.AppendFormat("%s.", PLASMA_VERSION_ALIAS);             // Branch [master]
  builder.AppendFormat("%d.", GetMajorVersion());            // Major [1]
  builder.AppendFormat("%d.", GetMinorVersion());            // Minor [5]
  builder.AppendFormat("%d.", GetPatchVersion());            // Patch [0]
  builder.AppendFormat("%d.", GetRevisionNumber());          // Revision [1501]
  builder.AppendFormat("%s.", GetShortChangeSetString());    // ShortChangeset [fb02756c46a4]
  builder.AppendFormat("%s.", PLASMA_PLATFORM_NAME);           // TargetOs [Windows]
  builder.AppendFormat("%s.", PlasmaArchitectureName);       // Architecture [x86]
  builder.AppendFormat("%s.", PlasmaConfigName);             // Config [Release]
  builder.Append("plasmabuild");
  String result = builder.ToString();
  return result;
}

int GetVersionId(StringParam versionIdFilePath)
{
  int localVersionId = -99;
  // make sure the file exists, if it doesn't assume the version is 0
  //(aka, the lowest and most likely to be replaced)
  if (FileExists(versionIdFilePath))
  {
    size_t fileSize;
    ::byte* data = ReadFileIntoMemory(versionIdFilePath.c_str(), fileSize, 1);
    data[fileSize] = 0;
    if (data == nullptr)
      return localVersionId;

    ToValue(String((char*)data), localVersionId);
    delete data;
  }
  return localVersionId;
}

} // namespace Plasma
