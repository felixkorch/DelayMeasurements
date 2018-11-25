//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Modified example from 'https://curl.haxx.se/libcurl/c/multithread.html'
//===----------------------------------------------------------------------===//

#include <cstdio>
#include <array>
#include <iostream>
#include <pthread.h>
#include <curl/curl.h>

namespace reqserver {

static std::size_t writeCallback(void *Buffer,
                                 std::size_t Size,
                                 std::size_t N,
                                 void *Data) {
  return Size * N;
}

static void *pullURL(void *URL) {
  CURL *Session = curl_easy_init();
  curl_easy_setopt(Session, CURLOPT_URL, URL);
  curl_easy_setopt(Session, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_perform(Session);
  curl_easy_cleanup(Session);

  return NULL;
}

} // namespace reqserver

using namespace reqserver;

int main(int argc, char **argv) {
  const std::array<const char *, 4> URLS = {
    "https://www.archlinux.org",
    "https://www.redhat.com",
    "https://www.ubunto.com",
    "https://www.gnu.org",
  };

  curl_global_init(CURL_GLOBAL_ALL);

  std::array<pthread_t, URLS.size()> Threads;
  for(int I = 0; I < URLS.size(); ++I) {
    int Error = pthread_create(&Threads[I],
                               NULL,
                               pullURL,
                               (void *)(URLS[I]));
    if(Error)
      std::cout << "Warning: Could not run thread number " << I
                << " on '" << URLS[I]
                << "' {ErrorCode: " << Error << "}\n";
    else
      std::cout << "Thread " << I << ", gets '" << URLS[I] << "'\n";
  }

  for(int I = 0; I < URLS.size(); ++I) {
    int Error = pthread_join(Threads[I], NULL);
    std::cout << "Thread for '" << URLS[I]
              << "' terminated with code " << Error << '\n';
  }

  return 0;
};
