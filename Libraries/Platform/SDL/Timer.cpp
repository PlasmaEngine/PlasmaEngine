// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

struct TimerPrivateData
{
  u64 mStartCount;
  u64 mCurrentTickCount;
  u64 mCurrentTickDelta;
  double mFrequency;
  double mCurTimeDelta;
  double mCurTime;
};

Timer::Timer()
{
  PlasmaConstructPrivateData(TimerPrivateData);

  self->mFrequency = (double)SDL_GetPerformanceFrequency();
  Reset();
}

Timer::~Timer()
{
  PlasmaDestructPrivateData(TimerPrivateData);
}

Timer::Timer(const Timer& rhs)
{
  memcpy(this, &rhs, sizeof(*this));
}

Timer& Timer::operator=(const Timer& rhs)
{
  if (this == &rhs)
    return *this;

  memcpy(this, &rhs, sizeof(*this));
  return *this;
}

void Timer::Reset()
{
  PlasmaGetPrivateData(TimerPrivateData);

  self->mStartCount = SDL_GetPerformanceCounter();
  self->mCurrentTickCount = 0;
  self->mCurrentTickDelta = 0;
  self->mCurTimeDelta = 0;
  self->mCurTime = 0;
}

void Timer::Update()
{
  PlasmaGetPrivateData(TimerPrivateData);

  u64 lastTickCount = self->mCurrentTickCount;
  u64 currentCount = SDL_GetPerformanceCounter();

  self->mCurrentTickCount = currentCount - self->mStartCount;
  self->mCurrentTickDelta = self->mCurrentTickCount - lastTickCount;

  self->mCurTime = self->mCurrentTickCount / self->mFrequency;
  self->mCurTimeDelta = self->mCurrentTickDelta / self->mFrequency;
}

double Timer::Time() const
{
  PlasmaGetPrivateData(TimerPrivateData);
  return self->mCurTime;
}

double Timer::TimeDelta() const
{
  PlasmaGetPrivateData(TimerPrivateData);
  return self->mCurTimeDelta;
}

double Timer::UpdateAndGetTime()
{
  Update();
  return Time();
}

TimeMs Timer::TimeMilliseconds() const
{
  return TimeMs(Time() * double(1000));
}

TimeMs Timer::TimeDeltaMilliseconds() const
{
  return TimeMs(TimeDelta() * double(1000));
}

TimeMs Timer::UpdateAndGetTimeMilliseconds()
{
  return TimeMs(UpdateAndGetTime() * double(1000));
}

double Timer::TimeNoUpdate() const
{
  PlasmaGetPrivateData(TimerPrivateData);
  u64 currentCount = SDL_GetPerformanceCounter();
  u64 TempTicks = currentCount - self->mStartCount;
  return TempTicks / self->mFrequency;
}

Timer::TickType Timer::GetTickTime() const
{
  PlasmaGetPrivateData(TimerPrivateData);
  u64 currentCount = SDL_GetPerformanceCounter();
  return currentCount - self->mStartCount;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  PlasmaGetPrivateData(TimerPrivateData);
  return (double)(ticks / self->mFrequency);
}

} // namespace Plasma
