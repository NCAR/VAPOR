add_executable (vaporversion vaporversion.cpp)

target_link_libraries (vaporversion common)

install (
	TARGETS vaporversion
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
