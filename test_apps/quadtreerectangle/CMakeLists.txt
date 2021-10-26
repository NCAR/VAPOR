
add_executable (test_quadtreerectangle "")
set_target_properties(test_quadtreerectangle PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test_binaries")

target_sources(test_quadtreerectangle PRIVATE test_quadtreerectangle.cpp ../smokeTests/gridTools.cpp ../smokeTests/gridTools.h)

target_link_libraries (test_quadtreerectangle common vdc wasp)
