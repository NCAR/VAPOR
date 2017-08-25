add_executable (ncdf2wasp ncdf2wasp.cpp)

target_link_libraries (ncdf2wasp common wasp)

install (
	TARGETS ncdf2wasp
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
