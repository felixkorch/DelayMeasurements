add_executable(HelloMCURL main.cpp)

target_link_libraries(HelloMCURL
  curl
  pthread)
