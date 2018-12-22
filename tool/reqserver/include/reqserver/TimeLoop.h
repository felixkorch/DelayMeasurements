//===- TimeLoop.h -----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Advances an object that has implemented the frame protocol with a fixed time
// interval.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_TIMELOOP_H
#define REQSERVER_TIMELOOP_H

#include <chrono>
#include <thread>
#include <type_traits>

namespace reqserver
{

template<class Derived, class Clock = std::chrono::steady_clock>
class FrameProtocol
{
  typename Clock::time_point time;

public:
  template<class Rep, class Period>
  void advance(const std::chrono::duration<Rep, Period>& duration)
  {
    auto old_time = time;
    auto new_time = old_time + duration;

    std::thread([this, old_time, new_time]() {
      static_cast<Derived*>(this)->time_changed(old_time, new_time);
    }).detach();

    time = new_time;
  }

  void set_time_now()
  {
    time = Clock::now();
  }
};

template<class Frame>
class TimeLoop
{
  // static_assert(std::is_base_of<FrameProtocol<Frame>, Frame>());

  template<class Rep, class Period>
  using Duration = std::chrono::duration<Rep, Period>;

  Frame frame;

public:
  template<class... Args>
  TimeLoop(Args&&... args) : frame(Frame(std::forward<Args>(args)...))
  {}

  template<class Rep, class Period>
  void run_with_interval(const Duration<Rep, Period>& duration)
  {
    while (1) {
      frame.advance(duration);
      std::this_thread::sleep_for(duration);
    }
  }
};

} // namespace reqserver

#endif // REQSERVER_UNSTEADYTIMELOOP_H
