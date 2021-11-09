add_executable (test_datamgr test_datamgr.cpp)
set_target_properties(test_datamgr PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${debug_output_dir}")

target_link_libraries (test_datamgr common vdc wasp)
