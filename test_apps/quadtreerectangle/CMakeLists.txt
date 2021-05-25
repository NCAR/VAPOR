
add_executable (test_quadtreerectangle "")

target_sources(test_quadtreerectangle PRIVATE test_quadtreerectangle.cpp ../smokeTests/gridTools.cpp ../smokeTests/gridTools.h)

target_link_libraries (test_quadtreerectangle common vdc wasp)
