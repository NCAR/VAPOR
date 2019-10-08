add_executable (tiff2geotiff tiff2geotiff.cpp geotiff_proj4.cpp getopt.cpp)

if (WIN32)
	target_link_libraries (tiff2geotiff libtiff geotiff proj_6_1 common)
else ()
	target_link_libraries (tiff2geotiff tiff geotiff proj common)
endif()

install (
	TARGETS tiff2geotiff
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
