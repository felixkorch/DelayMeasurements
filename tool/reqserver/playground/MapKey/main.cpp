//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Implements custom key type for map.
//===----------------------------------------------------------------------===//

#include "reqserver/Settings.h"
#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/instance.hpp>

namespace reqserver
{

bool operator<(const bsoncxx::types::b_oid& a, const bsoncxx::types::b_oid& b)
{
  return a.value < b.value;
}

class Key
{
  bsoncxx::types::b_oid raw_value;

public:
  Key(bsoncxx::types::b_oid raw_value) : raw_value(raw_value) {}

  friend bool operator<(const Key& a, const Key& b);
  friend bool operator==(const Key& a, const Key& b);
};

bool operator<(const Key& a, const Key& b)
{
  return a.raw_value < b.raw_value;
}

bool operator==(const Key& a, const Key& b)
{
  return a.raw_value == b.raw_value;
}

} // namespace reqserver

using namespace reqserver;

int main(int argc, char** argv)
{
  using namespace bsoncxx::builder::basic;

  mongocxx::instance instance;
  Settings settings;

  auto client = mongocxx::client{settings.mongo_uri()};
  auto collection = client[settings.mongo_database()]["MapKey.temp"];
  collection.insert_one(make_document(kvp("key", std::int32_t(5))));
  collection.insert_one(make_document(kvp("key", std::int32_t(66))));

  std::map<Key, std::int32_t> oid_map;

  auto cursor = collection.find(make_document());

  for (auto& doc : cursor) {
    oid_map.emplace(
        std::make_pair(doc["_id"].get_oid(), doc["key"].get_int32()));
  }
  for (auto& x : oid_map) {
    std::cout << "value: " << x.second << '\n';
  }

  return 0;
}