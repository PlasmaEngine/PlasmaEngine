// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{
LightningDefineType(Revision, builder, type)
{
  type->HandleManager = LightningManagerId(PointerManager);
  LightningBindFieldProperty(ChangeSet);
  LightningBindFieldProperty(User);
  LightningBindFieldProperty(Date);
  LightningBindFieldProperty(Summary);
}

class FileSourceControl : public SourceControl
{
  void Add(Status& status, StringParam filePath) override
  {
    // nothing to do
  }

  void Remove(Status& status, StringParam filePath) override
  {
    DeleteFile(filePath);
  }

  void Rename(Status& status, StringParam sourcePath, StringParam destPath) override
  {
    MoveFile(destPath, sourcePath);
  }

  void GetRevisions(Status& status, StringParam path, Array<Revision>& revisions) override
  {
    status.SetFailed("Not supported");
  }
};

SourceControl* GetMercurial();
SourceControl* GetSvn();

SourceControl* GetSourceControl(StringParam sourceControlType)
{
  static FileSourceControl fileSourceControl;

  if (sourceControlType == "Mercurial")
    return GetMercurial();
  else if (sourceControlType == "Svn")
    return GetSvn();
  else
    return &fileSourceControl;
}

int RunSimpleCommandLine(Status& status, StringParam commandLine)
{
  TextStreamBuffer buffer;
  SimpleProcess process;
  process.ExecProcess("CommandLine", commandLine.c_str(), &buffer);
  int returnCode = process.WaitForClose();
  if (returnCode != 0)
    status.SetFailed(buffer.ToString());
  return returnCode;
}

} // namespace Plasma
