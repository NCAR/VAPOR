add_executable (vdc2raw vdc2raw.cpp)

target_link_libraries (vdc2raw common vdc)

install (
	TARGETS vdc2raw
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
