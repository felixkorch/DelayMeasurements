//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Calls lambdas on timed cycles.
//===----------------------------------------------------------------------===//

#include "reqserver/Time.h"
#include "reqserver/TimeLoop.h"
#include "reqserver/CycleCaller.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

namespace reqserver
{

class CycleFrame
{
  const Clock::time_point init_time;

public:

  CycleFrame() : init_time(Clock::now()) {}

  std::vector<std::unique_ptr<CycleCallerBase>> callers;

  template <class Rep, class Period>
  bool advance(const Clock::time_point& time,
               const std::chrono::duration<Rep, Period>& delta)
  {
    std::cout << "\t" << std::chrono::duration<float>(time - init_time).count()
              << "s\n";
    std::for_each(callers.begin(),
                  callers.end(),
                  [delta](auto& x) { x->incr_timer(delta); });
    return true;
  }

};

} // reqserver

using namespace reqserver;

int main(int argc, char** argv) {
  CycleFrame frame;
  for(int i = 0; i < 16; ++i) {
    auto dur = 100 + 64 * i;
    int n = 0;
    frame.callers.push_back(
      unique_cycle_caller(
          Milliseconds(dur),
          [i, dur, &n]() {
            std::cout << n++ << ". (" << i << ", " << dur << "ms)\n";
          }));
  }
  TimeLoop time_loop(std::move(frame));
  time_loop.run_for(Seconds(4), Milliseconds(100));
  return 0;
}
