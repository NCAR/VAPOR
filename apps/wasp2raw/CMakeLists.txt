add_executable (wasp2raw wasp2raw.cpp)

target_link_libraries (wasp2raw common wasp)

install (
	TARGETS wasp2raw
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
