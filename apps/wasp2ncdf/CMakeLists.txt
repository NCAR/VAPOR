add_executable (wasp2ncdf wasp2ncdf.cpp)

target_link_libraries (wasp2ncdf common wasp)

OpenMPInstall (
	TARGETS wasp2ncdf
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
