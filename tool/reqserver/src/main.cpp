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

#include <bsoncxx/json.hpp>

namespace reqserver
{

namespace builder
{
using namespace bsoncxx::builder::basic;
} // namespace builder

class Frame
{
  mongocxx::client client;
  mongocxx::collection collection;

  class PullDescriptor
  {
    const std::string name;
    const std::string url;

    PullDescriptor(const std::string& name,
                   const std::string& url,
                   const Milliseconds& interval)
        : name(name), url(url), interval(interval)
    { }

    static std::int32_t get_number(const bsoncxx::document::element& n)
    {
      switch(n.type()) {
        case bsoncxx::type::k_double:
          return std::int32_t(n.get_double());
        case bsoncxx::type::k_int32:
          return std::int32_t(n.get_int32());
      default:
        throw std::runtime_error("Invalid interval type");
      }
    }

  public:
    using KeyType = std::string;

    const Milliseconds interval;

    PullDescriptor() : interval(0) {}

    PullDescriptor(const PullDescriptor& other)
        : name(other.name), url(other.url), interval(other.interval) {}

    PullDescriptor(const bsoncxx::document::view& doc)
        : PullDescriptor(std::string(doc["name"].get_utf8()),
                         std::string(doc["url"].get_utf8()),
                         Milliseconds(get_number(doc["interval"]))) {}

    KeyType key() const
    { return name; }

    auto pull() const
    { return WebPull::pull_site(url); }
  };

  static auto make_caller(const PullDescriptor& descr,
                          mongocxx::collection& collection)
  {
    return shared_cycle_caller(
        descr.interval,
        [&collection, descr]() {
          using namespace bsoncxx::builder::basic;

          std::cout << "pulling from '" << descr.key() << "'\n";

          collection.update_one(
              make_document(kvp("name", descr.key())),
              make_document(
                  kvp("$push",
                      [&descr](sub_document doc) {
                        return doc.append(kvp("measurements",
                                              descr.pull().serialize()));
                      })));
        });
  }

  std::map<PullDescriptor::KeyType, std::shared_ptr<CycleCallerBase>> sites;

  auto fetch_descriptors()
  {
    using namespace bsoncxx::builder::basic;

    std::vector<PullDescriptor> container;
    auto cursor = collection.find(make_document());
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
          // sites does not contain doc
          if(sites.find(descr.key()) == sites.end())
            sites.emplace(std::make_pair(descr.key(),
                                         make_caller(descr, collection)));

          auto cycle = sites[descr.key()];
          if(cycle->interval() != descr.interval)
            cycle->set_interval(descr.interval);
          cycle->incr_timer(delta);
        });
    return true;
  }

};

} // namespace reqserver

using namespace reqserver;

int main(int argc, char** argv)
{
  auto time_loop = TimeLoop(Frame());
  time_loop.run(Seconds(1));
  return 0;
}
