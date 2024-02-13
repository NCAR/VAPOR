add_executable( ptr_cache ptr_cache.cpp )
target_link_libraries( ptr_cache PUBLIC gtest_main )

include(GoogleTest)
gtest_discover_tests( ptr_cache )
