//===- Settings.h -----------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Reads settings from reqserver.json file.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_SETTINGS_H
#define REQSERVER_SETTINGS_H

#include "reqserver/JSONFormat.h"
#include "reqserver/Util.h"
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <fcntl.h>
#include <memory>
#include <mongocxx/uri.hpp>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

namespace reqserver
{

class Settings
{
public:
  const static int version = 2;

private:
  static const auto read_file(const std::string_view& path)
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

  static void write_file(const std::string_view& path,
                         const std::string_view& str)
  {
    auto fd = open(path.data(), O_WRONLY);
    if (fd < 0) {
      std::stringstream stream;
      stream << "Failed to write file '" << path << "'";
      warning(stream.str(), __FILE__, __LINE__);
      return;
    }
    ftruncate(fd, 0);
    write(fd, str.data(), str.size());
    close(fd);
  }

  Format<version> dict;

  static Format<version> decode(const bsoncxx::document::view& doc)
  {
    auto dict = Format<version>::decode(doc);
    if (!dict.has_value())
      throw std::runtime_error("Could not decode 'reqserver.json'");
    return dict.value();
  }

public:
  Settings(const std::string_view& path = "reqserver.json")
      : dict(decode(bsoncxx::from_json(read_file(path).get())))
  {
    write_file(path, dict.serialize());
  }

  mongocxx::uri mongo_uri() const
  {
    std::stringstream stream;
    stream << dict.protocol << "://" << dict.user << ':' << dict.password << '@'
           << dict.host << ':' << dict.port << '/' << dict.database;
    return bsoncxx::string::view_or_value(stream.str());
  }

  std::string database() const
  {
    return dict.database;
  }
};

} // namespace reqserver

#endif // REQSERVER_SETTINGS_H
