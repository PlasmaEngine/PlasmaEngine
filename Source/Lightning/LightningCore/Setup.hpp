// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef LIGHTNING_HPP
#  define LIGHTNING_HPP

namespace Lightning
{
namespace SetupFlags
{
enum Enum
{
  None = 0,
  CustomAssertHandlerOrNoAsserts = (1 << 0),
  NoDocumentationStrings = (1 << 1),
  DoNotShutdown = (1 << 2),
  DoNotShutdownMemory = (1 << 3)
};
typedef unsigned Type;
} // namespace SetupFlags

// Initializes the shared global memory manager and builds all the static bound
// libraries
class PlasmaShared LightningSetup
{
public:
  // Controls default setup parameters (such as whether we optimize out
  // documentation string, etc) Note: No Lightning classes should be created before
  // this occurs
  LightningSetup(SetupFlags::Type flags = SetupFlags::None);

  // Shuts down the shared global memory manager and releases any static
  // libraries Note: No Lightning classes should be created after this occurs
  ~LightningSetup();

  // The setup is a singleton, which means two may not exist at the same time
  // However, you can create LightningSetup and destroy it, then create another
  // LightningSetup On destruction, this Instance will be cleared to null
  static LightningSetup* Instance;

  // Whatever flags we were created with
  SetupFlags::Enum Flags;
};

// A simple macro that we specle everywhere to ensure that the user initializes
// Lightning
#  define LightningErrorIfNotStarted(Name)                                                                                 \
    ErrorIf(LightningSetup::Instance == nullptr,                                                                           \
            "In order to use the Lightning " #Name " you must create the LightningSetup type and hold on to it")

// A convenient form of parsed main arguments (easily comparable and queryable)
class PlasmaShared MainArguments
{
public:
  // The first argument of the argv is generally a path to the executable
  String ExecutablePath;

  // All commands start with a '-' and generally a value follows (or empty if
  // another command directly followed)
  HashMap<String, Array<String>> CommandToValues;

  // Any stray value that isn't preceeded by a command gets put here (a typical
  // use is for file inputs and so on)
  Array<String> InputValues;

  // Whether a particular command was present
  bool HasCommand(StringParam command);

  // Gets the last value passed in for a particular command
  String GetCommandValue(StringParam command);

  // Gets the last value passed in for a particular command as a pointer (easy
  // to test for null)
  String* GetCommandValuePointer(StringParam command);

  // Gets the array of all commands
  Array<String>& GetCommandValues(StringParam command);
};

// Parsers arguments that we typically get from main into the above structure
PlasmaShared void LightningParseMainArguments(int argc, char* argv[], MainArguments& argumentsOut);

// Processes command line arguments for running Lightning standalone (invokes
// Startup/Shutdown)
PlasmaShared int LightningMain(int argc, char* argv[]);

// Waits for a debugger to be attached (optionally can breakpoint upon
// attachment) Note, if this is called and the breakpoint option is on, it will
// ALWAYS breakpoint when running from a debugger
PlasmaShared void LightningWaitForDebugger(bool breakpointWhenAttached);
} // namespace Lightning

#endif
