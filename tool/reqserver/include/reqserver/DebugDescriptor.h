//===- DebugDescriptor.h -----------------------------------------*- C++-*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Override output stream behavior for debug purposes.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_DEBUGDESCRIPTOR_H
#define REQSERVER_DEBUGDESCRIPTOR_H

#include <iostream>
#include <chrono>

template<class T>
struct DebugDescriptor {
  const T& value;
  DebugDescriptor(const T& value) : value(value){};
};

template<class CharT>
std::basic_ostream<CharT>&
operator<<(std::basic_ostream<CharT>& stream,
           const DebugDescriptor<std::chrono::system_clock::time_point>& time)
{
  char str[32];
  auto ctime = std::chrono::system_clock::to_time_t(time.value);
  std::strftime(str, sizeof(str), "%F %T %z", std::localtime(&ctime));
  return stream << str;
}

#endif // REQSERVER_DEBUGDESCRIPTOR_H