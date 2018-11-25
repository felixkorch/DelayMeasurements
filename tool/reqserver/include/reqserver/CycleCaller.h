//===- CycleCaller.h --------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Calls lambdas during runtime scheduled after repeating cycles.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_CYCLECALLER_H
#define REQSERVER_CYCLECALLER_H

#include "reqserver/Time.h"
#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>

namespace reqserver
{

class CycleCallerBase
{
  template <class Rep, class Period>
  using duration = std::chrono::duration<Rep, Period>;

  template <class Clock, class Duration>
  using time_point = std::chrono::time_point<Clock, Duration>;

  int cycle_duration;
  int cycle_progress;

  virtual void trigger() {}

  bool incr_timer(int ms)
  {
    cycle_progress += ms;
    if(cycle_progress < cycle_duration)
      return false;
    cycle_progress %= cycle_duration;
    trigger();
    return true;
  }

public:

  CycleCallerBase(const Milliseconds& interval)
      : cycle_duration(interval.count()), cycle_progress(0) {}

  template <class Rep, class Period>
  CycleCallerBase(const duration<Rep, Period>& interval)
      : CycleCallerBase(milliseconds(interval)) {}

  template <class Rep, class Period>
  auto incr_timer(const duration<Rep, Period>& dur)
  { return incr_timer(milliseconds(dur).count()); }

  auto interval() const
  { return Milliseconds(cycle_duration); }

  void set_interval(const Milliseconds& new_value )
  { cycle_duration = new_value.count(); }

};

template <class Lambda>
class CycleCaller : public CycleCallerBase
{
  template <class Rep, class Period>
  using duration = std::chrono::duration<Rep, Period>;

  const Lambda f;

  void trigger()
  { f(); }

public:

  template <class Rep, class Period>
  CycleCaller(const duration<Rep, Period>& interval, const Lambda& f)
      : f(f), CycleCallerBase(interval) {}
};

template <class Rep, class Period, class Lambda>
static auto unique_cycle_caller(
    const std::chrono::duration<Rep, Period>& interval,
    const Lambda& f)
{ return std::make_unique<CycleCaller<Lambda>>(interval, f); }


template <class Rep, class Period, class Lambda>
static auto shared_cycle_caller(
    const std::chrono::duration<Rep, Period>& interval,
    const Lambda& f)
{ return std::make_shared<CycleCaller<Lambda>>(interval, f); }

} // namespace reqserver

#endif // REQSERVER_CYCLECALLER_H
