//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Put a simple mongo document on a server.
//===----------------------------------------------------------------------===//

#include <iostream>
#include <chrono>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>

namespace reqserver
{

} // namespace reqserver

using namespace reqserver;

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::sub_array;

int main(int argc, char** argv) {
  auto doc = bsoncxx::builder::basic::document();
  doc.append(kvp("hello", "world"));
  doc.append(kvp("subdoc",
                 [](sub_document subdoc) {
                   subdoc.append(kvp("some-key", "some-value"),
                                 kvp("key2", 1337));
                 }),
             kvp("subarr",
                 [](sub_array subarr) {
                   subarr.append(1,
                                 "hello",
                                 false,
                                 5,
                                 [](sub_document subdoc) {
                                   subdoc.append(kvp("kk", "vv"),
                                                 kvp("k2", 98));
                                 });
                       }));
  std::cout << bsoncxx::to_json(doc) << "\n";
  return 0;
}
