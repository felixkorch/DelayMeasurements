//===- WebPull.h ------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Pulls a url with CURL easy and offers methods in order to serialize the
// result.
//===----------------------------------------------------------------------===//

#ifndef REQSERVER_WEBPULL_H
#define REQSERVER_WEBPULL_H

#include "reqserver/Time.h"
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <curl/curl.h>

#include <iostream>

namespace reqserver
{

class WebPull
{
  SysClock::time_point date;
  Milliseconds duration;
  std::int64_t size;
  std::int32_t code;

  static std::size_t write_callback(void* buf,
                                    std::size_t size,
                                    std::size_t n,
                                    void* user_data)
  {
    std::cout << "  size : " << n << '\n';
    *static_cast<std::size_t*>(user_data) += size * n;
    return size * n;
  }

  WebPull(const SysClock::time_point& date,
          const Milliseconds& duration,
          const std::size_t& size,
          std::uint32_t code)
      : date(date), duration(duration), size(size), code(code) {}

  template <class Lambda>
  static auto measure_time(const Lambda& f)
  {
    auto now = Clock::now();
    f();
    return Clock::now() - now;
  }

public:

  auto serialize(bsoncxx::builder::basic::sub_document& doc) const
  {
    using namespace bsoncxx::builder::basic;

    doc.append(
        kvp("date", bsoncxx::types::b_date(date)),
        kvp("duration", duration.count()),
        kvp("size", size),
        kvp("code", code));
    return doc;
  }

  static WebPull pull_site(const std::string_view& url)
  {
    CURL *handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url.data());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    auto size = std::make_unique<std::size_t>(0);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, size.get());
    auto date = SysClock::now();
    // When I store both start and stop the clocks gets messed up. It seems to
    // work when I only store start timer.
    auto duration = measure_time([handle] { curl_easy_perform(handle); });
    std::int64_t code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(handle);

    return WebPull(date, milliseconds(duration), *size, code);
  }
};

} // namespace reqserver

#endif // REQSERVER_WEBPULL_H
