add_executable (test_pyengine test_pyengine.cpp)
set_target_properties(test_pyengine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${debug_output_dir}")

target_link_libraries (test_pyengine render common vdc wasp ${Python_LIBRARIES})
