// MIT Licensed (see LICENSE.md).
#pragma once
#include "Typedefs.hpp"
// quick ARM compiler fix, project isn't seeing/building this before console.hpp
#include <stdarg.h>

namespace Plasma
{

// Console Filtering
typedef unsigned int FilterType;
namespace Filter
{
enum Enum
{
  DefaultFilter = 0x0001,     // No filter provided
  UserFilter = 0x0002,        // Filter for object creation.
  ErrorFilter = 0x0004,       // Filter for all Error and warnings
  ResourceFilter = 0x0008,    // Filter for Resource Operations
  EngineFilter = 0x0010,      // Filter for Core Engine Operations
  ActiveFilter = 0x0020,      // Filter for debugging
  PerformanceFilter = 0x0040, // Filter for performance (Framerate, etc)
  PhysicsFilter = 0x0080,     // Filter for Physics
  ArchiveFilter = 0x0100,     // Filter for Archival operations
};
} // namespace Filter

// LogFilter Interface
class ConsoleListener
{
public:
  // ConsoleListener Interface

  // Print a null terminated message.
  virtual void Print(FilterType filterType, cstr message) = 0;
  // Flush out output (Called when crashing)
  virtual void Flush()
  {
  }
  // Auto remove on destructor
  virtual ~ConsoleListener();
};

/// ConsoleListener that redirects to the debugger's console output
class DebuggerListener : public ConsoleListener
{
public:
  void Print(FilterType filterType, cstr message) override;
};

/// ConsoleListener that redirects to the standard output
class StdOutListener : public ConsoleListener
{
public:
  void Print(FilterType filterType, cstr message) override;
};

// The Console for debugging, diagnostic, and displaying engine status.
// Uses C Style output semantics.
// Example: DebugPrint("Finished loading Node %d\n", 56);
class Console
{
public:
  static void Print(Filter::Enum filter, cstr format, ...);
  static void PrintRaw(Filter::Enum filter, cstr data);
  static void Add(ConsoleListener* listener);
  static void Remove(ConsoleListener* lister);
  static void FlushAll();

private:
  static void PrintVa(Filter::Enum, cstr format, va_list va);
};

} // namespace Plasma

// Enable print of debug messages to the console
#if !defined(PLASMA_ENABLE_DEBUG_CONSOLE)
#  if defined(PlasmaDebug)
#    define PLASMA_ENABLE_DEBUG_CONSOLE 1
#  else
#    define PLASMA_ENABLE_DEBUG_CONSOLE 0
#  endif
#endif

#define PlasmaPrintFilter(filter, ...) ::Plasma::Console::Print(filter, __VA_ARGS__)
#define PlasmaPrint(...) ::Plasma::Console::Print(Plasma::Filter::DefaultFilter, __VA_ARGS__)

// Debug printing functions will only print in debug builds

#if PLASMA_ENABLE_DEBUG_CONSOLE

#  define DebugPrintFilter(filter, ...) ::Plasma::Console::Print(filter, __VA_ARGS__)
#  define DebugPrint(...) ::Plasma::Console::Print(Plasma::Filter::DefaultFilter, __VA_ARGS__)
#  define DebugTrace(format, ...)                                                                                      \
    ::Plasma::Console::Print(Plasma::Filter::DefaultFilter, "%s(%d) : " format, __FILE__, __LINE__, __VA_ARGS__)

#else

#  define DebugPrintFilter(...) ((void)0)
#  define DebugPrint(...) ((void)0)
#  define DebugTrace(...) ((void)0)

#endif
