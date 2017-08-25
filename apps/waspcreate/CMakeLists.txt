add_executable (waspcreate waspcreate.cpp)

target_link_libraries (waspcreate common wasp)

install (
	TARGETS waspcreate
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
