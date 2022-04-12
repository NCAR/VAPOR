add_executable (test_udunits test_udunits.cpp)
set_target_properties(test_udunits PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${debug_output_dir}")

target_link_libraries (test_udunits common vdc wasp)
