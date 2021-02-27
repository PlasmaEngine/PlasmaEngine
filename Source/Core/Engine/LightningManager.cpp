// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

namespace Events
{
DefineEvent(ScriptsCompiledPrePatch);
DefineEvent(ScriptsCompiledCommit);
DefineEvent(ScriptsCompiledPatch);
DefineEvent(ScriptsCompiledPostPatch);
DefineEvent(ScriptCompilationFailed);
} // namespace Events

// Lightning Compile Event
LightningDefineType(LightningCompileEvent, builder, type)
{
}

LightningCompileEvent::LightningCompileEvent(HashSet<ResourceLibrary*>& modifiedLibraries) :
    mModifiedLibraries(modifiedLibraries)
{
}

bool LightningCompileEvent::WasTypeModified(BoundType* type)
{
  forRange (ResourceLibrary* lib, mModifiedLibraries.All())
  {
    if (lib->BuiltType(type))
      return true;
  }

  return false;
}

BoundType* LightningCompileEvent::GetReplacingType(BoundType* oldType)
{
  if (!WasTypeModified(oldType))
    return nullptr;

  forRange (ResourceLibrary* lib, mModifiedLibraries.All())
  {
    if (BoundType* newType = lib->GetReplacingType(oldType))
      return newType;
  }

  return nullptr;
}

// Lightning Manager
LightningManager::LightningManager() :
    mVersion(0),
    mShouldAttemptCompile(true),
    mLastCompileResult(CompileResult::CompilationSucceeded)
{
  ConnectThisTo(PL::gEngine, Events::EngineUpdate, OnEngineUpdate);

  EventConnect(&mDebugger, Lightning::Events::DebuggerPause, &LightningManager::OnDebuggerPause, this, &mDebugger);
  EventConnect(&mDebugger, Lightning::Events::DebuggerResume, &LightningManager::OnDebuggerResume, this, &mDebugger);
  EventConnect(&mDebugger, Lightning::Events::DebuggerPauseUpdate, &LightningManager::OnDebuggerPauseUpdate, this, &mDebugger);
  EventConnect(
      &mDebugger, Lightning::Events::DebuggerBreakNotAllowed, &LightningManager::OnDebuggerBreakNotAllowed, this, &mDebugger);
}

void LightningManager::TriggerCompileExternally()
{
  // Currently the version is used to detect duplicate errors
  // If something is externally triggering a compile (such as saving, project
  // loading, playing a game, etc) then we want to show duplicate errors again.
  ++mVersion;
  InternalCompile();
}

void LightningManager::InternalCompile()
{
  if (PL::gEngine->IsReadOnly())
  {
    DoNotifyWarning("Lightning", "Cannot recompile scripts while in read-only mode.");
    return;
  }

  if (!mShouldAttemptCompile)
    return;
  mShouldAttemptCompile = false;

  forRange (ResourceLibrary* resourceLibrary, PL::gResources->LoadedResourceLibraries.Values())
  {
    if (resourceLibrary->CompileScripts(mPendingLibraries) == false)
    {
      Event eventToSend;
      this->DispatchEvent(Events::ScriptCompilationFailed, &eventToSend);
      mLastCompileResult = CompileResult::CompilationFailed;
      return;
    }
  }

  // If there are no pending libraries, nothing was compiled
  ErrorIf(mPendingLibraries.Empty(),
          "If the mShouldAttemptCompile flag was set, we should always have "
          "pending libraries (even at startup with no scripts)!");

  // Since we binary cache archetypes (in a way that is NOT saving the data
  // tree, but rather a 'known serialization format' then if we moved any
  // properties around in any script it would completely destroy how the
  // archetypes were cached The simplest solution is to clear the cache
  ArchetypeManager::GetInstance()->FlushBinaryArchetypes();

  // Scripts were successfully compiled
  LightningCompileEvent compileEvent(mPendingLibraries);

  // If Events::ScriptsCompiledPrePatch is dispatched, we MUST dispatch the
  // PostPatch event after. There cannot be a return in between them. This is
  // due to how we re-initialize Cogs and rebuild Archetype's (see
  // Archetype::sRebuilding)
  this->DispatchEvent(Events::ScriptsCompiledPrePatch, &compileEvent);
  // Library commits must happen after all systems have handle PrePatch
  this->DispatchEvent(Events::ScriptsCompiledCommit, &compileEvent);

  // Unload ALL affected libraries before committing any of them.
  forRange (ResourceLibrary* resourceLibrary, compileEvent.mModifiedLibraries.All())
    resourceLibrary->PreCommitUnload();

  forRange (ResourceLibrary* resourceLibrary, compileEvent.mModifiedLibraries.All())
    resourceLibrary->Commit();

  // @TrevorS: Refactor this to remove a global dependence on a single library.
  if (mPendingFragmentProjectLibrary)
  {
    mCurrentFragmentProjectLibrary = mPendingFragmentProjectLibrary;
    mPendingFragmentProjectLibrary = nullptr;
  }

  this->DispatchEvent(Events::ScriptsCompiledPatch, &compileEvent);
  this->DispatchEvent(Events::ScriptsCompiledPostPatch, &compileEvent);

  MetaDatabase::GetInstance()->ClearRemovedLibraries();

  mPendingLibraries.Clear();

  mLastCompileResult = CompileResult::CompilationSucceeded;
}

void LightningManager::OnEngineUpdate(UpdateEvent* event)
{
  InternalCompile();
}

void LightningManager::OnDebuggerPause(Lightning::DebuggerEvent* event)
{
  if (GetApplicationName() != sEditorName)
    return;

  ScriptEvent toSend;
  toSend.Location = *event->Location;
  toSend.Script = (DocumentResource*)event->Location->CodeUserData;
  PL::gResources->DispatchEvent(Events::DebuggerPaused, &toSend);
}

void LightningManager::OnDebuggerResume(Lightning::DebuggerEvent* event)
{
  if (GetApplicationName() != sEditorName)
    return;

  ScriptEvent toSend;
  toSend.Location = *event->Location;
  toSend.Script = (DocumentResource*)event->Location->CodeUserData;
  PL::gResources->DispatchEvent(Events::DebuggerResumed, &toSend);
}

void LightningManager::OnDebuggerPauseUpdate(Lightning::DebuggerEvent* event)
{
  if (GetApplicationName() != sEditorName)
    return;

  PL::gEngine->mIsDebugging = true;
  PL::gEngine->Update();
  PL::gEngine->mIsDebugging = false;
}

void LightningManager::OnDebuggerBreakNotAllowed(Lightning::DebuggerTextEvent* event)
{
  if (GetApplicationName() != sEditorName)
    return;

  DoNotifyWarning("Debugger", event->Text);
}

} // namespace Plasma
