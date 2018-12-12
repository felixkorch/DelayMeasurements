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
#include <ctime>
#include <iostream>
#include <optional>

namespace reqserver
{

using Clock = std::chrono::steady_clock;
using SysClock = std::chrono::system_clock;
using Seconds = std::chrono::seconds;
using Milliseconds = std::chrono::milliseconds;
using Microseconds = std::chrono::microseconds;

template<class Rep, class Period>
constexpr auto milliseconds(const std::chrono::duration<Rep, Period>& dur)
{
  return std::chrono::duration_cast<Milliseconds>(dur);
}

template<class Rep, class Period>
std::optional<Milliseconds>
milliseconds(const std::optional<std::chrono::duration<Rep, Period>>& dur)
{
  if (!dur.has_value())
    return {};
  return dur.value();
}

std::optional<Milliseconds> milliseconds(const std::optional<std::int32_t>& n)
{
  if (!n.has_value())
    return {};
  return Milliseconds(n.value());
}

template<class CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& stream,
                                      const SysClock::time_point& time)
{
  auto t = SysClock::to_time_t(time);
  char str[32];
  std::strftime(str, sizeof(str), "%F %T %z", std::localtime(&t));
  return stream << str;
}

} // namespace reqserver

#endif // REQSERVER_TIME_H
