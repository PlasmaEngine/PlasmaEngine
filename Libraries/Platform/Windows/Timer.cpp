// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

struct TimerPrivateData
{
  long long mStartCount;
  long long mCurrentTickCount;
  long long mCurrentTickDelta;
  double mFrequency;
  double mCurTimeDelta;
  double mCurTime;
};

Timer::Timer()
{
  PlasmaConstructPrivateData(TimerPrivateData);

  LARGE_INTEGER Frequency;
  QueryPerformanceFrequency(&Frequency);
  // mInvFrequency = 1.0 / (double)Frequency.QuadPart;
  self->mFrequency = (double)Frequency.QuadPart;
  Reset();
}

Timer::~Timer()
{
  PlasmaDestructPrivateData(TimerPrivateData);
}

Timer::Timer(const Timer& rhs)
{
  // Our entire structure is mem-copyable
  memcpy(this, &rhs, sizeof(*this));
}

Timer& Timer::operator=(const Timer& rhs)
{
  if (this == &rhs)
    return *this;
  // Our entire structure is mem-copyable
  memcpy(this, &rhs, sizeof(*this));
  return *this;
}

void Timer::Reset()
{
  PlasmaGetPrivateData(TimerPrivateData);
  LARGE_INTEGER Count;
  QueryPerformanceCounter(&Count);
  self->mStartCount = Count.QuadPart;

  self->mCurrentTickCount = 0;
  self->mCurrentTickDelta = 0;
  self->mCurTimeDelta = 0;
  self->mCurTime = 0;
}

void Timer::Update()
{
  PlasmaGetPrivateData(TimerPrivateData);
  TickType lastTickCount = self->mCurrentTickCount;
  LARGE_INTEGER CurrCount;
  QueryPerformanceCounter(&CurrCount);
  self->mCurrentTickCount = (CurrCount.QuadPart - self->mStartCount);
  self->mCurrentTickDelta = self->mCurrentTickCount - lastTickCount;

  // mCurTime =  mCurrentTickCount * mInvFrequency;
  self->mCurTime = self->mCurrentTickCount / self->mFrequency;
  // mCurTimeDelta = mCurrentTickDelta * mInvFrequency;
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
  LARGE_INTEGER CurrCount;
  QueryPerformanceCounter(&CurrCount);
  TickType TempTicks = (CurrCount.QuadPart - self->mStartCount);
  // return (double)(TempTicks * mInvFrequency);
  return (double)(TempTicks / self->mFrequency);
}

Timer::TickType Timer::GetTickTime() const
{
  PlasmaGetPrivateData(TimerPrivateData);
  LARGE_INTEGER CurrCount;
  QueryPerformanceCounter(&CurrCount);
  return CurrCount.QuadPart - self->mStartCount;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  PlasmaGetPrivateData(TimerPrivateData);
  // return (double)(ticks* mInvFrequency);
  return (double)(ticks / self->mFrequency);
}

} // namespace Plasma
