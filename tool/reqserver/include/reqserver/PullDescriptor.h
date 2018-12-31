//===- PullDescriptor.h -----------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Contains the header data fetched from the server.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_PULLDESCRIPTOR_H
#define REQSERVER_PULLDESCRIPTOR_H

#include "reqserver/Util.h"
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <cstdint>
#include <mongocxx/collection.hpp>
#include <optional>
#include <stdexcept>
#include <string>

namespace reqserver
{

/// --------------------------------------------------------------------------
/// \class: PullDescriptor
///
/// Contains data for the header of a site document on the database.
class PullDescriptor
{
  using StringView = decltype(bsoncxx::types::b_utf8::value);

  PullDescriptor(
      const bsoncxx::oid& oid,
      const StringView& name,
      const StringView& url,
      const std::chrono::milliseconds interval,
      const std::optional<std::chrono::system_clock::time_point>& date)
      : oid(oid), name(name), url(url), interval(interval), date(date)
  {}

public:
  const bsoncxx::oid oid;
  const std::string name;
  const std::string url;
  const std::chrono::milliseconds interval;
  const std::optional<std::chrono::system_clock::time_point> date;

  PullDescriptor(const PullDescriptor& other)
      : oid(other.oid), name(other.name), url(other.url),
        interval(other.interval)
  {}

  // Initializes a PullDescriptor from a bson document. Format checking should
  // be done from functions for each field that throws when invalid format
  // accures.
  PullDescriptor(const bsoncxx::document::view& doc)
      : PullDescriptor(doc["_id"].get_oid().value,
                       doc["name"].get_utf8().value,
                       doc["url"].get_utf8().value,
                       std::chrono::milliseconds(to_int32(doc["interval"])),
                       try_optional([&doc]() { return to_date(doc["date"]); }))
  {}

  /// --------------------------------------------------------------------------
  /// \func: auto each_descriptor()
  ///
  template<class Lambda>
  void static each_descriptor(mongocxx::collection& collection,
                              const Lambda& lambda)
  {
    using namespace bsoncxx::builder::basic;

    // Get cursor of header data from server.
    auto cursor = collection.aggregate(mongocxx::pipeline().project(
        make_document(kvp("name", true), kvp("url", true),
                      kvp("interval", true), kvp("date", [](sub_document doc) {
                        doc.append(kvp("$max", "$measurements.date"));
                      }))));
    std::for_each(cursor.begin(), cursor.end(), [&lambda](auto&& doc) {
      // Exceptions may arise if the format of a document does not comply to
      // PullDescriptor.
      try {
        lambda(PullDescriptor(doc));
      } catch (const std::exception& exception) {
        warning(exception, __FILE__, __LINE__);
      }
    });
  }
};

} // namespace reqserver

#endif // REQSERVER_PULLDESCRIPTOR_H