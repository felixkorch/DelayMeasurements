add_executable(SinglePull
  main.cpp)

target_link_libraries(SinglePull
  curl
  mongocxx
  bsoncxx)

include_directories(${LIBMONGOCXX_INCLUDE_DIRS})
