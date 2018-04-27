add_executable (tiff2geotiff tiff2geotiff.cpp geotiff_proj4.cpp getopt.cpp)

target_link_libraries (tiff2geotiff tiff geotiff proj)

install (
	TARGETS tiff2geotiff
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
