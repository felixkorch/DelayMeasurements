add_executable(CURLMultiThread
  main.cpp)

target_link_libraries(CURLMultiThread
  pthread
  curl)
