add_executable (vaporpychecker vaporpychecker.cpp)

target_link_libraries (vaporpychecker common render)

install (
	TARGETS vaporpychecker
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
