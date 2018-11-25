add_executable(BSONBuilder
  main.cpp)

target_link_libraries(BSONBuilder
  mongocxx
  bsoncxx)

include_directories(${LIBMONGOCXX_INCLUDE_DIRS})
