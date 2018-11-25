//===- Time.h ---------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Clocks, dates and times.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_TIME_H
#define REQSERVER_TIME_H

#include <chrono>

namespace reqserver
{

using Clock = std::chrono::steady_clock;
using SysClock = std::chrono::system_clock;
using Seconds = std::chrono::seconds;
using Milliseconds = std::chrono::milliseconds;
using Microseconds = std::chrono::microseconds;

template <class Rep, class Period>
constexpr auto milliseconds(const std::chrono::duration<Rep, Period>& dur)
{ return std::chrono::duration_cast<Milliseconds>(dur); }


} // namespace reqserver


#endif // REQSERVER_TIME_H
