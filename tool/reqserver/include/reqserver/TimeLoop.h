//===- TimeLoop.h -----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Time loop that might lack behind if the lambdas takes to long to execute.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_TIMELOOP_H
#define REQSERVER_TIMELOOP_H

#include "reqserver/Time.h"
#include <chrono>
#include <optional>
#include <thread>

namespace reqserver
{

template<class Frame>
class TimeLoop
{
  template<class Rep, class Period>
  using duration = std::chrono::duration<Rep, Period>;

  template<class Clock, class Duration>
  using time_point = std::chrono::time_point<Clock, Duration>;

  Frame frame;
  Clock::time_point prev_time;

  template<class Rep, class Period>
  bool sleep_for(const duration<Rep, Period>& dur)
  {
    std::this_thread::sleep_for(dur);
    return true;
  }

  template<class Pred, class Rep, class Period>
  int run_once(const Pred& p, const duration<Rep, Period>& interval)
  {
    auto start = Clock::now();
    auto delta = start - prev_time;
    if (auto adv = frame.advance(start, delta); !(adv && p(start, delta)))
      return adv ? 0 : 2;
    if (sleep_for(interval - (Clock::now() - start)) == false)
      return 0;
    prev_time = start;
    return 3;
  }

public:
  TimeLoop(Frame&& frame) : frame(std::move(frame)) {}

  template<class Rep, class Period, class Pred>
  bool run_while(const duration<Rep, Period>& interval, const Pred& p)
  {
    prev_time = Clock::now();
    if (sleep_for(interval) == false)
      return false;
    while (true) {
      if (auto r = run_once(p, interval); (r & 1) == 0)
        return r & 2;
    }
  }

  template<class Rep1, class Period1, class Rep2, class Period2>
  bool run_for(const duration<Rep1, Period1>& interval,
               const duration<Rep2, Period2>& dur)
  {
    auto stop = Clock::now() + dur;
    return run_while(interval, [stop](auto now, auto) { return now < stop; });
  }

  template<class Rep, class Period>
  bool run(const duration<Rep, Period>& interval)
  {
    return run_while(interval, [](auto, auto) { return true; });
  }
};

template<class Frame>
auto pokus_time_loop(Frame&& frame)
{
  return TimeLoop<Frame>(std::move(frame));
}

} // namespace reqserver

#endif // REQSERVER_UNSTEADYTIMELOOP_H
