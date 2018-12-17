//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// reqserver main implementation.
//===----------------------------------------------------------------------===//

#include "reqserver/CycleCaller.h"
#include "reqserver/PullDescriptor.h"
#include "reqserver/Settings.h"
#include "reqserver/Time.h"
#include "reqserver/TimeLoop.h"
#include "reqserver/Util.h"
#include "reqserver/WebPull.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <curl/curl.h>
#include <map>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/exception/exception.hpp>
#include <vector>

namespace reqserver
{

class Frame
{
  mongocxx::client client;
  mongocxx::collection collection;

  /// --------------------------------------------------------------------------
  /// \func: static auto make_caller
  ///   @descr: Pull descriptor.
  ///   @collection: Destination collection.
  ///
  /// Makes a cyclic caller that measures the time it takes to download a url
  /// and pushes the result to the database.
  static auto make_caller(const PullDescriptor&& descr,
                          mongocxx::collection& collection)
  {
    auto caller = shared_cycle_caller(
        descr.interval,
        [&collection, name = std::move(descr.name), url = std::move(descr.url),
         date = std::move(descr.date), id = std::move(descr.oid)]() {
          using namespace bsoncxx::builder::basic;

          std::cout << '[' << SysClock::now() << "] pulling from '" << name
                    << "'\n";

          // Try to push pulled data to measurements array for the site.
          try {
            collection.update_one(
                make_document(kvp("_id", id.bson())),
                make_document(kvp("$push", [&url](sub_document doc) {
                  doc.append(kvp("measurements", [&url](sub_document doc) {
                    WebPull::pull_site(url).serialize(doc);
                  }));
                })));
          } catch (const mongocxx::exception& exception) {
            warning(exception, __FILE__, __LINE__);
          }
        });
    if (descr.date.has_value())
      caller->forward(SysClock::now() - descr.date.value());
    return caller;
  }

  /// --------------------------------------------------------------------------
  /// \ivar: std::map<OID, std::shared_ptr<CycleCallerBase>> sites
  ///
  /// Map containing a key and a cycle caller for each sites that is being
  /// measured.
  std::map<OID, std::shared_ptr<CycleCallerBase>> sites;

  /// --------------------------------------------------------------------------
  /// \func: auto fetch_descriptors()
  ///
  /// Makes a quary to the mongo server and returns a list of pull descriptors
  /// based on the data.
  auto fetch_descriptors()
  {
    using namespace bsoncxx::builder::basic;

    std::vector<PullDescriptor> container;
    // Get cursor of header data from server.
    auto cursor = collection.aggregate(mongocxx::pipeline().project(
        make_document(kvp("name", true), kvp("url", true),
                      kvp("interval", true), kvp("date", [](sub_document doc) {
                        doc.append(kvp("$max", "$measurements.date"));
                      }))));
    // Catch exceptions regarding bad format from the server.
    std::for_each(cursor.begin(), cursor.end(), [&container](auto&& doc) {
      try {
        container.emplace_back(doc);
      } catch (const std::exception& exception) {
        warning(exception, __FILE__, __LINE__);
      }
    });
    return std::move(container);
  }

public:
  Frame()
  {
    Settings settings;
    client = mongocxx::client{settings.mongo_uri()};
    collection = client[settings.database()]["sites"];
  }

  /// --------------------------------------------------------------------------
  /// \func: bool advance
  ///   @time: Time of advance.
  ///   @delta: Duration since last advance.
  ///
  /// Implements frame in order to use object in TimeLoop. Updates the map
  /// containing the cycle callers pulling data from sites.
  template<class Rep, class Period>
  bool advance(const Clock::time_point& time,
               const std::chrono::duration<Rep, Period>& delta)
  {
    auto& sites = this->sites;
    auto& collection = this->collection;
    const auto descriptors = fetch_descriptors();
    // Not checking if the list of descriptors is empty can cause SEGMENTAION
    // FAULT from the for_each call below if it goes from a non-empty state
    // to an empty state.
    if (descriptors.empty()) {
      sites.clear();
      return true;
    }
    // Remove non existant keys.
    std::for_each(sites.cbegin(), sites.cend(),
                  [&sites, &descriptors](auto& x) {
                    using namespace bsoncxx::builder::basic;
                    // If a key from the map can't be found among the
                    // descriptors it has been removed from the server
                    auto descr = std::find_if(
                        descriptors.cbegin(), descriptors.cend(),
                        [&x](auto& descr) { return x.first == descr.key(); });
                    // Remove pairs where the key is not found.
                    if (descr == descriptors.cend())
                      sites.erase(sites.find(x.first));
                  });
    // Add new callers and increment old ones.
    std::for_each(
        descriptors.begin(), descriptors.end(),
        [&sites, &collection, delta](auto& descr) {
          if (sites.find(descr.key()) == sites.end()) {
            // Make and add cycle caller for the site if the key is not in
            // the map.
            sites.emplace(std::make_pair(
                descr.key(), make_caller(std::move(descr), collection)));
            return;
          }
          // Increment cycle interval. Triggers will block!
          auto cycle = sites[descr.key()];
          if (cycle->interval() != descr.interval)
            cycle->set_interval(descr.interval);
          cycle->increment(delta);
        });
    return true;
  }

}; // class Frame

} // namespace reqserver

using namespace reqserver;

int main(int argc, char** argv)
{
  curl_global_init(CURL_GLOBAL_ALL);
  make_time_loop(Frame()).run(Seconds(1));
  curl_global_cleanup();
  return 0;
}
