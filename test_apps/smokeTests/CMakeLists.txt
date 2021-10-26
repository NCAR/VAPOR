add_executable (
    testGrid 
    testGrid.cpp 
    gridTools.cpp 
    gridTools.h
)
set_target_properties(testGrid PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_TEST_OUTPUT_DIRECTORY}")

add_executable (
    testDataMgr 
    testDataMgr.cpp 
    dataMgrTools.cpp 
    dataMgrTools.h 
    gridTools.cpp
    gridTools.h
)
set_target_properties(testDataMgr PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_TEST_OUTPUT_DIRECTORY}")

target_link_libraries (testGrid common vdc wasp)
target_link_libraries (testDataMgr common vdc wasp)
