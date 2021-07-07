add_executable (vdcdump vdcdump.cpp)

target_link_libraries (vdcdump common vdc)

OpenMPInstall (
	TARGETS vdcdump
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
