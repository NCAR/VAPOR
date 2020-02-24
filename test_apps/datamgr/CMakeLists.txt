add_executable (test_datamgr test_datamgr.cpp)
add_executable (test_grid test_grid.cpp)

target_link_libraries (test_datamgr common vdc wasp)
target_link_libraries (test_grid common vdc wasp)
