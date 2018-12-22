//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// main file of the reqserver tool.
//===----------------------------------------------------------------------===//

#include "reqserver/PullDescriptor.h"
#include "reqserver/Settings.h"
#include "reqserver/StrideIterator.h"
#include "reqserver/TimeLoop.h"
#include "reqserver/Util.h"
#include "reqserver/WebPull.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <queue>
#include <tuple>
#include <vector>

namespace reqserver
{

///===---------------------------------------------------------------------===//
/// \var time_loop_interval
///
/// Determines how often often the database should be checked for changes.
const static std::chrono::milliseconds time_loop_interval(1000);

///===---------------------------------------------------------------------===//
/// \class Frame
///
/// Implements 'FrameProtocol' in order to be used with a 'TimeLoop'.
class Frame : public FrameProtocol<Frame, std::chrono::system_clock>
{
  mongocxx::pool pool;
  const std::string database;

public:
  Frame(const Settings& settings)
      : pool(settings.mongo_uri()), database(settings.database())
  {
    set_time_now();
  }

  ///===-------------------------------------------------------------------===//
  /// \func time_changed
  ///   @old_time: Time of the previous click from the time loop.
  ///   @new_time: Time of the current click from the time loop.
  ///
  /// Implements FrameProtocol
  ///
  /// The function can be viewed as being performed in two steps. First it
  /// fetches the pull headers from the database. The headers is used in order
  /// to create a priority queue that contains all the pulls that should be
  /// performed in the time between the current time and the time of the coming
  /// click. Secondly, it goes through that queue and perform each download at
  /// the time it should be made.
  template<class TimePoint>
  void time_changed(const TimePoint& old_time, const TimePoint& new_time)
  {
    // DEBUG
    // std::cout << " {old: " << old_time << ", new: " << new_time
    //           << ", delta: " << milliseconds(new_time - old_time).count()
    //           << "}\n";

    // Acquire thread safe connection to the database.
    auto client = pool.acquire();
    auto collection = (*client)[database]["sites"];

    // Named tuple that stores the time point the pull should be made, the id of
    // the document on the database and the URL to the website to be pulled.
    using Element = std::tuple<TimePoint, OID, std::string>;

    // Create a priority queue that stores all pulls that should be performed
    // until the next click.
    auto comparator = [](Element& a, Element& b) {
      return std::get<0>(a) > std::get<0>(b);
    };
    std::priority_queue<Element, std::vector<Element>, decltype(comparator)>
        queue(std::move(comparator));

    PullDescriptor::each_descriptor(collection, [&new_time, &queue](
                                                    const auto& descriptor) {
      // Make stride iterator that iterates over the times a pull should be
      // performed for the descriptor.
      const auto& stride = descriptor.interval;
      const auto& pull_time = descriptor.date.value_or(new_time);
      StrideIterator it(new_time + stride - (new_time - pull_time) % stride,
                        stride);

      // If no previous pulls has been made for the descriptor it should be
      // downloaded as soon as possible.
      if (!descriptor.date.has_value())
        queue.emplace(
            std::make_tuple(new_time, descriptor.key(), descriptor.url));

      // Queue all pulls that should be performed until the next click from the
      // time loop.
      //
      // TODO:
      // It is assumed that the distance between each click is equal to
      // 'time_loop_interval'. Find a more general and maintanable solution.
      const auto end = new_time + time_loop_interval;
      for (; *it < end; ++it)
        queue.emplace(std::make_tuple(*it, descriptor.key(), descriptor.url));
    });

    while (!queue.empty()) {
      // The top element is always the one with that should be pulled the
      // soonest.
      const auto& time = std::get<0>(queue.top());
      const auto& id = std::get<1>(queue.top());
      const auto& url = std::get<2>(queue.top());

      // Blocks until it is time to download the website.
      std::this_thread::sleep_until(time);

      std::cout << '[' << std::chrono::system_clock::now() << "] pulling from '"
                << url << "'\n";

      try {
        using namespace bsoncxx::builder::basic;
        collection.update_one(
            make_document(kvp("_id", id.bson())),
            make_document(kvp("$push", [&url](sub_document doc) {
              doc.append(kvp("measurements", [&url](sub_document doc) {
                // This call is currently blocking until the website is fully
                // downloaded.
                //
                // It's possible to thread each call but it would
                // not scale since each thread would require it's own client and
                // connection to the database.
                //
                // A preferable solution is to do the pulls using multi curl.
                WebPull::pull_site(url).serialize(doc);
              }));
            })));
      } catch (const mongocxx::exception& exception) {
        warning(exception, __FILE__, __LINE__);
      }
      // Pop the element in order to access the next one.
      queue.pop();
    }
  }
};

} // namespace reqserver

using namespace reqserver;

int main(int argc, char** argv)
{
  // Initialize libraries.
  mongocxx::instance instance;
  curl_global_init(CURL_GLOBAL_ALL);

  // The run loop is launched in its own thread in order to keep things simple.
  // However since both the main thread and the run loop thread is mostly
  // inactive a task switcher could be implemented if this is considered too
  // expensive.
  std::thread(
      [] { TimeLoop<Frame>(Settings()).run_with_interval(time_loop_interval); })
      .detach();

  std::cout << "reqserver launced\n"
            << "Quit program with 'Q+Enter'\n\n";

  // Main thread is blocked until stdin is written to.
  while (char input = std::getc(stdin)) {
    if (input == 'q' || input == 'Q')
      break;
  }

  curl_global_cleanup();
  return 0;
}
