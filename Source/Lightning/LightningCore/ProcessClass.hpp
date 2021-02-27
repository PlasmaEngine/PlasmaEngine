// MIT Licensed (see LICENSE.md).

// Include protection
#pragma once
#ifndef LIGHTNING_PROCESSCLASS_HPP
#  define LIGHTNING_PROCESSCLASS_HPP

namespace Lightning
{
/// Class to interface with another process. Allows re-directing standard input,
/// output, and error.
class PlasmaShared ProcessClass : public Plasma::Process
{
public:
  LightningDeclareType(ProcessClass, TypeCopyMode::ReferenceType);

  ProcessClass();
  ~ProcessClass();

  void Start(Plasma::ProcessStartInfo& info);

  // Close the process handle this does not force the process to exit.
  void Close();
  void CloseStreams();

  HandleOf<FileStreamClass> GetStandardError();
  HandleOf<FileStreamClass> GetStandardInput();
  HandleOf<FileStreamClass> GetStandardOutput();

  bool mRedirectStandardError;
  bool mRedirectStandardInput;
  bool mRedirectStandardOutput;

  HandleOf<FileStreamClass> mStandardError;
  HandleOf<FileStreamClass> mStandardInput;
  HandleOf<FileStreamClass> mStandardOutput;
};

} // namespace Lightning

#endif
