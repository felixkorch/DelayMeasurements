add_executable(MongoTest
  main.cpp)

target_link_libraries(MongoTest
  mongocxx
  bsoncxx)

include_directories(${LIBMONGOCXX_INCLUDE_DIRS})
