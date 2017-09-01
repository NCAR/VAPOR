add_executable (cf2vdc cf2vdc.cpp)

target_link_libraries (cf2vdc common vdc)

install (
	TARGETS cf2vdc
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
