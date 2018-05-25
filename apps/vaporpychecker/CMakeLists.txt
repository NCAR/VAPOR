add_executable (vaporpychecker vaporpychecker.cpp)

target_link_libraries (vaporpychecker render common vdc)

install (
	TARGETS vaporpychecker
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
