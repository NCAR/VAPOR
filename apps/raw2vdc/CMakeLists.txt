add_executable (raw2vdc raw2vdc.cpp)

target_link_libraries (raw2vdc common vdc)

install (
	TARGETS raw2vdc
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
