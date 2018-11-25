add_executable(CycleCaller
  main.cpp)

target_link_libraries(CycleCaller
  pthread
  curl)
