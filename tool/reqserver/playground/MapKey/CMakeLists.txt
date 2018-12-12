include_directories(${LIBMONGOCXX_INCLUDE_DIRS})

add_executable(MapKey main.cpp)

target_link_libraries(MapKey
  mongocxx
  bsoncxx)
