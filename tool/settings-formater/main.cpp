//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Converts reqserver.json files to the current format.
//===----------------------------------------------------------------------===//

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace reqserver
{

class FormatBase
{
  std::string version_string(std::uint32_t major, std::uint32_t minor)
  {
    std::stringstream stream;
    stream << major << "." << minor;
  }
};

template <std::uint32_t Major, std::uint32_t Minor>
class Format
{
  static_assert(false, "Non-existant version");
};

template <>
class Format<0, 1>
{
  std::string mongo_uri;
  std::string mongo_database;
};

template <>
class Format<0, 2>
{
  std::string server_url;
  std::string database;
  std::string user;
  std::string password;
};

} // namespace reqserver

using namespace reqserver;

int main(int argc, char **argv)
{
  std::cout << "hello world!\n";
  return 0;
}