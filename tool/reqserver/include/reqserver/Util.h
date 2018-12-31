//===- Util.h ----------------------------------------------------*- C++
//-*-===//
//
// License goes here.
//===-----------------------------------------------------------------------===//
//
// Simple utility functions that are hard to bind to a theme.
//===-----------------------------------------------------------------------===//

#ifndef REQSERVER_UTIL_H
#define REQSERVER_UTIL_H

#include "reqserver/DebugDescriptor.h"
#include <bsoncxx/types.hpp>
#include <iostream>
#include <optional>

namespace reqserver
{

bool operator<(const bsoncxx::types::b_oid& a, const bsoncxx::types::b_oid& b)
{
  return a.value < b.value;
}

template<class Lambda>
std::optional<std::invoke_result_t<Lambda>> try_optional(const Lambda& f)
{
  try {
    return f();
  } catch (...) {
    return std::nullopt;
  }
}

void warning(const std::string& message, const char* file, int line)
{
  std::cerr << '[' << DebugDescriptor(std::chrono::system_clock::now())
            << "] Warning " << file << ':' << line << " - " << message << '\n';
}

template<class Exception>
void warning(const Exception& exception, const char* file, int line)
{
  warning(std::string(exception.what()), file, line);
}

static std::int32_t to_int32(const bsoncxx::document::element& element)
{
  switch (element.type()) {
  case bsoncxx::type::k_double:
    return element.get_double();
  case bsoncxx::type::k_int32:
    return element.get_int32();
  case bsoncxx::type::k_int64:
    return element.get_int64();
  default:
    throw std::runtime_error(
        "Can not convert bson value into std::int32_t from current type. "
        "Expected bson 'Int32', 'Int64' or 'Double'.");
  }
}

std::string to_string(const bsoncxx::document::element& element)
{
  return std::string(element.get_utf8().value);
}

static std::chrono::system_clock::time_point
to_date(const bsoncxx::document::element& element)
{
  switch (element.type()) {
  case bsoncxx::type::k_date:
    return element.get_date();
  default:
    throw std::runtime_error("Expected bson date type");
  }
}

} // namespace reqserver

#endif // REQSERVER_UTIL_H