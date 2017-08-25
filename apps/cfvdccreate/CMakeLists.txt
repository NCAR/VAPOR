add_executable (cfvdccreate cfvdccreate.cpp)

target_link_libraries (cfvdccreate common vdc)

install (
	TARGETS cfvdccreate
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
