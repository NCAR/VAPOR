add_executable (raw2wasp raw2wasp.cpp)

target_link_libraries (raw2wasp common wasp)

install (
	TARGETS raw2wasp
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
