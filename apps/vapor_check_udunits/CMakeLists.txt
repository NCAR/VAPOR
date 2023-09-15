add_executable (vapor_check_udunits vapor_check_udunits.cpp)

target_link_libraries (vapor_check_udunits common vdc)

OpenMPInstall (
	TARGETS vapor_check_udunits
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
