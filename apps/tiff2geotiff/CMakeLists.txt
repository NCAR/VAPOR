add_executable (tiff2geotiff tiff2geotiff.cpp geotiff_proj4.cpp getopt.cpp)

target_link_libraries (tiff2geotiff tiff geotiff proj)

if (NOT WIN32)
	target_compile_options(tiff2geotiff PRIVATE "-Wno-deprecated-register")
endif ()
