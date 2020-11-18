add_executable (
    testGrid 
    testGrid.cpp 
    gridTools.cpp 
    gridTools.h
)

add_executable (
    testDataMgr 
    testDataMgr.cpp 
    dataMgrTools.cpp 
    dataMgrTools.h 
    gridTools.cpp
    gridTools.h
)

add_executable (
    testProj
    testProj.cpp
)

target_link_libraries (testGrid common vdc wasp)
target_link_libraries (testDataMgr common vdc wasp)
target_link_libraries (testProj common vdc )
