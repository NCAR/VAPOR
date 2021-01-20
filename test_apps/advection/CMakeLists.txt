add_executable (benchmark benchmark.cpp)

target_link_libraries (benchmark common vdc wasp flow)

if(UNIX AND NOT APPLE)
  target_compile_definitions(benchmark PRIVATE OMP_POSSIBLE)
endif()
