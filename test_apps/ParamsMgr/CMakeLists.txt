add_executable (test_ParamsMgr test_ParamsMgr.cpp)
set_target_properties(test_ParamsMgr PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${debug_output_dir}")

target_link_libraries (test_ParamsMgr vdc params common wasp)
