//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
// License goes here.
//===----------------------------------------------------------------------===//
//
// Simple MCurl program.
//===----------------------------------------------------------------------===//

#include <cstddef>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <curl/curl.h>

namespace reqserver {

static int SocketFunction(CURL *Ses,
                          curl_socket_t Socket,
                          int Status,
                          void *UserData,
                          void *SocketData) {
  std::cout << "SocketFunction (" << Socket
            << ":" << Status << ") "
            << "Data: " << UserData << "\n";

  auto Handle = static_cast<CURLM *>(UserData);

  if(Status == CURL_POLL_REMOVE) {
    curl_multi_remove_handle(Handle, Ses);
    curl_easy_cleanup(Ses);
    return 0;
  }

  return 0;
}

static int TimerFunction(CURLM *Handle,
                         long Timeout,
                         void *UserData) {
  std::cout << "TimerFunction " << Timeout << "\n";
  if(Timeout == -1)
    return 0;
  if(Timeout < 16) {
    std::this_thread::sleep_for(std::chrono::milliseconds(Timeout));
    int R;
    curl_multi_socket_action(Handle, CURL_SOCKET_TIMEOUT, 0, &R);
    return 0;
  }
  std::thread(
      [Handle, Timeout]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(Timeout));
        int R;
        curl_multi_socket_action(Handle, CURL_SOCKET_TIMEOUT, 0, &R);
      }).detach();
  return 0;
}

static std::size_t WriteCallback(void *Buffer,
                                 std::size_t Size,
                                 std::size_t N,
                                 void *Data) {
  std::cout << "Pulled " << Size * N << " bytes from '"
            << static_cast<const char *>(Data) << "' \n";
  return Size * N;
}

static CURL *makeEasySession(const char *URL) {
  CURL *Ses = curl_easy_init();
  curl_easy_setopt(Ses, CURLOPT_URL, URL);
  curl_easy_setopt(Ses, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(Ses, CURLOPT_WRITEDATA, URL);

  return Ses;
}

} // namespace reqserver

using namespace reqserver;

int main(int argc, char **argv) {
  curl_global_init(CURL_GLOBAL_ALL);

  CURLM *MultiHandle = curl_multi_init();
  if(MultiHandle == NULL) {
    std::cout << "Failed to init multi handle\n";
    return -1;
  }
  curl_multi_setopt(MultiHandle, CURLMOPT_SOCKETFUNCTION, SocketFunction);
  curl_multi_setopt(MultiHandle, CURLMOPT_SOCKETDATA, MultiHandle);
  curl_multi_setopt(MultiHandle, CURLMOPT_TIMERFUNCTION, TimerFunction);

  std::array<const char *, 2> URLS = {
    "https://www.archlinux.org",
    "https://www.redhat.org",
  };
  for(const char *URL : URLS) {
    CURL *Ses = makeEasySession(URL);
    curl_multi_add_handle(MultiHandle, Ses);
  }

  int Running;
  curl_multi_socket_action(MultiHandle, CURL_SOCKET_TIMEOUT, 0, &Running);

  for(int I = 0; I < 30; I++)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  curl_multi_cleanup(MultiHandle);

  return 0;
}
