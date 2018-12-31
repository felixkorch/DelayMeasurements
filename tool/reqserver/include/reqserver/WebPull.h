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

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <curl/curl.h>
#include <string>

namespace reqserver
{

class WebPull
{
  std::chrono::system_clock::time_point date;
  std::chrono::milliseconds duration;
  std::int64_t size;
  std::int32_t code;

  static std::size_t
  write_callback(void* buf, std::size_t size, std::size_t n, void* user_data)
  {
    *static_cast<std::size_t*>(user_data) += size * n;
    return size * n;
  }

  WebPull(const std::chrono::system_clock::time_point& date,
          const std::chrono::milliseconds& duration,
          const std::size_t& size,
          std::uint32_t code)
      : date(date), duration(duration), size(size), code(code)
  {}

  template<class Lambda>
  static auto measure_time(const Lambda& f)
  {
    auto now = std::chrono::steady_clock::now();
    f();
    return std::chrono::steady_clock::now() - now;
  }

public:
  void serialize(bsoncxx::builder::basic::sub_document& doc) const
  {
    using namespace bsoncxx::builder::basic;

    doc.append(kvp("date", bsoncxx::types::b_date(date)),
               kvp("duration", duration.count()), kvp("size", size),
               kvp("code", code));
  }

  static WebPull pull_site(const std::string_view& url)
  {
    CURL* handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url.data());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    std::size_t size = 0;
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &size);
    auto date = std::chrono::system_clock::now();
    // When the start and stop time is called from this function the duration
    // gets messed up. This is a simple hack that fixes it.
    auto duration = measure_time([handle] { curl_easy_perform(handle); });
    std::int64_t code = 0;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(handle);

    return WebPull(
        date, std::chrono::duration_cast<std::chrono::milliseconds>(duration),
        size, code);
  }
};

} // namespace reqserver

#endif // REQSERVER_WEBPULL_H
