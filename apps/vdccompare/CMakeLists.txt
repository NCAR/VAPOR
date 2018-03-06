add_executable (vdccompare vdccompare.cpp)

target_link_libraries (vdccompare common vdc)

install (
	TARGETS vdccompare
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
