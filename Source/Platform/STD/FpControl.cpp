// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include <cfenv>

namespace Plasma
{

// by default, we want all of the fpu exceptions apart from inexact
///(inexact happens in lots of odd places...) and underflow
uint FpuControlSystem::DefaultMask = FE_INEXACT | FE_UNDERFLOW;

/// Stores that by default floating point exceptions are enabled
bool FpuControlSystem::Active = true;

struct FpuPrivateData
{
  std::fexcept_t mOldState;
};

ScopeFpuExceptionsEnabler::ScopeFpuExceptionsEnabler()
{
  PlasmaConstructPrivateData(FpuPrivateData);
  /// only scope change if the fpu control system is active
  if (FpuControlSystem::Active == false)
    return;

  // get the old state so we know what to go back to
  fegetexceptflag(&(self->mOldState), FE_ALL_EXCEPT);
  feclearexcept(FE_ALL_EXCEPT);
  // set the new state
  std::fexcept_t currState;
  fesetexceptflag(&currState, FpuControlSystem::DefaultMask);
}

ScopeFpuExceptionsEnabler::~ScopeFpuExceptionsEnabler()
{
  PlasmaGetPrivateData(FpuPrivateData);
  /// only scope change if the fpu control system is active
  if (FpuControlSystem::Active == false)
    return;

  // set the old state back
  feclearexcept(FE_ALL_EXCEPT);
  fesetexceptflag(&(self->mOldState), FE_ALL_EXCEPT);

  PlasmaDestructPrivateData(FpuPrivateData);
}

ScopeFpuExceptionsDisabler::ScopeFpuExceptionsDisabler()
{
  PlasmaConstructPrivateData(FpuPrivateData);
  /// only scope change if the fpu control system is active
  if (FpuControlSystem::Active == false)
    return;

  // get the old state
  fegetexceptflag(&(self->mOldState), FE_ALL_EXCEPT);
  // set all of the exception flags which disables all fp exceptions.
  std::fexcept_t currState = FE_ALL_EXCEPT;
  fesetexceptflag(&currState, FE_ALL_EXCEPT);
}

ScopeFpuExceptionsDisabler::~ScopeFpuExceptionsDisabler()
{
  PlasmaGetPrivateData(FpuPrivateData);
  /// only scope change if the fpu control system is active
  if (FpuControlSystem::Active == false)
    return;

  // clear any pending fp exceptions otherwise there may be a
  //'deferred crash' as soon as the exceptions are enabled.
  feclearexcept(FE_ALL_EXCEPT);

  // now reset the exceptions to what they were
  fesetexceptflag(&(self->mOldState), FE_ALL_EXCEPT);

  PlasmaDestructPrivateData(FpuPrivateData);
}

} // namespace Plasma
