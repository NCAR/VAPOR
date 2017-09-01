add_executable (vdcdump vdcdump.cpp)

target_link_libraries (vdcdump common vdc)

install (
	TARGETS vdcdump
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
