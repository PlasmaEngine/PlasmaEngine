// MIT Licensed (see LICENSE.md).
#pragma once

namespace Plasma
{

void CrashStartCallback(CrashInfo& info, void* userData);
void CrashPreMemoryDumpCallback(void* userData);
bool CrashCustomMemoryCallback(MemoryRange& memRange, void* userData);

void CrashLoggingCallback(CrashHandlerParameters& params, CrashInfo& info, void* userData);
void SendCrashReport(CrashHandlerParameters& params, void* userData);

} // namespace Plasma
