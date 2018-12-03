//===- Settings.h -----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Reads settings from reqserver.json file.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_SETTINGS_H
#define REQSERVER_SETTINGS_H

#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <fcntl.h>
#include <memory>
#include <mongocxx/uri.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

namespace reqserver
{

class Settings
{
  static const auto read_file(const std::string_view &path)
  {
    auto error = [&path]() {
      std::stringstream stream;
      stream << "Could not read file '" << path << "'";
      return std::runtime_error(stream.str());
    };
    auto fd = open(path.data(), O_RDONLY);
    if (fd < 0)
      throw error();

    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
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
  Settings(const std::string_view &path = "reqserver.json")
      : jobj(bsoncxx::from_json(read_file(path).get()))
  {}

  const mongocxx::uri mongo_uri()
  {
    return bsoncxx::string::view_or_value(
        jobj.view()["mongo-uri"].get_utf8().value);
  }

  const std::string mongo_database()
  {
    auto view = jobj.view()["mongo-database"].get_utf8().value;
    return std::string(view.begin(), view.end());
  }
};

} // namespace reqserver

#endif // REQSERVER_SETTINGS_H
