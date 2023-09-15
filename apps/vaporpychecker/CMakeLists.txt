add_executable (vaporpychecker vaporpychecker.cpp)

target_link_libraries (vaporpychecker common render ${Python_LIBRARIES})

OpenMPInstall (
	TARGETS vaporpychecker
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
