// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

using namespace ExportUtility;

HashSet<String>& EmscriptenExportTarget::GetAdditionalExcludedFiles()
{
  static HashSet<String> files;
  files.Insert("wast");
  files.Insert("wasm");
  files.Insert("map");
  files.Insert("exe");
  files.Insert("dll");
  files.Insert("pak");
  files.Insert("dat");
  files.Insert("bin");
  files.Insert("py");
  files.Insert("js");
  files.Insert("html");
  files.Insert("PlasmaEditor.data");
  return files;
}

EmscriptenExportTarget::EmscriptenExportTarget(Exporter* exporter, String targetName) :
    ExportTarget(exporter, targetName)
{
}

void EmscriptenExportTarget::ExportApplication()
{
  {
    String outputDirectory = FilePath::Combine(GetTemporaryDirectory(), "Web", "PlasmaExport");

    Status status;
    mExporter->CopyContent(status, outputDirectory, this);
    if (status.Failed())
    {
      DoNotifyWarning("Web Build", status.Message);
      return;
    }

    // Archive the content to create a zip file that will be the virtual file
    // system on the web
    Archive virtualFileSystem(ArchiveMode::Compressing);
    virtualFileSystem.ArchiveDirectory(status, outputDirectory);

    if (status.Failed())
    {
      DoNotifyError("Web Build", status.Message);
      return;
    }

    // TODO: Select web build version and copy out that version for export
    // Copy the web build of Plasma Engine to the the specified export folder
    // location Update this string to your web-build location for testing.
    Cog* configCog = PL::gEngine->GetConfigCog();
    ReturnIf(!configCog, , "Unable to get the config cog");
    MainConfig* mainConfig = configCog->has(MainConfig);
    ReturnIf(!mainConfig, , "Unable to get the MainConfig on the config cog");
    String webBuildPath = FilePath::Combine(mainConfig->DataDirectory, "WebBuild");

    if (!DirectoryExists(webBuildPath))
    {
      DoNotifyError("Web Build", "The WebBuild directory does not exist inside the Data directory.");
      return;
    }

    FileRange webBuildFiles(webBuildPath);

    while (!webBuildFiles.Empty())
    {
      FileEntry entry = webBuildFiles.FrontEntry();
      String targetFile = FilePath::Combine(mExporter->mOutputDirectory, entry.mFileName);
      if (FileExists(targetFile))
        DeleteFile(targetFile);
      CopyFile(targetFile, entry.GetFullPath());
      webBuildFiles.PopFront();
    }

    // PlasmaEditor.data is the name that the web build expects the virtual file
    // system to be
    String zipOut = FilePath::Combine(mExporter->mOutputDirectory, "PlasmaEditor.data");
    if (FileExists(zipOut))
      DeleteFile(zipOut);
    virtualFileSystem.WriteZipFile(zipOut);

    // Get the size of the achieved content folder and set the file size in
    // PlasmaEditor.js so it can properly load the virtual file system
    uint zipSize = virtualFileSystem.ComputeZipSize();

    // Open the PlasmaEditor.js file and search for information we need to update
    File plasmaJsFile;
    String plasmaJsFilepath = FilePath::Combine(webBuildPath, "PlasmaEditor.js");
    String fileContent = ReadFileIntoString(plasmaJsFilepath);

    if (fileContent.Empty())
    {
      DoNotifyWarning("Web Build", "Failed to read PlasmaEditor.js, aborting export");
      return;
    }

    // Find the text to replace
    StringRange end = fileContent.FindFirstOf("\"end\": ");
    StringRange filename = fileContent.FindFirstOf(", \"filename\":");
    StringRange remotePackageSize = fileContent.FindFirstOf("\"remote_package_size\": ");
    StringRange packageUuid = fileContent.FindFirstOf(", \"package_uuid\":");

    // Using the above saved locations of content within the PlasmaEditor.js file
    // build a js file that will load our exported content properly
    StringBuilder plasmaJsBuilder;
    plasmaJsBuilder.Append(fileContent.SubString(fileContent.Begin(), end.End()));
    plasmaJsBuilder.Append(ToString(zipSize));
    plasmaJsBuilder.Append(fileContent.SubString(filename.Begin(), remotePackageSize.End()));
    plasmaJsBuilder.Append(ToString(zipSize));
    plasmaJsBuilder.Append(fileContent.SubString(packageUuid.Begin(), fileContent.End()));

    // Write out the updated PlasmaEditor.js file contents to our export location
    String outputPlasmaJsFile = plasmaJsBuilder.ToString();
    File outputFile;
    String plasmaEditorJsPath = FilePath::Combine(mExporter->mOutputDirectory, "PlasmaEditor.js");
    outputFile.Open(plasmaEditorJsPath, FileMode::Write, FileAccessPattern::Sequential, FileShare::Unspecified, &status);
    if (status.Failed())
    {
      DoNotifyWarning("File Write Error", "Failed to write PlasmaEditor.js, aborting export");
      return;
    }

    outputFile.Write((byte*)outputPlasmaJsFile.Data(), outputPlasmaJsFile.SizeInBytes());
    outputFile.Close();
  }

  DoNotify("Exported", "Project has been exported for the Web.", "Disk");
}

void EmscriptenExportTarget::ExportContentFolders(Cog* projectCog)
{
  // For now when exporting web builds using the export content folder option
  // just export the application
  ExportApplication();
}

void EmscriptenExportTarget::CopyInstallerSetupFile(StringParam dest,
                                                    StringParam source,
                                                    StringParam projectName,
                                                    Guid guid)
{
  // Installer setup might be changed to not be on the target if inno setup can
  // be used to generate an installer based on the platform
}

} // namespace Plasma
