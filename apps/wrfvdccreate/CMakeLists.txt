add_executable (wrfvdccreate wrfvdccreate.cpp)

target_link_libraries (wrfvdccreate common vdc)

install (
	TARGETS wrfvdccreate
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
