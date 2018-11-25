//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implementation of a run loop.
//===----------------------------------------------------------------------===//

#include "reqserver/Time.h"
#include <chrono>
#include <thread>
#include <iostream>

namespace reqserver
{

template <class Frame>
class RunLoop
{
  Milliseconds frame_interval;

public:

  template <class Duration>
  void set_frame_interval(const Duration& interval)
  { frame_interval = std::chrono::duration_cast<Milliseconds>(interval); }

  void start(const Frame& frame)
  {
    Frame cur_frame = frame;

    while(cur_frame.stop_running() == false) {
      cur_frame = cur_frame.process();
      std::this_thread::sleep_for(frame_interval);
    }
  }
};

class HelloFrame
{
  int counter;

public:
  HelloFrame(int c) : counter(c) {}

  HelloFrame process()
  {
    if(counter == 0)
      return HelloFrame(0);
    std::cout << "Hello World! (" << counter << ")\n";
    return HelloFrame(counter - 1);
  }

  bool stop_running() const
  { return counter == 0; }
};

} // namespace reqserver

using namespace reqserver;

int main(int argc, char** argv)
{
  RunLoop<HelloFrame> run_loop;
  run_loop.set_frame_interval(Milliseconds(100));
  run_loop.start(HelloFrame(16));
  return 0;
}
