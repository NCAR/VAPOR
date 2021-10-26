add_executable (test_datamgr test_datamgr.cpp)
set_target_properties(test_datamgr PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_DEBUG_OUTPUT_DIRECTORY}")

target_link_libraries (test_datamgr common vdc wasp)
