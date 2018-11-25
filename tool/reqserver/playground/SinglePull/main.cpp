//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Pulls from a single site and pushes the data to a mongo database.
//===----------------------------------------------------------------------===//

#include "reqserver/Time.h"
#include "reqserver/Settings.h"
#include "reqserver/TimeLoop.h"
#include "reqserver/WebPull.h"
#include <iostream>
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>

namespace reqserver
{

class WebPullFrame
{
  mongocxx::client client;
  mongocxx::collection collection;

public:

  WebPullFrame()
  {
    Settings settings;
    client = mongocxx::client{settings.mongo_uri()};
    collection = client[settings.mongo_database()]["site.archlinux.com"];
  }

  template <class Rep, class Period>
  bool advance(const Clock::time_point& time,
               const std::chrono::duration<Rep, Period>& delta)
  {
    std::cout << "pull website...\n";
    collection.insert_one(
        WebPull::pull_site("https://www.archlinux.org").serialize());
    return true;
  }
};

} // namespace reqserver

using namespace reqserver;

int main(int argc, char** argv)
{
  auto time_loop = TimeLoop(WebPullFrame());
  time_loop.run_for(Seconds(5), Seconds(21));
  return 0;
}
