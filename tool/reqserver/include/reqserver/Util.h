//===- Util.h ---------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Simple utility functions that are hard to bind to a theme.
//===----------------------------------------------------------------------===//

#include <bsoncxx/types.hpp>
#include <iostream>

namespace reqserver
{
bool operator<(const bsoncxx::types::b_oid& a, const bsoncxx::types::b_oid& b)
{
  return a.value < b.value;
}

class OID
{
  bsoncxx::types::b_oid raw_value;

public:
  OID(bsoncxx::types::b_oid raw_value) : raw_value(raw_value) {}

  friend bool operator<(const OID& a, const OID& b);
  friend bool operator==(const OID& a, const OID& b);
};

bool operator<(const OID& a, const OID& b)
{
  return a.raw_value < b.raw_value;
}

bool operator==(const OID& a, const OID& b)
{
  return a.raw_value == b.raw_value;
}

template<class Exception>
void warning(const Exception& exception, const char* file, int line)
{
  std::cerr << '[' << SysClock::now() << "] Warning " << file << ':' << line
            << " - " << exception.what() << '\n';
}

} // namespace reqserver