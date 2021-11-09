add_executable (test_params2 test_params2.cpp)
set_target_properties(test_params2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${debug_output_dir}")

target_link_libraries (test_params2 params common vdc wasp)
