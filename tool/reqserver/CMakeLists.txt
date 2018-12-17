cmake_minimum_required(VERSION 3.2)
project(reqserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(REQSERVER_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(REQSERVER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(REQSERVER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected (see CMAKE_BUILD_TYPE), default is Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${REQSERVER_INCLUDE_DIR})

# NOTE:
# Local fix that should not be included in commits.
link_directories("/usr/local/lib")

find_package(CURL REQUIRED)
find_package(libmongocxx REQUIRED)

option(REQSERVER_BUILD_PLAYGROUND "Build playground projects by default." OFF)
add_subdirectory(playground)
add_subdirectory(src)
