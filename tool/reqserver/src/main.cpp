//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// reqserver main implementation.
//===----------------------------------------------------------------------===//

#include "reqserver/Time.h"
#include "reqserver/Settings.h"
#include "reqserver/TimeLoop.h"
#include "reqserver/CycleCaller.h"
#include "reqserver/WebPull.h"
#include <utility>
#include <type_traits>
#include <memory>
#include <optional>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>
#include <bsoncxx/types.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>

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
    PullDescriptor(const std::string& name,
                   const std::string& url,
                   const Milliseconds& interval,
                   const std::optional<SysClock::time_point>& date)
        : name(name), url(url), interval(interval), date(date) {}

    static std::int32_t get_number(const bsoncxx::document::element& element)
    {
      switch(element.type()) {
      case bsoncxx::type::k_double: return element.get_double();
      case bsoncxx::type::k_int32: return element.get_int32();
      case bsoncxx::type::k_int64: return element.get_int64();
      default:
        throw std::runtime_error("Invalid interval type");
      }
    }

    static std::optional<SysClock::time_point> get_date(
        const bsoncxx::document::element& element)
    {
      if(element.type() != bsoncxx::type::k_date)
        return std::nullopt;
      return element.get_date();
    }

  public:
    using KeyType = std::string;

    const std::string name;
    const std::string url;
    const Milliseconds interval;
    const std::optional<SysClock::time_point> date;

    PullDescriptor() : interval(0) {}

    PullDescriptor(const PullDescriptor& other)
        : name(other.name), url(other.url), interval(other.interval) {}

    PullDescriptor(const bsoncxx::document::view& doc)
        : PullDescriptor(std::string(doc["name"].get_utf8()),
                         std::string(doc["url"].get_utf8()),
                         Milliseconds(get_number(doc["interval"])),
                         get_date(doc["date"])) {}

    KeyType key() const
    { return name; }
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
        [&collection,
         name = std::move(descr.name),
         url = std::move(descr.url),
         date = std::move(descr.date)]
        () {
          using namespace bsoncxx::builder::basic;

          std::cout << '[' << SysClock::now() << "] pulling from '"
                    << name << "'\n";

          collection.update_one(
              make_document(kvp("name", name)),
              make_document(
                  kvp("$push",
                      [&url](sub_document doc) {
                        doc.append(
                            kvp("measurements",
                                [&url](sub_document doc) {
                                  WebPull::pull_site(url).serialize(doc);
                                }));
                      })));
        });
    if(descr.date.has_value())
      caller->forward(SysClock::now() - descr.date.value());
    return caller;
  }

  /// --------------------------------------------------------------------------
  /// \ivar: std::map<PullDescriptor::KeyType,
  ///                 std::shared_ptr<CycleCallerBase>> sites
  ///
  /// Map containing a key and a cycle caller for each sites that is being
  /// measured.
  std::map<PullDescriptor::KeyType, std::shared_ptr<CycleCallerBase>> sites;

  /// --------------------------------------------------------------------------
  /// \func: auto fetch_descriptors()
  ///
  /// Makes a quary to the mongo server and returns a list of pull descriptors
  /// based on the data.
  auto fetch_descriptors()
  {
    using namespace bsoncxx::builder::basic;

    std::vector<PullDescriptor> container;

    auto cursor = collection.aggregate(
        mongocxx::pipeline()
        .project(
            make_document(
                kvp("name", true),
                kvp("url", true),
                kvp("interval", true),
                kvp("date",
                    [](sub_document doc) {
                      doc.append(kvp("$max", "$measurements.date"));
                    }))));
    std::for_each(
        cursor.begin(),
        cursor.end(),
        [&container](auto&& doc) { container.emplace_back(doc); });
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
  template <class Rep, class Period>
  bool advance(const Clock::time_point& time,
               const std::chrono::duration<Rep, Period>& delta)
  {
    auto& sites = this->sites;
    auto& collection = this->collection;
    auto descriptors = fetch_descriptors();
    std::for_each(
        sites.cbegin(),
        sites.cend(),
        [&sites, &descriptors](auto& x) {
          using namespace bsoncxx::builder::basic;

          auto descr = std::find_if(descriptors.cbegin(),
                                    descriptors.cend(),
                                    [&x](auto& descr) {
                                      return x.first == descr.key();
                                    });
          if(descr == descriptors.cend())
            sites.erase(sites.find(x.first));
        });
    std::for_each(
        descriptors.begin(),
        descriptors.end(),
        [&sites, &collection, delta](auto& descr) {
          if(sites.find(descr.key()) == sites.end()) {
            // Make and add cycle caller for the site if the key is not in
            // the map.
            sites.emplace(std::make_pair(descr.key(),
                                         make_caller(std::move(descr),
                                                     collection)));
            return;
          }
          auto cycle = sites[descr.key()];
          if(cycle->interval() != descr.interval)
            cycle->set_interval(descr.interval);
          cycle->increment(delta);
        });
    return true;
  }

};

} // namespace reqserver

using namespace reqserver;

int main(int argc, char** argv)
{
  curl_global_init(CURL_GLOBAL_ALL);

  auto time_loop = TimeLoop(Frame());
  time_loop.run(Seconds(1));

  curl_global_cleanup();
  return 0;
}
