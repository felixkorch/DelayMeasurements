//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Simple connection to a mongo server.
//===----------------------------------------------------------------------===//

#include "reqserver/Settings.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/collection.hpp>


using namespace reqserver;

int main(int argc, char** argv)
{
  using namespace bsoncxx::builder::basic;

  Settings settings;
  mongocxx::instance instance;
  auto client = mongocxx::client{settings.mongo_uri()};
  auto session = client.start_session();
  auto collection = client[settings.mongo_database()]["MongoTest.temp"];
  collection.insert_one(make_document(kvp("hello", "world")));

  std::this_thread::sleep_for(std::chrono::minutes(1));

  collection.drop();

  return 0;
}
