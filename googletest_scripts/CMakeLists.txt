# Add an executable that tests a specific part of VAPOR.
add_executable( ptr_cache_test ptr_cache_test.cpp )
target_include_directories( ptr_cache_test PRIVATE ${PROJECT_SOURCE_DIR}/include/vapor/ )
target_link_libraries( ptr_cache_test PUBLIC GTest::gtest_main )


include(GoogleTest)

# Enable this executable to be tested using `ctest .`
gtest_discover_tests( ptr_cache_test )
