add_executable (test_grid_iter test_grid_iter.cpp)
set_target_properties(test_grid_iter PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_DEBUG_OUTPUT_DIRECTORY}")

target_link_libraries (test_grid_iter common vdc wasp)
