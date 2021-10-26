add_executable (test_pyengine test_pyengine.cpp)
set_target_properties(test_pyengine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_DEBUG_OUTPUT_DIRECTORY}")

target_link_libraries (test_pyengine render common vdc wasp)
