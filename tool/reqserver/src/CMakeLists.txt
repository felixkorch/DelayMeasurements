set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${REQSERVER_BINARY_DIR})
include_directories(${LIBMONGOCXX_INCLUDE_DIRS})

add_executable(reqserver
  main.cpp)

target_link_libraries(reqserver
  curl
  mongocxx
  bsoncxx)

install(TARGETS reqserver
  RUNTIME DESTINATION bin)
