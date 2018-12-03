//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// reqserver main implementation.
//===----------------------------------------------------------------------===//

#include "reqserver/CycleCaller.h"
#include "reqserver/Settings.h"
#include "reqserver/Time.h"
#include "reqserver/TimeLoop.h"
#include "reqserver/Util.h"
#include "reqserver/WebPull.h"
#include <algorithm>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/types.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace reqserver
{

class Frame
{
  mongocxx::client client;
  mongocxx::collection collection;

  /// --------------------------------------------------------------------------
  /// \class: PullDescriptor
  ///
  /// Contains data for the header of a site document on the database.
  class PullDescriptor
  {
    using StringView = decltype(bsoncxx::types::b_utf8::value);

    static std::int32_t get_number(const bsoncxx::document::element& element)
    {
      switch (element.type()) {
      case bsoncxx::type::k_double:
        return element.get_double();
      case bsoncxx::type::k_int32:
        return element.get_int32();
      case bsoncxx::type::k_int64:
        return element.get_int64();
      default:
        throw std::runtime_error(
            "Can not convert bson value into std::int32_t from current type. "
            "Expected bson 'Int32', 'Int64' or 'Double'.");
      }
    }

    PullDescriptor(const OID oid,
                   const StringView& name,
                   const StringView& url,
                   const Milliseconds interval,
                   const std::optional<SysClock::time_point>& date)
        : oid(oid), name(name), url(url), interval(interval), date(date)
    {}

    static std::optional<SysClock::time_point>
    get_date(const bsoncxx::document::element& element)
    {
      switch (element.type()) {
      case bsoncxx::type::k_null:
        return std::nullopt;
      case bsoncxx::type::k_date:
        return element.get_date();
      default:
        throw std::runtime_error("Expected bson date type");
      }
    }

  public:
    const OID oid;
    const std::string name;
    const std::string url;
    const Milliseconds interval;
    const std::optional<SysClock::time_point> date;

    PullDescriptor(const PullDescriptor& other)
        : oid(other.oid), name(other.name), url(other.url),
          interval(other.interval)
    {}

    // Initializes a PullDescriptor from a bson document. Format checking should
    // be done from functions for each field that throws when invalid format
    // accures.
    PullDescriptor(const bsoncxx::document::view& doc)
        : PullDescriptor(doc["_id"].get_oid(),
                         doc["name"].get_utf8().value,
                         doc["url"].get_utf8().value,
                         Milliseconds(get_number(doc["interval"])),
                         get_date(doc["date"]))
    {}

    auto key() const
    {
      return oid;
    }
  };

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
         date = std::move(descr.date)]() {
          using namespace bsoncxx::builder::basic;

          std::cout << '[' << SysClock::now() << "] pulling from '" << name
                    << "'\n";

          try {
            collection.update_one(
                make_document(kvp("name", name)),
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
    collection = client[settings.mongo_database()]["sites"];
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
    // Not checking if the list of descriptors is can cause SEGMENTAION FAULT
    // from the for_each call (193) if it goes from a non-empty state to an
    // empty state.
    //
    // It will cause the last key-value pair to remain in the map but it is not
    // critical since it actually speeds things up not having to
    // perform the loop below when no descriptors should be checked and it will
    // be removed before it can cause any harm.
    if (descriptors.empty())
      return true;
    // (:193)
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
  pokus_time_loop(Frame()).run(Seconds(1));
  curl_global_cleanup();
  return 0;
}
