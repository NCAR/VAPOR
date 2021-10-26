add_executable (test_datamgr test_datamgr.cpp)
set_target_properties(test_datamgr PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test_binaries")

target_link_libraries (test_datamgr common vdc wasp)
