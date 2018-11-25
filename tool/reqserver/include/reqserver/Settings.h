//===- Settings.h -----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Reads settings from reqserver.json file.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_SETTINGS_H
#define REQSERVER_SETTINGS_H

#include <memory>
#include <string>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/uri.hpp>

namespace reqserver
{

class Settings
{
  static const auto read_file(const std::string& path)
  {
    auto error = [&path]() {
                   std::stringstream stream;
                   stream << "Could not read file '" << path << "'";
                   return std::runtime_error(stream.str());
                 };
    auto fd = open(path.c_str(), O_RDONLY);
    if(fd < 0)
      throw error();

    off_t size = lseek(fd, 0, SEEK_END);
    if(size < 0) {
      close(fd);
      throw error();
    }
    lseek(fd, 0, SEEK_SET);

    auto data = std::make_unique<char[]>(size + 1);
    read(fd, data.get(), size);
    data[size] = '\0';

    close(fd);
    return std::move(data);
  }

  bsoncxx::document::value jobj;

public:

  Settings(const std::string& path = "reqserver.json")
      : jobj(bsoncxx::from_json(read_file(path).get())) {}

  const mongocxx::uri mongo_uri()
  { return mongocxx::uri(std::string(jobj.view()["mongo-uri"].get_utf8())); }

  const std::string mongo_database()
  { return std::string(jobj.view()["mongo-database"].get_utf8()); }
};

} // reqserver

#endif // REQSERVER_SETTINGS_H
