//===- JSONFormat.h ---------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Makes sure the reqserver.json is always in the latest encoding format.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_JSONFORMAT_H
#define REQSERVER_JSONFORMAT_H

#include "reqserver/Util.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <cstdint>
#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace reqserver
{

class ValidDocument
{
  bsoncxx::document::view data;

  ValidDocument(const bsoncxx::document::view& document) : data(document) {}

public:
  static std::optional<ValidDocument>
  validate(const bsoncxx::document::view& document,
           const std::vector<const std::string>& required)
  {
    for (auto& key : required) {
      if (document.find(key) == document.end())
        return std::nullopt;
    }
    return ValidDocument(document);
  }

  auto document() const
  {
    return data;
  }
};

template<std::uint32_t Version>
struct Format {
  // static_assert(false, "Non-existant version");
};

template<>
struct Format<1> {
  using Self = Format<1>;
  const static std::uint32_t version = 1;

  std::string mongo_uri;
  std::string mongo_database;

private:
  Format(const bsoncxx::document::view& doc)
      : mongo_uri(doc["mongo-uri"].get_utf8().value),
        mongo_database(doc["mongo-database"].get_utf8().value)
  {}

public:
  static std::optional<Self> decode(const bsoncxx::document::view& doc)
  {
    auto validated =
        ValidDocument::validate(doc, {"mongo-uri", "mongo-database"});
    if (!validated.has_value())
      return std::nullopt;
    return Format(doc);
  }
};

template<>
struct Format<2> {
  using Self = Format<2>;
  const static std::uint32_t version = 2;

  std::string protocol;
  std::string host;
  int port;
  std::string database;
  std::string user;
  std::string password;

  Format(const Format<1>& old)
  {
    std::smatch match;
    std::regex_match(
        old.mongo_uri, match,
        std::regex("(\\w+)://(\\w+):(\\w+)@([\\w\\./]+):(\\d+)/([\\w]+)"));

    protocol = match[1].str();
    user = match[2].str();
    password = match[3].str();
    host = match[4].str();
    port = std::stoul(match[5].str());
    database = match[6].str();
  }

private:
  Format(const ValidDocument& doc)
      : protocol(to_string(doc.document()["protocol"])),
        user(to_string(doc.document()["user"])),
        password(to_string(doc.document()["password"])),
        host(to_string(doc.document()["host"])),
        port(to_int32(doc.document()["port"])),
        database(to_string(doc.document()["database"]))
  {}

public:
  static std::optional<Format> decode(const bsoncxx::document::view& doc)
  {
    auto validated = ValidDocument::validate(
        doc, {"protocol", "user", "password", "host", "port", "database"});
    if (!validated.has_value()) {
      auto validated = Format<version - 1>::decode(doc);
      if (!validated.has_value())
        return std::nullopt;
      return Format(validated.value());
    }
    return Format(validated.value());
  }

  std::string serialize() const
  {
    using namespace bsoncxx::builder::basic;

    document doc;
    doc.append(kvp("version", int(Format::version)), kvp("protocol", protocol),
               kvp("user", user), kvp("password", password), kvp("host", host),
               kvp("port", port), kvp("database", database));
    return bsoncxx::to_json(doc);
  }
};

using LatestFormat = Format<2>;

} // namespace reqserver

#endif // REQSERVER_JSONFORMAT_H