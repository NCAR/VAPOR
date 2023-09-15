add_executable (GetRange GetRange.cpp)
target_link_libraries (GetRange vdc)
set_target_properties(GetRange PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${test_output_dir}")
