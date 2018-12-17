//===- PullDescriptor.h -----------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Contains the header data fetched from the server.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_PULLDESCRIPTOR_H
#define REQSERVER_PULLDESCRIPTOR_H

#include "reqserver/Time.h"
#include "reqserver/Util.h"
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/types.hpp>
#include <cstdint>
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

  PullDescriptor(const OID oid,
                 const StringView& name,
                 const StringView& url,
                 const Milliseconds interval,
                 const std::optional<SysClock::time_point>& date)
      : oid(oid), name(name), url(url), interval(interval), date(date)
  {}

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
                       Milliseconds(to_int32(doc["interval"])),
                       to_date(doc["date"]))
  {}

  auto key() const
  {
    return oid;
  }
};

} // namespace reqserver

#endif // REQSERVER_PULLDESCRIPTOR_H