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

namespace reqserver
{

class CycleCallerBase
{
  template <class Rep, class Period>
  using duration = std::chrono::duration<Rep, Period>;

  template <class Clock, class Duration>
  using time_point = std::chrono::time_point<Clock, Duration>;

  int interval_dur;
  int progress;

  virtual void trigger() {}

  bool advance(int ms)
  {
    progress += ms;
    if(progress < interval_dur)
      return false;
    progress %= interval_dur;
    return true;
  }

public:

  CycleCallerBase(const Milliseconds& interval,
                  const Milliseconds& progress = Milliseconds(0))
      : interval_dur(interval.count()),
        progress(progress.count() % interval.count()) {}

  template <class Rep1, class Period1, class Rep2, class Period2>
  CycleCallerBase(const duration<Rep1, Period1>& interval,
                  const duration<Rep2, Period2>& progress =
                  duration<Rep2, Period2>::zero())
      : CycleCallerBase(milliseconds(interval), milliseconds(progress)) {}

  /// --------------------------------------------------------------------------
  /// \func: void forward
  ///   @dur: Duration to increase the timer.
  ///
  /// Increases the timer by duration without triggering the cycle even if it
  /// passes the interval. Returns true if the interval was passed.
  template <class Rep, class Period>
  bool forward(const duration<Rep, Period>& dur)
  { return advance(milliseconds(dur).count()); }

  /// --------------------------------------------------------------------------
  /// \func: void increment
  ///   @dur: Duration to increase the timer.
  ///
  /// Increases the timer by duration and triggers the call if it passes the
  /// interval. Returns true if the interval was passed.
  template <class Rep, class Period>
  bool increment(const duration<Rep, Period>& dur)
  {
    if(advance(milliseconds(dur).count())) {
      trigger();
      return true;
    }
    return false;
  }

  auto interval() const
  { return Milliseconds(interval_dur); }

  void set_interval(const Milliseconds& new_value )
  { interval_dur = new_value.count(); }

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
