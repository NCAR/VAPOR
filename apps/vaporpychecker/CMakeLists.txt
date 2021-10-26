add_executable (vaporpychecker vaporpychecker.cpp)

target_link_libraries (vaporpychecker common render)

OpenMPInstall (
	TARGETS vaporpychecker
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
