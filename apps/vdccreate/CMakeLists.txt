add_executable (vdccreate vdccreate.cpp)

target_link_libraries (vdccreate common vdc)

OpenMPInstall (
	TARGETS vdccreate
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
